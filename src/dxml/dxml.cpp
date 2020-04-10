#define dxml_NOMMAP

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#ifndef dxml_NOMMAP
#include <sys/mman.h>
#endif // dxml_NOMMAP
#include <sys/stat.h>
#include "dxml.h"

#define dxml_WS   "\t\r\n "  // whitespace
#define dxml_ERRL 128        // maximum error string length

typedef struct dxml_root *dxml_root_t;
struct dxml_root {       // additional data for the root tag
    struct dxml xml;     // is a super-struct built on top of dxml struct
    dxml_t cur;          // current xml tree insertion point
    char *m;              // original xml string
    size_t len;           // length of allocated memory for mmap, -1 for malloc
    char *u;              // UTF-8 conversion of string if original was UTF-16
    char *s;              // start of work area
    char *e;              // end of work area
    char **ent;           // general entities (ampersand sequences)
    char ***attr;         // default attributes
    char ***pi;           // processing instructions
    short standalone;     // non-zero if <?xml standalone="yes"?>
    char err[dxml_ERRL]; // error string
};

char *dxml_NIL[] = { NULL }; // empty, null terminated array of strings

// returns the first child tag with the given name or NULL if not found
dxml_t dxml_child(dxml_t xml, const char *name)
{
    xml = (xml) ? xml->child : NULL;
    while (xml && strcmp(name, xml->name)) xml = xml->sibling;
    return xml;
}

// returns the Nth tag with the same name in the same subsection or NULL if not
// found
dxml_t dxml_idx(dxml_t xml, int idx)
{
    for (; xml && idx; idx--) xml = xml->next;
    return xml;
}

// returns the value of the requested tag attribute or NULL if not found
const char *dxml_attr(dxml_t xml, const char *attr)
{
    int i = 0, j = 1;
    dxml_root_t root = (dxml_root_t)xml;

    if (! xml || ! xml->attr) return NULL;
    while (xml->attr[i] && strcmp(attr, xml->attr[i])) i += 2;
    if (xml->attr[i]) return xml->attr[i + 1]; // found attribute

    while (root->xml.parent) root = (dxml_root_t)root->xml.parent; // root tag
    for (i = 0; root->attr[i] && strcmp(xml->name, root->attr[i][0]); i++);
    if (! root->attr[i]) return NULL; // no matching default attributes
    while (root->attr[i][j] && strcmp(attr, root->attr[i][j])) j += 3;
    return (root->attr[i][j]) ? root->attr[i][j + 1] : NULL; // found default
}

// same as dxml_get but takes an already initialized va_list
dxml_t dxml_vget(dxml_t xml, va_list ap)
{
    char *name = va_arg(ap, char *);
    int idx = -1;

    if (name && *name) {
        idx = va_arg(ap, int);    
        xml = dxml_child(xml, name);
    }
    return (idx < 0) ? xml : dxml_vget(dxml_idx(xml, idx), ap);
}

// Traverses the xml tree to retrieve a specific subtag. Takes a variable
// length list of tag names and indexes. The argument list must be terminated
// by either an index of -1 or an empty string tag name. Example: 
// title = dxml_get(library, "shelf", 0, "book", 2, "title", -1);
// This retrieves the title of the 3rd book on the 1st shelf of library.
// Returns NULL if not found.
dxml_t dxml_get(dxml_t xml, ...)
{
    va_list ap;
    dxml_t r;

    va_start(ap, xml);
    r = dxml_vget(xml, ap);
    va_end(ap);
    return r;
}

// returns a null terminated array of processing instructions for the given
// target
const char **dxml_pi(dxml_t xml, const char *target)
{
    dxml_root_t root = (dxml_root_t)xml;
    int i = 0;

    if (! root) return (const char **)dxml_NIL;
    while (root->xml.parent) root = (dxml_root_t)root->xml.parent; // root tag
    while (root->pi[i] && strcmp(target, root->pi[i][0])) i++; // find target
    return (const char **)((root->pi[i]) ? root->pi[i] + 1 : dxml_NIL);
}

// set an error string and return root
dxml_t dxml_err(dxml_root_t root, char *s, const char *err, ...)
{
    va_list ap;
    int line = 1;
    char *t, fmt[dxml_ERRL];
    
    for (t = root->s; t < s; t++) if (*t == '\n') line++;
    snprintf(fmt, dxml_ERRL, "[error near line %d]: %s", line, err);

    va_start(ap, err);
    vsnprintf(root->err, dxml_ERRL, fmt, ap);
    va_end(ap);

    return &root->xml;
}

