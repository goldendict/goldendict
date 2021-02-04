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

#ifndef EB_APPENDIX_H
#define EB_APPENDIX_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EB_BUILD_LIBRARY
#include "eb.h"
#else
#include <ebu/eb.h>
#endif

/*
 * Function declarations.
 */
/* appendix.c */
void eb_initialize_appendix(EB_Appendix *appendix);
void eb_finalize_appendix(EB_Appendix *appendix);
EB_Error_Code eb_bind_appendix(EB_Appendix *appendix, const char *path);
int eb_is_appendix_bound(EB_Appendix *appendix);
EB_Error_Code eb_appendix_path(EB_Appendix *appendix, char *path);

/* appsub.c */
EB_Error_Code eb_load_all_appendix_subbooks(EB_Appendix *appendix);
EB_Error_Code eb_appendix_subbook_list(EB_Appendix *appendix,
    EB_Subbook_Code *subbook_list, int *subbook_count);
EB_Error_Code eb_appendix_subbook(EB_Appendix *appendix,
    EB_Subbook_Code *subbook_code);
EB_Error_Code eb_appendix_subbook_directory(EB_Appendix *appendix,
    char *directory);
EB_Error_Code eb_appendix_subbook_directory2(EB_Appendix *appendix,
    EB_Subbook_Code subbook_code, char *directory);
EB_Error_Code eb_set_appendix_subbook(EB_Appendix *appendix,
    EB_Subbook_Code subbook_code);
void eb_unset_appendix_subbook(EB_Appendix *appendix);

/* narwalt.c */
int eb_have_narrow_alt(EB_Appendix *appendix);
EB_Error_Code eb_narrow_alt_start(EB_Appendix *appendix, int *start);
EB_Error_Code eb_narrow_alt_end(EB_Appendix *appendix, int *end);
EB_Error_Code eb_narrow_alt_character_text(EB_Appendix *appendix,
    int character_number, char *text);
EB_Error_Code eb_forward_narrow_alt_character(EB_Appendix *appendix,
    int n, int *character_number);
EB_Error_Code eb_backward_narrow_alt_character(EB_Appendix *appendix,
    int n, int *character_number);

/* stopcode.c */
int eb_have_stop_code(EB_Appendix *appendix);
EB_Error_Code eb_stop_code(EB_Appendix *appendix, int *);

/* widealt.c */
int eb_have_wide_alt(EB_Appendix *appendix);
EB_Error_Code eb_wide_alt_start(EB_Appendix *appendix, int *start);
EB_Error_Code eb_wide_alt_end(EB_Appendix *appendix, int *end);
EB_Error_Code eb_wide_alt_character_text(EB_Appendix *appendix,
    int character_number, char *text);
EB_Error_Code eb_forward_wide_alt_character(EB_Appendix *appendix, int n,
    int *character_number);
EB_Error_Code eb_backward_wide_alt_character(EB_Appendix *appendix, int n,
    int *character_number);

/* for backward compatibility */
#define eb_suspend_appendix eb_unset_appendix_subbook
#define eb_initialize_all_appendix_subbooks eb_load_all_appendix_subbooks

#ifdef __cplusplus
}
#endif

#endif /* not EB_APPENDIX_H */
