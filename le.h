/* readexe - Prints EXE info a la objdump/dumpbin/efd 
 * le.h - Structure information for 32-bit Linear Executable format
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

#ifndef LE_H
#define LE_H

#include <stdint.h>

struct exe_le_header { 
    char        magic[2]; /* "LE" or "LX" */
    uint8_t     byteOrder;
    uint8_t     wordOrder;
    uint32_t    level;
    uint16_t    cpuType;
    uint16_t    osType;
    uint32_t    version;
    union {
        uint32_t flags;
        struct {
            uint32_t    _reserved1       : 1;
            uint32_t    libInit          : 1;
            uint32_t    _reserved2       : 1;
            uint32_t    noInternalFixups : 1;
            uint32_t    noExternalFixups : 1;
            uint32_t    _reserved3       : 2;
            uint32_t    pmIncompat       : 1;
            uint32_t    pmCompat         : 1;
            uint32_t    usesPM           : 1;
            uint32_t    _reserved4       : 2;
            uint32_t    moduleNotLoaded  : 1;
            uint32_t    _reserved5       : 2;
            uint32_t    libraryModule    : 1;
            uint32_t    _reserved6       : 7;
            uint32_t    protectedLibrary : 1;
            uint32_t    deviceDriver     : 1;
            uint32_t    _reserved7       : 7;
        };
    };
    uint32_t    pages;
    uint32_t    startingObject;
    uint32_t    entryPoint;
    uint32_t    stackObject;
    uint32_t    stackPointer;
    uint32_t    pageSize;
    union {
        uint32_t    lastPage;   /* LE */
        uint32_t    pageShift;  /* LX */
    };
    uint32_t    fixupSize;
    uint32_t    fixupChecksum;
    uint32_t    loaderSize;
    uint32_t    loaderChecksum;
    uint32_t    objectTableOffset;
    uint32_t    objectCount;
    uint32_t    objectMapOffset;
    uint32_t    idataMapOffset;
    uint32_t    resourceOffset;
    uint32_t    resourceCount;
    uint32_t    resourceNameOffset;
    uint32_t    entryTableOffset;
    uint32_t    moduleDirectiveOffset;
    uint32_t    moduleDirectiveCount;
    uint32_t    fixupPageTableOffset;
    uint32_t    fixupRecordTableOffset;
    uint32_t    importModuleNameTableOffset;
    uint32_t    importModuleNameTableCount;
    uint32_t    importProcNameTableOffset;
    uint32_t    pageChecksumTableOffset;
    uint32_t    preloadPageCount;
    uint32_t    nonresidentNameTableOffset;
    uint32_t    nonresidentNameTableSize;
    uint32_t    nonresidentNameTableChecksum;
    uint32_t    autodataObject;
    uint32_t    debugSymbolsfOffset;
    uint32_t    debugSymbolsfSize;
    uint32_t    instancePagePreloadCount;
    uint32_t    instancePageDemandLoadCount;
    uint32_t    heapSize;
    uint32_t    stackSize;
    union {
        uint8_t lxReserved[20];
        struct  { 
            uint8_t leReserved[8];
            uint32_t windowsResourceOffset;
            uint32_t windowsResourceSize;
            uint32_t windowsDeviceID;
            uint32_t windowsDDKVersion;
        };
    };
};

enum exe_le_header_cputypes {
    CPU_286 = 1,
    CPU_386,
    CPU_486,
    CPU_586
};

enum exe_le_ostypes {
    LE_OS_OS2 = 1,
    LE_OS_WINDOWS = 4
};

#endif /* LE_H */
