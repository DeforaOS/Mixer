/* $Id$ */
/* Copyright (c) 2017 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mixer */
/* All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */



#ifndef DESKTOP_MIXER_CONTROL_H
# define DESKTOP_MIXER_CONTROL_H

# include <stdarg.h>
# include <gtk/gtk.h>
# include <System/string.h>


/* MixerControlPlugin */
typedef struct _MixerControl MixerControl;

typedef struct _MixerControlPlugin MixerControlPlugin;

typedef struct _MixerControlPluginHelper
{
	MixerControl * control;

	int (*mixercontrol_set)(MixerControl * control);
} MixerControlPluginHelper;

typedef struct _MixerControlDefinition
{
	String const * icon;
	String const * name;
	String const * description;

	/* callbacks */
	MixerControlPlugin * (*init)(MixerControlPluginHelper * helper,
			String const * type, va_list properties);
	void (*destroy)(MixerControlPlugin * plugin);

	int (*get)(MixerControlPlugin * plugin, va_list properties);
	String const * (*get_type)(MixerControlPlugin * plugin);
	GtkWidget * (*get_widget)(MixerControlPlugin * plugin);
	int (*set)(MixerControlPlugin * plugin, va_list properties);
} MixerControlDefinition;

#endif /* !DESKTOP_MIXER_CONTROL_H */
