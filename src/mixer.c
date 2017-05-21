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
/* FIXME:
 * - on NetBSD sometimes the "mute" control is before the corresponding knob */



#if defined(__NetBSD__)
# include <sys/ioctl.h>
# include <sys/audioio.h>
#else
# include <sys/ioctl.h>
# include <sys/soundcard.h>
#endif
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <Desktop.h>
#include "mixer.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

/* compatibility */
#if !GTK_CHECK_VERSION(2, 12, 0)
# define GTK_ICON_LOOKUP_GENERIC_FALLBACK 0
#endif


/* Mixer */
/* private */
/* types */
#ifdef AUDIO_MIXER_DEVINFO
typedef struct _MixerClass
{
	int mixer_class;
	audio_mixer_name_t label;
	GtkWidget * hbox;
	int page;
} MixerClass;
#endif

typedef struct _MixerLevel
{
	uint8_t channels[8];
	size_t channels_cnt;
} MixerLevel;

typedef struct _MixerControl
{
	int index;
	int type;
	union {
		int ord;
		int mask;
		MixerLevel level;
	} un;
} MixerControl;

struct _Mixer
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * widget;
	GtkWidget * notebook;
	GtkWidget * properties;
	PangoFontDescription * bold;

	/* internals */
	char * device;
#ifdef AUDIO_MIXER_DEVINFO
	int fd;

	MixerClass * mc;
	size_t mc_cnt;
#else
	int fd;
#endif
};


/* constants */
#define MIXER_DEFAULT_DEVICE "/dev/mixer"


/* prototypes */
static int _mixer_error(Mixer * mixer, char const * message, int ret);
/* accessors */
static int _mixer_get_control(Mixer * mixer, int index, MixerControl * control);
static int _mixer_set_control(Mixer * mixer, int index, MixerControl * control);
/* useful */
static void _mixer_scrolled_window_add(GtkWidget * window, GtkWidget * widget);
static void _mixer_show_view(Mixer * mixer, int view);


/* public */
/* mixer_new */
static GtkWidget * _new_frame_label(GdkPixbuf * pixbuf, char const * name,
		char const * label);
#ifdef AUDIO_MIXER_DEVINFO
static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e);
static GtkWidget * _new_mute(Mixer * mixer, int dev,
		struct audio_mixer_enum * e);
static GtkWidget * _new_set(Mixer * mixer, int dev, struct audio_mixer_set * s);
/* callbacks */
static void _new_enum_on_toggled(GtkWidget * widget, gpointer data);
#if GTK_CHECK_VERSION(3, 0, 0)
static void _new_mute_on_notify(GObject * object, GParamSpec * spec,
		gpointer data);
#else
static void _new_mute_on_toggled(GtkWidget * widget, gpointer data);
#endif
static void _new_set_on_toggled(GtkWidget * widget, gpointer data);
#endif
static GtkWidget * _new_value(Mixer * mixer, int index, GtkWidget ** bbox);
/* callbacks */
static void _new_bind_on_toggled(GtkWidget * widget, gpointer data);
static void _new_value_on_changed(GtkWidget * widget, gpointer data);

