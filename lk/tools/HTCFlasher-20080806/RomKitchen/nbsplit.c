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
#include <stdlib.h>

#define PAY_EXT ".payload"
#define EXTRA_EXT ".extra"


int main(int argc, char *argv[])
{

	size_t payloadChunkSize = 0, extraChunkSize = 0;  // safe default
	size_t bytes;
	char *filearg;
	char *filename;
	char *buffer;
	FILE *nb, *payload, *extra;

	printf("NBSplit 2.1rc2\n");

	if(argc < 3)
	{
	printf("Usage: %s -hermes|-kaiser|-titan|-wizard|-athena|-sp\n"
		"|-data <number> -extra <number> <filename.nb>\n"
		"Generates <filename.nb.payload> and <filename.nb.extra> from <filename.nb>\n"
		"<filename.nb> will NOT get overwritten.\n", argv[0]);
		return 1;
	}


	if(!strcmp(argv[1], "-wizard") || !strcmp(argv[1], "-athena"))
	{
		printf("Wizard and Athena ROMs do not contain extra bytes.\n"
		"You need not run %s. Just copy <filename.nb> to <filename.nb.payload>\n", argv[0]);
		return 0;
	}

	// payload and extra chunk sizes explicitely given
	if(argc > 5 && !strcmp(argv[1], "-data") && !strcmp(argv[3], "-extra"))  // need nbsplit -data xxx -extra yyy <filename.nb>
	{
		payloadChunkSize = atoi(argv[2]);
		extraChunkSize = atoi(argv[4]);
	}


	if(!strcmp(argv[1], "-hermes") || !strcmp(argv[1], "-sp"))
	{
		payloadChunkSize = 0x200;
		extraChunkSize = 0x08;
	}

	if(!strcmp(argv[1], "-titan") || !strcmp(argv[1], "-kaiser"))
	{
		payloadChunkSize = 0x800;
		extraChunkSize = 0x08;
	}

#if 0 // Does not work with WM6 emulator images
	if(!strcmp(argv[1], "-emu"))
	{
		payloadChunkSize = 0xF000;
		extraChunkSize = 0x1000;
	}
#endif

	if(payloadChunkSize == 0)  // no matching argument given
	{
		printf("Wrong or missing parameters.\n");
		printf("Usage: %s -hermes|-kaiser|-titan|-acer|-wizard|-athena|-sp\n"
			   "|-data <number> -extra <number> <filename.nb>\n"
			   "Generates <filename.nb.payload> and <filename.nb.extra> from <filename.nb>\n"
			   "<filename.nb> will NOT get overwritten.\n", argv[0]);
		return 1;
	}

	filearg = argv[argc-1];  // last argument is the filename
	filename = (char *)malloc(strlen(filearg) + strlen(PAY_EXT) + strlen(EXTRA_EXT));

	if(!filename)
	{
		printf("Error allocating memory\n");
		return 1;
	}


	if(NULL == (nb = fopen(filearg, "rb")))
	{
		printf("Could not open input file %s\n", filearg);
		free(filename);
		return 1;
	}

	sprintf(filename, "%s%s", filearg, PAY_EXT);

	if(NULL == (payload = fopen(filename, "w+b")))
	{
		printf("Could not open output file %s\n", filename);
		free(filename);
		fclose(nb);
		return 1;
	}

	sprintf(filename, "%s%s", filearg, EXTRA_EXT);

	if(NULL == (extra = fopen(filename, "w+b")))
	{
		printf("Could not open output file %s\n", filename);
		free(filename);
		fclose(payload);
		fclose(nb);
		return 1;
	}

	free(filename);

	printf("Using data chunk size = 0x%x and extra chunk size = 0x%x\n"
		"on file %s\n", payloadChunkSize, extraChunkSize, filearg );

	buffer = (char *)malloc(payloadChunkSize + extraChunkSize);

	do
	{
		bytes = fread(buffer, 1, payloadChunkSize, nb);
		if(bytes != fwrite(buffer, 1, bytes, payload))
		{
			printf("Error writing to payload file. Generated files are unusable!\n");
			free(buffer);
			fclose(nb);
			fclose(payload);
			fclose(extra);
			return 1;
		}

		if(bytes != payloadChunkSize)
			break;

		bytes = fread(buffer, 1, extraChunkSize, nb);
		if(bytes != fwrite(buffer, 1, bytes, extra))
		{
			printf("Error writing to extra file. Generated files are unusable!\n");
			free(buffer);
			fclose(nb);
			fclose(payload);
			fclose(extra);
			return 1;
		}
		if(bytes != extraChunkSize)
			break;
	}
	while(1);

	free(buffer);

	fclose(nb);
	fclose(payload);
	fclose(extra);

	printf("Done.\n");

	return 0;
}
