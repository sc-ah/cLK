/*
 * nbextract.c
 *
 *  Created on: May 24, 2023
 *      Author: J0SH1X
 */

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdint.h>

struct PartEntry {
  uint8_t BootInd;
  uint8_t FirstHead;
  uint8_t FirstSector;
  uint8_t FirstTrack;
  uint8_t FileSystem;
  uint8_t LastHead;
  uint8_t LastSector;
  uint8_t LastTrack;
  uint32_t StartSector;
  uint32_t TotalSectors;
};


int main(int argc, char* argv[]) {
    printf("nbextract v1.0 by J0SH1X\n");
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <nbhFile> <binFile>\n", argv[0]);
        return 1;
    }

    char* nbhFile = argv[1];
    char* binFile = argv[2];

    extract(nbhFile, binFile);

    return 0;
}

void extract(char* nbhFile, char* binFile) {
    FILE* nbh = fopen(nbhFile, "r");
    if (nbh == NULL) {
        fprintf(stderr, "Failed to open %s\n", nbhFile);
        exit(1);
    }

    FILE* bin = fopen(binFile, "w");
    if (bin == NULL) {
        fprintf(stderr, "Failed to create %s\n", binFile);
        exit(1);
    }

    fseek(nbh, 0x800, SEEK_SET);  // Skip the file header block
    fseek(nbh, 0x800, SEEK_CUR);  // Skip the MSFLASH50 block

    char sector[0x800];
    size_t bytesRead = 0;
    size_t totalBytesRead = 0;
    while ((bytesRead = fread(sector, 1, sizeof(sector), nbh)) > 0) {
        if (bytesRead == sizeof(sector)) {
            fwrite(sector, 1, bytesRead, bin);
        } else {
            // Exclude the last 12 bytes (tags) from being written
            fwrite(sector, 1, bytesRead - 12, bin);
        }

        totalBytesRead += bytesRead;
    }

    fclose(nbh);
    fclose(bin);

    // Create a new file and write the modified content without the first 32 bytes and trailing 'F' bytes
    FILE* extracted = fopen("extracted.bin", "w");
    if (extracted == NULL) {
        fprintf(stderr, "Failed to create extracted.bin\n");
        exit(1);
    }

    bin = fopen(binFile, "r");
    if (bin == NULL) {
        fprintf(stderr, "Failed to open %s\n", binFile);
        exit(1);
    }

    // Skip the first 32 bytes
    fseek(bin, 16, SEEK_SET);

    // Copy the modified content to the new file, excluding the first 32 bytes and trailing 'F' bytes
    size_t bytesToWrite = totalBytesRead - 32;
    while (bytesToWrite > 0) {
        char byte = fgetc(bin);
        if (byte != 'F') {
            fputc(byte, extracted);
            bytesToWrite--;
        } else {
            // Stop writing if 'F' bytes are encountered at the end
            if (bytesToWrite == totalBytesRead - 32)
                break;
        }
    }

    fclose(bin);
    fclose(extracted);

    // Remove the original binFile and rename the new file
    remove(binFile);
    rename("extracted.bin", binFile);
}


// ....*
//D6 04 00 00 00 00 00 00 00 00 00 00 00 00 00
