/* $Id$ */
/* Copyright (c) 2009-2014 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mixer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



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
	desktop_help_contents(PACKAGE, "mixer");
}
