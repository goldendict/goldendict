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

#ifndef EB_DEFS_H
#define EB_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <limits.h>

#ifdef EB_BUILD_LIBRARY
#include "sysdefs.h"
#include "zio.h"
#else
#include <eb/sysdefs.h>
#include <eb/zio.h>
#endif

#ifdef EB_ENABLE_PTHREAD
#include <pthread.h>
#endif

/*
 * Disc code
 */
#define EB_DISC_EB			0
#define EB_DISC_EPWING			1
#define EB_DISC_INVALID			-1

/*
 * Character codes.
 */
#define EB_CHARCODE_ISO8859_1		1
#define EB_CHARCODE_JISX0208		2
#define EB_CHARCODE_JISX0208_GB2312	3
#define EB_CHARCODE_INVALID		-1

/*
 * Special book ID for cache to represent "no cache data for any book".
 */
#define EB_BOOK_NONE			-1

/*
 * Special disc code, subbook code, multi search ID, and multi search
 * entry ID, for representing error state.
 */
#define EB_SUBBOOK_INVALID		-1
#define EB_MULTI_INVALID		-1

/*
 * Size of a page (The term `page' means `block' in JIS X 4081).
 */
#define EB_SIZE_PAGE			2048

/*
 * Maximum length of a word to be searched.
 */
#define EB_MAX_WORD_LENGTH             255

/*
 * Maximum length of an EB* book title.
 */
#define EB_MAX_EB_TITLE_LENGTH		30

/*
 * Maximum length of an EPWING book title.
 */
#define EB_MAX_EPWING_TITLE_LENGTH	80

/*
 * Maximum length of a book title.
 */
#define EB_MAX_TITLE_LENGTH		80

/*
 * Maximum length of a word to be searched.
 */
#if defined(PATH_MAX)
#define EB_MAX_PATH_LENGTH		PATH_MAX
#elif defined(MAXPATHLEN)
#define EB_MAX_PATH_LENGTH		MAXPATHLEN
#else
#define EB_MAX_PATH_LENGTH		1024
#endif

/*
 * Maximum length of a directory name.
 */
#define EB_MAX_DIRECTORY_NAME_LENGTH	8

/*
 * Maximum length of a file name under a certain directory.
 * prefix(8 chars) + '.' + suffix(3 chars) + ';' + digit(1 char)
 */
#define EB_MAX_FILE_NAME_LENGTH		14

/*
 * Maximum length of a label for multi-search entry.
 */
#define EB_MAX_MULTI_LABEL_LENGTH	30

/*
 * Maximum length of alternation text string for a private character.
 */
#define EB_MAX_ALTERNATION_TEXT_LENGTH	31

/*
 * Maximum length of title for multi search.
 */
#define EB_MAX_MULTI_TITLE_LENGTH	32

/*
 * Maximum number of font heights in a subbok.
 */
#define EB_MAX_FONTS			4

/*
 * Maximum number of subbooks in a book.
 */
#define EB_MAX_SUBBOOKS			50

/*
 * Maximum number of multi-search types in a subbook.
 */
#define EB_MAX_MULTI_SEARCHES		10

/*
 * Maximum number of entries in a multi-search.
 */
#define EB_MAX_MULTI_ENTRIES		5

/*
 * Maximum number of entries in a keyword search.
 */
#define EB_MAX_KEYWORDS			EB_MAX_MULTI_ENTRIES

/*
 * Maximum number of entries in a cross search.
 */
#define EB_MAX_CROSS_ENTRIES		EB_MAX_MULTI_ENTRIES

/*
 * Maximum number of characters for alternation cache.
 */
#define EB_MAX_ALTERNATION_CACHE	16

/*
 * The number of text hooks.
 */
#define EB_NUMBER_OF_HOOKS		54

/*
 * The number of search contexts required by a book.
 */
#define EB_NUMBER_OF_SEARCH_CONTEXTS	EB_MAX_MULTI_ENTRIES

/*
 * Types for various codes.
 */
