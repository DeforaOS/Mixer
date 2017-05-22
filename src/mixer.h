/* $Id$ */
/* Copyright (c) 2009-2017 Pierre Pronchery <khorben@defora.org> */
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



#ifndef MIXER_MIXER_H
# define MIXER_MIXER_H

# include <gtk/gtk.h> /* XXX should not be necessary */
# include <System/string.h>


/* Mixer */
/* types */
typedef enum _MixerLayout
{
	ML_HORIZONTAL,
	ML_TABBED,
	ML_VERTICAL
} MixerLayout;

typedef struct _MixerProperties
{
	char name[32];
	char version[16];
	char device[16];
} MixerProperties;

typedef struct _Mixer Mixer;


/* functions */
Mixer * mixer_new(GtkWidget * window, String const * device, MixerLayout layout);
void mixer_delete(Mixer * mixer);

/* accessors */
int mixer_get_properties(Mixer * mixer, MixerProperties * properties);
GtkWidget * mixer_get_widget(Mixer * mixer);

int mixer_set_enum(Mixer * mixer, GtkWidget * widget);
int mixer_set_mute(Mixer * mixer, GtkWidget * widget);
int mixer_set_set(Mixer * mixer, GtkWidget * widget);
int mixer_set_value(Mixer * mixer, GtkWidget * widget, gdouble value);

/* useful */
void mixer_properties(Mixer * mixer);

void mixer_show(Mixer * mixer);
void mixer_show_all(Mixer * mixer);
void mixer_show_class(Mixer * mixer, char const * name);

#endif /* !MIXER_MIXER_H */
