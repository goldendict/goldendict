/*
 *
 *  ao_private.c
 *
 *       Copyright (C) Stan Seibert - July 2001
 *
 *  This file is part of libao, a cross-platform audio output library.  See
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

#ifndef __AO_PRIVATE_H__
#define __AO_PRIVATE_H__

/* --- Operating System Compatibility --- */

/*
  OpenBSD systems with a.out binaries require dlsym()ed symbols to be
  prepended with an underscore, so we need the following nasty #ifdef
  hack.
*/
#if defined(__OpenBSD__) && !defined(__ELF__)
#define dlsym(h,s) dlsym(h, "_" s)
#endif

/* RTLD_NOW is the preferred symbol resolution behavior, but
 * some platforms do not support it.  The autoconf script will have
 * already defined DLOPEN_FLAG if the default is unacceptable on the
 * current platform.
 *
 * ALSA requires RTLD_GLOBAL.
 */
#if !defined(DLOPEN_FLAG)
#define DLOPEN_FLAG (RTLD_NOW | RTLD_GLOBAL)
#endif

/* --- Constants --- */

#ifndef AO_SYSTEM_CONFIG
#define AO_SYSTEM_CONFIG "/etc/libao.conf"
#endif
#ifndef AO_USER_CONFIG
#define AO_USER_CONFIG   "/.libao"
#endif

/* --- Structures --- */

typedef struct ao_config {
	char *default_driver;
} ao_config;

typedef enum {
  AO_OUTPUT_MATRIX_UNDEFINED=0,   /* matrix unset */
  AO_OUTPUT_MATRIX_FIXED=1,       /* fixed, immutable channel order, eg, ALSA */
  AO_OUTPUT_MATRIX_COLLAPSIBLE=2, /* fixed order but only used channels sent, eg MACOS */
  AO_OUTPUT_MATRIX_PERMUTABLE=3,  /* channel map is fully permutable. eg Pulse */
} ao_outorder;

struct ao_device {
	int  type; /* live output or file output? */
	int  driver_id;
	ao_functions *funcs;
	FILE *file; /* File for output if this is a file driver */

  /* input not necessarily == output. Right now, byte order, channel
     count, and channel mappings may be altered. */

	int  client_byte_format;
	int  machine_byte_format;
	int  driver_byte_format;
	char *swap_buffer;
	int  swap_buffer_size; /* Bytes allocated to swap_buffer */

        int input_channels;
        int output_channels;
        int bytewidth;
        int rate;

        ao_outorder output_matrix_order;
        char *output_matrix;  /* physical output channel
                                 ordering/numbering matrix set by
                                 driver if there's a channel
                                 name->number mapping useful to the
                                 backend driver in some way.  Eg,
                                 Pulse has fully permutable input
                                 channel masks, but specific channels
                                 locations (eg, 'Center') still have
                                 assigned numbers even if not a
                                 specific slot int he input
                                 interleave. */
        int   output_mask;
        int  *input_map;      /* input permutation mapping from each
                                 input channel to a location in the
                                 output_matrix. Made by ao_open,
                                 intended for convenience use by
                                 driver in device open. */

        char *inter_matrix;   /* channel matrix as presented to the
                                 backend API */
        int  *inter_permute;  /* maps from each channel in the
                                 inter_matrix back to an input channel
                                 (if any) */

	void *internal;       /* Pointer to driver-specific data */

        int verbose;
};

struct ao_functions {
	int (*test)(void);
	ao_info* (*driver_info)(void);
	int (*device_init)(ao_device *device);
	int (*set_option)(ao_device *device, const char *key,
			  const char *value);
	int (*open)(ao_device *device, ao_sample_format *format);
	int (*play)(ao_device *device, const char *output_samples,
			   uint_32 num_bytes);
	int (*close)(ao_device *device);
	void (*device_clear)(ao_device *device);
	const char* (*file_extension)(void);
};

/* --- Functions --- */

void ao_read_config_files (ao_config *config);

#define adebug(format, ...) {\
    if(!device || device->verbose==2){                                  \
      if(strcmp(format,"\n")){                                          \
        if(device && device->funcs->driver_info()->short_name){         \
          fprintf(stderr,"ao_%s debug: " format,device->funcs->driver_info()->short_name,__VA_ARGS__); \
        }else{                                                          \
          fprintf(stderr,"debug: " format,__VA_ARGS__);                 \
        }                                                               \
      }else{                                                            \
        fprintf(stderr,"\n");                                           \
      }                                                                 \
    }                                                                   \
  }

#define averbose(format, ...) {\
    if(!device || device->verbose>0){                                   \
      if(strcmp(format,"\n")){                                          \
        if(device && device->funcs->driver_info()->short_name){         \
          fprintf(stderr,"ao_%s info: " format,device->funcs->driver_info()->short_name,__VA_ARGS__); \
        }else{                                                          \
          fprintf(stderr,"info: " format,__VA_ARGS__);                  \
        }                                                               \
      }else{                                                            \
        fprintf(stderr,"\n");                                           \
      }                                                                 \
    }                                                                   \
  }

#define ainfo(format, ...) {\
    if(!device || device->verbose>=0){                                  \
      if(strcmp(format,"\n")){                                          \
        if(device && device->funcs->driver_info()->short_name){         \
          fprintf(stderr,"ao_%s info: " format,device->funcs->driver_info()->short_name,__VA_ARGS__); \
        }else{                                                          \
          fprintf(stderr,"info: " format,__VA_ARGS__);                  \
        }                                                               \
      }else{                                                            \
        fprintf(stderr,"\n");                                           \
      }                                                                 \
    }                                                                   \
  }

#define awarn(format, ...) {\
    if(!device || device->verbose>=0){                                  \
      if(strcmp(format,"\n")){                                          \
        if(device && device->funcs->driver_info()->short_name){         \
          fprintf(stderr,"ao_%s WARNING: " format,device->funcs->driver_info()->short_name,__VA_ARGS__); \
        }else{                                                          \
          fprintf(stderr,"WARNING: " format,__VA_ARGS__);               \
        }                                                               \
      }else{                                                            \
        fprintf(stderr,"\n");                                           \
      }                                                                 \
    }                                                                   \
  }

#define aerror(format, ...) {                                           \
    if(!device || device->verbose>=0){                                  \
      if(strcmp(format,"\n")){                                          \
        if(device && device->funcs->driver_info()->short_name){         \
          fprintf(stderr,"ao_%s ERROR: " format,device->funcs->driver_info()->short_name,__VA_ARGS__); \
        }else{                                                          \
          fprintf(stderr,"ERROR: " format,__VA_ARGS__);                 \
        }                                                               \
      }else{                                                            \
        fprintf(stderr,"\n");                                           \
      }                                                                 \
    }                                                                   \
  }

#endif /* __AO_PRIVATE_H__ */
