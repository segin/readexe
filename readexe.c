/* readexe - Prints EXE info a la objdump/dumpbin/efd 
 * 
 * Copyright © 2019-2024 Kirn Gill II <segin2005@gmail.com>
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
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <err.h> /* -I. or such for platforms without err.h */

#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# define VERSION "0.1.4"
#endif

/* Big assumptions on little-endianiness here */

#include "mz.h"
#include "ne.h"
#include "le.h"
#include "w3.h"


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
    struct exe_le_header *le;               /* Linear Executable (LE/LX) header */
    struct exe_w3_header *w3;               /* W3 header */
    int wx_modcount;                        /* W3/W4 LE module count */
};

void read_ne_exe(struct THIS *this);
void read_ne_segments(struct THIS *this);
void read_ne_modules_import(struct THIS *this);
void read_next_header(struct THIS *this);
void read_ne_header(struct THIS *this);
void get_ne_modules_count(struct THIS *this);
void read_le_header(struct THIS *this);
void read_le_exe(struct THIS *this);
void read_w3_exe(struct THIS *this);
void read_mz_reloc(struct THIS *this);

struct THIS *init_this(void);
void destroy_this(struct THIS *this);
void display_help(struct THIS *this);
int main(int argc, char *argv[]);

void read_ne_exe(struct THIS *this) {
    if ((this->ne = (struct exe_ne_header *) malloc(sizeof(struct exe_ne_header)))) { 
        fseek(this->fd, this->mzx->nextHeader, SEEK_SET);
        if (fread(this->ne, 1, sizeof(struct exe_ne_header), this->fd)!= sizeof(struct exe_ne_header)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else {
            read_ne_header(this);
            read_ne_modules_import(this);
            read_ne_segments(this);
        }
    } else err(1, "Cannot allocate memory");
    return;
}

void read_ne_segments(struct THIS *this) {
    uint32_t seg, segsz, minalloc;

    printf("\n\n");
    if ((this->nesegs = (struct exe_ne_segment *) malloc(sizeof(struct exe_ne_segment) * this->ne->segmentCount))) {
        fseek(this->fd, this->mzx->nextHeader + this->ne->segmentTableOffset, SEEK_SET);

        if (fread(this->nesegs, 1, (sizeof(struct exe_ne_segment) * this->ne->segmentCount), this->fd) != (sizeof(struct exe_ne_segment) * this->ne->segmentCount)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else {
            for(int i = 0; i < this->ne->segmentCount; i++) {
                printf("Segment %d: %s%s%s%s%s%s%s%s\n", i, 
                    this->nesegs[i].segType ? "DATA " : "CODE ",
                    this->nesegs[i].allocated ? "ALLOCATED " : "",
                    this->nesegs[i].loaded ? "LOADED " : "",
                    this->nesegs[i].relocatable ? "MOVEABLE " : "",
                    this->nesegs[i].shared ? "PURE " : "IMPURE ",
                    this->nesegs[i].preload ? "PRELOAD " : "",
                    this->nesegs[i].relocations ? "RELOCINFO " : "",
                    this->nesegs[i].discardable ? "DISCARD " : ""
                );
                printf("  Offset      (file)   Length   (dec)     Mem \n");
                /* While the underlying structures contain 16-bit values, 32-bit values are used in RAM to account for the case of a value of zero, equal to 0x10000. */
                seg = this->nesegs[i].segmentOffset; 
                segsz = (uint32_t) this->nesegs[i].segmentSize ? this->nesegs[i].segmentSize : 0x10000;
                minalloc = (uint32_t) this->nesegs[i].minimumAllocation ? this->nesegs[i].minimumAllocation : 0x10000;
                printf("  0x%04"PRIx32"  0x%08"PRIx32"   0x%04"PRIx32"   %5"PRIu32"  0x%04"PRIx32"\n\n", 
                    seg, 
                    seg << this->ne->offsetShiftCount, 
                    segsz, 
                    segsz,
                    minalloc);
            }
        }
    } else err(1, "Cannot allocate memory");
}

void read_ne_relocs(struct THIS *this) {
    uint16_t segmentRelocationEntries, i, j;
    off_t oldoffset = ftell(this->fd);
    struct exe_ne_reloc *relocentry = malloc(sizeof(struct exe_ne_reloc));;
    
    if(!relocentry) err(1, "Cannot allocate memory");
    for(i=0;i<this->ne->segmentCount;i++) {
        fseek(this->fd, this->nesegs[i].segmentOffset << this->ne->offsetShiftCount, SEEK_SET);
        printf("Relocation table for segment %d:\n", i);
        fread(&segmentRelocationEntries, 2, 1, this->fd);
        if (segmentRelocationEntries) {
            for(j=0;j<segmentRelocationEntries;j++){
                fread(relocentry, 1, sizeof(struct exe_ne_reloc), this->fd);
                printf(" [%3d] ", j);
                switch(relocentry->relocationType){
                    case RELTYPE_INTREF:
                        printf(" [%3d] ");
                        break;
                };    
            }
        } else 
            printf("No relocations for segment.\n");
        printf("\n");
    }
    fseek(this->fd, oldoffset, SEEK_SET);
    free(relocentry);
}

char *get_ne_import_module_name(struct THIS *this, int module) {
    int i = module * 2;
    off_t oldoffset = ftell(this->fd), soff;
    uint16_t loff;
    uint8_t size;
    char *name;

    fseek(this->fd, (this->ne->modulesTableOffset + this->mzx->nextHeader + i), SEEK_SET);
    if (fread(&loff, 1, sizeof(uint16_t), this->fd)!= sizeof(uint16_t)) {
        if (ferror(this->fd)) warn("Cannot read %s", this->fname);
        if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
    } else {
        soff = (this->ne->importedNamesTableOffset + this->mzx->nextHeader) + loff;
        fseek(this->fd, soff, SEEK_SET);
        size = fgetc(this->fd);
        if ((int8_t) size != -1) {
            if (name = malloc(size + 1)) { 
                memset(name, 0, size + 1);
                if (fread(name, 1, size, this->fd)!= size) {
                    if (ferror(this->fd)) warn("Cannot read %s", this->fname);
                    if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
                } else {
                    fseek(this->fd, oldoffset, SEEK_SET);
                    return name;
                }
            } else err(1, "Cannot allocate memory");
        } else warnx("Unexpected end of file: %s", this->fname);
    }
    /* we should never return NULL or even get here; either we have a valid malloc()'d string pointer or execution cannot continue.*/
    fprintf(stderr, "Reached end of get_ne_import_module_name()!");
    abort();
    return NULL; 
}

void read_ne_modules_import(struct THIS *this) {
    int i;
    char *name;

    printf(
        "\n\n"
        "Imported modules:\n"
        "-----------------\n"
    );
    for(i=0;i<this->ne->modRefCount;i++) {
        if(name = get_ne_import_module_name(this, i)) {
            printf("  [%2d]: %s\n", i+1, name);
            free(name);
        } else errx(1, "Cannot read file: ", this->fname);
    }
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
            msg = "MT MS-DOS 4.0";
            break;
        case OS_WIN386:
            msg = "Windows/386";
            break;
        case OS_BOSS:
            msg = "Borland Operating System Services or HX Extender DPMI-16";
            break;
        case OS_HX:
            msg = "HX Extender DPMI-32";
            break;
        case OS_PHARLAP286OS2:
            msg = "Phar Lap 286|DOS-Extender (OS/2)";
            break;
        case OS_PHARLAP286WIN:
            msg = "Phar Lap 286|DOS-Extender (Windows)";
            break;
    }
    printf("Target operating system:\t%s (0x%02"PRIx8")\n", msg, this->ne->targetOS);
    printf("Executable flags:\t\t%s%s%s%s\n", 
        this->ne->os2LFN ? "LONGFILENAME " : "",
        this->ne->os2PMode ? "PROTECTEDMODE " : "",
        this->ne->os2Fonts ? "PROPORTIONALFONTS " : "",
        this->ne->fastLoad ? "GANGLOADAREA " : ""); 
    if(this->ne->fastLoad) { 
        printf("GangLoad/FastLoad area offset:\t0x%04"PRIx16"\n", this->ne->returnThunksOffset);
        printf("GangLoad/FastLoad area size:\t0x%04"PRIx16"\n", this->ne->segmentReferenceOffset);        
    }
    printf("Windows version:\t\t%"PRIu8".%"PRIu8" (0x%04"PRIx16")\n", this->ne->windowsVersionMajor, this->ne->windowsVersionMinor, this->ne->windowsVersion);
}

void read_le_header(struct THIS *this) {
    char *msg = "";
}

void read_le_exe(struct THIS *this) {
    if ((this->le = (struct exe_le_header *) malloc(sizeof(struct exe_le_header)))) { 
        fseek(this->fd, this->mzx->nextHeader, SEEK_SET);
        if (fread(this->le, 1, sizeof(struct exe_le_header), this->fd)!= sizeof(struct exe_le_header)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else
            printf("Linear Executable format is a WIP. No output code yet.\n");
    } else err(1, "Cannot allocate memory");
    return;
}

void read_w3_exe(struct THIS *this) {
    struct exe_w3_modentry mod;
    
    if ((this->w3 = (struct exe_w3_header *) malloc(sizeof(struct exe_w3_header)))) { 
        fseek(this->fd, this->mzx->nextHeader, SEEK_SET);
        if (fread(this->w3, 1, sizeof(struct exe_w3_header), this->fd)!= sizeof(struct exe_w3_header)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else {
            printf("VMM version: %"PRIu8".%"PRIu8" (0x%04"PRIx16")\n", this->w3->vmm_major, this->w3->vmm_minor, this->w3->vmm_version);
            printf(
                "VxD Module Table:\n"
                "   ID   Name          Offset      Size       (dec)\n"
                "------------------------------------------------------\n"
            );
            this->wx_modcount = this->w3->modcount;
            for(int i=0; i<this->wx_modcount; i++)  
                if (fread(&mod, 1, sizeof(struct exe_w3_modentry), this->fd)!= sizeof(struct exe_w3_modentry)) {
                    if (ferror(this->fd)) warn("Cannot read %s", this->fname);
                    if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
                } else 
                    printf("  [%02x] \"%s\"     0x%08"PRIx32"  0x%08"PRIx32" (%"PRIu32" bytes)\n", i, mod.name, mod.offset, mod.size, mod.size);
        }
    } else err(1, "Cannot allocate memory");
    return; 
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
                read_le_exe(this);
            } else if ((next_magic[0] == 'W') && (next_magic[1] == '3')) {
                printf("\n\n");
                printf("W3 Executable header found at offset 0x%08"PRIx32"\n", this->mzx->nextHeader);
                read_w3_exe(this);                           
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
    if (this->w3) free(this->w3);
        if (this->le) free(this->le);
    if (this->nemods) free(this->nemods);
    if (this->nesegs) free(this->nesegs);
    if (this->ne) free(this->ne);
    if (this->mzx) free(this->mzx);
    if (this->mz) free(this->mz);
    if (this->fd) if ((fclose(this->fd))) err(1, "Cannot close %s", this->fname);
    free(this);
}

void read_mz_reloc(struct THIS *this) {
    struct exe_mz_reloc reloc;
    off_t oldoffset = ftell(this->fd);

    printf("MZ EXE relocaton table\n"
           "Number of relocations: %d\n", this->mz->relocationEntries);
    fseek(this->fd, this->mz->relocationOffset, SEEK_SET);
    for(int i=0; i<this->mz->relocationEntries; i++)
        if (fread(&reloc, 1, sizeof(struct exe_mz_reloc), this->fd)!= sizeof(struct exe_mz_reloc)) {
            if (ferror(this->fd)) warn("Cannot read %s", this->fname);
            if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
        } else
            printf("  [%d] %04x:%04x\n", i, reloc.segment, reloc.offset);
    fseek(this->fd, oldoffset, SEEK_SET);
    return;
}

void display_help(struct THIS *this) { 
    printf(
        "readexe: Displays information on various Microsoft EXE formats.\n"
        "Version "VERSION"\n\n"
        "  Usage: readexe [-h] [-n offset] EXEFILE.EXE\n\n"
        "  -n\tManually specify offset to next header.\n"
            "\toffset is read as decimal unless prefixed 0x/0X.\n"
        "  -h\tDisplay this help.\n\n"
        "Report bugs at https://github.com/segin/readexe\n"
    );
    destroy_this(this);
    exit(0);
}

int main(int argc, char *argv[]) {
    const uint32_t mz_page_size = 512;
    const uint32_t mz_paragraph_size = 16;
    struct THIS *this;
    uint32_t memuse;
    int option;
    char *endptr;
    long int noffset = -1;

#ifdef NEED_ERR
    setprogname(argv[0]);
#endif
    this = init_this();
    if (argc < 2) { 
        warnx("Not enough arguments.");
        display_help(this);
    }
    while((option = getopt(argc, argv, "hn:")) != -1) {
        switch(option) {
            case 'h':
            case '?':
                display_help(this);
                break;
            case 'n':
                noffset = strtoul(optarg, &endptr, 0);
                if (*endptr != '\0' || (noffset == ULONG_MAX && errno == ERANGE)) err(1, "Invalid value: %s\n", optarg);
                break;
            default:
                abort();
        }
    }
    if(optind < argc) this->fname = argv[optind]; else display_help(this);
    if (!(this->fd = fopen(this->fname, "rb"))) err(1, "Cannot open %s", this->fname);
    if (noffset != -1) { 
        if (!(this->mzx = malloc(sizeof(struct exe_mz_new_header)))) err(1, "Cannot allocate memory");
        this->mzx->nextHeader = noffset;
        read_next_header(this);
    }
    if (!(this->mz = malloc(sizeof(struct exe_mz_header)))) err(1, "Cannot allocate memory");
    if (fread(this->mz, 1, sizeof(struct exe_mz_header), this->fd) != sizeof(struct exe_mz_header)) {
        if (ferror(this->fd)) warn("Cannot read %s", this->fname);
        if (feof(this->fd)) warnx("Unexpected end of file: %s", this->fname);
    } else {
        if (    ((this->mz->magic[0] == 'M') && (this->mz->magic[1] == 'Z')) 
            ||  ((this->mz->magic[1] == 'M') && (this->mz->magic[0] == 'Z')) ) {
            printf("%s:\n", argv[1]);
            printf("DOS executable with magic:\t%c%c (0x%"PRIx8"%"PRIx8")\n", this->mz->magic[0], this->mz->magic[1], this->mz->magic[1], this->mz->magic[0]);
            printf("Number of executable pages:\t0x%04"PRIx16" (%"PRIu32"+ bytes)\n", this->mz->pageCount, ((this->mz->pageCount - 1) * mz_page_size));
            printf("Size of final page:\t\t0x%08"PRIx16" (%"PRIu16" bytes)\n", this->mz->lastPageSize, this->mz->lastPageSize);
            memuse = (((this->mz->pageCount - 1) * mz_page_size) + this->mz->lastPageSize);
            printf("Total code size:\t\t0x%08"PRIx32" (%"PRIu32" bytes)\n", memuse, memuse);
            printf("Total relocation entries:\t0x%04"PRIx16"\n", this->mz->relocationEntries);
            printf("Header size in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", this->mz->hdrSize, (this->mz->hdrSize * mz_paragraph_size));
            printf("Minimum heap in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", this->mz->minMemory, (this->mz->minMemory * mz_paragraph_size));
            printf("Maximum heap in paragraphs:\t0x%04"PRIx16" (%"PRIu32" bytes)\n", this->mz->maxMemory, (this->mz->maxMemory * mz_paragraph_size));
            memuse += (this->mz->minMemory * mz_paragraph_size);
            printf("Minimum memory to load:\t\t%"PRIu32" bytes\n", memuse);
            printf("Initial CS:IP (entrypoint):\t%04"PRIx16":%04"PRIx16"\n", this->mz->initCodeSeg, this->mz->initInstPtr);
            printf("Initial SS:SP (stack):\t\t%04"PRIx16":%04"PRIx16"\n", this->mz->stackSegment, this->mz->stackPointer);
            printf("Checksum:\t\t\t0x%04"PRIx16"\n", this->mz->checksum);
            printf("Relocation table offset:\t0x%04"PRIx16"\n", this->mz->relocationOffset);
            printf("Overlay:\t\t\t0x%04"PRIx16"\n\n", this->mz->overlayNumber);
            if (this->mz->relocationEntries) read_mz_reloc(this);
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
        } else
            fprintf(stdout, "Not a DOS/MZ executable: %s\n", this->fname);
    }
    destroy_this(this);
    return(0);
}
