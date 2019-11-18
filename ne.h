#ifndef NE_H
#define NE_H

#include <stdint.h>

struct exe_ne_header {
    char        magic[2];
    uint8_t     linkerMajor;
    uint8_t     linkerMinor;
    uint16_t    entryTableOffset; /* from start of NE header */
    uint16_t    entryTableSize;
    uint32_t    fileCrc;
    struct progflags {
        unsigned int    dataType   : 2;
        unsigned int    globalInit : 1;
        unsigned int    pmModeOnly : 1;
        unsigned int    ops8086    : 1;
        unsigned int    ops80286   : 1;
        unsigned int    ops80386   : 1;
        unsigned int    ops80x87   : 1;
    };
    struct appflags {
        unsigned int    appType    : 2;
        unsigned int    _reserved2 : 1;
        unsigned int    executable : 1;
        unsigned int    linkErrors : 1;
        unsigned int    _reserved3 : 1;
        unsigned int    libraryBit : 1;
    };
    uint8_t     autoDataSegAddr;
    uint8_t     _reserved4;

};

enum exe_ne_header_data_typebits {
    DATA_SINGLEDATA = 1,
    DATA_MULTIPLEDATA,
    DATA_AUTODATA
};

enum exe_ne_header_app_typebits {
    APP_FULLSCREEN = 1,
    APP_COMPATIBLE,
    APP_WINPM
};

#endif
