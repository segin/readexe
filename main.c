#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "mz.h"
#include "ne.h"

void read_ne_exe(FILE *fd, const struct exe_mz_new_header *mzx, const char fname[]) {

}

void read_next_header(FILE *fd, const struct exe_mz_new_header *mzx, const char fname[]) {
    char next_magic[2];
    int ret;

    if((ret = fseek(fd, mzx->nextHeader, SEEK_SET))) {
        if ((ret = ferror(fd))) warn("Cannot read %s", fname);
        if ((ret = feof(fd))) warnx("Unexpected end of file: %s", fname);
    }
}

int main(int argc, char *argv[]) {
    struct exe_mz_header *mz = NULL;
    struct exe_mz_new_header *mzx = NULL;
    FILE *fd;
    int ret;

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
                printf("DOS executable with header\t%c%c\n", mz->magic[0], mz->magic[1]);
                printf("Number of executable pages:\t0x%04x (%d+ bytes)\n", mz->pageCount, ((mz->pageCount - 1) * 512));
                printf("Size of final page:\t\t%d bytes\n", mz->lastPageSize);
                printf("Total code size:\t\t0x%08x (%d bytes)\n", 
                    (((mz->pageCount - 1) * 512) + mz->lastPageSize),
                    (((mz->pageCount - 1) * 512) + mz->lastPageSize));
                printf("Total relocation entries:\t0x%04x\n", mz->relocationEntries);
                printf("Header size in paragraphs:\t0x%04x (%d bytes)\n", mz->hdrSize, (mz->hdrSize * 16));
                printf("Minimum memory in paragraphs:\t0x%04x (%d bytes)\n", mz->minMemory, (mz->minMemory * 16));
                printf("Maximum memory in paragraphs:\t0x%04x (%d bytes)\n", mz->maxMemory, (mz->maxMemory * 16));
                printf("Initial stack segment:\t\t0x%04x\n", mz->stackSegment);
                printf("Initial stack pointer:\t\t0x%04x\n", mz->stackPointer);
                printf("Checksum:\t\t\t0x%04x\n", mz->checksum);
                printf("Initial CS:IP far pointer:\t%04x:%04x\n", (mz->initCodeSegIP >> 16), (mz->initCodeSegIP & 0xFFFF));
                printf("Relocation table offset:\t0x%04x\n", mz->relocationOffset);
                printf("Overlay:\t\t\t0x%04x\n", mz->overlayNumber);
                /* check for next header */
                if(mz->relocationOffset >= 0x40) {
                    if (!(mzx = malloc(sizeof(struct exe_mz_new_header)))) err(1, "Cannot allocate memory");
                    ret = fread(mzx, 1, sizeof(struct exe_mz_new_header), fd);
                    if (ret != sizeof(struct exe_mz_new_header)) {
                        if ((ret = ferror(fd))) warn("Cannot read %s", argv[1]);
                        if ((ret = feof(fd))) warnx("Unexpected end of file: %s", argv[1]);
                    } else {
                        printf("Offset to NE/PE header:\t\t0x%08x\n", mzx->nextHeader);
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