Mixer * mixer_new(GtkWidget * window, char const * device, MixerLayout layout)
{
	Mixer * mixer;
	GtkSizeGroup * hgroup;
	GtkSizeGroup * vgroup;
	GtkWidget * scrolled = NULL;
	GtkWidget * label;
	GtkWidget * widget;
	GtkWidget * hvbox = NULL;
	GtkWidget * hbox;
	GtkWidget * bbox;
	GtkWidget * control;
	int i;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_devinfo_t md;
	mixer_devinfo_t md2;
	MixerClass * p;
	size_t u;
	GtkWidget * vbox2;
	char * name;
#else
	int value;
	char const * labels[] = SOUND_DEVICE_LABELS;
	char const * names[] = SOUND_DEVICE_NAMES;
#endif

	if((mixer = malloc(sizeof(*mixer))) == NULL)
		return NULL;
	if(device == NULL)
		device = MIXER_DEFAULT_DEVICE;
	mixer->device = strdup(device);
	mixer->fd = open(device, O_RDWR);
	mixer->window = window;
	mixer->properties = NULL;
	mixer->bold = NULL;
#ifdef AUDIO_MIXER_DEVINFO
	mixer->mc = NULL;
	mixer->mc_cnt = 0;
#endif
	hgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vgroup = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);
	if(mixer->device == NULL || mixer->fd < 0)
	{
		_mixer_error(NULL, device, 0);
		mixer_delete(mixer);
		return NULL;
	}
	/* widgets */
	mixer->bold = pango_font_description_new();
	pango_font_description_set_weight(mixer->bold, PANGO_WEIGHT_BOLD);
	/* classes */
	mixer->notebook = NULL;
	if(layout == ML_TABBED)
		mixer->notebook = gtk_notebook_new();
	else
	{
		scrolled = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
				GTK_POLICY_AUTOMATIC, (layout == ML_VERTICAL)
				? GTK_POLICY_AUTOMATIC : GTK_POLICY_NEVER);
		hvbox = gtk_box_new((layout == ML_VERTICAL)
				? GTK_ORIENTATION_VERTICAL
				: GTK_ORIENTATION_HORIZONTAL, 0);
		if(layout == ML_VERTICAL)
			gtk_box_set_homogeneous(GTK_BOX(hvbox), TRUE);
		_mixer_scrolled_window_add(scrolled, hvbox);
	}
	for(i = 0;; i++)
	{
#ifdef AUDIO_MIXER_DEVINFO
		md.index = i;
		if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.type != AUDIO_MIXER_CLASS)
			continue;
		if((p = realloc(mixer->mc, sizeof(*mixer->mc)
						* (mixer->mc_cnt + 1))) == NULL)
		{
			_mixer_error(NULL, "realloc", 1);
			mixer_delete(mixer);
			return NULL;
		}
		mixer->mc = p;
		p = &mixer->mc[mixer->mc_cnt++];
		p->mixer_class = md.mixer_class;
		memcpy(&p->label, &md.label, sizeof(md.label));
		p->hbox = NULL;
		p->page = -1;
#else
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
		if(mixer->notebook != NULL)
		{
			label = _new_frame_label(NULL, _("All"), NULL);
			gtk_widget_show_all(label);
			scrolled = gtk_scrolled_window_new(NULL, NULL);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(
						scrolled),
					GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
			_mixer_scrolled_window_add(scrolled, hbox);
			gtk_notebook_append_page(GTK_NOTEBOOK(mixer->notebook),
					scrolled, label);
		}
		else
			gtk_box_pack_start(GTK_BOX(hvbox), hbox, FALSE, TRUE,
					0);
		break;
#endif
	}
	/* controls */
	for(i = 0;; i++)
	{
#ifdef AUDIO_MIXER_DEVINFO
		md.index = i;
		if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) < 0)
			break;
		if(md.type == AUDIO_MIXER_CLASS)
			continue;
		for(u = 0; u < mixer->mc_cnt; u++)
			if(mixer->mc[u].mixer_class == md.mixer_class)
				break;
		if(u == mixer->mc_cnt)
			continue;
		hbox = mixer->mc[u].hbox;
		bbox = NULL;
		control = NULL;
		switch(md.type)
		{
			case AUDIO_MIXER_ENUM:
				control = _new_enum(mixer, i, &md.un.e);
				break;
			case AUDIO_MIXER_SET:
				control = _new_set(mixer, i, &md.un.s);
				break;
			case AUDIO_MIXER_VALUE:
				bbox = gtk_button_box_new(
						GTK_ORIENTATION_VERTICAL);
				gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox),
						GTK_BUTTONBOX_START);
				gtk_size_group_add_widget(vgroup, bbox);
				control = _new_value(mixer, i, &bbox);
				break;
		}
		if(control == NULL)
			continue;
		vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
		gtk_container_set_border_width(GTK_CONTAINER(vbox2), 4);
		gtk_box_pack_start(GTK_BOX(vbox2), control, TRUE, TRUE, 0);
		label = _new_frame_label(NULL, md.label.name, NULL);
		widget = gtk_frame_new(NULL);
		gtk_size_group_add_widget(hgroup, widget);
		gtk_frame_set_label_widget(GTK_FRAME(widget), label);
		gtk_container_add(GTK_CONTAINER(widget), vbox2);
		if(hbox == NULL)
		{
			mixer->mc[u].hbox = gtk_box_new(
					GTK_ORIENTATION_HORIZONTAL, 4);
			hbox = mixer->mc[u].hbox;
			gtk_container_set_border_width(GTK_CONTAINER(hbox), 4);
			if(mixer->notebook != NULL)
			{
				if((name = strdup(mixer->mc[u].label.name))
						!= NULL)
					name[0] = toupper(
							(unsigned char)name[0]);
				label = _new_frame_label(NULL,
						mixer->mc[u].label.name, name);
				free(name);
				gtk_widget_show_all(label);
				scrolled = gtk_scrolled_window_new(NULL, NULL);
				gtk_scrolled_window_set_policy(
						GTK_SCROLLED_WINDOW(scrolled),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_NEVER);
				_mixer_scrolled_window_add(scrolled, hbox);
				mixer->mc[u].page = gtk_notebook_append_page(
						GTK_NOTEBOOK(mixer->notebook),
						scrolled, label);
			}
			else if(hvbox != NULL)
				gtk_box_pack_start(GTK_BOX(hvbox), hbox, FALSE,
						TRUE, 0);
		}
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
		/* add a mute button if relevant */
		if(bbox == NULL)
			continue;
		md2.index = md.index + 1;
		if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md2) < 0)
			break;
		if(md2.type == AUDIO_MIXER_CLASS)
			continue;
		for(u = 0; u < mixer->mc_cnt; u++)
			if(mixer->mc[u].mixer_class == md2.mixer_class)
				break;
		if(u == mixer->mc_cnt)
			continue;
		u = strlen(md.label.name);
		if(md2.type != AUDIO_MIXER_ENUM || strncmp(md.label.name,
					md2.label.name, u) != 0
				|| (u = strlen(md2.label.name)) < 6
				|| strcmp(&md2.label.name[u - 5], ".mute") != 0)
			continue;
		if((widget = _new_mute(mixer, i + 1, &md2.un.e)) == NULL)
			continue;
		gtk_container_add(GTK_CONTAINER(bbox), widget);
		i++;
