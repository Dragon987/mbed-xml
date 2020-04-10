#include "common.h"

auto dxml_open(const char* filename) -> dxml_t {
    auto fp = fopen(filename, "r");
    if (!fp) {
        puts("No file\r");
        exit(0);
    }
    auto tmp = dxml_parse_fp(fp);
    fclose(fp);
    return tmp;
}