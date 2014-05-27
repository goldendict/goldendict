/*                                                            -*- C -*-
 * Copyright (c) 1997-2006  Motoyuki Kasahara
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef EB_FONT_H
#define EB_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#ifdef EB_BUILD_LIBRARY
#include "defs.h"
#else
#include <eb/defs.h>
#endif

/*
 * Font types.
 */
#define EB_FONT_16		0
#define EB_FONT_24		1
#define EB_FONT_30		2
#define EB_FONT_48		3
#define EB_FONT_INVALID		-1

/*
 * Font sizes.
 */
#define EB_SIZE_NARROW_FONT_16		16
#define EB_SIZE_WIDE_FONT_16		32
#define EB_SIZE_NARROW_FONT_24		48
#define EB_SIZE_WIDE_FONT_24		72
#define EB_SIZE_NARROW_FONT_30		60
#define EB_SIZE_WIDE_FONT_30		120
#define EB_SIZE_NARROW_FONT_48		144
#define EB_SIZE_WIDE_FONT_48		288

/*
 * Font width.
 */
#define EB_WIDTH_NARROW_FONT_16		8
#define EB_WIDTH_WIDE_FONT_16		16
#define EB_WIDTH_NARROW_FONT_24		16
#define EB_WIDTH_WIDE_FONT_24		24
#define EB_WIDTH_NARROW_FONT_30		16
#define EB_WIDTH_WIDE_FONT_30		32
#define EB_WIDTH_NARROW_FONT_48		24
#define EB_WIDTH_WIDE_FONT_48		48

/*
 * Font height.
 */
#define EB_HEIGHT_FONT_16		16
#define EB_HEIGHT_FONT_24		24
#define EB_HEIGHT_FONT_30		30
#define EB_HEIGHT_FONT_48		48

/*
 * Bitmap image sizes.
 */
#define EB_SIZE_NARROW_FONT_16_XBM		184
#define EB_SIZE_WIDE_FONT_16_XBM		284
#define EB_SIZE_NARROW_FONT_16_XPM		266
#define EB_SIZE_WIDE_FONT_16_XPM		395
#define EB_SIZE_NARROW_FONT_16_GIF		186
#define EB_SIZE_WIDE_FONT_16_GIF		314
#define EB_SIZE_NARROW_FONT_16_BMP		126
#define EB_SIZE_WIDE_FONT_16_BMP		126
#define EB_SIZE_NARROW_FONT_16_PNG		131
#define EB_SIZE_WIDE_FONT_16_PNG		147

#define EB_SIZE_NARROW_FONT_24_XBM		383
#define EB_SIZE_WIDE_FONT_24_XBM		533
#define EB_SIZE_NARROW_FONT_24_XPM		555
#define EB_SIZE_WIDE_FONT_24_XPM		747
#define EB_SIZE_NARROW_FONT_24_GIF		450
#define EB_SIZE_WIDE_FONT_24_GIF		642
#define EB_SIZE_NARROW_FONT_24_BMP		158
#define EB_SIZE_WIDE_FONT_24_BMP		158
#define EB_SIZE_NARROW_FONT_24_PNG		171
#define EB_SIZE_WIDE_FONT_24_PNG		195

#define EB_SIZE_NARROW_FONT_30_XBM		458
#define EB_SIZE_WIDE_FONT_30_XBM		833
#define EB_SIZE_NARROW_FONT_30_XPM		675
#define EB_SIZE_WIDE_FONT_30_XPM		1155
#define EB_SIZE_NARROW_FONT_30_GIF		552
#define EB_SIZE_WIDE_FONT_30_GIF		1032
#define EB_SIZE_NARROW_FONT_30_BMP		182
#define EB_SIZE_WIDE_FONT_30_BMP		182
#define EB_SIZE_NARROW_FONT_30_PNG		189
#define EB_SIZE_WIDE_FONT_30_PNG		249

