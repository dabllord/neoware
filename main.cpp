#include <cstdint>
#include <memory_mgr.hpp>
#include <post_render.hpp>
#include <string>
#include <swappy.hpp>
#include <thread>

KittyMemoryMgr _memory_mgr;
ElfScanner ue4_elf, swappy_elf;
fuobject_array* guobject_array;

void cheat_main() {
  if (!_memory_mgr.initialize(getpid(), EK_MEM_OP_SYSCALL, false) &&
      !_memory_mgr.initialize(getpid(), EK_MEM_OP_IO, false)) {
    LOG_INFO("KittyMemory initialize failed");
    return;
  }

  do {
    ue4_elf = _memory_mgr.getMemElf("libUE4.so");
  } while (!ue4_elf.isValid());

  LOG_INFO("ue4_base -> %lx", ue4_elf.base());

  do {
    swappy_elf = _memory_mgr.getMemElf("libswappy.so");
  } while (!swappy_elf.isValid());

  LOG_INFO("swappy_base -> %lx", swappy_elf.base());

  guobject_array = fuobject_array::get_objects();
  
  

  std::thread(swappy::initialize).detach();
  std::thread(post_render::initialize).detach();

  return;
}

__attribute__((constructor)) void main_constructor() {
  std::thread(cheat_main).detach();
}