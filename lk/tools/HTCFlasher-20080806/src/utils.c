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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "main.h"

static int control_cs = 5;

/* control_c - capture ^C */
int control_c(int sig)
{
	control_cs--;
	if (control_cs == 0)
		exit(1);
	printf("^C%d\n", control_cs);
	return 0;
}

/* parseHTCBlock - rerturn info inside HTCS/HTCE block */
char* parseHTCBlock()
{
	unsigned long i,j;
	unsigned long start=0,end=0;
	static char retbuf[BUFSIZE];

	memset(retbuf, 0, BUFSIZE);

	for (i = 0; i < BUFSIZE; i++) {
		if ((i >= 3) && (buf[i-3] == 'H') &&
		 (buf[i-2] == 'T') && (buf[i-1] == 'C')) {
			if (buf[i] == 'S') 
				start = i+1;
			else if (buf[i] == 'E') {
				/* substract checksum (4-byte) + HTCE */
				end = i-7;
				break;
			}
		}
	}

	if (i == BUFSIZE && DEBUG == 1)
		printf("[] Can't find HTCS/HTCE block\n");

	j = 0;
	for (i = start; i < end ; i++) {
		retbuf[j]=buf[i];
		j++;
	}
	retbuf[j]='\0';
	
	return retbuf;
}

void zenity(int i)
{
	if (ZENITY) {
		fflush(stdout);
		fprintf (stderr,"%d ZENITY\n",i);
		fflush(stdout);
	}
}

/* progress - send progress command to SPL and display a nice bar */
void progress(int pc)
{
	char *columns = getenv("COLUMNS");
	int tmp, cols = 80;
	char msg[100];

	zenity(pc);
	sprintf(msg, "progress %d", pc);
	fsend(msg);

	if (!ZENITY) {
		(pc < 0) ? pc = 0 : (pc > 100) ? pc = 100 : 0;

		printf("\e[K %3d%% [", pc);

		if (columns)
			cols = atoi(columns);
		cols -= 10;

		for(tmp = (cols * pc) / 100; tmp; tmp--)
			printf("#");

		for(tmp = cols - ((cols * pc) / 100); tmp; tmp--)
			printf("-");

		printf("]\r");
		fflush(stdout);
	}
}

/* hexdump - show hexdump output of the SPL response */
void hexdump(size_t len, size_t width)
{
        unsigned int rows, pos, c, i;
        const char *start, *rowpos, *data;

        data = buf;
        start = data;
        pos = 0;
        rows = (len % width) == 0 ? len / width : len / width + 1;
        for (i = 0; i < rows; i++) {
                rowpos = data;
                printf("%05x: ", pos);
                do {
                        c = *data++ & 0xff;
                        if ((size_t)(data - start) <= len) {
                                printf(" %02x", c);
                        } else {
                                printf("   ");
                        }
                } while(((data - rowpos) % width) != 0);
                printf("  |");
                data -= width;
                do {
                        c = *data++;
                        if (isprint(c) == 0 || c == '\t') {
                                c = '.';
                        }
                        if ((size_t)(data - start) <= len) {
                                printf("%c", c);
                        } else {
                                printf(" ");
                        }
                } while(((data - rowpos) % width) != 0);
                printf("|\n");
                pos += width;
        }
}