#else
		if(i == SOUND_MIXER_NONE)
			break;
		if(ioctl(mixer->fd, MIXER_READ(i), &value) != 0)
			continue;
# if GTK_CHECK_VERSION(3, 0, 0)
		bbox = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
# else
		bbox = gtk_vbutton_box_new();
# endif
		gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox),
				GTK_BUTTONBOX_START);
		gtk_size_group_add_widget(vgroup, bbox);
		control = _new_value(mixer, i, &bbox);
		gtk_container_set_border_width(GTK_CONTAINER(control), 4);
		label = _new_frame_label(NULL, names[i], labels[i]);
		widget = gtk_frame_new(NULL);
		gtk_size_group_add_widget(hgroup, widget);
		gtk_frame_set_label_widget(GTK_FRAME(widget), label);
		gtk_container_add(GTK_CONTAINER(widget), control);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
#endif
	}
	mixer->widget = (mixer->notebook != NULL) ? mixer->notebook : scrolled;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_show_class(mixer, AudioCoutputs);
#endif
	gtk_widget_show_all(mixer->widget);
	return mixer;
}

static GtkWidget * _new_frame_label(GdkPixbuf * pixbuf, char const * name,
		char const * label)
{
	GtkIconTheme * icontheme;
	GtkWidget * hbox;
	GtkWidget * widget;
	struct
	{
		char const * name;
		char const * icon;
	} icons[] = {
		{ "beep",	"audio-volume-high"	},
		{ "cd",		"media-cdrom"		},
		{ "dacsel",	"audio-card"		},
		{ "input",	"stock_mic"		},
		{ "line",	"stock_volume"		},
		{ "master",	"audio-volume-high"	},
		{ "mic",	"audio-input-microphone"},
		{ "monitor",	"utilities-system-monitor"},
		{ "output",	"audio-volume-high"	},
		{ "pcm",	"audio-volume-high"	},
		{ "rec",	"gtk-media-record"	},
		{ "source",	"audio-card"		},
		{ "vol",	"audio-volume-high"	}
	};
	size_t i;
	size_t len;
	const int size = 16;

	icontheme = gtk_icon_theme_get_default();
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	for(i = 0; pixbuf == NULL && i < sizeof(icons) / sizeof(*icons); i++)
		if(strncmp(icons[i].name, name, strlen(icons[i].name)) == 0)
			pixbuf = gtk_icon_theme_load_icon(icontheme,
					icons[i].icon, size,
					GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
	if(pixbuf == NULL)
	{
		len = strlen(name);
		/* more generic fallbacks */
		if(strstr(name, "sel") != NULL)
			pixbuf = gtk_icon_theme_load_icon(icontheme,
					"multimedia", size,
					GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
		else if(len > 5 && strcmp(&name[len - 5], ".mute") == 0)
			pixbuf = gtk_icon_theme_load_icon(icontheme,
					"audio-volume-muted", size,
					GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
	}
	if(pixbuf != NULL)
	{
		widget = gtk_image_new_from_pixbuf(pixbuf);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	}
	if(label == NULL)
		label = name;
	widget = gtk_label_new(label);
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

#ifdef AUDIO_MIXER_DEVINFO
static GtkWidget * _new_enum(Mixer * mixer, int dev,
		struct audio_mixer_enum * e)
{
	MixerControl * mc;
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;
	GSList * group = NULL;
	int * q;

	if(e->num_mem <= 0 || (mc = malloc(sizeof(*mc))) == NULL)
		return NULL;
	if(_mixer_get_control(mixer, dev, mc) != 0)
	{
		free(mc);
		return NULL;
	}
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	for(i = 0; i < e->num_mem; i++)
	{
		widget = gtk_radio_button_new_with_label(group,
				e->member[i].label.name);
		group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
		if(mc->un.ord == i)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
					TRUE);
		g_object_set_data(G_OBJECT(widget), "ctrl", mc);
		if((q = malloc(sizeof(*q))) != NULL)
		{
			*q = e->member[i].ord;
			g_object_set_data(G_OBJECT(widget), "ord", q);
		}
		g_signal_connect(widget, "toggled", G_CALLBACK(
					_new_enum_on_toggled), mixer);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	return vbox;
}

static GtkWidget * _new_mute(Mixer * mixer, int dev,
		struct audio_mixer_enum * e)
{
	MixerControl * mc;
#if !GTK_CHECK_VERSION(3, 0, 0)
	GtkWidget * hbox;
#endif
	GtkWidget * widget;
	gboolean active;

	if(e->num_mem != 2 || (mc = malloc(sizeof(*mc))) == NULL)
		return NULL;
	if(_mixer_get_control(mixer, dev, mc) != 0)
	{
		free(mc);
		return NULL;
	}
# if GTK_CHECK_VERSION(3, 0, 0)
	widget = gtk_switch_new();
	active = (strcmp(e->member[mc->un.ord].label.name, "on") == 0)
		? FALSE : TRUE;
	gtk_switch_set_active(GTK_SWITCH(widget), active);
	g_object_set_data(G_OBJECT(widget), "ctrl", mc);
	g_signal_connect(widget, "notify::active",
			G_CALLBACK(_new_mute_on_notify), mixer);
# else
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	widget = gtk_image_new_from_icon_name("audio-volume-muted",
			GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(_("Mute"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	widget = gtk_toggle_button_new();
	gtk_container_add(GTK_CONTAINER(widget), hbox);
	active = (strcmp(e->member[mc->un.ord].label.name, "on") == 0)
		? TRUE : FALSE;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), active);
	g_object_set_data(G_OBJECT(widget), "ctrl", mc);
	g_signal_connect(widget, "toggled", G_CALLBACK(_new_mute_on_toggled),
			mixer);
# endif
	return widget;
}

static GtkWidget * _new_set(Mixer * mixer, int dev, struct audio_mixer_set * s)
{
	MixerControl * mc;
	GtkWidget * vbox;
	int i;
	GtkWidget * widget;
	int * q;

	if(s->num_mem <= 0 || (mc = malloc(sizeof(*mc))) == NULL)
		return NULL;
	if(_mixer_get_control(mixer, dev, mc) != 0)
	{
		free(mc);
		return NULL;
	}
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_homogeneous(GTK_BOX(vbox), TRUE);
	for(i = 0; i < s->num_mem; i++)
	{
		widget = gtk_check_button_new_with_label(
				s->member[i].label.name);
		if(mc->un.mask & (1 << i))
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
					TRUE);
		g_object_set_data(G_OBJECT(widget), "ctrl", mc);
		if((q = malloc(sizeof(*q))) != NULL)
		{
			*q = s->member[i].mask;
			g_object_set_data(G_OBJECT(widget), "mask", q);
		}
		g_signal_connect(widget, "toggled", G_CALLBACK(
					_new_set_on_toggled), mixer);
		gtk_box_pack_start(GTK_BOX(vbox), widget, FALSE, TRUE, 0);
	}
	return vbox;
}

/* callbacks */
static void _new_enum_on_toggled(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == TRUE)
		mixer_set_enum(mixer, widget);
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void _new_mute_on_notify(GObject * object, GParamSpec * spec,
		gpointer data)
{
	Mixer * mixer = data;
	(void) spec;

	mixer_set_mute(mixer, GTK_WIDGET(object));
}
#else
static void _new_mute_on_toggled(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;

	mixer_set_mute(mixer, widget);
}
#endif

static void _new_set_on_toggled(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	mixer_set_set(mixer, widget);
}
#endif /* AUDIO_MIXER_DEVINFO */

static GtkWidget * _new_value(Mixer * mixer, int index, GtkWidget ** bbox)
{
#if !GTK_CHECK_VERSION(3, 14, 0)
	GtkWidget * align;
#endif
	GtkWidget * vbox;
	GtkWidget * hbox;
	GtkWidget * image;
	GtkWidget * widget;
	GtkWidget * bind = NULL;
	GSList * list = NULL;
	size_t i;
	gdouble v;
	MixerControl * mc;

	if((mc = malloc(sizeof(*mc))) == NULL)
		return NULL;
	if(_mixer_get_control(mixer, index, mc) != 0
			|| mc->un.level.channels_cnt <= 0)
	{
		free(mc);
		return NULL;
	}
	/* bind button */
	if(mc->un.level.channels_cnt >= 2)
	{
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
		image = gtk_image_new_from_stock(GTK_STOCK_CONNECT,
				GTK_ICON_SIZE_BUTTON);
		gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, TRUE, 0);
		widget = gtk_label_new(_("Bind"));
		gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
		bind = gtk_toggle_button_new();
		gtk_container_add(GTK_CONTAINER(bind), hbox);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bind), TRUE);
		g_signal_connect(bind, "toggled", G_CALLBACK(
					_new_bind_on_toggled), image);
	}
	/* sliders */
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	for(i = 0; i < mc->un.level.channels_cnt; i++)
	{
#if GTK_CHECK_VERSION(3, 0, 0)
		widget = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, 0.0,
				100.0, 1.0);
#else
		widget = gtk_vscale_new_with_range(0.0, 100.0, 1.0);
#endif
		gtk_range_set_inverted(GTK_RANGE(widget), TRUE);
		v = (mc->un.level.channels[i] / 255.0) * 100.0;
		gtk_range_set_value(GTK_RANGE(widget), v);
		if(bind != NULL)
		{
			g_object_set_data(G_OBJECT(widget), "bind", bind);
			list = g_slist_append(list, widget);
		}
		g_object_set_data(G_OBJECT(widget), "ctrl", mc);
		g_object_set_data(G_OBJECT(widget), "channel",
				&mc->un.level.channels[i]);
		g_signal_connect(widget, "value-changed", G_CALLBACK(
					_new_value_on_changed), mixer);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	}
	if(mc->un.level.channels_cnt < 2)
		return hbox;
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(hbox, GTK_ALIGN_CENTER);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
#else
	align = gtk_alignment_new(0.5, 0.5, 0.0, 1.0);
	gtk_container_add(GTK_CONTAINER(align), hbox);
	gtk_box_pack_start(GTK_BOX(vbox), align, TRUE, TRUE, 0);
#endif
	g_object_set_data(G_OBJECT(bind), "list", list);
	gtk_container_add(GTK_CONTAINER(*bbox), bind);
	gtk_box_pack_start(GTK_BOX(vbox), *bbox, FALSE, TRUE, 0);
	return vbox;
}