// Recursively decodes entity and character references and normalizes new lines
// ent is a null terminated array of alternating entity names and values. set t
// to '&' for general entity decoding, '%' for parameter entity decoding, 'c'
// for cdata sections, ' ' for attribute normalization, or '*' for non-cdata
// attribute normalization. Returns s, or if the decoded string is longer than
// s, returns a malloced string that must be freed.
char *dxml_decode(char *s, char **ent, char t)
{
    char *e, *r = s, *m = s;
    long b, c, d, l;

    for (; *s; s++) { // normalize line endings
        while (*s == '\r') {
            *(s++) = '\n';
            if (*s == '\n') memmove(s, (s + 1), strlen(s));
        }
    }
    
    for (s = r; ; ) {
        while (*s && *s != '&' && (*s != '%' || t != '%') && !isspace(*s)) s++;

        if (! *s) break;
        else if (t != 'c' && ! strncmp(s, "&#", 2)) { // character reference
            if (s[2] == 'x') c = strtol(s + 3, &e, 16); // base 16
            else c = strtol(s + 2, &e, 10); // base 10
            if (! c || *e != ';') { s++; continue; } // not a character ref

            if (c < 0x80) *(s++) = c; // US-ASCII subset
            else { // multi-byte UTF-8 sequence
                for (b = 0, d = c; d; d /= 2) b++; // number of bits in c
                b = (b - 2) / 5; // number of bytes in payload
                *(s++) = (0xFF << (7 - b)) | (c >> (6 * b)); // head
                while (b) *(s++) = 0x80 | ((c >> (6 * --b)) & 0x3F); // payload
            }

            memmove(s, strchr(s, ';') + 1, strlen(strchr(s, ';')));
        }
        else if ((*s == '&' && (t == '&' || t == ' ' || t == '*')) ||
                 (*s == '%' && t == '%')) { // entity reference
            for (b = 0; ent[b] && strncmp(s + 1, ent[b], strlen(ent[b]));
                 b += 2); // find entity in entity list

            if (ent[b++]) { // found a match
                if ((c = strlen(ent[b])) - 1 > (e = strchr(s, ';')) - s) {
                    l = (d = (s - r)) + c + strlen(e); // new length
                    r = (char*)((r == m) ? strcpy((char*)malloc(l), r) : realloc(r, l));
                    e = strchr((s = r + d), ';'); // fix up pointers
                }

                memmove(s + c, e + 1, strlen(e)); // shift rest of string
                strncpy(s, ent[b], c); // copy in replacement text
            }
            else s++; // not a known entity
        }
        else if ((t == ' ' || t == '*') && isspace(*s)) *(s++) = ' ';
        else s++; // no decoding needed
    }

    if (t == '*') { // normalize spaces for non-cdata attributes
        for (s = r; *s; s++) {
            if ((l = strspn(s, " "))) memmove(s, s + l, strlen(s + l) + 1);
            while (*s && *s != ' ') s++;
        }
        if (--s >= r && *s == ' ') *s = '\0'; // trim any trailing space
    }
    return r;
}

// called when parser finds start of new tag
void dxml_open_tag(dxml_root_t root, char *name, char **attr)
{
    dxml_t xml = root->cur;
    
    if (xml->name) xml = dxml_add_child(xml, name, strlen(xml->txt));
    else xml->name = name; // first open tag

    xml->attr = attr;
    root->cur = xml; // update tag insertion point
}

// called when parser finds character content between open and closing tag
void dxml_char_content(dxml_root_t root, char *s, size_t len, char t)
{
    dxml_t xml = root->cur;
    char *m = s;
    size_t l;

    if (! xml || ! xml->name || ! len) return; // sanity check

    s[len] = '\0'; // null terminate text (calling functions anticipate this)
    len = strlen(s = dxml_decode(s, root->ent, t)) + 1;

    if (! *(xml->txt)) xml->txt = s; // initial character content
    else { // allocate our own memory and make a copy
        xml->txt = (char*)((xml->flags & dxml_TXTM) // allocate some space
                   ? realloc(xml->txt, (l = strlen(xml->txt)) + len)
                   : strcpy((char*)malloc((l = strlen(xml->txt)) + len), xml->txt));
        strcpy(xml->txt + l, s); // add new char content
        if (s != m) free(s); // free s if it was malloced by dxml_decode()
    }

    if (xml->txt != m) dxml_set_flag(xml, dxml_TXTM);
}

// called when parser finds closing tag
dxml_t dxml_close_tag(dxml_root_t root, char *name, char *s)
{
    if (! root->cur || ! root->cur->name || strcmp(name, root->cur->name))
        return dxml_err(root, s, "unexpected closing tag </%s>", name);

    root->cur = root->cur->parent;
    return NULL;
}

// checks for circular entity references, returns non-zero if no circular
// references are found, zero otherwise
int dxml_ent_ok(char *name, char *s, char **ent)
{
    int i;

    for (; ; s++) {
        while (*s && *s != '&') s++; // find next entity reference
        if (! *s) return 1;
        if (! strncmp(s + 1, name, strlen(name))) return 0; // circular ref.
        for (i = 0; ent[i] && strncmp(ent[i], s + 1, strlen(ent[i])); i += 2);
        if (ent[i] && ! dxml_ent_ok(name, ent[i + 1], ent)) return 0;
    }
}

