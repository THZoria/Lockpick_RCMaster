/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2018-2021 CTCaer
 * Copyright (c) 2019-2021 shchmue
 * Copyright (c) 2018 M4xw
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GFX_H_
#define _GFX_H_

#include <utils/types.h>

#define EPRINTF(text) gfx_printf("%k"text"%k\n", 0xFFFF0000, 0xFFCCCCCC)
#define EPRINTFARGS(text, args...) gfx_printf("%k"text"%k\n", 0xFFFF0000, args, 0xFFCCCCCC)
#define WPRINTF(text) gfx_printf("%k"text"%k\n", 0xFFFFDD00, 0xFFCCCCCC)
#define WPRINTFARGS(text, args...) gfx_printf("%k"text"%k\n", 0xFFFFDD00, args, 0xFFCCCCCC)

typedef struct _gfx_ctxt_t
{
	u32 *fb;
	u32 width;
	u32 height;
	u32 stride;
} gfx_ctxt_t;

typedef struct _gfx_con_t
{
	gfx_ctxt_t *gfx_ctxt;
	u32 fntsz;
	u32 x;
	u32 y;
	u32 savedx;
	u32 savedy;
	u32 fgcol;
	int fillbg;
	u32 bgcol;
	bool mute;
} gfx_con_t;

// Global gfx console and context.
extern gfx_ctxt_t gfx_ctxt;
extern gfx_con_t gfx_con;

/* Legacy color definitions - Kept for backward compatibility */
#define COLOR_RED    0xFFFF0000  // Now matches Hekate TXT_CLR_ERROR
#define COLOR_YELLOW 0xFFFFDD00  // Now matches Hekate TXT_CLR_WARNING
#define COLOR_GREEN  0xFF40FF00
#define COLOR_BLUE   0xFF00DDFF
#define COLOR_VIOLET 0xFF8040FF
#define COLOR_DEFAULT 0xFF1B1B1B

/* Hekate-style color palette - Authentic Hekate colors from bootloader/gfx/gfx.h */
#define COLOR_WHITE      0xFFFFFFFF  // Pure white
#define COLOR_SOFT_WHITE 0xFFCCCCCC  // Hekate default text (TXT_CLR_DEFAULT)
#define COLOR_CYAN       0xFF00CCFF  // Hekate light cyan (TXT_CLR_CYAN_L)
#define COLOR_CYAN_L     0xFF00CCFF  // Hekate light cyan (TXT_CLR_CYAN_L)
#define COLOR_TURQUOISE  0xFF00FFCC  // Hekate turquoise (TXT_CLR_TURQUOISE)
#define COLOR_ORANGE     0xFFFFBA00  // Hekate orange (TXT_CLR_ORANGE)
#define COLOR_GREENISH   0xFF96FF00  // Hekate toxic green (TXT_CLR_GREENISH)
#define COLOR_WARNING    0xFFFFDD00  // Hekate warning yellow (TXT_CLR_WARNING)
#define COLOR_ERROR      0xFFFF0000  // Hekate error red (TXT_CLR_ERROR)
#define COLOR_GREEN_D    0xFF008800  // Hekate dark green (TXT_CLR_GREEN_D)
#define COLOR_RED_D      0xFF880000  // Hekate dark red (TXT_CLR_RED_D)
#define COLOR_GREY       0xFF888888  // Hekate grey (TXT_CLR_GREY)
#define COLOR_GREY_M     0xFF555555  // Hekate medium grey (TXT_CLR_GREY_M)
#define COLOR_GREY_DM    0xFF444444  // Hekate darker grey (TXT_CLR_GREY_DM)
#define COLOR_GREY_D     0xFF303030  // Hekate darkest grey (TXT_CLR_GREY_D)

static const u32 colors[6] = {COLOR_CYAN_L, COLOR_TURQUOISE, COLOR_GREENISH, COLOR_SOFT_WHITE, COLOR_ORANGE, COLOR_WHITE};

void gfx_init_ctxt(u32 *fb, u32 width, u32 height, u32 stride);
void gfx_clear_grey(u8 color);
void gfx_clear_partial_grey(u8 color, u32 pos_x, u32 height);
void gfx_clear_color(u32 color);
void gfx_con_init();
void gfx_con_setcol(u32 fgcol, int fillbg, u32 bgcol);
void gfx_con_getpos(u32 *x, u32 *y);
void gfx_con_setpos(u32 x, u32 y);
void gfx_putc(char c);
void gfx_puts(const char *s);
void gfx_printf(const char *fmt, ...);
void gfx_hexdump(u32 base, const void *buf, u32 len);
void gfx_hexdiff(u32 base, const void *buf1, const void *buf2, u32 len);

void gfx_set_pixel(u32 x, u32 y, u32 color);
void gfx_line(int x0, int y0, int x1, int y1, u32 color);
void gfx_put_small_sep();
void gfx_put_big_sep();
void gfx_set_rect_grey(const u8 *buf, u32 size_x, u32 size_y, u32 pos_x, u32 pos_y);
void gfx_set_rect_rgb(const u8 *buf, u32 size_x, u32 size_y, u32 pos_x, u32 pos_y);
void gfx_set_rect_argb(const u32 *buf, u32 size_x, u32 size_y, u32 pos_x, u32 pos_y);
void gfx_render_bmp_argb(const u32 *buf, u32 size_x, u32 size_y, u32 pos_x, u32 pos_y);

#endif