/* callbacks */
static void _new_bind_on_toggled(GtkWidget * widget, gpointer data)
{
	GtkWidget * image = data;
	gboolean active;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	gtk_image_set_from_stock(GTK_IMAGE(image),
			active ? GTK_STOCK_CONNECT : GTK_STOCK_DISCONNECT,
			GTK_ICON_SIZE_BUTTON);
}

static void _new_value_on_changed(GtkWidget * widget, gpointer data)
{
	Mixer * mixer = data;
	gdouble value;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %f, %p)\n", __func__, (void *)widget,
			value, (void *)data);
#endif
	value = gtk_range_get_value(GTK_RANGE(widget));
	mixer_set_value(mixer, widget, value);
}


/* mixer_delete */
void mixer_delete(Mixer * mixer)
{
	if(mixer->fd >= 0)
		close(mixer->fd);
	if(mixer->device != NULL)
		free(mixer->device);
	if(mixer->bold != NULL)
		pango_font_description_free(mixer->bold);
	free(mixer);
}


/* accessors */
/* mixer_get_properties */
int mixer_get_properties(Mixer * mixer, MixerProperties * properties)
{
#ifdef AUDIO_MIXER_DEVINFO
	audio_device_t ad;

	if(ioctl(mixer->fd, AUDIO_GETDEV, &ad) != 0)
		return -_mixer_error(mixer, "AUDIO_GETDEV", 1);
	snprintf(properties->name, sizeof(properties->name), "%s", ad.name);
	snprintf(properties->version, sizeof(properties->version), "%s",
			ad.version);
	snprintf(properties->device, sizeof(properties->device), "%s",
			ad.config);
#else
	struct mixer_info mi;
	int version;

	if(ioctl(mixer->fd, SOUND_MIXER_INFO, &mi) != 0)
		return -_mixer_error(mixer, "SOUND_MIXER_INFO", 1);
	if(ioctl(mixer->fd, OSS_GETVERSION, &version) != 0)
		return -_mixer_error(mixer, "OSS_GETVERSION", 1);
	snprintf(properties->name, sizeof(properties->name), "%s", mi.name);
	snprintf(properties->version, sizeof(properties->version), "%u.%u",
			(version >> 16) & 0xffff, version & 0xffff);
	snprintf(properties->device, sizeof(properties->device), "%s",
			mixer->device);
#endif
	return 0;
}


