/* $Id$ */
/* Copyright (c) 2015 Pierre Pronchery <khorben@defora.org> */
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



#include <System.h>
#include "window.h"


/* MixerWindow */
/* private */
/* types */
struct _MixerWindow
{
	Mixer * mixer;

	/* widgets */
	GtkWidget * window;
#ifndef EMBEDDED
	GtkWidget * menubar;
#endif
};


/* prototypes */
/* callbacks */


/* public */
/* functions */
/* mixerwindow_new */
MixerWindow * mixerwindow_new(char const * device, MixerLayout layout,
		gboolean embedded)
{
	MixerWindow * mixer;
	GtkAccelGroup * group;

	if((mixer = object_new(sizeof(*mixer))) == NULL)
		return NULL;
	group = gtk_accel_group_new();
	mixer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	mixer->mixer = NULL;
	if(mixer->window != NULL)
	{
		gtk_widget_realize(mixer->window);
		mixer->mixer = mixer_new(mixer->window, device, layout,
				embedded);
	}
	if(mixer->mixer == NULL)
	{
		mixerwindow_delete(mixer);
		return NULL;
	}
	return mixer;
}


/* mixerwindow_delete */
void mixerwindow_delete(MixerWindow * mixer)
{
	if(mixer->mixer != NULL)
		mixer_delete(mixer->mixer);
	if(mixer->window != NULL)
		gtk_widget_destroy(mixer->window);
	object_delete(mixer);
}


/* useful */
/* mixerwindow_about */
void mixerwindow_about(MixerWindow * mixer)
{
	mixer_about(mixer->mixer);
}


/* mixerwindow_properties */
void mixerwindow_properties(MixerWindow * mixer)
{
	mixer_properties(mixer->mixer);
}


/* mixerwindow_show */
void mixerwindow_show(MixerWindow * mixer)
{
	mixer_show(mixer->mixer);
}


/* mixerwindow_show_all */
void mixerwindow_show_all(MixerWindow * mixer)
{
	mixer_show_all(mixer->mixer);
}


/* mixerwindow_show_class */
void mixerwindow_show_class(MixerWindow * mixer, char const * name)
{
	mixer_show_class(mixer->mixer, name);
}
