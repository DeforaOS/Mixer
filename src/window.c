/* $Id$ */
/* Copyright (c) 2015 Pierre Pronchery <khorben@defora.org> */
static char _copyright[] =
"Copyright © 2009-2015 Pierre Pronchery <khorben@defora.org>";
/* This file is part of DeforaOS Desktop Mixer */
static char _license[] =
"This program is free software: you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, version 3 of the License.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.  If not, see <http://www.gnu.org/licenses/>.";



#if defined(__NetBSD__)
# include <sys/ioctl.h>
# include <sys/audioio.h>
#else
# include <sys/ioctl.h>
# include <sys/soundcard.h>
#endif
#include <string.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION(3, 0, 0)
# include <gtk/gtkx.h>
#endif
#include <System.h>
#include <Desktop.h>
#include "callbacks.h"
#include "window.h"
#include "../config.h"
#define _(string) (string)
#define N_(string) (string)


/* MixerWindow */
/* private */
/* types */
struct _MixerWindow
{
	Mixer * mixer;
	gboolean fullscreen;

	/* widgets */
	GtkWidget * window;
#ifndef EMBEDDED
	GtkWidget * menubar;
#endif
	GtkWidget * about;
};


/* constants */
static char const * _authors[] =
{
	"Pierre Pronchery <khorben@defora.org>",
	NULL
};

#ifdef EMBEDDED
static const DesktopAccel _mixer_accel[] =
{
	{ G_CALLBACK(on_file_close), GDK_CONTROL_MASK, GDK_KEY_W },
	{ G_CALLBACK(on_file_properties), GDK_MOD1_MASK, GDK_KEY_Return },
	{ G_CALLBACK(on_view_all), GDK_CONTROL_MASK, GDK_KEY_A },
# ifdef AUDIO_MIXER_DEVINFO
	{ G_CALLBACK(on_view_outputs), GDK_CONTROL_MASK, GDK_KEY_O },
	{ G_CALLBACK(on_view_inputs), GDK_CONTROL_MASK, GDK_KEY_I },
	{ G_CALLBACK(on_view_record), GDK_CONTROL_MASK, GDK_KEY_R },
	{ G_CALLBACK(on_view_monitor), GDK_CONTROL_MASK, GDK_KEY_N },
	{ G_CALLBACK(on_view_equalization), GDK_CONTROL_MASK, GDK_KEY_E },
	{ G_CALLBACK(on_view_mix), GDK_CONTROL_MASK, GDK_KEY_X },
	{ G_CALLBACK(on_view_modem), GDK_CONTROL_MASK, GDK_KEY_M },
# endif
	{ NULL, 0, 0 }
};
#endif /* EMBEDDED */

#ifndef EMBEDDED
static const DesktopMenu _mixer_menu_file[] =
{
	{ N_("_Properties"), G_CALLBACK(on_file_properties),
		GTK_STOCK_PROPERTIES, GDK_MOD1_MASK, GDK_KEY_Return },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_Close"), G_CALLBACK(on_file_close), GTK_STOCK_CLOSE,
		GDK_CONTROL_MASK, GDK_KEY_W },
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _mixer_menu_view[] =
{
	{ N_("_Fullscreen"), G_CALLBACK(on_view_fullscreen),
# if GTK_CHECK_VERSION(2, 8, 0)
		GTK_STOCK_FULLSCREEN,
# else
		NULL,
# endif
		0, GDK_KEY_F11 },
	{ "", NULL, NULL, 0, 0 },
	{ N_("_All"), G_CALLBACK(on_view_all), "stock_select-all",
		GDK_CONTROL_MASK, GDK_KEY_A },
# ifdef AUDIO_MIXER_DEVINFO
	{ N_("_Outputs"), G_CALLBACK(on_view_outputs), "audio-volume-high",
		GDK_CONTROL_MASK, GDK_KEY_O },
	{ N_("_Inputs"), G_CALLBACK(on_view_inputs), "stock_mic",
		GDK_CONTROL_MASK, GDK_KEY_I },
	{ N_("_Record"), G_CALLBACK(on_view_record), "gtk-media-record",
		GDK_CONTROL_MASK, GDK_KEY_R },
	{ N_("Mo_nitor"), G_CALLBACK(on_view_monitor),
		"utilities-system-monitor", GDK_CONTROL_MASK, GDK_KEY_N },
	{ N_("_Equalization"), G_CALLBACK(on_view_equalization), "multimedia",
		GDK_CONTROL_MASK, GDK_KEY_E },
	{ N_("Mi_x"), G_CALLBACK(on_view_mix), "stock_volume", GDK_CONTROL_MASK,
		GDK_KEY_X },
	{ N_("_Modem"), G_CALLBACK(on_view_modem), "modem", GDK_CONTROL_MASK,
		GDK_KEY_M },
# endif
	{ NULL, NULL, NULL, 0, 0 }
};

