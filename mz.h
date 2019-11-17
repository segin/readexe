/* Define DOS EXE header format */

#include <stdint.h>

struct exe_mz_header { 
    char        magic[2];
    uint16_t    lastPageSize;
    uint16_t    pageCount;
    uint16_t    relocationEntries;
    uint16_t    hdrSize;
    uint16_t    minMemory; /* in addition to EXE size, in paragraphs */
    uint16_t    maxMemory; /* Maximum pre-allocated memory */
    uint16_t    stackSegment;
    uint16_t    stackPointer;
    uint16_t    checksum;
    uint32_t    initCodeSegIP;
    uint16_t    relocationOffset;   /* 0x40 or more for NE/PE/LE/LX/etc. */
    uint16_t    overlayNumber;  /* 0 for main program */
};

struct exe_mz_new_header {
    char        unknown[4];
    uint16_t    behaviors;
    char        behaviorsAdditonal[26];
    uint32_t    nextHeader; /* offset to actual NE/PE header */ 
};