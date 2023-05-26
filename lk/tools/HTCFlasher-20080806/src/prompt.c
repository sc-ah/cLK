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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "main.h"

/* fake help */
void showcommandshelp()
{
	printf("Please see XDA-Developers wiki for the list of available commands\n");
	printf("http://wiki.xda-developers.com/\n\n");
}

/* prompt - quick mtty-like implementation */
void prompt()
{
	char str[513];
	int pid;

	pid = fork();
	if (pid) {
		while(1) {
			fgets(str, 512, stdin);
			if (!strncmp (str, "retuoR", 6)) {
				write (fd,"r",1);
				sleep (1);
				write (fd,"e",1);
				sleep (1);
				write (fd,"t",1);
				sleep (1);
				write (fd,"u",1);
				sleep (1);
				write (fd,"o",1);
				sleep (1);
				write (fd,"R",1);
				sleep (1);
			}
			else 
			if (str[0] == '?')
				showcommandshelp();
			else
			if (!strncmp (str, "quit", 4)) {
				close(fd);
				exit(0);
			}
			else
			if (write(fd, str, strlen(str) - 1) == -1) {
				perror("write");
				exit(1);
			}
			write(fd, "\r\n", 2);
		}
	}
	else 
	while(1) {
		if (read(fd, &buf, 1) == -1) {
			perror("read");
			exit (1);
		}
		write(1, &buf, 1);
	}

	close(fd);
	exit(0);
}
