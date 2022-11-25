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
#include <string.h>
#include <inttypes.h>
#include <err.h> /* -I. or such for platforms without err.h */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Big assumptions on little-endianiness here */

#include "mz.h"
#include "ne.h"
#include "le.h"

struct THIS {
    FILE *fd;                               /* standard I/O library file descriptor */
    char *fname;                            /* File name of the executable we're inspecting, passed as the sole argument. */
    struct exe_mz_header *mz;               /* DOS (MZ) header */
    struct exe_mz_new_header *mzx;          /* eXtended DOS (MZ) header */
    struct exe_ne_header *ne;               /* New Executable (NE) header */
    struct exe_ne_segment *nesegs;          /* NE segments */
    int ne_importCount;                     /* number of entries in NE imported names table  */
    int ne_moduleCount;                     /* number of module references in modules table */
    struct exe_ne_module *nemods;           /* NE imported modules */
};

void read_ne_exe(struct THIS *this);
void read_ne_segments(struct THIS *this);
void read_ne_names_import(struct THIS *this);
void read_next_header(struct THIS *this);
void read_ne_header(struct THIS *this);
void get_ne_modules_count(struct THIS *this);

struct THIS *init_this(void);
void destroy_this(struct THIS *this);
int main(int argc, char *argv[]);

