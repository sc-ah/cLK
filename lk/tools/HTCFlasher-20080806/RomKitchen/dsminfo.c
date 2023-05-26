/*
 * HTCFlasher - http://htc-flasher.googlecode.com
 *
 * dsminfo - Linux ROM Kitchen tool
 *
 * Copyright (C) 2007-2008 Pau Oliva Fora - pof @ <eslack.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * opinion) any later version. See <http://www.gnu.org/licenses/gpl.html>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dsm.h"

void info()
{
	printf("=== dsminfo v1.0\n");
	printf("=== Part of Linux ROM Kitchen\n");
	printf("=== (c)2008 Pau Oliva Fora - pof @ XDA-Developers\n\n");
}

int main(int argc, char **argv) {

	FILE *dsmfile;
	struct DeviceManifestHeader header;
	struct FileEntry fileentry;
	int i,j;
	int seek;
	char pkgname[1024];
	char filename[1024];
	int verbose=0;

	if (argc < 2) {
		info();
		fprintf(stderr,"Usage: %s filename.rgu [-v]\n", argv[0]);
		return 1;
	}

	if ((argc == 3) && (!strcmp(argv[2],"-v"))) {
		info();
		printf("[] Verbose mode enabled\n\n");
		verbose = 1;
	}

        dsmfile = fopen(argv[1], "rb");
        if (dsmfile == NULL) {
                fprintf(stderr, "[!!] Can't open '%s'\n", argv[1]);
                fclose(dsmfile);
                return 1;
        }

        if (fread(&header,1,sizeof(DeviceManifestHeader),dsmfile)!=sizeof(DeviceManifestHeader)) {
                fprintf(stderr,"[!!] Can't read header\n");
                fclose(dsmfile);
                return 1;
        }

	if (verbose) {
		printf("StructSize:   %ld\n", header.StructSize);
		printf("PackageVersion:   %d.%d.%d.%d\n", header.PackageVersion1,header.PackageVersion2,header.PackageVersion3,header.PackageVersion4);
		printf("PrevPkgVersion:   %d.%d.%d.%d\n", header.PrevPkgVersion1,header.PrevPkgVersion2,header.PrevPkgVersion3,header.PrevPkgVersion4);
		printf("ProcessorID:   %ld\n", header.ProcessorID);
		printf("OSVersion:   %ld\n", header.OSVersion);
		printf("PlatformID:   %ld\n", header.PlatformID);
		printf("NameLength:   %ld\n", header.NameLength);
		printf("NameOffset:   %ld\n", header.NameOffset);
		printf("DependentCount:   %ld\n", header.DependentCount);
		printf("DependentOffset:   %ld\n", header.DependentOffset);
		printf("ShadowCount:   %ld\n", header.ShadowCount);
		printf("ShadowOffset:   %ld\n", header.ShadowOffset);
		printf("FileCount:   %ld\n", header.FileCount);
		printf("FileListOffset:   %ld\n", header.FileListOffset);
		printf("CERTData:   %ld\n", header.CERTData);
		printf("CERTDataOffset:   %ld\n\n", header.CERTDataOffset);
	}

	/* get the Package Name */
	fseek(dsmfile, header.NameOffset, SEEK_SET);
	fread(&pkgname, 1, header.NameLength*2, dsmfile); 
	printf("PKGNAME: ");
	for (i = 0; i < (header.NameLength * 2); i += 2) {
		printf("%c", pkgname[i]);
	}
        printf("\n");
	
	/* get the file list */
	fseek(dsmfile, header.FileListOffset, SEEK_SET);
	for (i = 1; i < (header.FileCount + 1); i++) {

        	seek = fread(&fileentry,1,sizeof(FileEntry),dsmfile);

		if (verbose) {
			printf("\nNameLength:   %ld\n", fileentry.NameLength);
			printf("Flags:   %ld\n", fileentry.Flags);
			printf("Offset:   %ld\n", fileentry.Offset);
			printf("Base:   %ld\n", fileentry.Base);
			printf("Size:   %ld\n", fileentry.Size);
		}

		/* get the file name */
		fseek(dsmfile, fileentry.Offset, SEEK_SET);
		fread(&filename, 1, fileentry.NameLength*2, dsmfile); 
		for (j = 0; j < (fileentry.NameLength * 2); j += 2) {
			printf("%c", filename[j]);
		}
		printf ("\n");

		fseek(dsmfile, header.FileListOffset + i*(sizeof(FileEntry)), SEEK_SET);
	}

	fclose(dsmfile);
	return (0);
}
