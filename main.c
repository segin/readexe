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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <err.h> /* -I. or such for platforms without err.h */

/* Big assumptions on little-endianiness here */

#include "mz.h"
#include "ne.h"

void read_ne_exe(FILE *fd, const struct exe_mz_new_header *mzx, const char fname[]);
void read_ne_segments(FILE *fd, const struct exe_ne_header *ne, const char fname[]);
void read_next_header(FILE *fd, const struct exe_mz_new_header *mzx, const char fname[]);
void read_ne_header(const struct exe_ne_header *ne, const struct exe_mz_new_header *mzx);
int main(int argc, char *argv[]);

void read_ne_exe(FILE *fd, const struct exe_mz_new_header *mzx, const char fname[]) {
    struct exe_ne_header *ne;
    int ret;

    if ((ne = malloc(sizeof(struct exe_ne_header)))) { 
        fseek(fd, mzx->nextHeader, SEEK_SET);
        ret = fread(ne, 1, sizeof(struct exe_ne_header), fd);
        if (ret != sizeof(struct exe_ne_header)) {
            if ((ret = ferror(fd))) warn("Cannot read %s", fname);
            if ((ret = feof(fd))) warnx("Unexpected end of file: %s", fname);
        } else {
            read_ne_header(ne, mzx);
            read_ne_segments(fd, ne, fname);
        }
    } else err(1, "Cannot allocate memory");
    if (ne) free (ne);
    return;
}

void read_ne_segments(FILE *fd, const struct exe_ne_header *ne, const char fname[]) {
    struct exe_ne_segment *neseg;
    int ret, i;

    printf("\n\n");
    if ((neseg = malloc(sizeof(struct exe_ne_segment) * ne->segmentCount))) { 
        fseek(fd, (ne->segmentTableOffset << ne->offsetShiftCount), SEEK_SET);
        ret = fread(neseg, 1, (sizeof(struct exe_ne_segment) * ne->segmentCount), fd);
        if (ret != (sizeof(struct exe_ne_segment) * ne->segmentCount)) {
            if ((ret = ferror(fd))) warn("Cannot read %s", fname);
            if ((ret = feof(fd))) warnx("Unexpected end of file: %s", fname);
        } else {
            for(i = 0; i < ne->segmentCount; i++) {
                printf("Segment %d: %s%s%s%s%s%s%s%s\n", i, 
                    neseg[i].segType ? "CODE " : "DATA ",
                    neseg[i].allocated ? "ALLOCATED " : "",
                    neseg[i].loaded ? "LOADED " : "",
                    neseg[i].relocatable ? "MOVEABLE " : "",
                    neseg[i].shared ? "PURE " : "IMPURE ",
                    neseg[i].preload ? "PRELOAD " : "",
                    neseg[i].relocations ? "RELOCINFO " : "",
                    neseg[i].discardable ? "DISCARD " : ""
                );
                printf("  Offset      (file)   Length   (dec) \n");
                printf("  0x%04"PRIx16"  0x%08"PRIx32"   0x%04"PRIx16"   %5"PRIu32"\n\n", 
                    neseg[i].segmentOffset, 
                    ((uint32_t) neseg[i].segmentOffset << ne->offsetShiftCount), 
                    neseg[i].segmentSize ? neseg[i].segmentSize : 0x10000, 
                    neseg[i].segmentSize ? neseg[i].segmentSize : 0x10000);
            }
        }
    } else err(1, "Cannot allocate memory");
    if (neseg) free (neseg);
    printf("Debugging method / read_ne_segments() reached.\n");
}

