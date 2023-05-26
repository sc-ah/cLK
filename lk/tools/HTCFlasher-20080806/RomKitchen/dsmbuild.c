/*
 * HTCFlasher - http://htc-flasher.googlecode.com
 *
 * dsmbuild - Linux ROM Kitchen tool
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
#include "dsm.h"

int main(int argc, char **argv) {

	DIR *d;
	struct dirent *dir;
	char dsmfilename[256];
	char folder[1024];
	char dsmfilepath[1280];
	char *fileext;
	int filecount=0;
	FILE *dsmfile;
	struct DeviceManifestHeader header;
	unsigned int guid[16];
	int i,j;
	char whatever[64];
	char whatever2[64];
	char tmpbuf[1024];
	struct FileEntry fileentry;
	int offset = 0;

	printf("=== dsmbuild v1.0\n");
	printf("=== Part of Linux ROM Kitchen\n");
	printf("=== Generates a valid DSM file from a package folder\n");
	printf("=== (c)2008 Pau Oliva Fora - pof @ XDA-Developers\n\n");

	memset(dsmfilename,0,sizeof(dsmfilename));
	memset(folder,0,sizeof(folder));
	memset(&header,0,sizeof(DeviceManifestHeader));

	if (argc < 2) {
		fprintf(stderr,"Usage: %s folder\n", argv[0]);
		return 0;
	}

	strcpy(folder, argv[1]);

	header.StructSize=sizeof(DeviceManifestHeader);
  	header.PackageFlags=2;
	header.ProcessorID=450;
	header.OSVersion=501;
	header.PlatformID=0;
	header.NameLength=40;
	header.NameOffset=0; // we'll find it later
	header.DependentCount=0;
	header.DependentOffset=0;
	header.ShadowCount=0;
	header.ShadowOffset=0;
	header.FileCount=0; // we'll find it later
	header.FileListOffset=header.StructSize;
	header.CERTData=0;
	header.CERTDataOffset=0;
	header.PackageVersion1=0;
	header.PackageVersion2=0;
	header.PackageVersion3=0;
	header.PackageVersion4=0;
	header.PrevPkgVersion1=0;
	header.PrevPkgVersion2=0;
	header.PrevPkgVersion3=0;
	header.PrevPkgVersion4=0;


	d = opendir(folder);
	if( d == NULL ) {
		fprintf(stderr, "Can't open dir '%s'\n", folder);
		return 1;
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
		} else {
			filecount++;
		}
	}
	closedir(d);

	
	if (strlen(dsmfilename) == 0) {
		fprintf(stderr,"[!!] Can't find dsm file in '%s'\n", folder);
		return 1;
	}

	if (strlen(dsmfilename) != 40) {
		fprintf(stderr,"[!!] Invalid dsm file name '%s'\n", dsmfilename);
		return 1;
	}

#if 0
	if (filecount == 0) {
		fprintf(stderr,"Empty dir '%s'\n", folder);
		return 1;
	}
#endif

	header.FileCount = filecount;
	header.NameOffset = header.StructSize+filecount*(sizeof(FileEntry));

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

	memcpy(&header.guidPackage[0], &guid[3], 1);
	memcpy(&header.guidPackage[1], &guid[2], 1);
	memcpy(&header.guidPackage[2], &guid[1], 1);
	memcpy(&header.guidPackage[3], &guid[0], 1);

	memcpy(&header.guidPackage[4], &guid[5], 1);
	memcpy(&header.guidPackage[5], &guid[4], 1);
	memcpy(&header.guidPackage[6], &guid[7], 1);
	memcpy(&header.guidPackage[7], &guid[6], 1);
	memcpy(&header.guidPackage[8], &guid[8], 1);
	memcpy(&header.guidPackage[9], &guid[9], 1);
	memcpy(&header.guidPackage[10], &guid[10], 1);
	memcpy(&header.guidPackage[11], &guid[11], 1);
	memcpy(&header.guidPackage[12], &guid[12], 1);
	memcpy(&header.guidPackage[13], &guid[13], 1);
	memcpy(&header.guidPackage[14], &guid[14], 1);
	memcpy(&header.guidPackage[15], &guid[15], 1);
	/* finished */

	strcat(folder, "/");
	strcat(folder, dsmfilename);
	strcpy(dsmfilepath, folder);
	strcpy(folder, argv[1]);

        dsmfile = fopen(dsmfilepath, "wb");
        if (dsmfile == NULL) {
                fprintf(stderr, "[!!] Can't open '%s'\n", dsmfilepath);
                fclose(dsmfile);
                exit(1);
        }


	fwrite(&header, 1, sizeof(DeviceManifestHeader), dsmfile);

	offset = sizeof(DeviceManifestHeader) + (sizeof(FileEntry)*(filecount)) + (header.NameLength * 2);

	/* dsm files must contain the rgu file on the first table entry */
	d = opendir(folder);
	while ((dir = readdir(d)) != NULL)
	{
		fileext = strrchr(dir->d_name, '.');
		if ((fileext != NULL) && (!strcmp (fileext, ".rgu"))) {
			printf ("RGU: %s\n", dir->d_name);
			fileentry.NameLength = strlen(dir->d_name);
			fileentry.Flags = 0;
			fileentry.Offset = offset;
			fileentry.Base = 0;
			fileentry.Size = 0;
			offset = offset + fileentry.NameLength*2;
			fwrite(&fileentry, 1, sizeof(FileEntry), dsmfile);
		}
	}
	closedir(d);

	/* now process the rest of the files */
	d = opendir(folder);
	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 ) {
			continue;
		}

		fileext = strrchr(dir->d_name, '.');

		if ((fileext != NULL) && (!strcmp (fileext, ".dsm"))) {
			continue;
		}

		if ((fileext != NULL) && (!strcmp (fileext, ".rgu"))) {
			continue;
		}

		printf ("[] Adding file table entry for %s\n",dir->d_name);

		fileentry.NameLength = strlen(dir->d_name);
		fileentry.Flags = 0;
		fileentry.Offset = offset;
		fileentry.Base = 0;
		fileentry.Size = 0;
		offset = offset + fileentry.NameLength*2;
		fwrite(&fileentry, 1, sizeof(FileEntry), dsmfile);
	}
	closedir(d);

	/* finally write the filenames */

	/* first the package name, set to the name of the dsm file */
	memset(tmpbuf,0,sizeof(tmpbuf));
	j=0;
	for (i=0; i< strlen(dsmfilename); i++) {
		tmpbuf[j]=dsmfilename[i];
		j++;
		tmpbuf[j]='\0';
		j++;
	}
	fwrite(tmpbuf, 1, strlen(dsmfilename)*2, dsmfile);

	/* second the filename of the rgu file, if it exists */
	d = opendir(folder);
	while ((dir = readdir(d)) != NULL)
	{
		fileext = strrchr(dir->d_name, '.');
		if ((fileext != NULL) && (!strcmp (fileext, ".rgu"))) {
			memset(tmpbuf,0,sizeof(tmpbuf));
			j=0;
			for (i=0; i< strlen(dir->d_name); i++) {
				tmpbuf[j]=dir->d_name[i];
				j++;
				tmpbuf[j]='\0';
				j++;
			}
			fwrite(tmpbuf, 1, strlen(dir->d_name)*2, dsmfile);
		}
	}
	closedir(d);

	/* third all the rest of filenames */
	d = opendir(folder);
	while ((dir = readdir(d)) != NULL)
	{
		if (strcmp(dir->d_name, "." ) == 0 || strcmp( dir->d_name, ".." ) == 0 ) {
			continue;
		}

		fileext = strrchr(dir->d_name, '.');

		if ((fileext != NULL) && (!strcmp (fileext, ".dsm"))) {
			continue;
		}

		if ((fileext != NULL) && (!strcmp (fileext, ".rgu"))) {
			continue;
		}

		printf ("[] Adding file name for: %s\n",dir->d_name);

		memset(tmpbuf,0,sizeof(tmpbuf));
		j=0;
		for (i=0; i< strlen(dir->d_name); i++) {
			tmpbuf[j]=dir->d_name[i];
			j++;
			tmpbuf[j]='\0';
			j++;
		}
		fwrite(tmpbuf, 1, strlen(dir->d_name)*2, dsmfile);

	}
	closedir(d);

	fclose(dsmfile);
	return (0);
}
