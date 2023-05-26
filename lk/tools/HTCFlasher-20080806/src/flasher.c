/*
 * HTCFlasher - http://htc-flasher.googlecode.com
 *
 * Copyright (C) 2007-2008 Pau Oliva Fora - pof @ <eslack.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * opinion) any later version. See <http://www.gnu.org/licenses/gpl.html>
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include "main.h"

/* ruustart - this must be done this way */
void ruustart()
{
	int i, verbose;
	char *ruustart="ruustart\r";
	char cmd[2];

	verbose = DEBUG;
	DEBUG = 0;

	printf("[] Setting RUU mode, please wait...");
	fflush(stdout);

	for (i = 0; i < strlen(ruustart); i++) {
		sprintf(cmd,"%c", ruustart[i]);
		rsend(cmd);
		get();
		printf(".");
		fflush(stdout);
	}
	printf("done\n");
	fflush(stdout);

	/* return DEBUG to normal state */
	DEBUG=verbose;
}

/* authBL - start RUU mode and authenticate to SPL */
void authBL()
{
	char *res;
	
	ruustart();

	fsend("password BsaD5SeoA");
	get();

	res = parseHTCBlock();
	if (strlen(res) != 0) 
		printf("[] SPL auth result (T=True, F=False): %s\n", res);
}

/* blockresult - parse SPL result after sending a data block */
int blockresult()
{
	/* 
	 * After we send a block of data encapsulated in HTCS + HTCE the bootloader
	 * returns a message encoded in HTCS + msg + 4byte checksum + HTCE, these
	 * are the known return values:
	 */
	char *BLOCK_OK =	"\x48\x54\x43\x53\x01\x00\x01\x00\x00\x00\x00\x00";
	char *UPDATE_END =	"\x48\x54\x43\x53\x01\x00\x02\x00\x00\x00\x00\x00";
	char *CRC_ERROR =	"\x48\x54\x43\x53\x01\x80\x03\x00\x01\x00\x00\x00";
	char *CERT_ERROR =	"\x48\x54\x43\x53\x02\x80\x06\x00\x00\x00\x00\x00";
	char *MODELID_ERROR =	"\x48\x54\x43\x53\x02\x80\x05\x00\x01\x00\x00\x00";
	char *PART_END =	"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	int i;

	for (i = 0; i < BUFSIZE; i++) {
		if (!strcmp(&buf[i], BLOCK_OK)) {
			if (DEBUG)
				printf ("\n[] WDATA BLOCK RECEIVED OK\n");
			return(0);
		}
		if (!strcmp(&buf[i], PART_END)) {
			if (DEBUG)
				printf ("\n[] NBH PART ENDED\n");
			return(0);
		}
		if (!strcmp(&buf[i], UPDATE_END)) {
			if (DEBUG)
				printf ("\n[] NBH UPLOAD COMPLETED OK\n");
			return(0);
		}
		if (!strcmp(&buf[i], CRC_ERROR)) {
			printf ("\n[!!] CHECKSUM ERROR\n");
			if (!DEBUG)
				hexdump(96, 16);
			return(1);
		}
		if (!strcmp(&buf[i], CERT_ERROR)) {
			printf ("\n[!!] CERTIFICATE ERROR\n");
			if (!DEBUG)
				hexdump(96, 16);
			return(1);
		}
		if (!strcmp(&buf[i], MODELID_ERROR)) {
			printf ("\n[!!] MODEL ID ERROR\n");
			if (!DEBUG)
				hexdump(96, 16);
			return(1);
		}
	}

	/* unknown error */
	printf("\n[] SPL OUTPUT:\n");
	if (!DEBUG)
		hexdump(96, 16);
	return(1);
}

/* flashNBH - flash a NBH file */
int flashNBH (char *flashfile)
{
	unsigned int dataCrc;
	unsigned int totalread;
	unsigned int readlen;
	unsigned int writelen;
	unsigned int lps;
	unsigned int p;

	int fd2, bread;
	off_t fsize;
	char command[100];

	unsigned char nbhbuffer[RUUBLOCK];

	authBL();

	fd2 = open(flashfile, O_RDONLY);
	if (fd2 < 0) {
		perror (flashfile);
		fprintf (stderr, "Can't open %s\n",flashfile);
		exit (1);
	}
	fsize = lseek(fd2, 0, SEEK_END);
	lseek(fd2, 0, SEEK_SET);

	/* flash NBH using wdata command */
	totalread = 0;
	do {
		p=(int)(((float)totalread / fsize) * 100);
		progress(p);
		
		if ((fsize - totalread) < RUUBLOCK)
			lps = fsize - totalread;
		else
			lps = RUUBLOCK;

		memset (nbhbuffer, 0, RUUBLOCK);
		readlen = read(fd2, nbhbuffer, lps);

		dataCrc = Crc32(nbhbuffer, lps, 0);

		sprintf(command,"wdata %x %x", lps, dataCrc);
		fsend(command);
		rsend("HTCS");

                bread = 0 ;
                do
                {
                        if ((bread - lps) < 0x2000)
				writelen = write (fd, nbhbuffer+bread, (bread-lps));
                        else
				writelen = write (fd, nbhbuffer+bread, 0x2000);
                        bread += writelen;
                } while (bread < lps);

		totalread += lps;

		rsend("HTCE");
		get();
		blockresult();

	} while (totalread < fsize);

	printf ("\nDone!\n");
	progress(100);
	sleep (1);
	fsend("ResetDevice");
	sleep (1);
	close (fd2);
	return 0;
}
