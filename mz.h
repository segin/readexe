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

/* Define DOS EXE header format */

#ifndef MZ_H
#define MZ_H

#include <stdint.h>

struct exe_mz_header { 
    char        magic[2];                   /* Usually "MZ" but sometimes "ZM" on binaries built with early DOS devtools, maybe those tools were ported from big-endian Unix machines but someone forgot to update the magic #define */
    uint16_t    lastPageSize;
    uint16_t    pageCount;
    uint16_t    relocationEntries;          /* How many entries are present in the relocation table */
    uint16_t    hdrSize;                    /* MS-DOS's EXE header can contain extra sections, like the new header section below. */
    uint16_t    minMemory;                  /* in addition to EXE size, in paragraphs */
    uint16_t    maxMemory;                  /* Maximum pre-allocated memory */
    uint16_t    stackSegment;
    uint16_t    stackPointer;
    uint16_t    checksum;                   /* Treat the bytes that are supposed to be here as 0x0000 when doing the computation. */
    union {
        uint32_t    initCodeSegIP;          /* aka EntryPoint()/_start() */
        struct {
            uint16_t    initInstPtr;
            uint16_t    initCodeSeg;
        };
    };
    uint16_t    relocationOffset;           /* 0x40 or more for NE/PE/LE/LX/etc. */
    uint16_t    overlayNumber;              /* 0 for main program */
};

struct exe_mz_new_header {
    char        unknown[4];
    uint16_t    behaviors;                  /* Multitasking MS-DOS behavior flags. */ 
    char        behaviorsAdditonal[26];     /* flexible format extra data, interpretation is dependent on bits set in behaviors bitfield. */ 
    uint32_t    nextHeader;                 /* offset to actual NE/PE header, aka e_lfanew */ 
};

struct exe_mz_reloc { 
    uint16_t    offset;                     /* */
    uint16_t    segment;
};

#endif
