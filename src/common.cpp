#include "common.h"

#include "string.h"

using uchar = unsigned char;

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

void create_tag_with_txt(dxml_t parent, const char *name, char *txt)
{
    auto child = dxml_add_child(parent, name, 0);
    dxml_set_txt(child, txt);
}

void create_tag_with_txt(dxml_t parent, const char *name, int val)
{
    auto child = dxml_add_child(parent, name, 0);
    char *txt = new char[3];
    memset(txt, 0, 3);
    snprintf(txt, 3, "%d", val);
    dxml_set_txt(child, txt);
}

void create_tag_with_txt(dxml_t parent, const char *name, uchar no_vals, uchar* vals)
{
    auto child = dxml_add_child(parent, name, 0);
    char *txt = new char[16];
    memset(txt, 0, 16);
    char *curr = txt;
    for (uchar i = 0; i < no_vals - 1; ++i)
    {
        snprintf(curr, 4, "%d,", *(vals++));
        curr = strchr(curr, '\0');
    }
    snprintf(curr, 3, "%d", *vals);

    dxml_set_txt(child, txt);
}

dxml_t create_tag_with_attr(dxml_t parent, const char *name, const char* attr_name, int val)
{
    auto child = dxml_add_child(parent, name, 0);
    char *txt = new char[3];
    memset(txt, 0, 3);
    snprintf(txt, 3, "%d", val);
    dxml_set_attr(child, attr_name, txt);
    return child;
}

dxml_t create_tag_with_attr_and_txt(dxml_t parent, const char *name, const char* attr_name, int attr_val, int txt_val)
{
    auto rv = create_tag_with_attr(parent, name, attr_name, attr_val);
    auto buff = static_cast<char*>(malloc(5 * sizeof (char)));
    snprintf(buff, 5, "%d", txt_val);
    dxml_set_txt(rv, buff);
    return rv;
}