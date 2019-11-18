#ifndef NE_H
#define NE_H

#include <stdint.h>

struct exe_ne_header {
    char        magic[2];
    uint8_t     linkerMajor;
    uint8_t     linkerMinor;
    uint16_t    entryTableOffset; /* from start of NE header */
    uint16_t    entryTableSize;
    uint32_t    _reserved1;
    struct {
        unsigned int    dataType : 2;
        unsigned int    : 0;
        unsigned int    _reserved2 : 3;
        unsigned int    executable : 1;
        unsigned int    linkErrors : 1;
        unsigned int    _reserved3 : 1;
        unsigned int    libraryBit : 1;
    };
};

enum exe_ne_header_typebits {
    SINGLEDATA = 1,
    MULTIPLEDATA = 2,
    AUTODATA = 3
};

#endif