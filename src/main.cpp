#include "mbed.h"
#include "FATFileSystem.h"

#include "dxml.h"
#include "kss2-bios.h"
#include "kss2-config.h"
#include "kss2-program.h"
#include "kss2-adaptiv.h"
#include "file-system.h"

template<typename T>
auto print_structure(const T* s) -> void {
    auto size = sizeof (T);
    for (size_t byte = 0; byte < size; ++byte)
        printf("%d ", *((const char*)s + byte));
    puts("\r");
}

auto main() -> int {
    drago::FileSystem<FATFileSystem> fs("fs");

    auto splan = new kss2_program::ssplan_t[3];
    auto dani = new kss2_program::stdan_t;
    auto praznik = new kss2_program::stpraznik_t;
    auto datum = new kss2_program::stdatum_t;

    kss2_program::load(splan, dani, praznik, datum, "/fs/KSS2-ProgramData-v01.xml");
}