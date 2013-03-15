/* Made up from data.h and other supplementary files of dictd-1.0.11 for the
 * GoldenDict program.
 */

/* data.h -- 
 * Created: Sat Mar 15 18:04:25 2003 by Aleksey Cheusov <vle@gmx.net>
 * Copyright 1994-2003 Rickard E. Faith (faith@dict.org)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 1, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Suite 500, Boston, MA 02110, USA.
 */

#ifndef _DICTZIP_H_
#define _DICTZIP_H_

#include <stdio.h>
#include <zlib.h>

#ifdef __WIN32
#include <windows.h>
#endif

#ifdef __cplusplus
extern "C" 
{
#endif


/* Excerpts from defs.h */

#define DICT_CACHE_SIZE 5

typedef struct dictCache {
   int           chunk;
   char          *inBuffer;
   int           stamp;
   int           count;
} dictCache;

typedef struct dictData {
#ifdef __WIN32
   HANDLE        fd;		/* file handle */
#else
   FILE *        fd;		/* file descriptor */
#endif

   unsigned long size;		/* size of file */
   
   int           type;
   const char    *filename;
   z_stream      zStream;
   int           initialized;

   int           headerLength;
   int           method;
   int           flags;
   time_t        mtime;
   int           extraFlags;
   int           os;
   int           version;
   int           chunkLength;
   int           chunkCount;
   int           *chunks;
   unsigned long *offsets;	/* Sum-scan of chunks. */
   const char    *origFilename;
   const char    *comment;
   unsigned long crc;
   unsigned long length;
   unsigned long compressedLength;
   int           stamp;
   dictCache     cache[DICT_CACHE_SIZE];
   char          errorString[512];
} dictData;


/* initialize .data file */
extern dictData *dict_data_open (
   const char *filename, int computeCRC);
/* */
extern void dict_data_close (
   dictData *data);

extern char *dict_data_read_ (
   dictData *data,
   unsigned long start, unsigned long end,
   const char *preFilter,
   const char *postFilter );

extern char *dict_error_str( dictData *data );

extern int        mmap_mode;

#ifdef __cplusplus
}  /* end extern "C" */
#endif

#endif /* _DICTZIP_H_ */