typedef int EB_Error_Code;
typedef int EB_Book_Code;
typedef int EB_Disc_Code;
typedef int EB_Case_Code;
typedef int EB_Suffix_Code;
typedef int EB_Character_Code;
typedef int EB_Font_Code;
typedef int EB_Word_Code;
typedef int EB_Subbook_Code;
typedef int EB_Index_Style_Code;
typedef int EB_Search_Code;
typedef int EB_Text_Code;
typedef int EB_Text_Status_Code;
typedef int EB_Multi_Search_Code;
typedef int EB_Hook_Code;
typedef int EB_Binary_Code;

/*
 * Typedef for Structures.
 */
#ifdef EB_ENABLE_PTHREAD
typedef struct EB_Lock_Struct              EB_Lock;
#endif
typedef struct EB_Position_Struct          EB_Position;
typedef struct EB_Alternation_Cache_Struct EB_Alternation_Cache;
typedef struct EB_Appendix_Subbook_Struct  EB_Appendix_Subbook;
typedef struct EB_Appendix_Struct          EB_Appendix;
typedef struct EB_Font_Struct              EB_Font;
typedef struct EB_Search_Struct            EB_Search;
typedef struct EB_Multi_Search_Struct      EB_Multi_Search;
typedef struct EB_Subbook_Struct           EB_Subbook;
typedef struct EB_Text_Context_Struct      EB_Text_Context;
typedef struct EB_Binary_Context_Struct    EB_Binary_Context;
typedef struct EB_Search_Context_Struct    EB_Search_Context;
typedef struct EB_Book_Struct              EB_Book;
typedef struct EB_Hit_Struct               EB_Hit;
typedef struct EB_Hook_Struct              EB_Hook;
typedef struct EB_Hookset_Struct           EB_Hookset;
typedef struct EB_BookList_Entry           EB_BookList_Entry;
typedef struct EB_BookList                 EB_BookList;

/*
 * Pthreads lock.
 */
#ifdef EB_ENABLE_PTHREAD
struct EB_Lock_Struct {
    /*
     * Lock count.  (For emulating recursive lock).
     */
    int lock_count;

    /*
     * Mutex for `lock_count'.
     */
    pthread_mutex_t lock_count_mutex;

    /*
     * Mutex for struct entity.
     */
    pthread_mutex_t entity_mutex;
};
#endif /* EB_ENABLE_PTHREAD */

/*
 * A pair of page and offset.
 */
struct EB_Position_Struct {
    /*
     * Page. (1, 2, 3 ...)
     */
    int page;

    /*
     * Offset in `page'. (0 ... 2047)
     */
    int offset;
};

/*
 * Chace of aternation text.
 */
struct EB_Alternation_Cache_Struct {
    /*
     * Character number.
     */
    int character_number;

    /*
     * Alternation string for `char_no'.
     */
    char text[EB_MAX_ALTERNATION_TEXT_LENGTH + 1];
};

/*
 * An appendix for a subbook.
 */
struct EB_Appendix_Subbook_Struct {

    /*
     * Initialization flag.
     */
    int initialized;

    /*
     * Subbook ID.
     */
    EB_Subbook_Code code;

    /*
     * Directory name.
     */
    char directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];

    /*
     * Sub-directory name. (EPWING only)
     */
    char data_directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];

    /*
     * File name.
     */
    char file_name[EB_MAX_FILE_NAME_LENGTH + 1];

    /*
     * Character code of the book.
     */
    EB_Character_Code character_code;

    /*
     * Start character number of the narrow/wide font.
     */
    int narrow_start;
    int wide_start;

    /*
     * End character number of the narrow/wide font.
     */
    int narrow_end;
    int wide_end;

    /*
     * Start page number of the narrow/wide font.
     */
    int narrow_page;
    int wide_page;

    /*
     * Stop code (first and second characters).
     */
    int stop_code0;
    int stop_code1;

    /*
     * Compression Information for appendix file.
     */
    Zio zio;
};

/*
 * Additional resources for a book.
 */
struct EB_Appendix_Struct {
    /*
     * Book ID.
     */
    EB_Book_Code code;

    /*
     * Path of the book.
     */
    char *path;

    /*
     * The length of the path.
     */
    size_t path_length;

    /*
     * Disc type.  EB (EB/EBG/EBXA/EBXA-C/S-EBXA) or EPWING.
     */
    EB_Disc_Code disc_code;

    /*
     * The number of subbooks the book has.
     */
    int subbook_count;

    /*
     * Subbook list.
     */
    EB_Appendix_Subbook *subbooks;