#define EB_SIZE_NARROW_FONT_48_XBM		983
#define EB_SIZE_WIDE_FONT_48_XBM		1883
#define EB_SIZE_NARROW_FONT_48_XPM		1419
#define EB_SIZE_WIDE_FONT_48_XPM		2571
#define EB_SIZE_NARROW_FONT_48_GIF		1242
#define EB_SIZE_WIDE_FONT_48_GIF		2394
#define EB_SIZE_NARROW_FONT_48_BMP		254
#define EB_SIZE_WIDE_FONT_48_BMP		446
#define EB_SIZE_NARROW_FONT_48_PNG		291
#define EB_SIZE_WIDE_FONT_48_PNG		435

#define EB_SIZE_FONT_IMAGE	EB_SIZE_WIDE_FONT_48_XPM	    

/*
 * Function declarations.
 */
/* bitmap.c */
EB_Error_Code eb_narrow_font_xbm_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_narrow_font_xpm_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_narrow_font_gif_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_narrow_font_bmp_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_narrow_font_png_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_wide_font_xbm_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_wide_font_xpm_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_wide_font_gif_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_wide_font_bmp_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_wide_font_png_size(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_bitmap_to_xbm(const char *bitmap, int width, int height,
    char *xbm, size_t *xbm_length);
EB_Error_Code eb_bitmap_to_xpm(const char *bitmap, int width, int height,
    char *xpm, size_t *xpm_length);
EB_Error_Code eb_bitmap_to_gif(const char *bitmap, int width, int height,
    char *gif, size_t *gif_length);
EB_Error_Code eb_bitmap_to_bmp(const char *bitmap, int width, int height,
    char *bmp, size_t *bmp_length);
EB_Error_Code eb_bitmap_to_png(const char *bitmap, int width, int height,
    char *png, size_t *png_length);

/* font.c */
EB_Error_Code eb_font(EB_Book *book, EB_Font_Code *font_code);
EB_Error_Code eb_set_font(EB_Book *book, EB_Font_Code font_code);
void eb_unset_font(EB_Book *book);
EB_Error_Code eb_font_list(EB_Book *book, EB_Font_Code *font_list,
    int *font_count);
int eb_have_font(EB_Book *book, EB_Font_Code font_code);
EB_Error_Code eb_font_height(EB_Book *book, int *height);
EB_Error_Code eb_font_height2(EB_Font_Code font_code, int *height);

/* narwfont.c */
int eb_have_narrow_font(EB_Book *book);
EB_Error_Code eb_narrow_font_width(EB_Book *book, int *width);
EB_Error_Code eb_narrow_font_width2(EB_Font_Code font_code, int *width);
EB_Error_Code eb_narrow_font_size(EB_Book *book, size_t *size);
EB_Error_Code eb_narrow_font_size2(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_narrow_font_start(EB_Book *book, int *start);
EB_Error_Code eb_narrow_font_end(EB_Book *book, int *end);
EB_Error_Code eb_narrow_font_character_bitmap(EB_Book *book, int, char *);
EB_Error_Code eb_forward_narrow_font_character(EB_Book *book, int, int *);
EB_Error_Code eb_backward_narrow_font_character(EB_Book *book, int, int *);

/* widefont.c */
int eb_have_wide_font(EB_Book *book);
EB_Error_Code eb_wide_font_width(EB_Book *book, int *width);
EB_Error_Code eb_wide_font_width2(EB_Font_Code font_code, int *width);
EB_Error_Code eb_wide_font_size(EB_Book *book, size_t *size);
EB_Error_Code eb_wide_font_size2(EB_Font_Code font_code, size_t *size);
EB_Error_Code eb_wide_font_start(EB_Book *book, int *start);
EB_Error_Code eb_wide_font_end(EB_Book *book, int *end);
EB_Error_Code eb_wide_font_character_bitmap(EB_Book *book,
    int character_number, char *bitmap);
EB_Error_Code eb_forward_wide_font_character(EB_Book *book, int n,
    int *character_number);
EB_Error_Code eb_backward_wide_font_character(EB_Book *book, int n,
    int *character_number);

#ifdef __cplusplus
}
#endif

#endif /* not EB_FONT_H */