/* mixer_get_widget */
GtkWidget * mixer_get_widget(Mixer * mixer)
{
	return mixer->widget;
}


/* mixer_set_enum */
int mixer_set_enum(Mixer * mixer, GtkWidget * widget)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t * p;
	int * q;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d\n", __func__, (void *)mixer,
			mixer->fd);
# endif
	p = g_object_get_data(G_OBJECT(widget), "ctrl");
	q = g_object_get_data(G_OBJECT(widget), "ord");
	if(p == NULL || q == NULL)
		return 1;
	p->un.ord = *q;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d ord=%d\n", __func__, (void *)mixer,
			mixer->fd, p->un.ord);
# endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	/* FIXME implement */
#endif
	return 0;
}


/* mixer_set_mute */
int mixer_set_mute(Mixer * mixer, GtkWidget * widget)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t * p;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d\n", __func__, (void *)mixer,
			mixer->fd);
# endif
	if((p = g_object_get_data(G_OBJECT(widget), "ctrl")) == NULL)
		return -1;
#if GTK_CHECK_VERSION(3, 0, 0)
	p->un.ord = gtk_switch_get_active(GTK_SWITCH(widget))
		? 0 : 1; /* XXX assumes 1 is "off" */
#else
	p->un.ord = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))
		? 1 : 0; /* XXX assumes 0 is "off" */