// called when the parser finds a processing instruction
void dxml_proc_inst(dxml_root_t root, char *s, size_t len)
{
    int i = 0, j = 1;
    char *target = s;

    s[len] = '\0'; // null terminate instruction
    if (*(s += strcspn(s, dxml_WS))) {
        *s = '\0'; // null terminate target
        s += strspn(s + 1, dxml_WS) + 1; // skip whitespace after target
    }

    if (! strcmp(target, "xml")) { // <?xml ... ?>
        if ((s = strstr(s, "standalone")) && ! strncmp(s + strspn(s + 10,
            dxml_WS "='\"") + 10, "yes", 3)) root->standalone = 1;
        return;
    }

    if (! root->pi[0]) *(root->pi = (char***)malloc(sizeof(char **))) = NULL; //first pi

    while (root->pi[i] && strcmp(target, root->pi[i][0])) i++; // find target
    if (! root->pi[i]) { // new target
        root->pi = (char***)realloc(root->pi, sizeof(char **) * (i + 2));
        root->pi[i] = (char**)malloc(sizeof(char *) * 3);
        root->pi[i][0] = target;
        root->pi[i][1] = (char *)(root->pi[i + 1] = NULL); // terminate pi list
        root->pi[i][2] = strdup(""); // empty document position list
    }

    while (root->pi[i][j]) j++; // find end of instruction list for this target
    root->pi[i] = (char**)realloc(root->pi[i], sizeof(char *) * (j + 3));
    root->pi[i][j + 2] = (char*)realloc(root->pi[i][j + 1], j + 1);
    strcpy(root->pi[i][j + 2] + j - 1, (root->xml.name) ? ">" : "<");
    root->pi[i][j + 1] = NULL; // null terminate pi list for this target
    root->pi[i][j] = s; // set instruction
}

// called when the parser finds an internal doctype subset
short dxml_internal_dtd(dxml_root_t root, char *s, size_t len)
{
    char q, *c, *t, *n = NULL, *v, **ent, **pe;
    int i, j;
    
    pe = (char**)memcpy(malloc(sizeof(dxml_NIL)), dxml_NIL, sizeof(dxml_NIL));

    for (s[len] = '\0'; s; ) {
        while (*s && *s != '<' && *s != '%') s++; // find next declaration

        if (! *s) break;
        else if (! strncmp(s, "<!ENTITY", 8)) { // parse entity definitions
            c = s += strspn(s + 8, dxml_WS) + 8; // skip white space separator
            n = s + strspn(s, dxml_WS "%"); // find name
            *(s = n + strcspn(n, dxml_WS)) = ';'; // append ; to name

            v = s + strspn(s + 1, dxml_WS) + 1; // find value
            if ((q = *(v++)) != '"' && q != '\'') { // skip externals
                s = strchr(s, '>');
                continue;
            }

            for (i = 0, ent = (*c == '%') ? pe : root->ent; ent[i]; i++);
            ent = (char**)realloc(ent, (i + 3) * sizeof(char *)); // space for next ent
            if (*c == '%') pe = ent;
            else root->ent = ent;

            *(++s) = '\0'; // null terminate name
            if ((s = strchr(v, q))) *(s++) = '\0'; // null terminate value
            ent[i + 1] = dxml_decode(v, pe, '%'); // set value
            ent[i + 2] = NULL; // null terminate entity list
            if (! dxml_ent_ok(n, ent[i + 1], ent)) { // circular reference
                if (ent[i + 1] != v) free(ent[i + 1]);
                dxml_err(root, v, "circular entity declaration &%s", n);
                break;
            }
            else ent[i] = n; // set entity name
        }
        else if (! strncmp(s, "<!ATTLIST", 9)) { // parse default attributes
            t = s + strspn(s + 9, dxml_WS) + 9; // skip whitespace separator
            if (! *t) { dxml_err(root, t, "unclosed <!ATTLIST"); break; }
            if (*(s = t + strcspn(t, dxml_WS ">")) == '>') continue;
            else *s = '\0'; // null terminate tag name
            for (i = 0; root->attr[i] && strcmp(n, root->attr[i][0]); i++);

            while (*(n = ++s + strspn(s, dxml_WS)) && *n != '>') {
                if (*(s = n + strcspn(n, dxml_WS))) *s = '\0'; // attr name
                else { dxml_err(root, t, "malformed <!ATTLIST"); break; }

                s += strspn(s + 1, dxml_WS) + 1; // find next token
                c = strdup((strncmp(s, "CDATA", 5)) ? "*" : " "); // is it cdata?
                if (! strncmp(s, "NOTATION", 8))
                    s += strspn(s + 8, dxml_WS) + 8;
                s = (*s == '(') ? strchr(s, ')') : s + strcspn(s, dxml_WS);
                if (! s) { dxml_err(root, t, "malformed <!ATTLIST"); break; }

                s += strspn(s, dxml_WS ")"); // skip white space separator
                if (! strncmp(s, "#FIXED", 6))
                    s += strspn(s + 6, dxml_WS) + 6;
                if (*s == '#') { // no default value
                    s += strcspn(s, dxml_WS ">") - 1;
                    if (*c == ' ') continue; // cdata is default, nothing to do
                    v = NULL;
                }
                else if ((*s == '"' || *s == '\'')  &&  // default value
                         (s = strchr(v = s + 1, *s))) *s = '\0';
                else { dxml_err(root, t, "malformed <!ATTLIST"); break; }

                if (! root->attr[i]) { // new tag name
                    root->attr = (char***)((! i) ? malloc(2 * sizeof(char **))
                                       : realloc(root->attr,
                                                 (i + 2) * sizeof(char **)));
                    root->attr[i] = (char**)malloc(2 * sizeof(char *));
                    root->attr[i][0] = t; // set tag name
                    root->attr[i][1] = (char *)(root->attr[i + 1] = NULL);
                }

                for (j = 1; root->attr[i][j]; j += 3); // find end of list
                root->attr[i] = (char**)realloc(root->attr[i],
                                        (j + 4) * sizeof(char *));

                root->attr[i][j + 3] = NULL; // null terminate list
                root->attr[i][j + 2] = c; // is it cdata?
                root->attr[i][j + 1] = (v) ? dxml_decode(v, root->ent, *c)
                                           : NULL;
                root->attr[i][j] = n; // attribute name 
            }
        }
        else if (! strncmp(s, "<!--", 4)) s = strstr(s + 4, "-->"); // comments
        else if (! strncmp(s, "<?", 2)) { // processing instructions
            if ((s = strstr(c = s + 2, "?>")))
                dxml_proc_inst(root, c, s++ - c);
        }
        else if (*s == '<') s = strchr(s, '>'); // skip other declarations
        else if (*(s++) == '%' && ! root->standalone) break;
    }

    free(pe);
    return ! *root->err;
}

