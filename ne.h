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
    struct {
        unsigned int    dataType   : 2;
        unsigned int    globalInit : 1;
        unsigned int    pmModeOnly : 1;
        unsigned int    ops8086    : 1;
        unsigned int    ops80286   : 1;
        unsigned int    ops80386   : 1;
        unsigned int    ops80x87   : 1;
    };
    struct {
        unsigned int    appType    : 3;
        unsigned int    os2FamExec : 1;
        unsigned int    executable : 1;
        unsigned int    linkErrors : 1;
        unsigned int    _reserved3 : 1;
        unsigned int    libraryBit : 1;
    };
    uint8_t     autoDataSegAddr;
    uint8_t     _reserved4;
    uint16_t    initHeapSize;
    uint16_t    initStackSize;
    uint32_t    entryPoint;
    uint32_t    initStackPtr;
    uint16_t    segmentCount;
    uint16_t    modRefCount;
    uint16_t    nonRezTblSize;
    uint16_t    segTableOffset;
    uint16_t    resTableOffset;


};

enum exe_ne_header_data_typebits {
    DATA_NONE,
    DATA_SINGLEDATA,
    DATA_MULTIPLEDATA,
    DATA_AUTODATA
};

enum exe_ne_header_app_typebits {
    APP_NONE,
    APP_FULLSCREEN,
    APP_COMPATIBLE,
    APP_WINPM
};

enum exe_ne_header_ostypes {
    OS_UNKNOWN,
    OS_OS2,             /* OS/2 1.x */
    OS_WINDOWS,         /* Windows */
    OS_MSDOS,           /* MS-DOS 4.00 for the European market used NE executables */
    OS_WIN386,          /* Windows/386 2.x specific */
    OS_BOSS             /* Borland Operating System Services */
};

#endif