void read_ne_header(const struct exe_ne_header *ne, const struct exe_mz_new_header *mzx) {
    char *msg;
    printf("New Executable with magic:\t%c%c\n", ne->magic[0], ne->magic[1]);
    printf("Linker version:\t\t\t%"PRIu8".%"PRIu8"\n", ne->linkerMajor, ne->linkerMinor);
    printf("Entry table offset:\t\t0x%04" PRIx16 " (File offset 0x%08" PRIx32 ")\n", ne->entryTableOffset, ((uint32_t) ne->entryTableOffset + mzx->nextHeader));
    printf("Entry table size:\t\t0x%04"PRIx16" (%"PRIu16" bytes)\n", ne->entryTableSize, ne->entryTableSize);
    printf("Header CRC:\t\t\t0x%08"PRIx32"\n", ne->fileCrc);
    printf(".EXE Flags:\t\t\t0x%02"PRIx8"\n", ne->progFlags);
    switch(ne->dataType) {
        case DATA_NONE:
            msg = "Not indicated";
            break;
        case DATA_SINGLEDATA:
            msg = "SINGLEDATA";
            break;
        case DATA_MULTIPLEDATA:
            msg = "MULTIPLEDATA";
            break;
        case DATA_AUTODATA:
            msg = "AUTODATA";
            break;
    }
    printf(" - Data Segment Model:\t\t%s\n", msg);
    printf(" - Global initialization:\t%s\n", ne->globalInit ? "true" : "false");
    printf(" - Protected Mode only:\t\t%s\n", ne->pmModeOnly ? "true" : "false");
    printf(" - 8086 opcodes used:\t\t%s\n", ne->ops8086 ? "true" : "false");
    printf(" - 80286 opcodes used:\t\t%s\n", ne->ops80286 ? "true" : "false");
    printf(" - 80386 opcodes used:\t\t%s\n", ne->ops80386 ? "true" : "false");
    printf(" - FPU/80x87 opcodes used:\t%s\n", ne->ops80x87 ? "true" : "false");
    printf("Application flags:\t\t0x%02"PRIx8"\n", ne->appFlags);
    switch(ne->appType) {
        case APP_NONE:
            msg = "Not indicated";
            break;
        case APP_FULLSCREEN:
            msg = "OS/2 Fullscreen CUI application";
            break;
        case APP_COMPATIBLE:
            msg = "OS/2 Presentation Manager compatible CUI application";
            break;
        case APP_WINPM:
            msg = "Windows or Presentation Manager GUI application";
            break;
    }
    printf(" - Application type:\t\t%s\n", msg);
    printf(" - OS/2 Family executable:\t%s\n", ne->os2FamExec ? "true" : "false");
    printf(" - Is executable:\t\t%s\n", ne->executable ? "true" : "false");
    printf(" - Generated with link errors:\t%s\n", ne->linkErrors ? "true" : "false");
    printf(" - Is library (DLL or driver):\t%s\n", ne->libraryBit ? "true" : "false");
    printf("AUTODATA segment address:\t0x%04"PRIx16"\n", ne->autoDataSegAddr);
    printf("Initial heap size:\t\t0x%04"PRIx16"\n", ne->initHeapSize);
    printf("Initial stack size:\t\t0x%04"PRIx16"\n", ne->initStackSize);
    printf("Initial CS:IP (entrypoint):\t%04"PRIx16":%04"PRIx16"\n", (ne->entryPoint >> 16), (ne->entryPoint & 0xFFFF));
    printf("Initial SS:SP (stack):\t\t%04"PRIx16":%04"PRIx16"\n", (ne->initStackPtr >> 16), (ne->initStackPtr & 0xFFFF));
    printf("Segment count:\t\t\t0x%04"PRIx16" (%"PRIu16")\n", ne->segmentCount, ne->segmentCount);
    printf("Module reference count:\t\t%04"PRIx16" (%"PRIu16")\n", ne->modRefCount, ne->modRefCount);
    printf("Non-resident name table size:\t0x%04" PRIx16 " (%"PRIu16" bytes)\n ", ne->nonResidentTableSize, ne->nonResidentTableSize);
    printf("Offset of segment table:\t0x%04" PRIx16 " (File offset 0x%08"PRIx32")\n", ne->segmentTableOffset, (ne->segmentTableOffset << ne->offsetShiftCount));
    printf("Offset of resource table:\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", ne->resourceTableOffset, (ne->resourceTableOffset << ne->offsetShiftCount));
    printf("Offset of resident name table:\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", ne->residentNamesTableOffset, (ne->residentNamesTableOffset << ne->offsetShiftCount));
    printf("Offset of module table:\t\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", ne->modulesTableOffset, (ne->modulesTableOffset << ne->offsetShiftCount));
    printf("Offset of imported names table:\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", ne->importedNamesTableOffset, (ne->importedNamesTableOffset << ne->offsetShiftCount));
    printf("Non-resident names table:\t0x%08"PRIx32" (File offset)\n", ne->nonResidentTableOffset);
    printf("Movable entry points:\t\t0x%08"PRIx32" (%"PRIu32")\n", ne->movableEntryPoints, ne->movableEntryPoints);
    printf("Offset shift count:\t\t0x%04"PRIx16" (%"PRIx16")\n", ne->offsetShiftCount, ne->offsetShiftCount);
    printf("Windows version:\t\t%"PRIu8".%"PRIu8" (0x%04"PRIx16")\n", ne->windowsVersionMajor, ne->windowsVersionMinor, ne->windowsVersion);

}

