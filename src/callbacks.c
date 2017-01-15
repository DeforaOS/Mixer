/* $Id$ */
/* Copyright (c) 2009-2015 Pierre Pronchery <khorben@defora.org> */
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



#ifdef DEBUG
# include <stdio.h>
#endif
#if defined(__NetBSD__)
# include <sys/audioio.h>
#endif
#include <Desktop.h>
#include "window.h"
#include "callbacks.h"
#include "../config.h"


/* public */
/* functions */
/* callbacks */
/* on_closex */
gboolean on_closex(gpointer data)
{
	(void) data;

	gtk_main_quit();
	return TRUE;
}


/* on_embedded */
void on_embedded(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show(mixer);
}


/* file menu */
/* on_file_properties */
void on_file_properties(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_properties(mixer);
}


/* on_file_close */
void on_file_close(gpointer data)
{
	on_closex(data);
}


/* on_view_all */
void on_view_all(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_all(mixer);
}


/* on_view_fullscreen */
void on_view_fullscreen(gpointer data)
{
	MixerWindow * mixer = data;
	gboolean fullscreen;

	fullscreen = mixerwindow_get_fullscreen(mixer);
	mixerwindow_set_fullscreen(mixer, !fullscreen);
}


#ifdef AUDIO_MIXER_DEVINFO
/* on_view_outputs */
void on_view_outputs(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_class(mixer, AudioCoutputs);
}


/* on_view_inputs */
void on_view_inputs(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_class(mixer, AudioCinputs);
}


/* on_view_record */
void on_view_record(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_class(mixer, AudioCrecord);
}


/* on_view_monitor */
void on_view_monitor(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_class(mixer, AudioCmonitor);
}


/* on_view_equalization */
void on_view_equalization(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_class(mixer, AudioCequalization);
}


/* on_view_mix */
void on_view_mix(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_class(mixer, "mix");
}


/* on_view_modem */
void on_view_modem(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_show_class(mixer, AudioCmodem);
}
#endif


/* on_help_about */
void on_help_about(gpointer data)
{
	MixerWindow * mixer = data;

	mixerwindow_about(mixer);
}


/* on_help_contents */
void on_help_contents(gpointer data)
{
	(void) data;

	desktop_help_contents(PACKAGE, "mixer");
}