// Converts a UTF-16 string to UTF-8. Returns a new string that must be freed
// or NULL if no conversion was needed.
char *dxml_str2utf8(char **s, size_t *len)
{
    char *u;
    size_t l = 0, sl, max = *len;
    long c, d;
    int b, be = (**s == '\xFE') ? 1 : (**s == '\xFF') ? 0 : -1;

    if (be == -1) return NULL; // not UTF-16

    u = (char*)malloc(max);
    for (sl = 2; sl < *len - 1; sl += 2) {
        c = (be) ? (((*s)[sl] & 0xFF) << 8) | ((*s)[sl + 1] & 0xFF)  //UTF-16BE
                 : (((*s)[sl + 1] & 0xFF) << 8) | ((*s)[sl] & 0xFF); //UTF-16LE
        if (c >= 0xD800 && c <= 0xDFFF && (sl += 2) < *len - 1) { // high-half
            d = (be) ? (((*s)[sl] & 0xFF) << 8) | ((*s)[sl + 1] & 0xFF)
                     : (((*s)[sl + 1] & 0xFF) << 8) | ((*s)[sl] & 0xFF);
            c = (((c & 0x3FF) << 10) | (d & 0x3FF)) + 0x10000;
        }

        while (l + 6 > max) u = (char*)realloc(u, max += dxml_BUFSIZE);
        if (c < 0x80) u[l++] = c; // US-ASCII subset
        else { // multi-byte UTF-8 sequence
            for (b = 0, d = c; d; d /= 2) b++; // bits in c
            b = (b - 2) / 5; // bytes in payload
            u[l++] = (0xFF << (7 - b)) | (c >> (6 * b)); // head
            while (b) u[l++] = 0x80 | ((c >> (6 * --b)) & 0x3F); // payload
        }
    }
    return *s = (char*)realloc(u, *len = l);
}

// frees a tag attribute list
void dxml_free_attr(char **attr) {
    int i = 0;
    char *m;
    
    if (! attr || attr == dxml_NIL) return; // nothing to free
    while (attr[i]) i += 2; // find end of attribute list
    m = attr[i + 1]; // list of which names and values are malloced
    for (i = 0; m[i]; i++) {
        if (m[i] & dxml_NAMEM) free(attr[i * 2]);
        if (m[i] & dxml_TXTM) free(attr[(i * 2) + 1]);
    }
    free(m);
    free(attr);
}