static const DesktopMenu _mixer_menu_help[] =
{
	{ N_("_Contents"), G_CALLBACK(on_help_contents), "help-contents", 0,
		GDK_KEY_F1 },
	{ N_("_About"), G_CALLBACK(on_help_about), GTK_STOCK_ABOUT, 0, 0 },
	{ NULL, NULL, NULL, 0, 0 }
};

static DesktopMenubar _mixer_menubar[] =
{
	{ N_("_File"), _mixer_menu_file },
	{ N_("_View"), _mixer_menu_view },
	{ N_("_Help"), _mixer_menu_help },
	{ NULL, NULL }
};
#else
static DesktopToolbar _mixer_toolbar[] =
{
	{ N_("Properties"), G_CALLBACK(on_file_properties),
		GTK_STOCK_PROPERTIES, GDK_MOD1_MASK, GDK_KEY_Return, NULL },
	{ "", NULL, NULL, 0, 0, NULL },
	{ N_("Fullscreen"), G_CALLBACK(on_view_fullscreen),
# if GTK_CHECK_VERSION(2, 8, 0)
		GTK_STOCK_FULLSCREEN,
# else
		"gtk-fullscreen",
# endif
		0, GDK_KEY_F11, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL },
	{ N_("All"), G_CALLBACK(on_view_all), "stock_select-all", 0, 0, NULL },
	{ N_("Outputs"), G_CALLBACK(on_view_outputs), "audio-volume-high", 0, 0,
		NULL },
	{ N_("Inputs"), G_CALLBACK(on_view_inputs), "stock_line-in", 0, 0,
		NULL },
	{ N_("Record"), G_CALLBACK(on_view_record), "gtk-media-record", 0, 0,
		NULL },
	{ N_("Monitor"), G_CALLBACK(on_view_monitor), "audio-input-microphone",
		0, 0, NULL },
	{ N_("Equalization"), G_CALLBACK(on_view_equalization), "multimedia", 0,
		0, NULL },
	{ N_("Mix"), G_CALLBACK(on_view_mix), "stock_volume", 0, 0, NULL },
	{ N_("Modem"), G_CALLBACK(on_view_modem), "modem", 0, 0, NULL },
	{ NULL, NULL, NULL, 0, 0, NULL }
};
#endif /* EMBEDDED */


