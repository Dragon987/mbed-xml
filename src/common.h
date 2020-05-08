#pragma once

#include <initializer_list>

extern char* strdup(const char*);

#include "dxml/dxml.h"

auto dxml_open(const char* filename) -> dxml_t;

template<typename ...T>
void create_children(dxml_t parent, const T& ...names)
{
    auto _ = {dxml_add_child(parent, names, 0)...};
}

void create_tag_with_txt(dxml_t parent, const char *name, char *txt);
void create_tag_with_txt(dxml_t parent, const char *name, int val);
void create_tag_with_txt(dxml_t parent, const char *name, unsigned char no_vals, unsigned char* vals);
dxml_t create_tag_with_attr(dxml_t parent, const char *name, const char* attr_name, int val);
dxml_t create_tag_with_attr_and_txt(dxml_t parent, const char *name, const char* attr_name, int attr_val, int txt_val);