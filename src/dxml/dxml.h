/* dxml.h
 *
 *   softverski modul korigovan 11.04.2020
 *
 */

#ifndef _dxml_H
#define _dxml_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
//#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define dxml_BUFSIZE 1024 // size of internal memory buffers
#define dxml_NAMEM   0x80 // name is malloced
#define dxml_TXTM    0x40 // txt is malloced
#define dxml_DUP     0x20 // attribute name and value are strduped

typedef struct dxml *dxml_t;
struct dxml {
    char *name;      // tag name
    char **attr;     // tag attributes { name, value, name, value, ... NULL }
    char *txt;       // tag character content, empty string if none
    size_t off;      // tag offset from start of parent tag character content
    dxml_t next;    // next tag with same name in this section at this depth
    dxml_t sibling; // next tag with different name in same section and depth
    dxml_t ordered; // next tag, same section and depth, in original order
    dxml_t child;   // head of sub tag list, NULL if none
    dxml_t parent;  // parent tag, NULL if current tag is root tag
    short flags;     // additional information
};

// Given a string of xml data and its length, parses it and creates an dxml
// structure. For efficiency, modifies the data by adding null terminators
// and decoding ampersand sequences. If you don't want this, copy the data and
// pass in the copy. Returns NULL on failure.
dxml_t dxml_parse_str(char *s, size_t len);

// A wrapper for dxml_parse_str() that accepts a file descriptor. First
// attempts to mem map the file. Failing that, reads the file into memory.
// Returns NULL on failure.
//dxml_t dxml_parse_fd(int fd);

// a wrapper for dxml_parse_fd() that accepts a file name
dxml_t dxml_parse_file(const char *file);

// Wrapper for dxml_parse_str() that accepts a file stream. Reads the entire
// stream into memory and then parses it. For xml files, use dxml_parse_file()
// or dxml_parse_fd()
dxml_t dxml_parse_fp(FILE *fp);

// returns the first child tag (one level deeper) with the given name or NULL
// if not found
dxml_t dxml_child(dxml_t xml, const char *name);

// returns the next tag of the same name in the same section and depth or NULL
// if not found
#define dxml_next(xml) ((xml) ? xml->next : NULL)

// Returns the Nth tag with the same name in the same section at the same depth
// or NULL if not found. An index of 0 returns the tag given.
dxml_t dxml_idx(dxml_t xml, int idx);

// returns the name of the given tag
#define dxml_name(xml) ((xml) ? xml->name : NULL)

// returns the given tag's character content or empty string if none
#define dxml_txt(xml) ((xml) ? xml->txt : "")

// returns the value of the requested tag attribute, or NULL if not found
const char *dxml_attr(dxml_t xml, const char *attr);

// Traverses the dxml sturcture to retrieve a specific subtag. Takes a
// variable length list of tag names and indexes. The argument list must be
// terminated by either an index of -1 or an empty string tag name. Example:
// title = dxml_get(library, "shelf", 0, "book", 2, "title", -1);
// This retrieves the title of the 3rd book on the 1st shelf of library.
// Returns NULL if not found.
dxml_t dxml_get(dxml_t xml, ...);

// Converts an dxml structure back to xml. Returns a string of xml data that
// must be freed.
char *dxml_toxml(dxml_t xml);

// returns a NULL terminated array of processing instructions for the given
// target
const char **dxml_pi(dxml_t xml, const char *target);

// frees the memory allocated for an dxml structure
void dxml_free(dxml_t xml);

// returns parser error message or empty string if none
const char *dxml_error(dxml_t xml);

// returns a new empty dxml structure with the given root tag name
dxml_t dxml_new(const char *name);

// wrapper for dxml_new() that strdup()s name
#define dxml_new_d(name) dxml_set_flag(dxml_new(strdup(name)), dxml_NAMEM)

// Adds a child tag. off is the offset of the child tag relative to the start
// of the parent tag's character content. Returns the child tag.
dxml_t dxml_add_child(dxml_t xml, const char *name, size_t off);

// wrapper for dxml_add_child() that strdup()s name
#define dxml_add_child_d(xml, name, off) \
    dxml_set_flag(dxml_add_child(xml, strdup(name), off), dxml_NAMEM)

// sets the character content for the given tag and returns the tag
dxml_t dxml_set_txt(dxml_t xml, const char *txt);

// wrapper for dxml_set_txt() that strdup()s txt
#define dxml_set_txt_d(xml, txt) \
    dxml_set_flag(dxml_set_txt(xml, strdup(txt)), dxml_TXTM)

// Sets the given tag attribute or adds a new attribute if not found. A value
// of NULL will remove the specified attribute. Returns the tag given.
dxml_t dxml_set_attr(dxml_t xml, const char *name, const char *value);

// Wrapper for dxml_set_attr() that strdup()s name/value. Value cannot be NULL
#define dxml_set_attr_d(xml, name, value) \
    dxml_set_attr(dxml_set_flag(xml, dxml_DUP), strdup(name), strdup(value))

// sets a flag for the given tag and returns the tag
dxml_t dxml_set_flag(dxml_t xml, short flag);

// removes a tag along with its subtags without freeing its memory
dxml_t dxml_cut(dxml_t xml);

// inserts an existing tag into an dxml structure
dxml_t dxml_insert(dxml_t xml, dxml_t dest, size_t off);

// Moves an existing tag to become a subtag of dest at the given offset from
// the start of dest's character content. Returns the moved tag.
#define dxml_move(xml, dest, off) dxml_insert(dxml_cut(xml), dest, off)

// removes a tag along with all its subtags
#define dxml_remove(xml) dxml_free(dxml_cut(xml))

#ifdef __cplusplus
}
#endif

#endif // _dxml_H

