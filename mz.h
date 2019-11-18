/* Define DOS EXE header format */

#ifndef MZ_H
#define MZ_H

#include <stdint.h>

struct exe_mz_header { 
    char        magic[2];           /* Usually "MZ" but sometimes "ZM" on binaries built with early DOS devtools, maybe those tools were ported from big-endian Unix machines but someone forgot to update the magic #define */
    uint16_t    lastPageSize;
    uint16_t    pageCount;
    uint16_t    relocationEntries;  /* How many entries are present in the relocation table */
    uint16_t    hdrSize;            /* MS-DOS's EXE header can contain extra sections, like the new header section below. */
    uint16_t    minMemory;          /* in addition to EXE size, in paragraphs */
    uint16_t    maxMemory;          /* Maximum pre-allocated memory */
    uint16_t    stackSegment;
    uint16_t    stackPointer;
    uint16_t    checksum;           /* Treat the bytes that are supposed to be here as 0x0000 when doing the computation. */
    uint32_t    initCodeSegIP;      /* aka EntryPoint()/_start() */
    uint16_t    relocationOffset;   /* 0x40 or more for NE/PE/LE/LX/etc. */
    uint16_t    overlayNumber;      /* 0 for main program */
};

struct exe_mz_new_header {
    char        unknown[4];
    uint16_t    behaviors;          /* it's a bitfield but I don't have documentation */ 
    char        behaviorsAdditonal[26]; /* flexible format extra data, interpretation is dependent on bits set in behaviors bitfield. */ 
    uint32_t    nextHeader;         /* offset to actual NE/PE header, aka e_lfanew */ 
};

#endif