void read_next_header(FILE *fd, const struct exe_mz_new_header *mzx, const char fname[]) {
    char next_magic[2];
    int ret;

    if((ret = fseek(fd, mzx->nextHeader, SEEK_SET))) {
        if ((ret = ferror(fd))) warn("Cannot read %s", fname);
        if ((ret = feof(fd))) warnx("Unexpected end of file: %s", fname);
    } else {
        ret = fread(&next_magic, 1, sizeof(next_magic), fd);
        if (ret != sizeof(next_magic)) {
            if ((ret = ferror(fd))) warn("Cannot read %s", fname);
            if ((ret = feof(fd))) warnx("Unexpected end of file: %s", fname);
        } else {
            if (((next_magic[0] == 'N') && (next_magic[1] == 'E'))) {
                printf("\n\n");
                printf("New Executable header found at offset 0x%08"PRIx32"\n", mzx->nextHeader);
                read_ne_exe(fd, mzx, fname);
            } else if (((next_magic[0] == 'P') && (next_magic[1] == 'E'))) {
                printf("\n\n");
                printf("Portable Executable header found at offset 0x%08"PRIx32"\n", mzx->nextHeader);
                // read_pe_exe(fd, mzx, fname);
            } else if (((next_magic[0] == 'L') && (next_magic[1] == 'E')) ||
                       ((next_magic[0] == 'L') && (next_magic[1] == 'X'))) {
                printf("\n\n");
                printf("Linear Executable header found at offset 0x%08"PRIx32"\n", mzx->nextHeader);
                // read_le_exe(fd, mzx, fname);
            } else {
                printf("\n\n");
                printf("Unknown next header type: %c%c/0x%04"PRIx16"\n", next_magic[0], next_magic[1], *((uint16_t *) &next_magic));
            }
        }
    }
}

int main(int argc, char *argv[]) {
    struct exe_mz_header *mz = NULL;
    struct exe_mz_new_header *mzx = NULL;
    const uint32_t mz_page_size = 512;
    const uint32_t mz_paragraph_size = 16;
    FILE *fd;
    int ret;

#ifdef NEED_ERR
    setprogname(argv[0]);
#endif
    printf("sizeof(struct exe_mz_header) = %u\n", sizeof(struct exe_mz_header));
    if (argc < 2) errx(1, "Not enough arguments.");
    if (!(fd = fopen(argv[1], "rb"))) err(1, "Cannot open %s", argv[1]);
    if (!(mz = malloc(sizeof(struct exe_mz_header)))) err(1, "Cannot allocate memory");
    ret = fread(mz, 1, sizeof(struct exe_mz_header), fd);
    if (ret != sizeof(struct exe_mz_header)) {
        if ((ret = ferror(fd))) warn("Cannot read %s", argv[1]);
        if ((ret = feof(fd))) warnx("Unexpected end of file: %s", argv[1]);
    } else {
        if (    ((mz->magic[0] == 'M') && (mz->magic[1] == 'Z')) 
            ||  ((mz->magic[1] == 'M') && (mz->magic[0] == 'Z')) ) {
                printf("%s:\n", argv[1]);
                printf("DOS executable with magic:\t%c%c\n", mz->magic[0], mz->magic[1]);
                printf("Number of executable pages:\t0x%04"PRIx16" (%"PRIu32"+ bytes)\n", mz->pageCount, ((mz->pageCount - 1) * mz_page_size));
                printf("Size of final page:\t\t%"PRIu16" bytes\n", mz->lastPageSize);
                printf("Total code size:\t\t0x%08"PRIx32" (%"PRIu32" bytes)\n",
                    (((mz->pageCount - 1) * mz_page_size) + mz->lastPageSize),
                    (((mz->pageCount - 1) * mz_page_size) + mz->lastPageSize));
                printf("Total relocation entries:\t0x%04"PRIx16"\n", mz->relocationEntries);
                printf("Header size in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", mz->hdrSize, (mz->hdrSize * mz_paragraph_size));
                printf("Minimum memory in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", mz->minMemory, (mz->minMemory * mz_paragraph_size));
                printf("Maximum memory in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", mz->maxMemory, (mz->maxMemory * mz_paragraph_size));
                printf("Initial stack segment:\t\t0x%04"PRIx16"\n", mz->stackSegment);
                printf("Initial stack pointer:\t\t0x%04"PRIx16"\n", mz->stackPointer);
                printf("Checksum:\t\t\t0x%04"PRIx16"\n", mz->checksum);
                printf("Initial CS:IP far pointer:\t%04"PRIx16":%04"PRIx16"\n", mz->initCodeSeg, mz->initInstPtr);
                printf("Relocation table offset:\t0x%04"PRIx16"\n", mz->relocationOffset);
                printf("Overlay:\t\t\t0x%04"PRIx16"\n", mz->overlayNumber);
                /* check for next header */
                if(mz->relocationOffset >= 0x40) {
                    if (!(mzx = malloc(sizeof(struct exe_mz_new_header)))) err(1, "Cannot allocate memory");
                    ret = fread(mzx, 1, sizeof(struct exe_mz_new_header), fd);
                    if (ret != sizeof(struct exe_mz_new_header)) {
                        if ((ret = ferror(fd))) warn("Cannot read %s", argv[1]);
                        if ((ret = feof(fd))) warnx("Unexpected end of file: %s", argv[1]);
                    } else {
                        printf("Offset to NE/PE header:\t\t0x%08"PRIx32"\n", mzx->nextHeader);
                        read_next_header(fd, mzx, argv[1]);
                    }
                }
        } else {
            fprintf(stdout, "Not a DOS/MZ executable: %s\n", argv[1]);
        }
    }
    if (mzx) free(mzx);
    if (mz) free(mz);
    if ((fclose(fd))) err(1, "Cannot close %s", argv[1]);
    return(0);
}
