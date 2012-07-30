/*
 * Copyright (c) 2008, Google Inc.
 * All rights reserved.
 *
 * Copyright (c) 2009-2010, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the 
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <debug.h>
#include <err.h>
#include <stdlib.h>
#include <dev/fbcon.h>
#include <splash.h>

#include "font5x12.h"
#include "font8x16.h"

struct pos {
	int x;
	int y;
};

static struct fbcon_config *config = NULL;

#define RGB565_BLACK		0x0000
#define RGB565_WHITE		0xffff
#define RGB565_RED		    0xf800
#define RGB565_GREEN		0x07e0
#define RGB565_BLUE		    0x001f
#define RGB565_YELLOW		0xffe0
#define RGB565_CYAN		    0x07ff
#define RGB565_MAGENTA		0xf81f

#define RGB888_BLACK        0x000000
#define RGB888_WHITE        0xffffff

#if 0
#define FONT_WIDTH		5
#define FONT_HEIGHT		12
#define FONT_PPCHAR		2
static unsigned *font = font5x12;
#else
#define FONT_WIDTH		8
#define FONT_HEIGHT		16
#define FONT_PPCHAR		16
static unsigned *font = font8x16;
#endif

static uint16_t	BGCOLOR;
static uint16_t	FGCOLOR;

static struct pos cur_pos;
static struct pos max_pos;

static unsigned top_margin;
static unsigned bottom_margin;

static unsigned reverse_font8x16(unsigned x)
{
	unsigned y = 0;
	for (uint8_t i = 0; i < 9; ++i) {
		y <<= 1;
		y |= (x & 1);
		x >>= 1;
	}
	return y;
}

static void fbcon_drawglyph(uint16_t *pixels, uint16_t paint, unsigned stride,
			    unsigned *glyph)
{
	stride -= FONT_WIDTH;
	for(unsigned i = 0; i < FONT_PPCHAR; i++) {
		unsigned x, y;
		unsigned data = glyph[i];
#if 1
		data = reverse_font8x16(data);
#endif
		for (y = 0; y < (FONT_HEIGHT / FONT_PPCHAR); ++y) {
			for (x = 0; x < FONT_WIDTH; ++x) {
				if (data & 1)
					*pixels = paint;
				data >>= 1;
				pixels++;
			}
			pixels += stride;
		}
	}
}

static void fbcon_flush(void)
{
	if (config->update_start)
		config->update_start();
	if (config->update_done)
		while (!config->update_done());
}

/* TODO: Take stride into account */
static void fbcon_scroll_up(void)
{
	unsigned bytes_per_bpp = ((config->bpp) / 8);
	unsigned short *dst = config->base + (config->width * FONT_HEIGHT * (top_margin + 1) * bytes_per_bpp);
	unsigned short *src = dst + (config->width * FONT_HEIGHT);
#if 0
	unsigned count = config->width * (config->height - FONT_HEIGHT);
	while(count--) {
		*dst++ = *src++;
	}
#else
	memcpy(dst, src, config->width * (config->height - FONT_HEIGHT) * bytes_per_bpp);
#endif
	/* clear the last line */
#if 0
	count = config->width * FONT_HEIGHT;
	while(count--) {
		*dst++ = BGCOLOR;
	}
#else
	memset(dst, BGCOLOR, config->width * FONT_HEIGHT);
#endif
	fbcon_flush();
}

/* TODO: take stride into account */
void fbcon_clear(void)
{
	unsigned bytes_per_bpp = ((config->bpp) / 8);
	unsigned count = config->width * ((config->height - (top_margin + bottom_margin)) * FONT_HEIGHT);
	memset(config->base + (config->width * FONT_HEIGHT * (top_margin + 1) * bytes_per_bpp), BGCOLOR, count * bytes_per_bpp);
}

void fbcon_set_top_margin(unsigned tm)
{
	top_margin = tm;
}

void fbcon_set_bottom_margin(unsigned bm)
{
	bottom_margin = bm;
}

void fbcon_set_colors(unsigned bg, unsigned fg)
{
	BGCOLOR = bg;
	FGCOLOR = fg;
}