#endif
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d ord=%d\n", __func__, (void *)mixer,
			mixer->fd, p->un.ord);
# endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	/* FIXME implement */
#endif
	return 0;
}


/* mixer_set_set */
int mixer_set_set(Mixer * mixer, GtkWidget * widget)
{
	gboolean active;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t * p;
	int * q;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d\n", __func__, (void *)mixer,
			mixer->fd);
# endif
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	p = g_object_get_data(G_OBJECT(widget), "ctrl");
	q = g_object_get_data(G_OBJECT(widget), "mask");
	if(p == NULL || q == NULL)
		return 1;
	if(ioctl(mixer->fd, AUDIO_MIXER_READ, p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_READ", 1);
	p->un.mask = (active) ? (p->un.mask | *q)
		: (p->un.mask - (p->un.mask & *q));
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d mask=%d\n", __func__,
			(void *)mixer, mixer->fd, p->un.mask);
# endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	/* FIXME implement */
#endif
	return 0;
}


/* mixer_set_value */
int mixer_set_value(Mixer * mixer, GtkWidget * widget, gdouble value)
{
	GtkWidget * b;
	MixerControl * mc;
	uint8_t * channel;
	size_t i;
	GSList * q;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %f) fd=%d\n", __func__, (void *)mixer,
			value, mixer->fd);
#endif
	b = g_object_get_data(G_OBJECT(widget), "bind");
	mc = g_object_get_data(G_OBJECT(widget), "ctrl");
	channel = g_object_get_data(G_OBJECT(widget), "channel");
	if(mc == NULL || channel == NULL)
		return 1;
	*channel = (value / 100.0) * 255;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(b)))
	{
		for(i = 0; i < mc->un.level.channels_cnt; i++)
			mc->un.level.channels[i] = *channel;
		if(b != NULL)
			for(q = g_object_get_data(G_OBJECT(b), "list");
					q != NULL; q = q->next)
				gtk_range_set_value(GTK_RANGE(q->data), value);
	}
	return _mixer_set_control(mixer, mc->index, mc);
}


/* useful */
/* mixer_properties */
static GtkWidget * _properties_label(Mixer * mixer, GtkSizeGroup * group,
		char const * label, char const * value);