// parse the given xml string and return an dxml structure
dxml_t dxml_parse_str(char *s, size_t len)
{
    dxml_root_t root = (dxml_root_t)dxml_new(NULL);
    char q, e, *d, **attr, **a = NULL; // initialize a to avoid compile warning
    int l, i, j;

    root->m = s;
    if (! len) return dxml_err(root, NULL, "root tag missing");
    root->u = dxml_str2utf8(&s, &len); // convert utf-16 to utf-8
    root->e = (root->s = s) + len; // record start and end of work area
    
    e = s[len - 1]; // save end char
    s[len - 1] = '\0'; // turn end char into null terminator

    while (*s && *s != '<') s++; // find first tag
    if (! *s) return dxml_err(root, s, "root tag missing");

    for (; ; ) {
        attr = (char **)dxml_NIL;
        d = ++s;
        
        if (isalpha(*s) || *s == '_' || *s == ':' || *s < '\0') { // new tag
            if (! root->cur)
                return dxml_err(root, d, "markup outside of root element");

            s += strcspn(s, dxml_WS "/>");
            while (isspace(*s)) *(s++) = '\0'; // null terminate tag name
  
            if (*s && *s != '/' && *s != '>') // find tag in default attr list
                for (i = 0; (a = root->attr[i]) && strcmp(a[0], d); i++);

            for (l = 0; *s && *s != '/' && *s != '>'; l += 2) { // new attrib
                attr = (char**)((l) ? realloc(attr, (l + 4) * sizeof(char *))
                           : malloc(4 * sizeof(char *))); // allocate space
                attr[l + 3] = (char*)((l) ? realloc(attr[l + 1], (l / 2) + 2)
                                  : malloc(2)); // mem for list of maloced vals
                strcpy(attr[l + 3] + (l / 2), " "); // value is not malloced
                attr[l + 2] = NULL; // null terminate list
                attr[l + 1] = ""; // temporary attribute value
                attr[l] = s; // set attribute name

                s += strcspn(s, dxml_WS "=/>");
                if (*s == '=' || isspace(*s)) { 
                    *(s++) = '\0'; // null terminate tag attribute name
                    q = *(s += strspn(s, dxml_WS "="));
                    if (q == '"' || q == '\'') { // attribute value
                        attr[l + 1] = ++s;
                        while (*s && *s != q) s++;
                        if (*s) *(s++) = '\0'; // null terminate attribute val
                        else {
                            dxml_free_attr(attr);
                            return dxml_err(root, d, "missing %c", q);
                        }

                        for (j = 1; a && a[j] && strcmp(a[j], attr[l]); j +=3);
                        attr[l + 1] = dxml_decode(attr[l + 1], root->ent, (a
                                                   && a[j]) ? *a[j + 2] : ' ');
                        if (attr[l + 1] < d || attr[l + 1] > s)
                            attr[l + 3][l / 2] = dxml_TXTM; // value malloced
                    }
                }
                while (isspace(*s)) s++;
            }

            if (*s == '/') { // self closing tag
                *(s++) = '\0';
                if ((*s && *s != '>') || (! *s && e != '>')) {
                    if (l) dxml_free_attr(attr);
                    return dxml_err(root, d, "missing >");
                }
                dxml_open_tag(root, d, attr);
                dxml_close_tag(root, d, s);
            }
            else if ((q = *s) == '>' || (! *s && e == '>')) { // open tag
                *s = '\0'; // temporarily null terminate tag name
                dxml_open_tag(root, d, attr);
                *s = q;
            }
            else {
                if (l) dxml_free_attr(attr);
                return dxml_err(root, d, "missing >"); 
            }
        }
        else if (*s == '/') { // close tag
            s += strcspn(d = s + 1, dxml_WS ">") + 1;
            if (! (q = *s) && e != '>') return dxml_err(root, d, "missing >");
            *s = '\0'; // temporarily null terminate tag name
            if (dxml_close_tag(root, d, s)) return &root->xml;
            if (isspace(*s = q)) s += strspn(s, dxml_WS);
        }
        else if (! strncmp(s, "!--", 3)) { // xml comment
            if (! (s = strstr(s + 3, "--")) || (*(s += 2) != '>' && *s) ||
                (! *s && e != '>')) return dxml_err(root, d, "unclosed <!--");
        }
        else if (! strncmp(s, "![CDATA[", 8)) { // cdata
            if ((s = strstr(s, "]]>")))
                dxml_char_content(root, d + 8, (s += 2) - d - 10, 'c');
            else return dxml_err(root, d, "unclosed <![CDATA[");
        }
        else if (! strncmp(s, "!DOCTYPE", 8)) { // dtd
            for (l = 0; *s && ((! l && *s != '>') || (l && (*s != ']' || 
                 *(s + strspn(s + 1, dxml_WS) + 1) != '>')));
                 l = (*s == '[') ? 1 : l) s += strcspn(s + 1, "[]>") + 1;
            if (! *s && e != '>')
                return dxml_err(root, d, "unclosed <!DOCTYPE");
            d = (l) ? strchr(d, '[') + 1 : d;
            if (l && ! dxml_internal_dtd(root, d, s++ - d)) return &root->xml;
        }
        else if (*s == '?') { // <?...?> processing instructions
            do { s = strchr(s, '?'); } while (s && *(++s) && *s != '>');
            if (! s || (! *s && e != '>')) 
                return dxml_err(root, d, "unclosed <?");
            else dxml_proc_inst(root, d + 1, s - d - 2);
        }
        else return dxml_err(root, d, "unexpected <");
        
        if (! s || ! *s) break;
        *s = '\0';
        d = ++s;
        if (*s && *s != '<') { // tag character content
            while (*s && *s != '<') s++;
            if (*s) dxml_char_content(root, d, s - d, '&');
            else break;
        }
        else if (! *s) break;
    }

    if (! root->cur) return &root->xml;
    else if (! root->cur->name) return dxml_err(root, d, "root tag missing");
    else return dxml_err(root, d, "unclosed tag <%s>", root->cur->name);
}

// Wrapper for dxml_parse_str() that accepts a file stream. Reads the entire
// stream into memory and then parses it. For xml files, use dxml_parse_file()
// or dxml_parse_fd()
dxml_t dxml_parse_fp(FILE *fp)
{
    dxml_root_t root;
    size_t l, len = 0;
    char *s;

    if (! (s = (char*)malloc(dxml_BUFSIZE))) return NULL;
    do {
        len += (l = fread((s + len), 1, dxml_BUFSIZE, fp));
        if (l == dxml_BUFSIZE) s = (char*)realloc(s, len + dxml_BUFSIZE);
    } while (s && l == dxml_BUFSIZE);

    if (! s) return NULL;
    root = (dxml_root_t)dxml_parse_str(s, len);
    root->len = -1; // so we know to free s in dxml_free()
    return &root->xml;
}