    /*
     * Current subbook.
     */
    EB_Appendix_Subbook *subbook_current;

    /*
     * ebnet socket file. 
     */
#ifdef EB_ENABLE_EBNET
    int ebnet_file;
#endif

    /*
     * Lock.
     */
#ifdef EB_ENABLE_PTHREAD
    EB_Lock lock;
#endif

    /*
     * Cache table for alternation text.
     */
    EB_Alternation_Cache narrow_cache[EB_MAX_ALTERNATION_CACHE];
    EB_Alternation_Cache wide_cache[EB_MAX_ALTERNATION_CACHE];
};

/*
 * A font in a subbook.
 */
struct EB_Font_Struct {
    /*
     * Font Code.
     * This font is not available, if the code is EB_FONT_INVALID.
     */
    EB_Font_Code font_code;

    /*
     * Whether the object has been initialized.
     */
    int initialized;

    /*
     * Character numbers of the start and end of the font.
     */
    int start;
    int end;

    /*
     * Page number of the start page of the font data.
     * Used in EB* only. (In EPWING, it is alyways 1).
     */
    int page;

    /*
     * File name of the font. (EPWING only)
     */
    char file_name[EB_MAX_FILE_NAME_LENGTH + 1];

    /*
     * Font data cache.
     */
    char *glyphs;

    /*
     * Compression Information.
     */
    Zio zio;
};

/*
 * Search methods in a subbook.
 */
struct EB_Search_Struct {
    /*
     * Index ID.
     */
    int index_id;

    /*
     * Page number of the start page of an index.
     * This search method is not available, if `start_page' is 0,
     */
    int start_page;
    int end_page;

    /*
     * Page number of the start page of candidates.
     * (for multi search entry)
     */
    int candidates_page;

    /*
     * Index style flags.
     */
    EB_Index_Style_Code katakana;
    EB_Index_Style_Code lower;
    EB_Index_Style_Code mark;
    EB_Index_Style_Code long_vowel;
    EB_Index_Style_Code double_consonant;
    EB_Index_Style_Code contracted_sound;
    EB_Index_Style_Code voiced_consonant;
    EB_Index_Style_Code small_vowel;
    EB_Index_Style_Code p_sound;
    EB_Index_Style_Code space;

    /*
     * Label. (for an entry in multi search)
     */
    char label[EB_MAX_MULTI_LABEL_LENGTH + 1];
};

/*
 * A multi-search entry in a subbook.
 */
struct EB_Multi_Search_Struct {
    /*
     * Search method information.
     */
    EB_Search search;

    /*
     * Search title. (EPWING only)
     */
    char title[EB_MAX_MULTI_TITLE_LENGTH + 1];

    /*
     * The number of entries the multi search has.
     */
    int entry_count;

    /*
     * List of Word entry information.
     */
    EB_Search entries[EB_MAX_MULTI_ENTRIES];
};

/*
 * A subbook in a book.
 */
struct EB_Subbook_Struct {
    /*
     * Whether the object has been initialized.
     */
    int initialized;

    /*
     * Index page.
     */
    int index_page;

    /*
     * Subbook ID.
     * This subbook is not available, if the code is EB_SUBBOOK_INVALID.
     */
    EB_Subbook_Code code;

    /*
     * File descriptor and compression information for text file.
     */
    Zio text_zio;

    /*
     * File descriptor and compression information for graphic file.
     */
    Zio graphic_zio;

    /*
     * File descriptor and compression information for sound file.
     */
    Zio sound_zio;

    /*
     * File descriptor and compression information for movie file.
     */
    Zio movie_zio;

    /*
     * Title of the subbook.
     */
    char title[EB_MAX_TITLE_LENGTH + 1];