/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget);

void mixer_properties(Mixer * mixer)
{
	GtkSizeGroup * group;
	GtkWidget * vbox;
	GtkWidget * hbox;
	MixerProperties mp;

	if(mixer->properties != NULL)
	{
		gtk_widget_show(mixer->properties);
		return;
	}
	if(mixer_get_properties(mixer, &mp) != 0)
		return;
	mixer->properties = gtk_message_dialog_new(GTK_WINDOW(mixer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
#if GTK_CHECK_VERSION(2, 6, 0)
			"%s", _("Properties"));
	gtk_message_dialog_format_secondary_text(
			GTK_MESSAGE_DIALOG(mixer->properties),
#endif
			"");
#if GTK_CHECK_VERSION(2, 10, 0)
	gtk_message_dialog_set_image(GTK_MESSAGE_DIALOG(mixer->properties),
			gtk_image_new_from_stock(GTK_STOCK_PROPERTIES,
				GTK_ICON_SIZE_DIALOG));
#endif
	gtk_window_set_title(GTK_WINDOW(mixer->properties), _("Properties"));
	g_signal_connect(mixer->properties, "delete-event", G_CALLBACK(
				_properties_on_closex), NULL);
	g_signal_connect(mixer->properties, "response", G_CALLBACK(
				gtk_widget_hide), NULL);
#if GTK_CHECK_VERSION(2, 14, 0)
	vbox = gtk_dialog_get_content_area(GTK_DIALOG(mixer->properties));
#else
	vbox = GTK_DIALOG(mixer->properties)->vbox;
#endif
	group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	hbox = _properties_label(mixer, group, _("Name: "), mp.name);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	hbox = _properties_label(mixer, group, _("Version: "), mp.version);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	hbox = _properties_label(mixer, group, _("Device: "), mp.device);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 2);
	gtk_widget_show_all(vbox);
	gtk_widget_show(mixer->properties);
}

