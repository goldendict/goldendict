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

#ifndef EB_EB_H
#define EB_EB_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef EB_BUILD_LIBRARY
#include "defs.h"
#else
#include <eb/defs.h>
#endif

#include <stdarg.h>

/*
 * Function declarations.
 */
/* book.c */
void eb_initialize_book(EB_Book *book);
EB_Error_Code eb_bind(EB_Book *book, const char *path);
void eb_finalize_book(EB_Book *book);
int eb_is_bound(EB_Book *book);
EB_Error_Code eb_path(EB_Book *book, char *path);
EB_Error_Code eb_disc_type(EB_Book *book, EB_Disc_Code *disc_code);
EB_Error_Code eb_character_code(EB_Book *book,
    EB_Character_Code *character_code);

/* copyright.h */
int eb_have_copyright(EB_Book *book);
EB_Error_Code eb_copyright(EB_Book *book, EB_Position *position);
EB_Error_Code eb_search_cross(EB_Book *book,
    const char * const input_words[]);

/* cross.c */
int eb_have_cross_search(EB_Book *book);

/* eb.c */
EB_Error_Code eb_initialize_library(void);
void eb_finalize_library(void);

/* endword.c */
int eb_have_endword_search(EB_Book *book);
EB_Error_Code eb_search_endword(EB_Book *book, const char *input_word);

/* exactword.c */
int eb_have_exactword_search(EB_Book *book);
EB_Error_Code eb_search_exactword(EB_Book *book, const char *input_word);

/* graphic.c */
int eb_have_graphic_search(EB_Book *book);

/* keyword.c */
int eb_have_keyword_search(EB_Book *book);
EB_Error_Code eb_search_keyword(EB_Book *book,
    const char * const input_words[]);

/* lock.c */
int eb_pthread_enabled(void);

/* log.c */
void eb_set_log_function(void (*function)(const char *message, va_list ap));
void eb_enable_log(void);
void eb_disable_log(void);
void eb_log(const char *message, ...);
void eb_log_stderr(const char *message, va_list ap);

/* menu.c */
int eb_have_menu(EB_Book *book);
EB_Error_Code eb_menu(EB_Book *book, EB_Position *position);
int eb_have_image_menu(EB_Book *book);
EB_Error_Code eb_image_menu(EB_Book *book, EB_Position *position);

/* multi.c */
int eb_have_multi_search(EB_Book *book);
EB_Error_Code eb_multi_title(EB_Book *book, EB_Multi_Search_Code multi_id,
    char *title);
EB_Error_Code eb_multi_search_list(EB_Book *book,
    EB_Multi_Search_Code *search_list, int *search_count);
EB_Error_Code eb_multi_entry_count(EB_Book *book,
    EB_Multi_Search_Code multi_id, int *entry_count);
EB_Error_Code eb_multi_entry_list(EB_Book *book,
    EB_Multi_Search_Code multi_id, int *entry_list, int *entry_count);
EB_Error_Code eb_multi_entry_label(EB_Book *book,
    EB_Multi_Search_Code multi_id, int entry_index, char *label);
int eb_multi_entry_have_candidates(EB_Book *book,
    EB_Multi_Search_Code multi_id, int entry_index);
EB_Error_Code eb_multi_entry_candidates(EB_Book *book,
    EB_Multi_Search_Code multi_id, int entry_index, EB_Position *position);
EB_Error_Code eb_search_multi(EB_Book *book, EB_Multi_Search_Code multi_id,
    const char * const input_words[]);

/* text.c */
int eb_have_text(EB_Book *book);
EB_Error_Code eb_text(EB_Book *book, EB_Position *position);

/* search.c */
EB_Error_Code eb_hit_list(EB_Book *book, int max_hit_count, EB_Hit *hit_list,
    int *hit_count);

/* subbook.c */
EB_Error_Code eb_load_all_subbooks(EB_Book *book);
EB_Error_Code eb_subbook_list(EB_Book *book, EB_Subbook_Code *subbook_list,
    int *subbook_count);
EB_Error_Code eb_subbook(EB_Book *book, EB_Subbook_Code *subbook_code);
EB_Error_Code eb_subbook_title(EB_Book *book, char *title);
EB_Error_Code eb_subbook_title2(EB_Book *book, EB_Subbook_Code subbook_code,
    char *title);
EB_Error_Code eb_subbook_directory(EB_Book *book, char *directory);
EB_Error_Code eb_subbook_directory2(EB_Book *book,
    EB_Subbook_Code subbook_code, char *directory);
EB_Error_Code eb_set_subbook(EB_Book *book, EB_Subbook_Code subbook_code);
void eb_unset_subbook(EB_Book *book);

/* word.c */
int eb_have_word_search(EB_Book *book);
EB_Error_Code eb_search_word(EB_Book *book, const char *input_word);

/* for backward compatibility */
#define eb_suspend eb_unset_subbook
#define eb_initialize_all_subbooks eb_load_all_subbooks

#ifdef __cplusplus
}
#endif

#endif /* not EB_EB_H */
