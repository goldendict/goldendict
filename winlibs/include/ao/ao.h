/*
 *
 *  ao.h 
 *
 *	Original Copyright (C) Aaron Holtzman - May 1999
 *      Modifications Copyright (C) Stan Seibert - July 2000, July 2001
 *      More Modifications Copyright (C) Jack Moffitt - October 2000
 *
 *  This file is part of libao, a cross-platform audio outputlibrary.  See
 *  README for a history of this source code.
 *
 *  libao is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  libao is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef __AO_H__
#define __AO_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "os_types.h"

/* --- Constants ---*/

#define AO_TYPE_LIVE 1
#define AO_TYPE_FILE 2


#define AO_ENODRIVER   1
#define AO_ENOTFILE    2
#define AO_ENOTLIVE    3
#define AO_EBADOPTION  4
#define AO_EOPENDEVICE 5
#define AO_EOPENFILE   6
#define AO_EFILEEXISTS 7
#define AO_EBADFORMAT  8

#define AO_EFAIL       100


#define AO_FMT_LITTLE 1
#define AO_FMT_BIG    2
#define AO_FMT_NATIVE 4

/* --- Structures --- */

typedef struct ao_info {
	int  type; /* live output or file output? */
	char *name; /* full name of driver */
	char *short_name; /* short name of driver */
        char *author; /* driver author */
	char *comment; /* driver comment */
	int  preferred_byte_format;
	int  priority;
	char **options;
	int  option_count;
} ao_info;

typedef struct ao_functions ao_functions;
typedef struct ao_device ao_device;

typedef struct ao_sample_format {
	int  bits; /* bits per sample */
	int  rate; /* samples per second (in a single channel) */
	int  channels; /* number of audio channels */
	int  byte_format; /* Byte ordering in sample, see constants below */
        char *matrix; /* input channel location/ordering */
} ao_sample_format;

typedef struct ao_option {
	char *key;
	char *value;
	struct ao_option *next;
} ao_option;

#if defined(AO_BUILDING_LIBAO)
#include "ao_private.h"
#endif

/* --- Functions --- */

/* library setup/teardown */
void ao_initialize(void);
void ao_shutdown(void);

/* device setup/playback/teardown */
int   ao_append_global_option(const char *key,
                              const char *value);
int          ao_append_option(ao_option **options,
                              const char *key,
                              const char *value);
void          ao_free_options(ao_option *options);
ao_device*       ao_open_live(int driver_id,
                              ao_sample_format *format,
                              ao_option *option);
ao_device*       ao_open_file(int driver_id,
                              const char *filename,
                              int overwrite,
                              ao_sample_format *format,
                              ao_option *option);

int                   ao_play(ao_device *device,
                              char *output_samples,
                              uint_32 num_bytes);
int                  ao_close(ao_device *device);

/* driver information */
int              ao_driver_id(const char *short_name);
int      ao_default_driver_id(void);
ao_info       *ao_driver_info(int driver_id);
ao_info **ao_driver_info_list(int *driver_count);
char       *ao_file_extension(int driver_id);

/* miscellaneous */
int          ao_is_big_endian(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __AO_H__ */
