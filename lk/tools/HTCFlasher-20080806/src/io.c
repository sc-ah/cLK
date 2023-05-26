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

#include <sys/select.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "main.h"

/* opendev - open serial device, 115200 baud, 8n1, no flow control */
void opendev()
{
	struct termios tc;

	fd = open (device, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror (device);
		fprintf (stderr, "[!!] Can't connect to %s\n", device);
		exit(1);
	}

	/* Get serial device parameters */
	tcgetattr(fd, &tc);

	tc.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
	                      | INLCR | IGNCR | ICRNL | IXON );
	tc.c_oflag &= ~OPOST;
	tc.c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
	tc.c_cflag &= ~( CSIZE | PARENB | CRTSCTS );
	tc.c_cflag |= CS8 | CREAD | CLOCAL ;
	tc.c_cc[VMIN] = 1;
	tc.c_cc[VTIME] = 3;

	/* Set port speed to 9600 baud */
	cfsetospeed(&tc, B19200);
	cfsetispeed (&tc, B0);

	/* Set TCSANOW mode of serial device */
	tcsetattr(fd, TCSANOW, &tc);

        if (write(fd, "\r", 1) == -1) {
		perror(device);
		fprintf (stderr, "[!!] can't write to %s\n", device);
		exit(1);
	}
}

/* read_wait - block with timeout until file descriptor is readable */
int read_wait(int timeout)
{
	fd_set  read_fds;
	fd_set  except_fds;
	struct timeval tv;
	struct timeval *tp;

	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);
	FD_ZERO(&except_fds);
	FD_SET(fd, &except_fds);

	if (timeout >= 0) {
		tv.tv_usec = 0;
		tv.tv_sec = timeout;
		tp = &tv;
	} else {
		tp = 0;
	}

	for (;;) {
		switch (select(fd + 1, &read_fds, (fd_set *) 0, &except_fds, tp)) {
			case -1:
				if (errno != EINTR)
					continue;
			case 0:
				errno = ETIMEDOUT;
				return (-1);
			default:
				return (0);
		}
	}
}

/* timed_read - read with deadline */
ssize_t timed_read(size_t len, int timeout)
{
	ssize_t ret;

	for (;;) {
		if (timeout > 0 && read_wait(timeout) < 0)
			return (-1);
		if ((ret = read(fd, rec, len)) < 0 && timeout > 0 && errno == EAGAIN) {
			printf("pausing to avoid read loop!\n");
			sleep(1);
			continue;
		} else if (ret < 0 && errno == EINTR) {
			continue;
		} else {
			return (ret);
		}
	}
}

/* get - get output from SPL, store it on buf */
void get()
{
	ssize_t ret;
	int i=0;

	memset(buf, 0, BUFSIZE);

	if (DEBUG)
		printf ("GET:\n[\n");

	while (1) {
		ret = timed_read(REC, READ_TIMEOUT);

		if (ret == -1)
			break;

		buf[i] = rec[0];
		i++;
	}

	if (DEBUG) {
		hexdump(i, 16);
		printf("]\n");
	}
}

/* send - send command to SPL, CRLF terminated */
void send(const char *command)
{
        if (DEBUG)
		printf ("\nSEND: [%s]\n", command);

        write(fd, command, strlen(command));
        write(fd, "\r\n", 2);
}

/* rsend - send command to SPL */
void rsend(const char *command)
{
        if (DEBUG)
		printf ("\nRSEND: [%s]\n", command);

        write(fd, command, strlen(command));
}

/* fsend - send commands to SPL in RUU mode */
void fsend (const char *command)
{
	int i;
	char ruucmd[101];

	if (DEBUG)
		printf ("\nFSEND: [%s]\n", command);

	for (i = 0; i < strlen(command); i++)
		ruucmd[i] = command[i];
	ruucmd[i] = '\r';

	for (i = i + 1; i < 100; i++)
		ruucmd[i] = 0;

        write(fd, ruucmd, 100);
}