static GtkWidget * _properties_label(Mixer * mixer, GtkSizeGroup * group,
		char const * label, char const * value)
{
	GtkWidget * hbox;
	GtkWidget * widget;

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	widget = gtk_label_new(label);
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_override_font(widget, mixer->bold);
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_widget_modify_font(widget, mixer->bold);
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_size_group_add_widget(group, widget);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
	widget = gtk_label_new(value);
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_set_halign(widget, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(widget), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	return hbox;
}

/* callbacks */
static gboolean _properties_on_closex(GtkWidget * widget)
{
	gtk_widget_hide(widget);
	return TRUE;
}


/* mixer_show */
void mixer_show(Mixer * mixer)
{
	gtk_widget_show(mixer->window);
}


/* mixer_show_all */
void mixer_show_all(Mixer * mixer)
{
	_mixer_show_view(mixer, -1);
}


/* mixer_show_class */
void mixer_show_class(Mixer * mixer, char const * name)
{
#ifdef AUDIO_MIXER_DEVINFO
	size_t u;

	if(mixer->notebook != NULL && name != NULL)
	{
		for(u = 0; u < mixer->mc_cnt; u++)
		{
			if(mixer->mc[u].hbox == NULL)
				continue;
			if(strcmp(mixer->mc[u].label.name, name) != 0)
				continue;
			gtk_notebook_set_current_page(GTK_NOTEBOOK(
						mixer->notebook),
					mixer->mc[u].page);
		}
		return;
	}
	for(u = 0; u < mixer->mc_cnt; u++)
		if(mixer->mc[u].hbox == NULL)
			continue;
		else if(name == NULL
				|| strcmp(mixer->mc[u].label.name, name) == 0)
			gtk_widget_show(mixer->mc[u].hbox);
		else
			gtk_widget_hide(mixer->mc[u].hbox);
#endif
}


/* private */
/* functions */
/* mixer_error */
static int _error_text(char const * message, int ret);

static int _mixer_error(Mixer * mixer, char const * message, int ret)
{
	GtkWidget * dialog;
	char const * error;

	if(mixer == NULL)
		return _error_text(message, ret);
	error = strerror(errno);
	dialog = gtk_message_dialog_new(GTK_WINDOW(mixer->window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE, "%s",
#if GTK_CHECK_VERSION(2, 6, 0)
			_("Error"));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
			"%s: %s", message,
#endif
			error);
	gtk_window_set_title(GTK_WINDOW(dialog), _("Error"));
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return ret;
}

static int _error_text(char const * message, int ret)
{
	fputs(PACKAGE ": ", stderr);
	perror(message);
	return ret;
}


/* accessors */
/* mixer_get_control */
static int _mixer_get_control(Mixer * mixer, int index, MixerControl * control)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t p;
	struct mixer_devinfo md;
	int i;
# ifdef DEBUG
	size_t u;
	char * sep = "";
# endif

	md.index = index;
	if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_DEVINFO", 1);
	p.dev = index;
	/* XXX this is necessary for some drivers and I don't like it */
	if((p.type = md.type) == AUDIO_MIXER_VALUE)
		p.un.value.num_channels = md.un.v.num_channels;
	if(ioctl(mixer->fd, AUDIO_MIXER_READ, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_READ", 1);
	control->index = index;
	control->type = p.type;
# ifdef DEBUG
	for(u = 0; u < mixer->mc_cnt; u++)
		if(mixer->mc[u].mixer_class == md.mixer_class)
			printf("%s", mixer->mc[u].label.name);
	printf(".%s=", md.label.name);
# endif
	switch(p.type)
	{
		case AUDIO_MIXER_ENUM:
			control->un.ord = p.un.ord;
# ifdef DEBUG
			for(i = 0; i < md.un.e.num_mem; i++)
			{
				if(md.un.e.member[i].ord != p.un.ord)
					continue;
				printf("%s%s", sep,
						md.un.e.member[i].label.name);
				break;
			}
# endif
			break;
		case AUDIO_MIXER_SET:
			control->un.mask = p.un.mask;
# ifdef DEBUG
			for(i = 0; i < md.un.s.num_mem; i++)
			{
				if((p.un.mask & (1 << i)) == 0)
					continue;
				printf("%s%s", sep,
						md.un.s.member[i].label.name);
				sep = ",";
			}
# endif
			break;
		case AUDIO_MIXER_VALUE:
			control->un.level.channels_cnt
				= p.un.value.num_channels;
			for(i = 0; i < p.un.value.num_channels; i++)
			{
				control->un.level.channels[i]
					= p.un.value.level[i];
# ifdef DEBUG
				printf("%s%u", sep, p.un.value.level[i]);
				sep = ",";
# endif
			}
			break;
	}
# ifdef DEBUG
	putchar('\n');
# endif
#else
	int value;

	control->index = index;
	if(ioctl(mixer->fd, MIXER_READ(index), &value) != 0)
		return -_mixer_error(NULL, "MIXER_READ", 1);
	control->type = 0;
	control->un.level.channels_cnt = 2;
	control->un.level.channels[0] = ((value & 0xff) * 255) / 100;
	control->un.level.channels[1] = (((value >> 8) & 0xff) * 255) / 100;
	return 0;
#endif
	return 0;
}


/* mixer_set_control */
static int _mixer_set_control(Mixer * mixer, int index, MixerControl * control)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t p;
	int i;

	p.dev = index;
	p.type = control->type;
	p.un.value.num_channels = control->un.level.channels_cnt;
	for(i = 0; i < p.un.value.num_channels; i++)
		p.un.value.level[i] = control->un.level.channels[i];
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	int level = 0;

	level |= (control->un.level.channels[0] * 100) / 255;
	level |= ((control->un.level.channels[1] * 100) / 255) << 8;
	if(ioctl(mixer->fd, MIXER_WRITE(index), &level) != 0)
		return -_mixer_error(mixer, "MIXER_WRITE", 1);
#endif
	return 0;
}


/* useful */
/* mixer_scrolled_window_add */
static void _mixer_scrolled_window_add(GtkWidget * window, GtkWidget * widget)
{
	GtkWidget * viewport;

	viewport = gtk_viewport_new(NULL, NULL);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(viewport), widget);
	gtk_container_add(GTK_CONTAINER(window), viewport);
}


/* mixer_show_view */
static void _mixer_show_view(Mixer * mixer, int view)
{
#ifdef AUDIO_MIXER_DEVINFO
	size_t u;

	if(view < 0)
	{
		for(u = 0; u < mixer->mc_cnt; u++)
			if(mixer->mc[u].hbox != NULL)
				gtk_widget_show(mixer->mc[u].hbox);
		return;
	}
	u = view;
	if(u >= mixer->mc_cnt)
		return;
	for(u = 0; u < mixer->mc_cnt; u++)
		if(mixer->mc[u].hbox == NULL)
			continue;
		else if(u == (size_t)view)
			gtk_widget_show(mixer->mc[u].hbox);
		else
			gtk_widget_hide(mixer->mc[u].hbox);
#endif
}
