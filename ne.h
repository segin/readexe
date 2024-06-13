/* readexe - Prints EXE info a la objdump/dumpbin/efd 
 * 
 * Copyright © 2019 Kirn Gill II <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES 
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE 
 * FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER 
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING 
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef NE_H
#define NE_H

#include <stdint.h>

struct exe_ne_header {
    char        magic[2];
    uint8_t     linkerMajor;
    uint8_t     linkerMinor;
    uint16_t    entryTableOffset;           /* from start of NE header */
    uint16_t    entryTableSize;
    uint32_t    fileCrc;
    union {
        uint8_t     progFlags;
        struct {
            uint8_t    dataType   : 2;
            uint8_t    globalInit : 1;
            uint8_t    pmModeOnly : 1;
            uint8_t    ops8086    : 1;
            uint8_t    ops80286   : 1;
            uint8_t    ops80386   : 1;
            uint8_t    ops80x87   : 1;
        };
    };
    union {
        uint8_t     appFlags;
        struct {
            uint8_t    appType    : 3;
            uint8_t    os2FamExec : 1;
            uint8_t    executable : 1;
            uint8_t    linkErrors : 1;
            uint8_t    _reserved3 : 1;
            uint8_t    libraryBit : 1;
        };
    };
    uint8_t     autoDataSegAddr;
    uint8_t     _reserved4;
    uint16_t    initHeapSize;
    uint16_t    initStackSize;
    uint32_t    entryPoint;
    uint32_t    initStackPtr;
    uint16_t    segmentCount;
    uint16_t    modRefCount;
    uint16_t    nonResidentTableSize;
    uint16_t    segmentTableOffset;
    uint16_t    resourceTableOffset;
    uint16_t    residentNamesTableOffset;   /* aka "Resident Names Table" */
    uint16_t    modulesTableOffset;         /* imports table, what DLLs are needed */
    uint16_t    importedNamesTableOffset;   /* imported symbols table */ 
    uint32_t    nonResidentTableOffset;
    uint16_t    movableEntryPoints;
    uint16_t    offsetShiftCount;           /* Except for the non-resident names table offset above, which is in bytes, all the other offsets are bit-shifted left, this value is the rhs for the << operator) */
    uint16_t    resourceTableSize;
    uint8_t     targetOS;                   /* defined in enum exe_ne_header_ostypes */
    union {
        uint8_t     exeFlags;
        struct {
            uint8_t    os2LFN       : 1;
            uint8_t    os2PMode     : 1;
            uint8_t    os2Fonts     : 1;
            uint8_t    fastLoad     : 1;
            uint8_t    _reserved5   : 4;
        };
    };
    uint16_t    returnThunksOffset;
    uint16_t    segmentReferenceOffset;
    uint16_t    minimumCodeSwapArea;
    union {
        uint16_t windowsVersion;
        struct {
            uint8_t windowsVersionMinor;
            uint8_t windowsVersionMajor;
        };
    };
};

struct exe_ne_segment {
    uint16_t    segmentOffset;              /* relative to the beginning of file, times 512 bytes. Maybe times ne->offsetShiftCount. */
    uint16_t    segmentSize;                /* if 0, then 65536, unless offset is also 0, then the segment is empty. */
    union {
        uint16_t    segmentFlags;
        struct {
            uint16_t    segType      : 1;
            uint16_t    allocated    : 1;
            uint16_t    loaded       : 1;
            uint16_t    _reserved1   : 1;
            uint16_t    relocatable  : 1;
            uint16_t    shared       : 1;
            uint16_t    preload      : 1;
            uint16_t    protection   : 1;
            uint16_t    relocations  : 1;
            uint16_t    _reserved2   : 3;
            uint16_t    discardable  : 1;
            uint16_t    _reserved3   : 3;
        };
    };
    uint16_t    minimumAllocation;
};

struct exe_ne_resource_infoblock {
    uint16_t    typeID; /* integer if high bit set, string offset otherwise. */
    uint16_t    count;
    uint32_t    _reserved;
};

/* Not a file format structure per se, just to make it easier to handle */
struct exe_ne_module { 
    uint8_t     size;
    uint16_t    *offsets;
};

struct exe_ne_import {
    uint8_t     size;
    int         offset;     /* For resolving module name table offsets */
    char *      name;
};

struct exe_ne_export {
    uint8_t     size;
    char *      name;
    uint8_t     ordinal;
};

struct exe_ne_reloc {
    uint8_t     addressType;
    uint8_t     relocationType;
    union {
        struct { 
            uint16_t    moduleReference;
            union {
                uint16_t    importNameOffset;
                uint16_t    importOrdinal;
            };
        };
        struct {
            uint8_t     segment;
            uint8_t     zero;
            uint16_t    ordinal;
        };
    };          
};

enum exe_ne_reloc_address_type {
    RADDR_LOWBYTE,
    RADDR_SELECTOR = 2,
    RADDR_POINTER32,
    RADDR_OFFSET16 = 5,
    RADDR_POINTER48 = 11,
    RADDR_OFFSET16 = 13
};

enum exe_ne_reloc_type {
    RELTYPE_INTREF,
    RELTYPE_IMPORD,
    RELTYPE_IMPNAME,
    RELTYPE_OSFIXUP
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
    OS_OS2,                                 /* OS/2 1.x */
    OS_WINDOWS,                             /* Windows */
    OS_MSDOS,                               /* MS-DOS 4.00 for the European market used NE executables */
    OS_WIN386,                              /* Windows/386 2.x specific */
    OS_BOSS,                                /* Borland Operating System Services and HX DPMI-16 */
    OS_HX,                                  /* HX DPMI-32 */
    OS_PHARLAP286OS2 = 0x81,                /* Phar Lap 286|DOS-Extender emulating OS/2 */
    OS_PHARLAP286WIN                        /* Phar Lap 286|DOS-Extender emulating Windows (?) */
};

#endif
