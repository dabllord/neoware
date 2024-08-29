#include "settings.hpp"
#include <string>

_settings setting;

const std::string get_current_processname() {
  FILE *file = fopen("/proc/self/cmdline", "rb");
  if (!file)
    return NULL;

  char *packagename = new char[0x40];
  fread(packagename, 1u, 0x40, file);
  fclose(file);

  return packagename;
}

void settings::load() {
  std::string dir = "/sdcard/Android/data/" + get_current_processname() +
                    "/files/neoware.ini";
  int fd = open(dir.c_str(), O_RDONLY);
  read(fd, &setting, sizeof(setting));

  close(fd);
}

void settings::save() {
  std::string dir = "/sdcard/Android/data/" + get_current_processname() +
                    "/files/neoware.ini";
  int fd = open(dir.c_str(), O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
  write(fd, &setting, sizeof(setting));

  close(fd);
}