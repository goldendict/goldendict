/*
 *
 *  plugin.h - function declarations for libao plugins
 *  
 *      Copyright (C) Stan Seibert - June 2001
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
#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
	
#include <stdlib.h>
#include "os_types.h"

int ao_plugin_test();
ao_info *ao_plugin_driver_info();
int ao_plugin_device_init(ao_device *device);
int ao_plugin_set_option(ao_device *device, const char *key, const char *value);
int ao_plugin_open(ao_device *device, ao_sample_format *format);
int ao_plugin_play(ao_device *device, const char *output_samples, 
		uint_32 num_bytes);
int ao_plugin_close(ao_device *device);
void ao_plugin_device_clear(ao_device *device);
const char *ao_plugin_file_extension();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __PLUGIN_H__ */