    /*
     * Subbook directory name.
     */
    char directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];

    /*
     * Sub-directory names. (EPWING only)
     */
    char data_directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    char gaiji_directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];
    char movie_directory_name[EB_MAX_DIRECTORY_NAME_LENGTH + 1];

    /*
     * File names.
     */
    char text_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    char graphic_file_name[EB_MAX_FILE_NAME_LENGTH + 1];
    char sound_file_name[EB_MAX_FILE_NAME_LENGTH + 1];

    /*
     * Compression hints of Text, graphic and sound files.
     * (temporary need, EPWING only).
     */
    Zio_Code text_hint_zio_code;
    Zio_Code graphic_hint_zio_code;
    Zio_Code sound_hint_zio_code;

    /*
     * Page number where search method titles are stored.
     * (temporary need, EPWING only).
     */
    int search_title_page;

    /*
     * The top page of search methods.
     */
    EB_Search word_alphabet;
    EB_Search word_asis;
    EB_Search word_kana;
    EB_Search endword_alphabet;
    EB_Search endword_asis;
    EB_Search endword_kana;
    EB_Search keyword;
    EB_Search menu;
    EB_Search image_menu;
    EB_Search cross;
    EB_Search copyright;
    EB_Search text;
    EB_Search sound;

    /*
     * The number of multi-search methods the subbook has.
     */
    int multi_count;

    /*
     * The top page of multi search methods.
     */
    EB_Multi_Search multis[EB_MAX_MULTI_SEARCHES];

    /*
     * Font list.
     */
    EB_Font narrow_fonts[EB_MAX_FONTS];
    EB_Font wide_fonts[EB_MAX_FONTS];

    /*
     * Current narrow and wide fonts.
     */
    EB_Font *narrow_current;
    EB_Font *wide_current;
};

/*
 * Length of cache buffer in a binary context.
 * It must be greater than 38, size of GIF preamble.
 * It must be greater than 44, size of WAVE sound header.
 * It must be greater than 118, size of BMP header + info + 16 rgbquads.
 */
#define EB_SIZE_BINARY_CACHE_BUFFER	128

/*
 * Context parameters for binary data.
 */
struct EB_Binary_Context_Struct {
    /*
     * Binary type ID.
     * The context is not active, if this code is EB_BINARY_INVALID.
     */
    EB_Binary_Code code;

    /*
     * Compress information.
     */
    Zio *zio;

    /*
     * Location of the the binary data, relative to the start of the file.
     */
    off_t location;

    /*
     * Data size.
     * Size zero means that the binary has no size information.
     */
    size_t size;

    /*
     * The current offset of binary data.
     */
    size_t offset;

    /*
     * Cache buffer.
     */
    char cache_buffer[EB_SIZE_BINARY_CACHE_BUFFER];

    /*
     * Length of cached data.
     */
    size_t cache_length;

    /*
     * Current offset of cached data.
     */
    size_t cache_offset;

    /*
     * Width of Image. (monochrome graphic only)
     */
    int width;
};

/*
 * Context parameters for text reading.
 */
struct EB_Text_Context_Struct {
    /*
     * Current text content type.
     * The context is not active, if this code is EB_TEXT_INVALID.
     */
    EB_Text_Code code;

    /*
     * Current offset pointer of the START or HONMON file.
     */
    off_t location;

    /*
     * The current point of a buffer on which text is written.
     */
    char *out;

    /*
     * Length of `out'.
     */
    size_t out_rest_length;
    
    /*
     * Unprocessed string that a hook function writes on text.
     */
    char *unprocessed;

    /*
     * Size of `unprocessed'.
     */
    size_t unprocessed_size;

    /*
     * Length of the current output text phrase.
     */
    size_t out_step;

    /*
     * Narrow character region flag.
     */
    int narrow_flag;

    /*
     * Whether a printable character has been appeared in the current
     * text content.
     */
    int printable_count;

    /* 
     * EOF flag of the current subbook.
     */
    int file_end_flag;

    /*
     * Status of the current text processing.
     */
    EB_Text_Status_Code text_status;

    /*
     * Skip until `skipcode' appears.
     */
    int skip_code;

    /*
     * Stop-code automatically set by EB Library.
     */
    int auto_stop_code;

    /*
     * The current candidate word for multi search.
     */
    char candidate[EB_MAX_WORD_LENGTH + 1];

    /*
     * Whether the current text point is in the candidate word or not.
     */
    int is_candidate;

    /*
     * Whether the current text point is in EBXA-C gaiji area.
     */
    int ebxac_gaiji_flag;
};

/*
 * Context parameters for word search.
 */
struct EB_Search_Context_Struct {
    /*
     * Current search method type.
     * The context is not active, if this code is EB_SEARCH_NONE.
     */
    EB_Search_Code code;

