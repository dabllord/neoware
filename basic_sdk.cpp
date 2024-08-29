#include "basic_sdk.hpp"

std::string get_object_name(uintptr_t obj) {
  if (!obj)
    return "";

  fname nameprivate = *(fname *)(obj + 0x18);
  std::string name(nameprivate.get_name());

  if (nameprivate.number > 0) {
    name += '_' + std::to_string(nameprivate.number);
  }

  auto pos = name.rfind('/');
  if (pos == std::string::npos) {
    return name;
  }

  return name.substr(pos + 1);
}

std::string get_object_fullname(uintptr_t obj) {
  std::string name;

  uintptr_t class_private = *reinterpret_cast<uintptr_t*>(obj + 0x10);

  if (class_private) {
    std::string temp;
    for (uintptr_t p = *reinterpret_cast<uintptr_t*>(obj + 0x20); p;
         p = *reinterpret_cast<uintptr_t*>(p + 0x20)) {
      temp = get_object_name(p) + "." + temp;
    }

    name = get_object_name(class_private);
    name += " ";
    name += temp;
    name += get_object_name(obj);
  }

  return name;
}

bool is_a(uintptr_t obj, uintptr_t sc) {
  for (auto super = *reinterpret_cast<uintptr_t*>(obj + 0x10); super;
       super = *reinterpret_cast<uintptr_t*>(super + 0x30)) {
    if (super == sc) {
      return true;
    }
  }

  return false;
}
