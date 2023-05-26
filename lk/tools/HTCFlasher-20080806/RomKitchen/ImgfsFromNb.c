/*
 * Original Source from:
 *   Copyright (C) 2005 mamaich @ xda-developers
 *   Copyright (C) 2007-2008 tadzio @ xda-developers
 *
 * Linux version:
 *   Copyright (C) 2008 Pau Oliva Fora - pof @ xda-developers
 *
 * There is no formal license with this code
 * You can basically do what you want with it
 *
 */


#include <stdio.h>
#include <string.h>
#include "linfls.h"

// This is the signature of an IMGFS
static const char IMGFS_GUID[]={0xF8, 0xAC, 0x2C, 0x9D, 0xE3, 0xD4, 0x2B, 0x4D, 
				0xBD, 0x30, 0x91, 0x6E, 0xD8, 0x4F, 0x31, 0xDC };

/* bufferedReadWrite - read from input file, write on output file */
int bufferedReadWrite(FILE *input, FILE *output, unsigned long length)
{
	unsigned char data[2048];
	unsigned long nread;

	while (length > 0) {
		nread = length;
		if (nread > sizeof(data))
			nread = sizeof(data);
		nread = fread(data, 1, nread, input);
		if (!nread)
			return 0;
		if (fwrite(data, 1, nread, output) != nread)
			return 0;
		length -= nread;
	}
	return 1;
}

// Start here...
int main(int argc, char **argv)
{
	static const char signature[] = "MSFLSH50";
	FILE *hNbFile;
	FILE *hRawFile;
	char Base[0x100];
	unsigned long Size;
	unsigned int bytesPerSector = 0;  // default;
	unsigned int ImgFsStart;
	unsigned int ImgFsEnd;
	unsigned int i;
	struct PARTENTRY part;
	struct FlashLayoutSector p;
	struct FlashRegion pRegion;

	printf("ImgfsFromNb 2.1rc2\n");

	if (argc < 3)
	{
		printf("Usage: %s <os.nb.payload> <imgfs.bin>\n"
			   "Creates a raw <imgfs.bin> file from <os.nb.payload>\n", argv[0]);
		return 2;
	}

	hNbFile = fopen(argv[1], "rb");
	if(hNbFile == NULL)
	{
		printf("Could not open input file '%s'. Aborting.\n", argv[1]);
		return 1;
	}

	
    	// find ImgFs partition

	// look into Partition Table at offset 0x1be, find ImgFs partition (i.e., type 4)
	for(i=0; i < 4; i++)
	{
		fseek(hNbFile, 0x1be + i*sizeof(PARTENTRY) , SEEK_SET);
		fread(&part,1,sizeof(PARTENTRY),hNbFile);

		if(part.Part_FileSystem == 0x25) // 0x25 is an ImgFS partition
			break;

	}

	if(i >= 4) // no ImgFs partition found
	{
		printf("Input file '%s' does not contain an ImgFs partition. Aborting.\n", argv[1]);
		return 1;
	}
	
	fseek(hNbFile, 0, SEEK_END);
	Size = ftell(hNbFile);
	fseek(hNbFile, 0, SEEK_SET);

	// find MSFLSH50 header
	// assumption: MSFLSH always starts at a sector border, and sectors size is always a multiple of 0x100
	for(i = 0; i < Size - sizeof(signature); i+=0x100)  
	{
		fseek(hNbFile, i, SEEK_SET);
		fread(&Base,1,0x100,hNbFile);
		if(memcmp(Base, signature, sizeof(signature)) == 0)  
			break;
	}

	if(i < Size - sizeof(signature))
	{
		fseek(hNbFile, i, SEEK_SET);
		fread(&p,1,sizeof(FlashLayoutSector),hNbFile);

		// Flash Region entries start right behind reserved entries
		fseek(hNbFile, i+sizeof(FlashLayoutSector)+p.cbReservedEntries, SEEK_SET);
		fread(&pRegion,1,sizeof(FlashRegion),hNbFile);

		// assumption: all region entries have same sector size value, so just read from the first one
		bytesPerSector = pRegion.dwBytesPerBlock/pRegion.dwSectorsPerBlock;

		// just to allow some visual control
		printf("\nSector size is 0x%x bytes\n", bytesPerSector);
	}

	// get partition start and end (relative to Base)
	ImgFsStart = part.Part_StartSector * bytesPerSector;
	ImgFsEnd = ImgFsStart + part.Part_TotalSectors * bytesPerSector;

	printf("ImgFs partition starts at 0x%08x and ends at 0x%08x\n", ImgFsStart, ImgFsEnd);

	// find first occurence of IMGFS_GUID between start and end of this partition
	for(i = ImgFsStart; i < ImgFsEnd - sizeof(IMGFS_GUID); i++)
	{
		fseek(hNbFile, i, SEEK_SET);
		fread(&Base,1,0x30,hNbFile);
		if(memcmp(IMGFS_GUID, Base, sizeof(IMGFS_GUID))==0 && (memcmp(Base+0x2c, "LZX", 3)==0 || memcmp(Base+0x2c, "XPR", 3)==0))
			break;
	}

	if(i >= ImgFsEnd - sizeof(IMGFS_GUID))
	{
		printf("No IMGFS signature found. Exiting.\n");
		return 1;
	}

	// write ImgFs
	printf("Dumping IMGFS at offset 0x%08x (size 0x%08x)\n", i, ImgFsEnd - i);

	hRawFile = fopen(argv[2], "wb");
	if(hRawFile == NULL)
	{
		printf("Could not create output file '%s'. Exiting.\n", argv[2]);
		return 1;
	}

	printf("Seek: %d, Size: %d\n", i, ImgFsEnd - i);
	
	fseek(hNbFile, i, SEEK_SET);
	bufferedReadWrite(hNbFile, hRawFile, (ImgFsEnd - i));

	fclose(hRawFile);
	fclose(hNbFile);
	printf("\nDone!\n");
	return 0;
}