void read_ne_exe(struct THIS *this) {
    if ((this->ne = (struct exe_ne_header *) malloc(sizeof(struct exe_ne_header)))) { 
        fseek(this->fd, this->mzx->nextHeader, SEEK_SET);
        if (fread(this->ne, 1, sizeof(struct exe_ne_header), this->fd)!= sizeof(struct exe_ne_header)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else {
            read_ne_header(this);
            read_ne_segments(this);
            get_ne_modules_count(this);
            printf("Modules count: %u\n", this->ne_moduleCount);
        }
    } else err(1, "Cannot allocate memory");
    return;
}

void read_ne_segments(struct THIS *this) {
    printf("\n\n");
    if ((this->nesegs = (struct exe_ne_segment *) malloc(sizeof(struct exe_ne_segment) * this->ne->segmentCount))) {
        fseek(this->fd, (this->ne->segmentTableOffset << this->ne->offsetShiftCount), SEEK_SET);

        if (fread(this->nesegs, 1, (sizeof(struct exe_ne_segment) * this->ne->segmentCount), this->fd) != (sizeof(struct exe_ne_segment) * this->ne->segmentCount)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else {
            for(int i = 0; i < this->ne->segmentCount; i++) {
                printf("Segment %d: %s%s%s%s%s%s%s%s\n", i, 
                    this->nesegs[i].segType ? "CODE " : "DATA ",
                    this->nesegs[i].allocated ? "ALLOCATED " : "",
                    this->nesegs[i].loaded ? "LOADED " : "",
                    this->nesegs[i].relocatable ? "MOVEABLE " : "",
                    this->nesegs[i].shared ? "PURE " : "IMPURE ",
                    this->nesegs[i].preload ? "PRELOAD " : "",
                    this->nesegs[i].relocations ? "RELOCINFO " : "",
                    this->nesegs[i].discardable ? "DISCARD " : ""
                );
                printf("  Offset      (file)   Length   (dec) \n");
                printf("  0x%04"PRIx16"  0x%08"PRIx32"   0x%04"PRIx16"   %5"PRIu32"\n\n", 
                    this->nesegs[i].segmentOffset, 
                    ((uint32_t) this->nesegs[i].segmentOffset << this->ne->offsetShiftCount), 
                    this->nesegs[i].segmentSize ? this->nesegs[i].segmentSize : 0x10000, 
                    this->nesegs[i].segmentSize ? this->nesegs[i].segmentSize : 0x10000);
            }
        }
    } else err(1, "Cannot allocate memory");
    printf("Debugging method / read_ne_segments() reached.\n");
}

void get_ne_modules_count(struct THIS *this) {
    uint16_t tmp;

    this->ne_moduleCount = 0;
    fseek(this->fd, (this->ne->modulesTableOffset + this->mzx->nextHeader), SEEK_SET);
    do { 
        fread(&tmp, 2, 1, this->fd); 
        if (tmp) this->ne_moduleCount++;
    } while (tmp);
}

void get_ne_names_import_count(struct THIS *this) {
    
}

void read_ne_names_import(struct THIS *this) {
    printf("Debug: this -> 0x%x\n", this);
}

void read_ne_header(struct THIS *this) {
    char *msg;
    printf("New Executable with magic:\t%c%c\n", this->ne->magic[0], this->ne->magic[1]);
    printf("Linker version:\t\t\t%"PRIu8".%"PRIu8"\n", this->ne->linkerMajor, this->ne->linkerMinor);
    printf("Entry table offset:\t\t0x%04" PRIx16 " (File offset 0x%08" PRIx32 ")\n", this->ne->entryTableOffset, ((uint32_t) this->ne->entryTableOffset + this->mzx->nextHeader));
    printf("Entry table size:\t\t0x%04"PRIx16" (%"PRIu16" bytes)\n", this->ne->entryTableSize, this->ne->entryTableSize);
    printf("Header CRC:\t\t\t0x%08"PRIx32"\n", this->ne->fileCrc);
    printf(".EXE Flags:\t\t\t0x%02"PRIx8"\n", this->ne->progFlags);
    switch(this->ne->dataType) {
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
    printf(" - Global initialization:\t%s\n", this->ne->globalInit ? "true" : "false");
    printf(" - Protected Mode only:\t\t%s\n", this->ne->pmModeOnly ? "true" : "false");
    printf(" - 8086 opcodes used:\t\t%s\n", this->ne->ops8086 ? "true" : "false");
    printf(" - 80286 opcodes used:\t\t%s\n", this->ne->ops80286 ? "true" : "false");
    printf(" - 80386 opcodes used:\t\t%s\n", this->ne->ops80386 ? "true" : "false");
    printf(" - FPU/80x87 opcodes used:\t%s\n", this->ne->ops80x87 ? "true" : "false");
    printf("Application flags:\t\t0x%02"PRIx8"\n", this->ne->appFlags);
    switch(this->ne->appType) {
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
    printf(" - OS/2 Family executable:\t%s\n", this->ne->os2FamExec ? "true" : "false");
    printf(" - Is executable:\t\t%s\n", this->ne->executable ? "true" : "false");
    printf(" - Generated with link errors:\t%s\n", this->ne->linkErrors ? "true" : "false");
    printf(" - Is library (DLL or driver):\t%s\n", this->ne->libraryBit ? "true" : "false");
    printf("AUTODATA segment address:\t0x%04"PRIx16"\n", this->ne->autoDataSegAddr);
    printf("Initial heap size:\t\t0x%04"PRIx16"\n", this->ne->initHeapSize);
    printf("Initial stack size:\t\t0x%04"PRIx16"\n", this->ne->initStackSize);
    printf("Initial CS:IP (entrypoint):\t%04"PRIx16":%04"PRIx16"\n", (this->ne->entryPoint >> 16), (this->ne->entryPoint & 0xFFFF));
    printf("Initial SS:SP (stack):\t\t%04"PRIx16":%04"PRIx16"\n", (this->ne->initStackPtr >> 16), (this->ne->initStackPtr & 0xFFFF));
    printf("Segment count:\t\t\t0x%04"PRIx16" (%"PRIu16")\n", this->ne->segmentCount, this->ne->segmentCount);
    printf("Module reference count:\t\t%04"PRIx16" (%"PRIu16")\n", this->ne->modRefCount, this->ne->modRefCount);
    printf("Non-resident name table size:\t0x%04"PRIx16" (%"PRIu16" bytes)\n", this->ne->nonResidentTableSize, this->ne->nonResidentTableSize);
    printf("Offset of segment table:\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", this->ne->segmentTableOffset, (this->ne->segmentTableOffset + this->mzx->nextHeader));
    printf("Offset of resource table:\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", this->ne->resourceTableOffset, (this->ne->resourceTableOffset + this->mzx->nextHeader));
    printf("Offset of resident name table:\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", this->ne->residentNamesTableOffset, (this->ne->residentNamesTableOffset + this->mzx->nextHeader));
    printf("Offset of module table:\t\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", this->ne->modulesTableOffset, (this->ne->modulesTableOffset + this->mzx->nextHeader));
    printf("Offset of imported names table:\t0x%04"PRIx16" (File offset 0x%08"PRIx32")\n", this->ne->importedNamesTableOffset, (this->ne->importedNamesTableOffset + this->mzx->nextHeader));
    printf("Non-resident names table:\t0x%08"PRIx32" (File offset)\n", this->ne->nonResidentTableOffset);
    printf("Movable entry points:\t\t0x%08"PRIx32" (%"PRIu32")\n", this->ne->movableEntryPoints, this->ne->movableEntryPoints);
    printf("Offset shift count:\t\t0x%04"PRIx16" (%"PRIu16")\n", this->ne->offsetShiftCount, this->ne->offsetShiftCount);
    printf("Resource table size:\t\t0x%04"PRIx16" (%"PRIu16")\n", this->ne->offsetShiftCount, this->ne->offsetShiftCount);
    switch(this->ne->targetOS) {
        case OS_UNKNOWN:
            msg = "Unknown";
            break;
        case OS_OS2:
            msg = "OS/2";
            break;
        case OS_WINDOWS:
            msg = "Windows";
            break;
        case OS_MSDOS:
            msg = "MS-DOS 4.00 (Europe)";
            break;
        case OS_WIN386:
            msg = "Windows/386";
            break;
        case OS_BOSS:
            msg = "Borland Operating System Services";
            break;
        case OS_PHARLAP286OS2:
            msg = "Phar Lap 286|DOS-Extender (OS/2)";
            break;
        case OS_PHARLAP286WIN:
            msg = "Phar Lap 286|DOS-Extender (Windows)";
            break;
    }
    printf("Target operating system:\t%s (0x%02"PRIx8")\n", msg, this->ne->targetOS);
    printf("Windows version:\t\t%"PRIu8".%"PRIu8" (0x%04"PRIx16")\n", this->ne->windowsVersionMajor, this->ne->windowsVersionMinor, this->ne->windowsVersion);
}

void read_next_header(struct THIS *this) {
    char next_magic[2];

    if(fseek(this->fd, this->mzx->nextHeader, SEEK_SET)) {
        if (ferror(this->fd)) warn("Cannot read %s", this->fname);
        if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
    } else {
        if (fread(&next_magic, 1, sizeof(next_magic), this->fd) != sizeof(next_magic)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else {
            if (((next_magic[0] == 'N') && (next_magic[1] == 'E'))) {
                printf("\n\n");
                printf("New Executable header found at offset 0x%08"PRIx32"\n", this->mzx->nextHeader);
                read_ne_exe(this);
            } else if (((next_magic[0] == 'P') && (next_magic[1] == 'E'))) {
                printf("\n\n");
                printf("Portable Executable header found at offset 0x%08"PRIx32"\n", this->mzx->nextHeader);
                // read_pe_exe(this);
            } else if (((next_magic[0] == 'L') && (next_magic[1] == 'E')) ||
                       ((next_magic[0] == 'L') && (next_magic[1] == 'X'))) {
                printf("\n\n");
                printf("Linear Executable header found at offset 0x%08"PRIx32"\n", this->mzx->nextHeader);
                // read_le_exe(this);
            } else {
                printf("\n\n");
                printf("Unknown next header type: %c%c/0x%04"PRIx16"\n", next_magic[0], next_magic[1], *((uint16_t *) &next_magic));
            }
        }
    }
}

struct THIS *init_this(void) {
    struct THIS *this;

    if (!(this = malloc(sizeof(struct THIS)))) err(1, "Cannot allocate memory");
    memset(this, 0, sizeof(struct THIS));
    return this;
}

void destroy_this(struct THIS *this) {
    if (this->nesegs) free(this->nesegs);
    if (this->ne) free(this->ne);
    if (this->mzx) free(this->mzx);
    if (this->mz) free(this->mz);
    if ((fclose(this->fd))) err(1, "Cannot close %s", this->fname);
}

int main(int argc, char *argv[]) {
    const uint32_t mz_page_size = 512;
    const uint32_t mz_paragraph_size = 16;
    struct THIS *this;

#ifdef NEED_ERR
    setprogname(argv[0]);
#endif
    this = init_this();
    if (argc < 2) errx(1, "Not enough arguments.");
    this->fname = argv[1];
    if (!(this->fd = fopen(this->fname, "rb"))) err(1, "Cannot open %s", this->fname);
    if (!(this->mz = malloc(sizeof(struct exe_mz_header)))) err(1, "Cannot allocate memory");
    if (fread(this->mz, 1, sizeof(struct exe_mz_header), this->fd) != sizeof(struct exe_mz_header)) {
        if (ferror(this->fd)) warn("Cannot read %s", this->fname);
        if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
    } else {
        if (    ((this->mz->magic[0] == 'M') && (this->mz->magic[1] == 'Z')) 
            ||  ((this->mz->magic[1] == 'M') && (this->mz->magic[0] == 'Z')) ) {
            printf("%s:\n", argv[1]);
            printf("DOS executable with magic:\t%c%c\n", this->mz->magic[0], this->mz->magic[1]);
            printf("Number of executable pages:\t0x%04"PRIx16" (%"PRIu32"+ bytes)\n", this->mz->pageCount, ((this->mz->pageCount - 1) * mz_page_size));
            printf("Size of final page:\t\t%"PRIu16" bytes\n", this->mz->lastPageSize);
            printf("Total code size:\t\t0x%08"PRIx32" (%"PRIu32" bytes)\n",
                (((this->mz->pageCount - 1) * mz_page_size) + this->mz->lastPageSize),
                (((this->mz->pageCount - 1) * mz_page_size) + this->mz->lastPageSize));
            printf("Total relocation entries:\t0x%04"PRIx16"\n", this->mz->relocationEntries);
            printf("Header size in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", this->mz->hdrSize, (this->mz->hdrSize * mz_paragraph_size));
            printf("Minimum memory in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", this->mz->minMemory, (this->mz->minMemory * mz_paragraph_size));
            printf("Maximum memory in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", this->mz->maxMemory, (this->mz->maxMemory * mz_paragraph_size));
            printf("Initial CS:IP (entrypoint):\t%04"PRIx16":%04"PRIx16"\n", this->mz->initCodeSeg, this->mz->initInstPtr);
            printf("Initial SS:SP (stack):\t\t%04"PRIx16":%04"PRIx16"\n", this->mz->stackSegment, this->mz->stackPointer);
            printf("Initial stack pointer:\t\t0x%04"PRIx16"\n", this->mz->stackPointer);
            printf("Checksum:\t\t\t0x%04"PRIx16"\n", this->mz->checksum);
            printf("Relocation table offset:\t0x%04"PRIx16"\n", this->mz->relocationOffset);
            printf("Overlay:\t\t\t0x%04"PRIx16"\n", this->mz->overlayNumber);
            /* check for next header */
            if(this->mz->relocationOffset >= 0x40) {
                if (!(this->mzx = malloc(sizeof(struct exe_mz_new_header)))) err(1, "Cannot allocate memory");
                if (fread(this->mzx, 1, sizeof(struct exe_mz_new_header), this->fd) != sizeof(struct exe_mz_new_header)) {
                    if (ferror(this->fd)) warn("Cannot read %s", this->fname);
                    if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
                } else {
                    printf("Offset to next header:\t\t0x%08"PRIx32"\n", this->mzx->nextHeader);
                    read_next_header(this);
                }
            }
        } else {
            fprintf(stdout, "Not a DOS/MZ executable: %s\n", this->fname);
        }
    }
    destroy_this(this);
    return(0);
}
