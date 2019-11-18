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
    };
};

enum exe_ne_header_typebits {
    SINGLEDATA = 1,
    MULTIPLEDATA = 2,
    AUTODATA = 3
};
