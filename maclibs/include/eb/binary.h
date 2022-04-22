/*                                                            -*- C -*-
 * Copyright (c) 2001-2006  Motoyuki Kasahara
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

#ifndef EB_BINARY_H
#define EB_BINARY_H

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
 * Function declarations.
 */
/* binary.c */
EB_Error_Code eb_set_binary_mono_graphic(EB_Book *book,
    const EB_Position *position, int width, int height);
EB_Error_Code eb_set_binary_gray_graphic(EB_Book *book,
    const EB_Position *position, int width, int height);
EB_Error_Code eb_set_binary_wave(EB_Book *book,
    const EB_Position *start_position, const EB_Position *end_position);
EB_Error_Code eb_set_binary_color_graphic(EB_Book *book,
    const EB_Position *position);
EB_Error_Code eb_set_binary_mpeg(EB_Book *book, const unsigned int *argv);
EB_Error_Code eb_read_binary(EB_Book *book, size_t binary_max_length,
    char *binary, ssize_t *binary_length);
void eb_unset_binary(EB_Book *book);

/* filename.c */
EB_Error_Code eb_compose_movie_file_name(const unsigned int *argv,
    char *composed_file_name);
EB_Error_Code eb_compose_movie_path_name(EB_Book *book,
    const unsigned int *argv, char *composed_path_name);
EB_Error_Code eb_decompose_movie_file_name(unsigned int *argv,
    const char *composed_file_name);

#ifdef __cplusplus
}
#endif

#endif /* not EB_BINARY_H */
