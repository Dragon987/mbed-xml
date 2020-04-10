#pragma once

#include "mbed.h"
#include "dxml/dxml.h"

auto dxml_open(const char* filename) -> dxml_t;

template<typename ...T>
void create_children(dxml_t parent, const T& ...names)
{
    auto _ = {dxml_add_child(parent, names, 0)...};
}
