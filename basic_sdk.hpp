#pragma once

#include <codecvt>
#include <memory_mgr.hpp>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

std::string get_object_name(uintptr_t obj);
std::string get_object_fullname(uintptr_t obj);
bool is_a(uintptr_t obj, uintptr_t sc);

class fuobject_item {
public:
  uintptr_t object;
  int32_t flags;
  int32_t cluster_index;
  int32_t serial_number;

  enum class object_flags : int32_t {
    none = 0,
    native = 1 << 25,
    async = 1 << 26,
    async_loading = 1 << 27,
    unreachable = 1 << 28,
    pending_kill = 1 << 29,
    root_set = 1 << 30,
    no_strong_reference = 1 << 31
  };

  inline bool is_unreachable() const {
    return !!(flags & static_cast<std::underlying_type_t<object_flags>>(
                          object_flags::unreachable));
  }
  inline bool is_pending_kill() const {
    return !!(flags & static_cast<std::underlying_type_t<object_flags>>(
                          object_flags::pending_kill));
  }
};

class tuobject_array {
public:
  inline int32_t count() const { return num_elements; }

  inline uintptr_t get_by_index(int32_t index) const {
    return objects[index].object;
  }

  inline fuobject_item *get_item_by_id(int32_t index) const {
    if (index < num_elements) {
      return &objects[index];
    }
    return nullptr;
  }

private:
  fuobject_item *objects;
  int32_t max_elements;
  int32_t num_elements;
};

class fuobject_array {
public:
  int32_t object_first_GC_index;
  int32_t object_last_non_GC_index;
  int32_t max_objects_not_considered_by_GC;
  int32_t open_for_fisregard_for_GC;
  tuobject_array obj_objects;

  uintptr_t find_object(const std::string &name) {
    static std::unordered_map<std::string, uintptr_t> cache;
    if (auto search = cache.find(name); search != cache.end()) {
      return search->second;
    }

    for (int i = 0; i < this->obj_objects.count(); ++i) {
      uintptr_t object = this->obj_objects.get_by_index(i);

      if (!object)
        continue;

      if (get_object_fullname(object) == name) {
        cache.insert({get_object_fullname(object), object});
        return object;
      }
    }
  }

  uintptr_t find_offset(const std::string &name) {
    static std::unordered_map<std::string, uintptr_t> cache;

    if (auto search = cache.find(name); search != cache.end()) {
      return search->second;
    }

    uintptr_t obj = find_object(name);

    if (obj) {
      int offset_internal = *reinterpret_cast<int *>(obj + 0x44);
      cache.insert({name, offset_internal});
      return offset_internal;
    }
  }
  static fuobject_array *get_objects() {
    static fuobject_array *objects;
    if (!objects) {
      objects = reinterpret_cast<fuobject_array *>(ue4_elf.base() +
                                                   guobject_array_offset);
    }

    return objects;
  }
};

template <class t> struct tarray {
public:
  inline tarray() {
    data = nullptr;
    count = max = 0;
  };

  inline int get_count() const { return count; };

  inline t &operator[](int i) { return data[i]; };

  inline const t &operator[](int i) const { return data[i]; };

  t *data;
  int32_t count;
  int32_t max;
};

class fname_entry {
public:
  int32_t index;
  char pad[0x8];

  union {
    char ansi_name[1024];
    wchar_t wide_name[1024];
  };

  inline const int32_t get_index() const { return index >> 1; }

  inline bool is_wide() const { return index & 0x1; }

  inline const char *get_ansi_name() const { return ansi_name; }

  inline const wchar_t *get_wide_name() const { return wide_name; }
};

template <typename element_type, int32_t max_total_elements,
          int32_t elements_per_chunk>
class tstatic_indirect_array_thread_safe_read {
public:
  inline size_t count() const { return num_elements; }

  inline bool is_valid_index(int32_t index) const {
    return index < count() && index > 0;
  }

  inline element_type const *const &operator[](int32_t index) const {
    return *get_item_ptr(index);
  }

private:
  inline element_type const *const *get_item_ptr(int32_t index) const {
    int32_t chunk_index = index / elements_per_chunk;
    int32_t within_chunk_index = index % elements_per_chunk;
    element_type **chunk = chunks[chunk_index];
    return chunk + within_chunk_index;
  }

  enum {
    chunk_table_size =
        (max_total_elements + elements_per_chunk - 1) / elements_per_chunk
  };

  element_type **chunks[chunk_table_size];
  int32_t num_elements;
  int32_t num_chunks;
};

using tname_entry_array =
    tstatic_indirect_array_thread_safe_read<fname_entry, 2 * 1024 * 1024,
                                            16384>;

struct fname {
  union {
    struct {
      int32_t comparison_index;
      int32_t number;
    };
  };

  inline fname() : comparison_index(0), number(0){};
  inline fname(int32_t i) : comparison_index(i), number(0){};

  fname(const char *nameToFind) : comparison_index(0), number(0) {
    static std::unordered_set<int> cache;

    for (auto i : cache) {
      if (!std::strcmp(get_names()[i]->get_ansi_name(), nameToFind)) {
        comparison_index = i;

        return;
      }
    }

    for (auto i = 0; i < get_names().count(); ++i) {
      if (get_names()[i] != nullptr) {
        if (!std::strcmp(get_names()[i]->get_ansi_name(), nameToFind)) {
          cache.insert(i);
          comparison_index = i;

          return;
        }
      }
    }
  };

  static tname_entry_array &get_names() {
    static tname_entry_array *names;
    if (!names) {
      names = reinterpret_cast<tname_entry_array *(*)()>(ue4_elf.base() +
                                                         get_names_offset)();
    }

    return *names;
  };

  inline const char *get_name() const {
    return get_names()[comparison_index]->get_ansi_name();
  };

  inline bool operator==(const fname &other) const {
    return comparison_index == other.comparison_index;
  };
};

struct fstring : private tarray<char16_t> {
  inline fstring() {}

  fstring(const char16_t *s) {
    data = const_cast<char16_t *>(s);
    max = count = std::char_traits<char16_t>::length(data) + 1;
  }

  fstring(const char *s) {
    if (s) {
      size_t length = strlen(s);
      count = max = length + 1;
      data = new char16_t[count];

      for (size_t i = 0; i < length; ++i) {
        data[i] = static_cast<char16_t>(s[i]);
      }

      data[length] = u'\0';
    }
  }

  inline bool is_valid() const { return data != nullptr; }

  inline const char *to_string() const {
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.to_bytes(std::u16string(data, data + count)).c_str();
  }
};

extern fuobject_array *guobject_array;