    /*
     * Function which compares word to search and pattern in an index page.
     */
    int (*compare_pre)(const char *word, const char *pattern,
	size_t length);
    int (*compare_single)(const char *word, const char *pattern,
	size_t length);
    int (*compare_group)(const char *word, const char *pattern,
	size_t length);

    /*
     * Result of comparison by `compare'.
     */
    int comparison_result;

    /*
     * Word to search.
     */
    char word[EB_MAX_WORD_LENGTH + 1];

    /*
     * Canonicalized word to search.
     */
    char canonicalized_word[EB_MAX_WORD_LENGTH + 1];

    /*
     * Page which is searched currently.
     */
    int page;

    /*
     * Offset which is searched currently in the page.
     */
    int offset;

    /*
     * Page ID of the current page.
     */
    int page_id;

    /*
     * How many entries in the current page.
     */
    int entry_count;

    /*
     * Entry index pointer.
     */
    int entry_index;

    /*
     * Length of the current entry.
     */
    int entry_length;

    /*
     * Arrangement style of entries in the current page (fixed or variable).
     */
    int entry_arrangement;

    /*
     * In a group entry or not.
     */
    int in_group_entry;

    /*
     * Current heading position (for keyword search).
     */
    EB_Position keyword_heading;
};

/*
 * A book.
 */
struct EB_Book_Struct {
    /*
     * Book ID.
     */
    EB_Book_Code code;

    /*
     * Disc type.  EB* or EPWING.
     */
    EB_Disc_Code disc_code;

    /*
     * Character code of the book.
     */
    EB_Character_Code character_code;

    /*
     * Path of the book.
     */
    char *path;

    /*
     * The length of the path.
     */
    size_t path_length;

    /*
     * The number of subbooks the book has.
     */
    int subbook_count;

    /*
     * Subbook list.
     */
    EB_Subbook *subbooks;

    /*
     * Current subbook.
     */
    EB_Subbook *subbook_current;

    /*
     * Context parameters for text reading.
     */
    EB_Text_Context text_context;

    /*
     * Context parameters for binary reading.
     */
    EB_Binary_Context binary_context;

    /*
     * Context parameters for text reading.
     */
    EB_Search_Context search_contexts[EB_NUMBER_OF_SEARCH_CONTEXTS];

    /*
     * ebnet socket file. 
     */
#ifdef EB_ENABLE_EBNET
    int ebnet_file;
#endif

    /*
     * Lock.
     */
#ifdef EB_ENABLE_PTHREAD
    EB_Lock lock;
#endif
};

/*
 * In a word search, heading and text locations of a matched entry
 * are stored.
 */
struct EB_Hit_Struct {
    /*
     * Heading position.
     */
    EB_Position heading;

    /*
     * Text position.
     */
    EB_Position text;
};

/*
 * A text hook.
 */
struct EB_Hook_Struct {
    /*
     * Hook code.
     */
    EB_Hook_Code code;

    /*
     * Hook function for the hook code `code'.
     */
    EB_Error_Code (*function)(EB_Book *book, EB_Appendix *appendix,
	void *container, EB_Hook_Code hook_code, int argc,
	const unsigned int *argv);
};

/*
 * A set of text hooks.
 */
struct EB_Hookset_Struct {
    /*
     * List of hooks.
     */
    EB_Hook hooks[EB_NUMBER_OF_HOOKS];

    /*
     * Lock.
     */
#ifdef EB_ENABLE_PTHREAD
    EB_Lock lock;
#endif
};

/*
 * An entry of book list.
 */
struct EB_BookList_Entry {
    /*
     * name.
     */
    char *name;

    /*
     * Title.
     */
    char *title;
};

/*
 * Book list.
 */
struct EB_BookList {
    /*
     * Book List ID.
     */
    EB_Book_Code code;

    /*
     * The number of book entries this list has.
     */
    int entry_count;

    /*
     * The maximum number of book entries that `entries' can memory.
     */
    int max_entry_count;

    /*
     * Book entries.
     */
    EB_BookList_Entry *entries;

    /*
     * Lock.
     */
#ifdef EB_ENABLE_PTHREAD
    EB_Lock lock;
#endif
};

/* for backward compatibility */
#define EB_Multi_Entry_Code int

#ifdef __cplusplus
}
#endif

#endif /* not EB_DEFS_H */
