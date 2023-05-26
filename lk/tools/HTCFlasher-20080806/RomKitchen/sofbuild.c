/*
 * HTCFlasher - http://htc-flasher.googlecode.com
 *
 * sofbuild - Linux ROM Kitchen tool
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
#include <dirent.h>

int main(int argc, char **argv) {

	DIR *d;
	struct dirent *dir;
	char dsmfilename[256];
	char folder[1024];
	char *fileext;
	FILE *packages_sof;
	unsigned int guid[16];
	char guidPackage[16];
	int i,j;
	char whatever[64];
	char whatever2[64];

	printf("=== sofbuild v1.0\n");
	printf("=== Part of Linux ROM Kitchen\n");
	printf("=== Generates 'packages.sof' from a folder containing all DSM files\n");
	printf("=== (c)2008 Pau Oliva Fora - pof @ XDA-Developers\n\n");

	memset(dsmfilename,0,sizeof(dsmfilename));
	memset(folder,0,sizeof(folder));

	if (argc < 2) {
		fprintf(stderr,"Usage: %s folder\n", argv[0]);
		return 0;
	}

	strcpy(folder, argv[1]);

	d = opendir(folder);
	if( d == NULL ) {
		fprintf(stderr, "[!!] Can't open dir '%s'\n", folder);
		return 1;
	}

        packages_sof = fopen("packages.sof", "wb");
        if (packages_sof == NULL) {
                fprintf(stderr, "[!!] Can't open 'packages.sof'\n");
                fclose(packages_sof);
                exit(1);
        }

	/* find dsm file */
	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 ) {
			continue;
		}

		fileext = strrchr(dir->d_name, '.');
		if ((fileext != NULL) && (!strcmp (fileext, ".dsm"))) {
			strcpy(dsmfilename, dir->d_name);
			printf ("[] Found DSM: %s\n", dsmfilename);

			if (strlen(dsmfilename) == 0) {
				fprintf(stderr,"[!!] Can't find dsm file in '%s'\n", folder);
				return 1;
			}

			if (strlen(dsmfilename) != 40) {
				fprintf(stderr,"[!!] Invalid dsm file name '%s'\n", dsmfilename);
				return 1;
			}


			/* all this crap is an ugly way to put the guid in the header */
			j=0;
			for (i=0; i < (strlen(dsmfilename)-4); i++) {
				if (dsmfilename[i] != '-') {
					whatever[j]=dsmfilename[i];
					j++;
				}
			}

			j=0;
			for (i=0; i < (strlen(whatever)); i++) {
				if ((i%2 == 0) && (i!=0)) {
					whatever2[j] = '-';
					j++;
				} 
				whatever2[j]=whatever[i];
				j++;
			}

			sscanf(whatever2,"%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x",
					&guid[0],&guid[1],&guid[2],&guid[3],&guid[4],&guid[5],&guid[6],&guid[7],
					&guid[8],&guid[9],&guid[10],&guid[11],&guid[12],&guid[13],&guid[14],&guid[15]);

			memcpy(&guidPackage[0], &guid[3], 1);
			memcpy(&guidPackage[1], &guid[2], 1);
			memcpy(&guidPackage[2], &guid[1], 1);
			memcpy(&guidPackage[3], &guid[0], 1);

			memcpy(&guidPackage[4], &guid[5], 1);
			memcpy(&guidPackage[5], &guid[4], 1);
			memcpy(&guidPackage[6], &guid[7], 1);
			memcpy(&guidPackage[7], &guid[6], 1);
			memcpy(&guidPackage[8], &guid[8], 1);
			memcpy(&guidPackage[9], &guid[9], 1);
			memcpy(&guidPackage[10], &guid[10], 1);
			memcpy(&guidPackage[11], &guid[11], 1);
			memcpy(&guidPackage[12], &guid[12], 1);
			memcpy(&guidPackage[13], &guid[13], 1);
			memcpy(&guidPackage[14], &guid[14], 1);
			memcpy(&guidPackage[15], &guid[15], 1);
			/* finished */

			fwrite(&guidPackage, 1, sizeof(guidPackage), packages_sof);

		}
	}

	closedir(d);
	fclose(packages_sof);

	return (0);
}