/* public */
/* functions */
/* mixerwindow_new */
MixerWindow * mixerwindow_new(char const * device, MixerLayout layout,
		gboolean embedded)
{
	MixerWindow * mixer;
	GtkAccelGroup * accel;
	GtkWidget * vbox;
	GtkWidget * widget;
	MixerProperties properties;
	char buf[80];
	unsigned long id;

	if((mixer = object_new(sizeof(*mixer))) == NULL)
		return NULL;
	accel = gtk_accel_group_new();
	mixer->window = NULL;
	mixer->about = NULL;
	if(embedded)
	{
		mixer->window = gtk_plug_new(0);
		g_signal_connect_swapped(mixer->window, "embedded", G_CALLBACK(
					on_embedded), mixer);
	}
	else
	{
		mixer->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_add_accel_group(GTK_WINDOW(mixer->window), accel);
		gtk_window_set_default_size(GTK_WINDOW(mixer->window), 800,
				300);
#if GTK_CHECK_VERSION(2, 6, 0)
		gtk_window_set_icon_name(GTK_WINDOW(mixer->window),
				"stock_volume");
#endif
		gtk_window_set_title(GTK_WINDOW(mixer->window), _("Mixer"));
		g_signal_connect_swapped(mixer->window, "delete-event",
			G_CALLBACK(on_closex), mixer);
	}
	mixer->mixer = NULL;
	mixer->fullscreen = FALSE;
	if(mixer->window != NULL)
	{
		gtk_widget_realize(mixer->window);
		mixer->mixer = mixer_new(mixer->window, device, layout);
	}
	if(mixer->mixer == NULL)
	{
		mixerwindow_delete(mixer);
		return NULL;
	}
#if GTK_CHECK_VERSION(3, 0, 0)
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
#else
	vbox = gtk_vbox_new(FALSE, 0);
#endif
#ifndef EMBEDDED
	/* menubar */
	if(embedded == FALSE)
	{
		if(layout == ML_TABBED)
			_mixer_menubar[1].menu = &_mixer_menu_view[3];
		widget = desktop_menubar_create(_mixer_menubar, mixer, accel);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
#else
	desktop_accel_create(_mixer_accel, mixer, accel);
	/* toolbar */
	if(embedded == FALSE)
	{
		if(layout != ML_TABBED)
			_mixer_toolbar[3].name = "";
		widget = desktop_toolbar_create(_mixer_toolbar, mixer, accel);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
#endif
	g_object_unref(accel);
	widget = mixer_get_widget(mixer->mixer);
	gtk_box_pack_start(GTK_BOX(vbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(mixer->window), vbox);
	gtk_widget_show_all(vbox);
	if(embedded)
	{
		/* print the window ID and force a flush */
		id = gtk_plug_get_id(GTK_PLUG(mixer->window));
		printf("%lu\n", id);
		fclose(stdout);
	}
	else
	{
		/* set the window title */
		if(mixer_get_properties(mixer->mixer, &properties) == 0)
		{
			snprintf(buf, sizeof(buf), "%s - %s%s%s", _("Mixer"),
					properties.name,
					strlen(properties.version) ? " " : "",
					properties.version);
			gtk_window_set_title(GTK_WINDOW(mixer->window), buf);
		}
		gtk_widget_show(mixer->window);
	}
	return mixer;
}


/* mixerwindow_delete */
void mixerwindow_delete(MixerWindow * mixer)
{
	if(mixer->mixer != NULL)
		mixer_delete(mixer->mixer);
	if(mixer->about != NULL)
		gtk_widget_destroy(mixer->about);
	if(mixer->window != NULL)
		gtk_widget_destroy(mixer->window);
	object_delete(mixer);
}


/* accessors */
/* mixerwindow_get_fullscreen */
gboolean mixerwindow_get_fullscreen(MixerWindow * mixer)
{
	return mixer->fullscreen;
}


/* mixerwindow_set_fullscreen */
void mixerwindow_set_fullscreen(MixerWindow * mixer, gboolean fullscreen)
{
	if(fullscreen)
		gtk_window_fullscreen(GTK_WINDOW(mixer->window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(mixer->window));
	mixer->fullscreen = fullscreen;
}


/* useful */
/* mixerwindow_about */
static gboolean _about_on_closex(GtkWidget * widget);

void mixerwindow_about(MixerWindow * mixer)
{
	if(mixer->about != NULL)
	{
		gtk_widget_show(mixer->about);
		return;
	}
	mixer->about = desktop_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(mixer->about), GTK_WINDOW(
				mixer->window));
	g_signal_connect(mixer->about, "delete-event", G_CALLBACK(
				_about_on_closex), NULL);
	desktop_about_dialog_set_authors(mixer->about, _authors);
	desktop_about_dialog_set_comments(mixer->about,
			_("Volume mixer for the DeforaOS desktop"));
	desktop_about_dialog_set_copyright(mixer->about, _copyright);
	desktop_about_dialog_set_license(mixer->about, _license);
	desktop_about_dialog_set_logo_icon_name(mixer->about, "stock_volume");
	desktop_about_dialog_set_name(mixer->about, PACKAGE);
	desktop_about_dialog_set_translator_credits(mixer->about,
			_("translator-credits"));
	desktop_about_dialog_set_version(mixer->about, VERSION);
	desktop_about_dialog_set_website(mixer->about,
			"http://www.defora.org/");
	gtk_widget_show(mixer->about);
}

static gboolean _about_on_closex(GtkWidget * widget)
{
	gtk_widget_hide(widget);
	return TRUE;
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