// A wrapper for dxml_parse_str() that accepts a file descriptor. First
// attempts to mem map the file. Failing that, reads the file into memory.
// Returns NULL on failure.
dxml_t dxml_parse_fd(int fd)
{
    dxml_root_t root;
    struct stat st;
    size_t l;
    void *m;

    if (fd < 0) return NULL;
    fstat(fd, &st);

#ifndef dxml_NOMMAP
    l = (st.st_size + sysconf(_SC_PAGESIZE) - 1) & ~(sysconf(_SC_PAGESIZE) -1);
    if ((m = mmap(NULL, l, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) !=
        MAP_FAILED) {
        madvise(m, l, MADV_SEQUENTIAL); // optimize for sequential access
        root = (dxml_root_t)dxml_parse_str(m, st.st_size);
        madvise(m, root->len = l, MADV_NORMAL); // put it back to normal
    }
    else { // mmap failed, read file into memory
#endif // dxml_NOMMAP
        l = read(fd, m = malloc(st.st_size), st.st_size);
        root = (dxml_root_t)dxml_parse_str((char*)m, l);
        root->len = -1; // so we know to free s in dxml_free()
#ifndef dxml_NOMMAP
    }
#endif // dxml_NOMMAP
    return &root->xml;
}

// Encodes ampersand sequences appending the results to *dst, reallocating *dst
// if length excedes max. a is non-zero for attribute encoding. Returns *dst
char *dxml_ampencode(const char *s, size_t len, char **dst, size_t *dlen,
                      size_t *max, short a)
{
    const char *e;
    
    for (e = s + len; s != e; s++) {
        while (*dlen + 10 > *max) *dst = (char*)realloc(*dst, *max += dxml_BUFSIZE);

        switch (*s) {
        case '\0': return *dst;
        case '&': *dlen += sprintf(*dst + *dlen, "&amp;"); break;
        case '<': *dlen += sprintf(*dst + *dlen, "&lt;"); break;
        case '>': *dlen += sprintf(*dst + *dlen, "&gt;"); break;
        case '"': *dlen += sprintf(*dst + *dlen, (a) ? "&quot;" : "\""); break;
        case '\n': *dlen += sprintf(*dst + *dlen, (a) ? "&#xA;" : "\n"); break;
        case '\t': *dlen += sprintf(*dst + *dlen, (a) ? "&#x9;" : "\t"); break;
        case '\r': *dlen += sprintf(*dst + *dlen, "&#xD;"); break;
        default: (*dst)[(*dlen)++] = *s;
        }
    }
    return *dst;
}

// Recursively converts each tag to xml appending it to *s. Reallocates *s if
// its length excedes max. start is the location of the previous tag in the
// parent tag's character content. Returns *s.
char *dxml_toxml_r(dxml_t xml, char **s, size_t *len, size_t *max,
                    size_t start, char ***attr)
{
    int i, j;
    char *txt = (xml->parent) ? xml->parent->txt : strdup("");
    size_t off = 0;

    // parent character content up to this tag
    *s = dxml_ampencode(txt + start, xml->off - start, s, len, max, 0);

    while (*len + strlen(xml->name) + 4 > *max) // reallocate s
        *s = (char*)realloc(*s, *max += dxml_BUFSIZE);

    *len += sprintf(*s + *len, "<%s", xml->name); // open tag
    for (i = 0; xml->attr[i]; i += 2) { // tag attributes
        if (dxml_attr(xml, xml->attr[i]) != xml->attr[i + 1]) continue;
        while (*len + strlen(xml->attr[i]) + 7 > *max) // reallocate s
            *s = (char*)realloc(*s, *max += dxml_BUFSIZE);

        *len += sprintf(*s + *len, " %s=\"", xml->attr[i]);
        dxml_ampencode(xml->attr[i + 1], -1, s, len, max, 1);
        *len += sprintf(*s + *len, "\"");
    }

    for (i = 0; attr[i] && strcmp(attr[i][0], xml->name); i++);
    for (j = 1; attr[i] && attr[i][j]; j += 3) { // default attributes
        if (! attr[i][j + 1] || dxml_attr(xml, attr[i][j]) != attr[i][j + 1])
            continue; // skip duplicates and non-values
        while (*len + strlen(attr[i][j]) + 7 > *max) // reallocate s
            *s = (char*)realloc(*s, *max += dxml_BUFSIZE);

        *len += sprintf(*s + *len, " %s=\"", attr[i][j]);
        dxml_ampencode(attr[i][j + 1], -1, s, len, max, 1);
        *len += sprintf(*s + *len, "\"");
    }
    *len += sprintf(*s + *len, ">");

    *s = (xml->child) ? dxml_toxml_r(xml->child, s, len, max, 0, attr) //child
                      : dxml_ampencode(xml->txt, -1, s, len, max, 0);  //data
    
    while (*len + strlen(xml->name) + 4 > *max) // reallocate s
        *s = (char*)realloc(*s, *max += dxml_BUFSIZE);

    *len += sprintf(*s + *len, "</%s>", xml->name); // close tag

    while (txt[off] && off < xml->off) off++; // make sure off is within bounds
    return (xml->ordered) ? dxml_toxml_r(xml->ordered, s, len, max, off, attr)
                          : dxml_ampencode(txt + off, -1, s, len, max, 0);
}

// Converts an dxml structure back to xml. Returns a string of xml data that
// must be freed.
char *dxml_toxml(dxml_t xml)
{
    dxml_t p = (xml) ? xml->parent : NULL, o = (xml) ? xml->ordered : NULL;
    dxml_root_t root = (dxml_root_t)xml;
    size_t len = 0, max = dxml_BUFSIZE;
    char *s = strcpy((char*)malloc(max), ""), *t, *n;
    int i, j, k;

    if (! xml || ! xml->name) return (char*)realloc(s, len + 1);
    while (root->xml.parent) root = (dxml_root_t)root->xml.parent; // root tag

    for (i = 0; ! p && root->pi[i]; i++) { // pre-root processing instructions
        for (k = 2; root->pi[i][k - 1]; k++);
        for (j = 1; (n = root->pi[i][j]); j++) {
            if (root->pi[i][k][j - 1] == '>') continue; // not pre-root
            while (len + strlen(t = root->pi[i][0]) + strlen(n) + 7 > max)
                s = (char*)realloc(s, max += dxml_BUFSIZE);
            len += sprintf(s + len, "<?%s%s%s?>\n", t, *n ? " " : "", n);
        }
    }

    xml->parent = xml->ordered = NULL;
    s = dxml_toxml_r(xml, &s, &len, &max, 0, root->attr);
    xml->parent = p;
    xml->ordered = o;

    for (i = 0; ! p && root->pi[i]; i++) { // post-root processing instructions
        for (k = 2; root->pi[i][k - 1]; k++);
        for (j = 1; (n = root->pi[i][j]); j++) {
            if (root->pi[i][k][j - 1] == '<') continue; // not post-root
            while (len + strlen(t = root->pi[i][0]) + strlen(n) + 7 > max)
                s = (char*)realloc(s, max += dxml_BUFSIZE);
            len += sprintf(s + len, "\n<?%s%s%s?>", t, *n ? " " : "", n);
        }
    }
    return (char*)realloc(s, len + 1);
}

// free the memory allocated for the dxml structure
void dxml_free(dxml_t xml)
{
    dxml_root_t root = (dxml_root_t)xml;
    int i, j;
    char **a, *s;

    if (! xml) return;
    dxml_free(xml->child);
    dxml_free(xml->ordered);

    if (! xml->parent) { // free root tag allocations
        for (i = 10; root->ent[i]; i += 2) // 0 - 9 are default entites (<>&"')
            if ((s = root->ent[i + 1]) < root->s || s > root->e) free(s);
        free(root->ent); // free list of general entities

        for (i = 0; (a = root->attr[i]); i++) {
            for (j = 1; a[j++]; j += 2) // free malloced attribute values
                if (a[j] && (a[j] < root->s || a[j] > root->e)) free(a[j]);
            free(a);
        }
        if (root->attr[0]) free(root->attr); // free default attribute list

        for (i = 0; root->pi[i]; i++) {
            for (j = 1; root->pi[i][j]; j++);
            free(root->pi[i][j + 1]);
            free(root->pi[i]);
        }            
        if (root->pi[0]) free(root->pi); // free processing instructions

        if (root->len == -1) free(root->m); // malloced xml data
#ifndef dxml_NOMMAP
        else if (root->len) munmap(root->m, root->len); // mem mapped xml data
#endif // dxml_NOMMAP
        if (root->u) free(root->u); // utf8 conversion
    }

    dxml_free_attr(xml->attr); // tag attributes
    if ((xml->flags & dxml_TXTM)) free(xml->txt); // character content
    if ((xml->flags & dxml_NAMEM)) free(xml->name); // tag name
    free(xml);
}

// return parser error message or empty string if none
const char *dxml_error(dxml_t xml)
{
    while (xml && xml->parent) xml = xml->parent; // find root tag
    return (xml) ? ((dxml_root_t)xml)->err : "";
}

// returns a new empty dxml structure with the given root tag name
dxml_t dxml_new(const char *name)
{
    static char *ent[] = { "lt;", "&#60;", "gt;", "&#62;", "quot;", "&#34;",
                           "apos;", "&#39;", "amp;", "&#38;", NULL };
    dxml_root_t root = (dxml_root_t)memset(malloc(sizeof(struct dxml_root)), 
                                             '\0', sizeof(struct dxml_root));
    root->xml.name = (char *)name;
    root->cur = &root->xml;
    strcpy(root->err, root->xml.txt = "");
    root->ent = (char**)memcpy(malloc(sizeof(ent)), ent, sizeof(ent));
    root->attr = root->pi = (char ***)(root->xml.attr = dxml_NIL);
    return &root->xml;
}

// inserts an existing tag into an dxml structure
dxml_t dxml_insert(dxml_t xml, dxml_t dest, size_t off)
{
    dxml_t cur, prev, head;

    xml->next = xml->sibling = xml->ordered = NULL;
    xml->off = off;
    xml->parent = dest;

    if ((head = dest->child)) { // already have sub tags
        if (head->off <= off) { // not first subtag
            for (cur = head; cur->ordered && cur->ordered->off <= off;
                 cur = cur->ordered);
            xml->ordered = cur->ordered;
            cur->ordered = xml;
        }
        else { // first subtag
            xml->ordered = head;
            dest->child = xml;
        }

        for (cur = head, prev = NULL; cur && strcmp(cur->name, xml->name);
             prev = cur, cur = cur->sibling); // find tag type
        if (cur && cur->off <= off) { // not first of type
            while (cur->next && cur->next->off <= off) cur = cur->next;
            xml->next = cur->next;
            cur->next = xml;
        }
        else { // first tag of this type
            if (prev && cur) prev->sibling = cur->sibling; // remove old first
            xml->next = cur; // old first tag is now next
            for (cur = head, prev = NULL; cur && cur->off <= off;
                 prev = cur, cur = cur->sibling); // new sibling insert point
            xml->sibling = cur;
            if (prev) prev->sibling = xml;
        }
    }
    else dest->child = xml; // only sub tag

    return xml;
}

// Adds a child tag. off is the offset of the child tag relative to the start
// of the parent tag's character content. Returns the child tag.
dxml_t dxml_add_child(dxml_t xml, const char *name, size_t off)
{
    dxml_t child;

    if (! xml) return NULL;
    child = (dxml_t)memset(malloc(sizeof(struct dxml)), '\0',
                            sizeof(struct dxml));
    child->name = (char *)name;
    child->attr = dxml_NIL;
    child->txt = "";

    return dxml_insert(child, xml, off);
}

// sets the character content for the given tag and returns the tag
dxml_t dxml_set_txt(dxml_t xml, const char *txt)
{
    if (! xml) return NULL;
    if (xml->flags & dxml_TXTM) free(xml->txt); // existing txt was malloced
    xml->flags &= ~dxml_TXTM;
    xml->txt = (char *)txt;
    return xml;
}

// Sets the given tag attribute or adds a new attribute if not found. A value
// of NULL will remove the specified attribute. Returns the tag given.
dxml_t dxml_set_attr(dxml_t xml, const char *name, const char *value)
{
    int l = 0, c;

    if (! xml) return NULL;
    while (xml->attr[l] && strcmp(xml->attr[l], name)) l += 2;
    if (! xml->attr[l]) { // not found, add as new attribute
        if (! value) return xml; // nothing to do
        if (xml->attr == dxml_NIL) { // first attribute
            xml->attr = (char**)malloc(4 * sizeof(char *));
            xml->attr[1] = strdup(""); // empty list of malloced names/vals
        }
        else xml->attr = (char**)realloc(xml->attr, (l + 4) * sizeof(char *));

        xml->attr[l] = (char *)name; // set attribute name
        xml->attr[l + 2] = NULL; // null terminate attribute list
        xml->attr[l + 3] = (char*)realloc(xml->attr[l + 1],
                                   (c = strlen(xml->attr[l + 1])) + 2);
        strcpy(xml->attr[l + 3] + c, " "); // set name/value as not malloced
        if (xml->flags & dxml_DUP) xml->attr[l + 3][c] = dxml_NAMEM;
    }
    else if (xml->flags & dxml_DUP) free((char *)name); // name was strduped

    for (c = l; xml->attr[c]; c += 2); // find end of attribute list
    if (xml->attr[c + 1][l / 2] & dxml_TXTM) free(xml->attr[l + 1]); //old val
    if (xml->flags & dxml_DUP) xml->attr[c + 1][l / 2] |= dxml_TXTM;
    else xml->attr[c + 1][l / 2] &= ~dxml_TXTM;

    if (value) xml->attr[l + 1] = (char *)value; // set attribute value
    else { // remove attribute
        if (xml->attr[c + 1][l / 2] & dxml_NAMEM) free(xml->attr[l]);
        memmove(xml->attr + l, xml->attr + l + 2, (c - l + 2) * sizeof(char*));
        xml->attr = (char**)realloc(xml->attr, (c + 2) * sizeof(char *));
        memmove(xml->attr[c + 1] + (l / 2), xml->attr[c + 1] + (l / 2) + 1,
                (c / 2) - (l / 2)); // fix list of which name/vals are malloced
    }
    xml->flags &= ~dxml_DUP; // clear strdup() flag
    return xml;
}

// sets a flag for the given tag and returns the tag
dxml_t dxml_set_flag(dxml_t xml, short flag)
{
    if (xml) xml->flags |= flag;
    return xml;
}

// removes a tag along with its subtags without freeing its memory
dxml_t dxml_cut(dxml_t xml)
{
    dxml_t cur;

    if (! xml) return NULL; // nothing to do
    if (xml->next) xml->next->sibling = xml->sibling; // patch sibling list

    if (xml->parent) { // not root tag
        cur = xml->parent->child; // find head of subtag list
        if (cur == xml) xml->parent->child = xml->ordered; // first subtag
        else { // not first subtag
            while (cur->ordered != xml) cur = cur->ordered;
            cur->ordered = cur->ordered->ordered; // patch ordered list

            cur = xml->parent->child; // go back to head of subtag list
            if (strcmp(cur->name, xml->name)) { // not in first sibling list
                while (strcmp(cur->sibling->name, xml->name))
                    cur = cur->sibling;
                if (cur->sibling == xml) { // first of a sibling list
                    cur->sibling = (xml->next) ? xml->next
                                               : cur->sibling->sibling;
                }
                else cur = cur->sibling; // not first of a sibling list
            }

            while (cur->next && cur->next != xml) cur = cur->next;
            if (cur->next) cur->next = cur->next->next; // patch next list
        }        
    }
    xml->ordered = xml->sibling = xml->next = NULL;
    return xml;
}

#ifdef dxml_TEST // test harness
int main(int argc, char **argv)
{
    dxml_t xml;
    char *s;
    int i;

    if (argc != 2) return fprintf(stderr, "usage: %s xmlfile\n", argv[0]);

    xml = dxml_parse_file(argv[1]);
    printf("%s\n", (s = dxml_toxml(xml)));
    free(s);
    i = fprintf(stderr, "%s", dxml_error(xml));
    dxml_free(xml);
    return (i) ? 1 : 0;
}
#endif // dxml_TEST
