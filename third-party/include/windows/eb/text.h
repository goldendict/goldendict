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

#ifndef EB_TEXT_H
#define EB_TEXT_H

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
 * Hook codes.
 * (When you add or remove a hook, update EB_NUMER_OF_HOOKS in defs.h.)
 */
#define EB_HOOK_NULL			-1
#define EB_HOOK_INITIALIZE		0
#define EB_HOOK_BEGIN_NARROW		1
#define EB_HOOK_END_NARROW		2
#define EB_HOOK_BEGIN_SUBSCRIPT		3
#define EB_HOOK_END_SUBSCRIPT		4

#define EB_HOOK_SET_INDENT		5
#define EB_HOOK_NEWLINE			6
#define EB_HOOK_BEGIN_SUPERSCRIPT	7
#define EB_HOOK_END_SUPERSCRIPT		8
#define EB_HOOK_BEGIN_NO_NEWLINE	9

#define EB_HOOK_END_NO_NEWLINE		10
#define EB_HOOK_BEGIN_EMPHASIS		11
#define EB_HOOK_END_EMPHASIS		12
#define EB_HOOK_BEGIN_CANDIDATE		13
#define EB_HOOK_END_CANDIDATE_GROUP	14

#define EB_HOOK_END_CANDIDATE_LEAF	15
#define EB_HOOK_BEGIN_REFERENCE		16
#define EB_HOOK_END_REFERENCE		17
#define EB_HOOK_BEGIN_KEYWORD		18
#define EB_HOOK_END_KEYWORD		19

#define EB_HOOK_NARROW_FONT		20
#define EB_HOOK_WIDE_FONT		21
#define EB_HOOK_ISO8859_1		22
#define EB_HOOK_NARROW_JISX0208		23
#define EB_HOOK_WIDE_JISX0208		24

#define EB_HOOK_GB2312			25
#define EB_HOOK_BEGIN_MONO_GRAPHIC	26
#define EB_HOOK_END_MONO_GRAPHIC	27
#define EB_HOOK_BEGIN_GRAY_GRAPHIC	28
#define EB_HOOK_END_GRAY_GRAPHIC	29

#define EB_HOOK_BEGIN_COLOR_BMP		30
#define EB_HOOK_BEGIN_COLOR_JPEG	31
#define EB_HOOK_BEGIN_IN_COLOR_BMP	32
#define EB_HOOK_BEGIN_IN_COLOR_JPEG	33
#define EB_HOOK_END_COLOR_GRAPHIC	34

#define EB_HOOK_END_IN_COLOR_GRAPHIC	35
#define EB_HOOK_BEGIN_WAVE		36
#define EB_HOOK_END_WAVE		37
#define EB_HOOK_BEGIN_MPEG		38
#define EB_HOOK_END_MPEG		39

#define EB_HOOK_BEGIN_GRAPHIC_REFERENCE	40
#define EB_HOOK_END_GRAPHIC_REFERENCE	41
#define EB_HOOK_GRAPHIC_REFERENCE	42
#define EB_HOOK_BEGIN_DECORATION	43
#define EB_HOOK_END_DECORATION		44

#define EB_HOOK_BEGIN_IMAGE_PAGE        45
#define EB_HOOK_END_IMAGE_PAGE          46
#define EB_HOOK_BEGIN_CLICKABLE_AREA    47
#define EB_HOOK_END_CLICKABLE_AREA      48

#define EB_HOOK_BEGIN_UNICODE		49
#define EB_HOOK_END_UNICODE		50
#define EB_HOOK_BEGIN_EBXAC_GAIJI	51
#define EB_HOOK_END_EBXAC_GAIJI		52
#define EB_HOOK_EBXAC_GAIJI		53

/*
 * Function declarations.
 */
/* hook.c */
void eb_initialize_hookset(EB_Hookset *hookset);
void eb_finalize_hookset(EB_Hookset *hookset);
EB_Error_Code eb_set_hook(EB_Hookset *hookset, const EB_Hook *hook);
EB_Error_Code eb_set_hooks(EB_Hookset *hookset, const EB_Hook *hook);
EB_Error_Code eb_hook_euc_to_ascii(EB_Book *book, EB_Appendix *appendix,
    void *container, EB_Hook_Code hook_code, int argc,
    const unsigned int *argv);
EB_Error_Code eb_hook_stop_code(EB_Book *book, EB_Appendix *appendix,
    void *container, EB_Hook_Code hook_code, int argc,
    const unsigned int *argv);
EB_Error_Code eb_hook_narrow_character_text(EB_Book *book,
    EB_Appendix *appendix, void *container, EB_Hook_Code hook_code, int argc,
    const unsigned int *argv);
EB_Error_Code eb_hook_wide_character_text(EB_Book *book,
    EB_Appendix *appendix, void *container, EB_Hook_Code hook_code, int argc,
    const unsigned int *argv);
EB_Error_Code eb_hook_newline(EB_Book *book, EB_Appendix *appendix,
    void *container, EB_Hook_Code hook_code, int argc,
    const unsigned int *argv);
EB_Error_Code eb_hook_empty(EB_Book *book, EB_Appendix *appendix,
    void *container, EB_Hook_Code hook_code, int argc,
    const unsigned int *argv);

/* readtext.c */
EB_Error_Code eb_seek_text(EB_Book *book, const EB_Position *position);
EB_Error_Code eb_tell_text(EB_Book *book, EB_Position *position);
EB_Error_Code eb_read_text(EB_Book *book, EB_Appendix *appendix,
    EB_Hookset *hookset, void *container, size_t text_max_length, char *text,
    ssize_t *text_length);
EB_Error_Code eb_read_heading(EB_Book *book, EB_Appendix *appendix,
    EB_Hookset *hookset, void *container, size_t text_max_length, char *text,
    ssize_t *text_length);
EB_Error_Code eb_read_rawtext(EB_Book *book, size_t text_max_length,
    char *text, ssize_t *text_length);
int eb_is_text_stopped(EB_Book *book);
EB_Error_Code eb_write_text_byte1(EB_Book *book, int byte1);
EB_Error_Code eb_write_text_byte2(EB_Book *book, int byte1, int byte2);
EB_Error_Code eb_write_text_string(EB_Book *book, const char *string);
EB_Error_Code eb_write_text(EB_Book *book, const char * stream,
    size_t stream_length);
const char *eb_current_candidate(EB_Book *book);
EB_Error_Code eb_forward_text(EB_Book *book, EB_Appendix *appendix);
EB_Error_Code eb_backward_text(EB_Book *book, EB_Appendix *appendix);

#ifdef __cplusplus
}
#endif

#endif /* not EB_TEXT_H */