static unsigned putc_sync = 0;

void fbcon_putc(char c)
{
	uint16_t *pixels;

	/* ignore anything that happens before fbcon is initialized */
	if (!config)
		return;

	if((unsigned char)c > 127)
		return;
	if((unsigned char)c < 32) {
		if(c == '\n')
			goto newline;
		return;
	}

	while(putc_sync)
		thread_sleep(10);

	putc_sync = 1;

	pixels = config->base;
	pixels += cur_pos.y * FONT_HEIGHT * config->width;
	pixels += cur_pos.x * (FONT_WIDTH + 1);

	fbcon_drawglyph(pixels, FGCOLOR, config->stride,
			font + (c - 32) * FONT_PPCHAR);

	cur_pos.x++;
	if (cur_pos.x < max_pos.x) {
		putc_sync = 0;
		return;
	}

newline:
	cur_pos.y++;
	cur_pos.x = 0;
	if(cur_pos.y >= max_pos.y) {
		cur_pos.y = max_pos.y - 1;
		fbcon_scroll_up();
	} else
		fbcon_flush();
	putc_sync = 0;
}

void fbcon_setup(struct fbcon_config *_config)
{
	uint32_t bg;
	uint32_t fg;

	ASSERT(_config);

	config = _config;

	switch (config->format) {
	case FB_FORMAT_RGB565:
		fg = RGB565_BLACK;
		bg = RGB565_WHITE;
		break;
	case FB_FORMAT_RGB888:
		fg = RGB888_WHITE;
		bg = RGB888_BLACK;
		break;
	default:
		dprintf(CRITICAL, "unknown framebuffer pixel format\n");
		ASSERT(0);
		break;
	}

	fbcon_set_colors(bg, fg);

	fbcon_set_top_margin(0);
	fbcon_set_bottom_margin(0);

	cur_pos.x = 0;
	cur_pos.y = top_margin + 1;

	max_pos.x = config->width / (FONT_WIDTH+1);
	max_pos.y = (config->height - 1) / FONT_HEIGHT - bottom_margin;

#if !DISPLAY_SPLASH_SCREEN
	fbcon_clear();
#endif
}

void fbcon_reset(void)
{
	fbcon_clear();

	cur_pos.x = 0;
	cur_pos.y = top_margin + 1;

	max_pos.x = config->width / (FONT_WIDTH+1);
	max_pos.y = (config->height - 1) / FONT_HEIGHT - bottom_margin;
}

struct fbcon_config* fbcon_display(void)
{
    return config;
}

void diplay_image_on_screen(void)
{
    unsigned i = 0;
    unsigned total_x = config->width;
    unsigned total_y = config->height;
    unsigned bytes_per_bpp = ((config->bpp) / 8);
    unsigned image_base = ((((total_y/2) - (SPLASH_IMAGE_WIDTH / 2) - 1) *
			    (config->width)) + (total_x/2 - (SPLASH_IMAGE_HEIGHT / 2)));
    fbcon_clear();

#if DISPLAY_TYPE_MIPI
    if (bytes_per_bpp == 3)
    {
        for (i = 0; i < SPLASH_IMAGE_WIDTH; i++)
        {
            memcpy (config->base + ((image_base + (i * (config->width))) * bytes_per_bpp),
		    imageBuffer_rgb888 + (i * SPLASH_IMAGE_HEIGHT * bytes_per_bpp),
		    SPLASH_IMAGE_HEIGHT * bytes_per_bpp);
        }
    }
    fbcon_flush();
    if(is_cmd_mode_enabled())
        mipi_dsi_cmd_mode_trigger();

#else
    if (bytes_per_bpp == 2)
    {
        for (i = 0; i < SPLASH_IMAGE_WIDTH; i++)
        {
            memcpy (config->base + ((image_base + (i * (config->width))) * bytes_per_bpp),
		    imageBuffer + (i * SPLASH_IMAGE_HEIGHT * bytes_per_bpp),
		    SPLASH_IMAGE_HEIGHT * bytes_per_bpp);
        }
    }
    fbcon_flush();
#endif
}
