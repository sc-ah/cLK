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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "main.h"

/* getinfo - this should work on any modern HTC device */
void getinfo()
{
	char *result;

	zenity(15);
	send("info 2");
	get();
	zenity(30);
	result = parseHTCBlock();
	if (strlen(result) != 0)
		printf("[] CID: %s\n", result);
	zenity(50);
	send("getdevinfo");
	get();
	zenity(75);
	result = parseHTCBlock();
	if (strlen(result) != 0)
		printf("[] ModelID: %s\n", result);
	else {
		authBL();
		fsend("getdevinfo");
		get();
		result = parseHTCBlock();
		zenity(85);
		if (strlen(result) != 0)
			printf("[] ModelID: %s\n", result);
		else
			printf("[!!] Your device can't be identified!\n");
		fsend("ruurun 0");
		fsend("ruurun 0");
		get();
		fsend("ResetDevice");
		fsend("ResetDevice");
		get();
	}
	zenity(100);
}
