/* $Id$ */
/* Copyright (c) 2009-2018 Pierre Pronchery <khorben@defora.org> */
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
#include <math.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <Desktop.h>
#include "control.h"
#include "common.h"
#include "mixer.h"
#include "../config.h"
#define _(string) gettext(string)
#define N_(string) (string)

/* compatibility */
#if !GTK_CHECK_VERSION(2, 12, 0)
# define GTK_ICON_LOOKUP_GENERIC_FALLBACK 0
#endif


/* constants */
#ifndef PROGNAME_MIXER
# define PROGNAME_MIXER	"mixer"
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
	uint8_t delta;
	size_t channels_cnt;
} MixerLevel;

/* XXX rename this type */
typedef struct _MixerControl2
{
	int index;
	int type;
	union {
		int ord;
		int mask;
		MixerLevel level;
	} un;

	MixerControl * control;
} MixerControl2;

struct _Mixer
{
	/* widgets */
	GtkWidget * window;
	GtkWidget * widget;
	GtkWidget * notebook;
	GtkWidget * properties;
	PangoFontDescription * bold;

	/* internals */
	String * device;
#ifdef AUDIO_MIXER_DEVINFO
	int fd;

	MixerClass * classes;
	size_t classes_cnt;
#else
	int fd;
#endif

	MixerControl2 * controls;
	size_t controls_cnt;

	guint source;
};


/* prototypes */
static int _mixer_error(Mixer * mixer, char const * message, int ret);

/* accessors */
static int _mixer_get_control(Mixer * mixer, MixerControl2 * control);
static int _mixer_set_control(Mixer * mixer, MixerControl2 * control);

static int _mixer_set_control_widget(Mixer * mixer, MixerControl2 * control);

static String const * _mixer_get_icon(String const * id);

/* useful */
static int _mixer_refresh_control(Mixer * mixer, MixerControl2 * control);

static void _mixer_scrolled_window_add(GtkWidget * window, GtkWidget * widget);

static void _mixer_show_view(Mixer * mixer, int view);


/* public */
/* mixer_new */
static GtkWidget * _new_frame_label(GdkPixbuf * pixbuf, char const * name,
		char const * label);
#ifdef AUDIO_MIXER_DEVINFO
static MixerControl * _new_enum(Mixer * mixer, int index,
		struct audio_mixer_enum * e, String const * id,
		String const * icon, String const * name);
static MixerControl * _new_set(Mixer * mixer, int index,
		struct audio_mixer_set * s, String const * id,
		String const * icon, String const * name);
#endif
static MixerControl * _new_value(Mixer * mixer, int index,
		GtkSizeGroup * vgroup, String const * id, String const * icon,
		String const * name);
/* callbacks */
static gboolean _new_on_refresh(gpointer data);

Mixer * mixer_new(GtkWidget * window, String const * device, MixerLayout layout)
{
	Mixer * mixer;
	GtkSizeGroup * hgroup;
	GtkSizeGroup * vgroup;
	GtkWidget * scrolled = NULL;
	GtkWidget * label;
	GtkWidget * widget;
	GtkWidget * hvbox = NULL;
	GtkWidget * hbox;
	MixerControl * control;
	MixerControl2 * q;
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
	mixer->device = string_new(device);
	mixer->fd = open(device, O_RDWR);
	mixer->window = window;
	mixer->properties = NULL;
	mixer->bold = NULL;
#ifdef AUDIO_MIXER_DEVINFO
	mixer->classes = NULL;
	mixer->classes_cnt = 0;
#endif
	mixer->controls = NULL;
	mixer->controls_cnt = 0;
	mixer->source = 0;
	if(mixer->device == NULL || mixer->fd < 0)
	{
		_mixer_error(NULL, device, 0);
		mixer_delete(mixer);
		return NULL;
	}
	hgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	vgroup = gtk_size_group_new(GTK_SIZE_GROUP_VERTICAL);
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
				: GTK_ORIENTATION_HORIZONTAL, 4);
		gtk_container_set_border_width(GTK_CONTAINER(hvbox), 2);
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
		if((p = realloc(mixer->classes, sizeof(*p)
						* (mixer->classes_cnt + 1)))
				== NULL)
		{
			_mixer_error(NULL, "realloc", 1);
			mixer_delete(mixer);
			return NULL;
		}
		mixer->classes = p;
		p = &mixer->classes[mixer->classes_cnt++];
		p->mixer_class = md.mixer_class;
		memcpy(&p->label, &md.label, sizeof(md.label));
		p->hbox = NULL;
		p->page = -1;
#else
		hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
		gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
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
		for(u = 0; u < mixer->classes_cnt; u++)
			if(mixer->classes[u].mixer_class == md.mixer_class)
				break;
		if(u == mixer->classes_cnt)
			continue;
		hbox = mixer->classes[u].hbox;
		control = NULL;
		switch(md.type)
		{
			case AUDIO_MIXER_ENUM:
				control = _new_enum(mixer, i, &md.un.e,
						md.label.name,
						_mixer_get_icon(md.label.name),
						md.label.name);
				break;
			case AUDIO_MIXER_SET:
				control = _new_set(mixer, i, &md.un.s,
						md.label.name,
						_mixer_get_icon(md.label.name),
						md.label.name);
				break;
			case AUDIO_MIXER_VALUE:
				control = _new_value(mixer, i, vgroup,
						md.label.name,
						_mixer_get_icon(md.label.name),
						md.label.name);
				break;
		}
		if(control == NULL)
			continue;
		if((q = realloc(mixer->controls, sizeof(*q)
						* (mixer->controls_cnt + 1)))
				== NULL)
		{
			mixercontrol_delete(control);
			/* FIXME report error */
			continue;
		}
		mixer->controls = q;
		q = &mixer->controls[mixer->controls_cnt++];
		q->index = md.index;
		q->type = md.type;
		q->control = control;
		widget = mixercontrol_get_widget(control);
		vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
		gtk_box_pack_start(GTK_BOX(vbox2), widget, TRUE, TRUE, 0);
		gtk_size_group_add_widget(hgroup, widget);
		if(hbox == NULL)
		{
			p = &mixer->classes[u];
			p->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
			hbox = p->hbox;
			gtk_container_set_border_width(GTK_CONTAINER(hbox), 2);
			if(mixer->notebook != NULL)
			{
				if((name = strdup(mixer->classes[u].label.name))
						!= NULL)
					name[0] = toupper(
							(unsigned char)name[0]);
				label = _new_frame_label(NULL,
						mixer->classes[u].label.name,
						name);
				free(name);
				gtk_widget_show_all(label);
				scrolled = gtk_scrolled_window_new(NULL, NULL);
				gtk_scrolled_window_set_policy(
						GTK_SCROLLED_WINDOW(scrolled),
						GTK_POLICY_AUTOMATIC,
						GTK_POLICY_NEVER);
				_mixer_scrolled_window_add(scrolled, p->hbox);
				p->page = gtk_notebook_append_page(
						GTK_NOTEBOOK(mixer->notebook),
						scrolled, label);
			}
			else if(hvbox != NULL)
				gtk_box_pack_start(GTK_BOX(hvbox), p->hbox,
						FALSE, TRUE, 0);
		}
		gtk_box_pack_start(GTK_BOX(hbox), vbox2, FALSE, TRUE, 0);
		/* add a mute button if relevant */
		if(md.type != AUDIO_MIXER_VALUE)
			continue;
		md2.index = md.index + 1;
		if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md2) < 0)
			break;
		if(md2.type == AUDIO_MIXER_CLASS)
			continue;
		for(u = 0; u < mixer->classes_cnt; u++)
			if(mixer->classes[u].mixer_class == md2.mixer_class)
				break;
		if(u == mixer->classes_cnt)
			continue;
		u = strlen(md.label.name);
		if(md2.type != AUDIO_MIXER_ENUM || strncmp(md.label.name,
					md2.label.name, u) != 0
				|| (u = strlen(md2.label.name)) < 6
				|| strcmp(&md2.label.name[u - 5], ".mute") != 0)
			continue;
		/* XXX may fail */
		mixercontrol_set(control, "show-mute", TRUE, NULL);
		i++;
#else
		if(i == SOUND_MIXER_NONE)
			break;
		if(ioctl(mixer->fd, MIXER_READ(i), &value) != 0)
			continue;
		if((q = realloc(mixer->controls, sizeof(*q)
						* (mixer->controls_cnt + 1)))
				== NULL)
			/* FIXME report error */
			continue;
		mixer->controls = q;
		q = &mixer->controls[mixer->controls_cnt];
		if((control = _new_value(mixer, i, vgroup, names[i],
						_mixer_get_icon(names[i]),
						labels[i]))
				== NULL)
			continue;
		q->index = i;
		q->type = 0;
		q->control = control;
		mixer->controls_cnt++;
		widget = mixercontrol_get_widget(control);
		gtk_size_group_add_widget(hgroup, widget);
		gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, TRUE, 0);
#endif
	}
	mixer->widget = (mixer->notebook != NULL) ? mixer->notebook : scrolled;
#ifdef AUDIO_MIXER_DEVINFO
	mixer_show_class(mixer, AudioCoutputs);
#endif
	gtk_widget_show_all(mixer->widget);
	mixer->source = g_timeout_add(500, _new_on_refresh, mixer);
	return mixer;
}

static GtkWidget * _new_frame_label(GdkPixbuf * pixbuf, char const * name,
		char const * label)
{
	GtkIconTheme * icontheme;
	GtkWidget * hbox;
	GtkWidget * widget;
	const int size = 16;

	icontheme = gtk_icon_theme_get_default();
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	if(pixbuf == NULL)
		pixbuf = gtk_icon_theme_load_icon(icontheme,
				_mixer_get_icon(name), size,
				GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);
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
static MixerControl * _new_enum(Mixer * mixer, int index,
		struct audio_mixer_enum * e, String const * id,
		String const * icon, String const * name)
{
	MixerControl2 mc;
	MixerControl * control;
	int i;
	char label[16];
	char value[16];

	if(e->num_mem <= 0)
		return NULL;
	mc.index = index;
	if(_mixer_get_control(mixer, &mc) != 0
			|| (control = mixercontrol_new(mixer, id, icon, name,
					"radio", "members", e->num_mem, NULL))
			== NULL)
		return NULL;
	for(i = 0; i < e->num_mem; i++)
	{
		snprintf(label, sizeof(label), "label%d", i);
		snprintf(value, sizeof(value), "value%d", i);
		if(mixercontrol_set(control, label, e->member[i].label.name,
					value, e->member[i].ord, NULL) != 0)
		{
			mixercontrol_delete(control);
			return NULL;
		}
	}
	if(_mixer_set_control_widget(mixer, &mc) != 0)
	{
		mixercontrol_delete(control);
		return NULL;
	}
	return control;
}

static MixerControl * _new_set(Mixer * mixer, int index,
		struct audio_mixer_set * s, String const * id,
		String const * icon, String const * name)
{
	MixerControl2 mc;
	MixerControl * control;
	int i;
	char label[16];
	char value[16];

	if(s->num_mem <= 0)
		return NULL;
	mc.index = index;
	if(_mixer_get_control(mixer, &mc) != 0
			|| (control = mixercontrol_new(mixer, id, icon, name,
					"set", "members", s->num_mem, NULL))
			== NULL)
		return NULL;
	mc.control = control;
	for(i = 0; i < s->num_mem; i++)
	{
		snprintf(label, sizeof(label), "label%d", i);
		snprintf(value, sizeof(value), "value%d", i);
		if(mixercontrol_set(control, label, s->member[i].label.name,
					value, s->member[i].mask, NULL) != 0)
		{
			mixercontrol_delete(control);
			return NULL;
		}
	}
	if(_mixer_set_control_widget(mixer, &mc) != 0)
	{
		mixercontrol_delete(control);
		return NULL;
	}
	return control;
}
#endif /* AUDIO_MIXER_DEVINFO */

static MixerControl * _new_value(Mixer * mixer, int index,
		GtkSizeGroup * vgroup, String const * id, String const * icon,
		String const * name)
{
	MixerControl2 mc;
	MixerControl * control;
	size_t i;
	gboolean bind = TRUE;

	mc.index = index;
	if(_mixer_get_control(mixer, &mc) != 0
			|| mc.un.level.channels_cnt <= 0
			|| (control = mixercontrol_new(mixer, id, icon, name,
					"channels",
					"channels", mc.un.level.channels_cnt,
					"delta", mc.un.level.delta,
					"vgroup", vgroup, NULL)) == NULL)
		return NULL;
	mc.control = control;
	/* detect if binding is in place */
	for(i = 1; i < mc.un.level.channels_cnt; i++)
		if(mc.un.level.channels[i] != mc.un.level.channels[0])
		{
			bind = FALSE;
			break;
		}
	if(mixercontrol_set(control, "bind", bind, NULL) != 0
			|| _mixer_set_control_widget(mixer, &mc) != 0)
	{
		mixercontrol_delete(control);
		return NULL;
	}
	return control;
}

/* callbacks */
static gboolean _new_on_refresh(gpointer data)
{
	Mixer * mixer = data;

	mixer_refresh(mixer);
	return TRUE;
}


/* mixer_delete */
void mixer_delete(Mixer * mixer)
{
	size_t i;

	if(mixer->source > 0)
		g_source_remove(mixer->source);
	for(i = 0; i < mixer->controls_cnt; i++)
		mixercontrol_delete(mixer->controls[i].control);
	free(mixer->controls);
	if(mixer->fd >= 0)
		close(mixer->fd);
	if(mixer->device != NULL)
		string_delete(mixer->device);
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


/* mixer_set */
static int _set_channels(Mixer * mixer, MixerControl * control);
#if defined(AUDIO_MIXER_DEVINFO)
static int _set_mute(Mixer * mixer, MixerControl * control);
static int _set_radio(Mixer * mixer, MixerControl * control);
static int _set_set(Mixer * mixer, MixerControl * control);
#endif

int mixer_set(Mixer * mixer, MixerControl * control)
{
	String const * type;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if((type = mixercontrol_get_type(control)) == NULL)
		return -1;
	else if(string_compare(type, "channels") == 0)
		return _set_channels(mixer, control);
#if defined(AUDIO_MIXER_DEVINFO)
	else if(string_compare(type, "mute") == 0)
		return _set_mute(mixer, control);
	else if(string_compare(type, "radio") == 0)
		return _set_radio(mixer, control);
	else if(string_compare(type, "set") == 0)
		return _set_set(mixer, control);
#endif
	return -1;
}

static int _set_channels(Mixer * mixer, MixerControl * control)
{
	size_t i;
	double value;
	MixerControl2 * mc;
	char buf[16];

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p) fd=%d\n", __func__, (void *)mixer,
			(void *)control, mixer->fd);
#endif
	for(i = 0; i < mixer->controls_cnt; i++)
		if(mixer->controls[i].control == control)
			break;
	if(i == mixer->controls_cnt)
		return -1;
	mc = &mixer->controls[i];
	if(_mixer_get_control(mixer, mc) != 0)
		return -1;
	for(i = 0; i < mc->un.level.channels_cnt; i++)
	{
		snprintf(buf, sizeof(buf), "value%zu", i);
		if(mixercontrol_get(control, buf, &value, NULL) != 0)
			return -1;
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() value%zu=%f\n",
				__func__, i, value);
#endif
		mc->un.level.channels[i] = (value * 255.0) / 100.0;
	}
	return _mixer_set_control(mixer, mc);
}

#if defined(AUDIO_MIXER_DEVINFO)
static int _set_mute(Mixer * mixer, MixerControl * control)
{
	size_t i;
	gboolean value;
	MixerControl2 * mc;
	mixer_ctrl_t p;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p, %p) fd=%d\n", __func__, (void *)mixer,
			(void *)control, mixer->fd);
# endif
	for(i = 0; i < mixer->controls_cnt; i++)
		if(mixer->controls[i].control == control)
			break;
	if(i == mixer->controls_cnt)
		return -1;
	mc = &mixer->controls[i];
	p.dev = mc->index;
	p.type = mc->type;
	if(mixercontrol_get(control, "value", &value, NULL) != 0)
		return -1;
	p.un.ord = value;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() ord=%d\n", __func__, p.un.ord);
# endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
	return 0;
}

static int _set_radio(Mixer * mixer, MixerControl * control)
{
	size_t i;
	MixerControl2 * mc;
	mixer_ctrl_t p;
	unsigned int value;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d\n", __func__, (void *)mixer,
			mixer->fd);
# endif
	for(i = 0; i < mixer->controls_cnt; i++)
		if(mixer->controls[i].control == control)
			break;
	if(i == mixer->controls_cnt)
		return -1;
	mc = &mixer->controls[i];
	p.dev = mc->index;
	p.type = mc->type;
	if(mixercontrol_get(control, "value", &value, NULL) != 0)
		return -1;
	p.un.ord = value;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() ord=%d\n", __func__, p.un.ord);
# endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
	return 0;
}

static int _set_set(Mixer * mixer, MixerControl * control)
{
	size_t i;
	unsigned int value;
	MixerControl2 * mc;
	mixer_ctrl_t p;

# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p) fd=%d\n", __func__, (void *)mixer,
			mixer->fd);
# endif
	for(i = 0; i < mixer->controls_cnt; i++)
		if(mixer->controls[i].control == control)
			break;
	if(i == mixer->controls_cnt)
		return -1;
	mc = &mixer->controls[i];
	p.dev = mc->index;
	p.type = mc->type;
	if(mixercontrol_get(control, "value", &value, NULL) != 0)
		return -1;
	p.un.mask = value;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() mask=%d\n", __func__, p.un.mask);
# endif
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
	return 0;
}
#endif


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
			gtk_image_new_from_icon_name("gtk-properties",
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


/* mixer_refresh */
int mixer_refresh(Mixer * mixer)
{
	int ret = 0;
	size_t i;

	for(i = 0; i < mixer->controls_cnt; i++)
		ret |= _mixer_refresh_control(mixer, &mixer->controls[i]);
	return ret;
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
void mixer_show_class(Mixer * mixer, String const * name)
{
#ifdef AUDIO_MIXER_DEVINFO
	size_t u;

	if(mixer->notebook != NULL && name != NULL)
	{
		for(u = 0; u < mixer->classes_cnt; u++)
		{
			if(mixer->classes[u].hbox == NULL)
				continue;
			if(strcmp(mixer->classes[u].label.name, name) != 0)
				continue;
			gtk_notebook_set_current_page(GTK_NOTEBOOK(
						mixer->notebook),
					mixer->classes[u].page);
		}
		return;
	}
	for(u = 0; u < mixer->classes_cnt; u++)
		if(mixer->classes[u].hbox == NULL)
			continue;
		else if(name == NULL
				|| strcmp(mixer->classes[u].label.name, name)
				== 0)
			gtk_widget_show(mixer->classes[u].hbox);
		else
			gtk_widget_hide(mixer->classes[u].hbox);
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
	fputs(PROGNAME_MIXER ": ", stderr);
	perror(message);
	return ret;
}


/* accessors */
/* mixer_get_control */
static int _mixer_get_control(Mixer * mixer, MixerControl2 * control)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t p;
	struct mixer_devinfo md;
	int i;
	uint16_t u16;
# ifdef DEBUG
	size_t u;
	char * sep = "";
# endif

	md.index = control->index;
	if(ioctl(mixer->fd, AUDIO_MIXER_DEVINFO, &md) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_DEVINFO", 1);
	p.dev = control->index;
	/* XXX this is necessary for some drivers and I don't like it */
	if((p.type = md.type) == AUDIO_MIXER_VALUE)
		p.un.value.num_channels = md.un.v.num_channels;
	if(ioctl(mixer->fd, AUDIO_MIXER_READ, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_READ", 1);
	control->type = p.type;
# ifdef DEBUG
	for(u = 0; u < mixer->classes_cnt; u++)
		if(mixer->classes[u].mixer_class == md.mixer_class)
			printf("%s", mixer->classes[u].label.name);
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
			printf("%s", "  {");
			for(i = 0; i < md.un.s.num_mem; i++)
				printf(" %s", md.un.s.member[i].label.name);
			printf("%s", " }");
# endif
			break;
		case AUDIO_MIXER_VALUE:
			u16 = md.un.v.delta;
			if((u16 = ceil((u16 * 100) / 255.0)) == 0)
				u16 = 1;
			control->un.level.delta = u16;
			control->un.level.channels_cnt
				= p.un.value.num_channels;
			for(i = 0; i < p.un.value.num_channels; i++)
			{
# ifdef DEBUG
				printf("%s%u", sep, p.un.value.level[i]);
				sep = ",";
# endif
				u16 = p.un.value.level[i];
				u16 = ceil((u16 * 100) / 255.0);
				control->un.level.channels[i] = u16;
			}
#ifdef DEBUG
			printf(" delta=%u", md.un.v.delta);
#endif
			break;
	}
# ifdef DEBUG
	putchar('\n');
# endif
#else
	int value;
	uint16_t u16;

	if(ioctl(mixer->fd, MIXER_READ(control->index), &value) != 0)
		return -_mixer_error(NULL, "MIXER_READ", 1);
	control->type = 0;
	control->un.level.delta = 1;
	control->un.level.channels_cnt = 2;
	u16 = value & 0xff;
	control->un.level.channels[0] = u16;
	u16 = (value & 0xff00) >> 8;
	control->un.level.channels[1] = u16;
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() % 3d % 3d\n", __func__,
			control->un.level.channels[0],
			control->un.level.channels[1]);
# endif
#endif
	return 0;
}


/* mixer_get_icon */
static String const * _mixer_get_icon(String const * id)
{
	struct
	{
		String const * name;
		String const * icon;
	} icons[] = {
		{ "beep",	"audio-volume-high"		},
		{ "cd",		"media-cdrom"			},
		{ "dac",	"audio-card"			},
		{ "input",	"stock_mic"			},
		{ "line",	"stock_volume"			},
		{ "master",	"audio-volume-high"		},
		{ "mic",	"audio-input-microphone"	},
		{ "monitor",	"utilities-system-monitor"	},
		{ "output",	"audio-volume-high"		},
		{ "pcm",	"audio-volume-high"		},
		{ "rec",	"gtk-media-record"		},
		{ "source",	"audio-card"			},
		{ "vol",	"audio-volume-high"		}
	};
	size_t len;
	size_t i;

	for(i = 0; i < sizeof(icons) / sizeof(*icons); i++)
		if(strncmp(icons[i].name, id, string_length(icons[i].name))
				== 0)
			return icons[i].icon;
	len = string_length(id);
	if(string_find(id, "sel") != NULL)
		return "multimedia";
	else if(len > 5 && string_compare(&id[len - 5], ".mute") == 0)
		return "audio-volume-muted";
	return "audio-volume-high";
}


/* mixer_set_control */
static int _mixer_set_control(Mixer * mixer, MixerControl2 * control)
{
#ifdef AUDIO_MIXER_DEVINFO
	mixer_ctrl_t p;
	int i;

	p.dev = control->index;
	p.type = control->type;
	p.un.value.num_channels = control->un.level.channels_cnt;
	for(i = 0; i < p.un.value.num_channels; i++)
		p.un.value.level[i] = control->un.level.channels[i];
	if(ioctl(mixer->fd, AUDIO_MIXER_WRITE, &p) != 0)
		return -_mixer_error(mixer, "AUDIO_MIXER_WRITE", 1);
#else
	int level;

	level = (control->un.level.channels[1] << 8)
		| control->un.level.channels[0];
# ifdef DEBUG
	fprintf(stderr, "DEBUG: %s() level=0x%04x\n", __func__, level);
# endif
	if(ioctl(mixer->fd, MIXER_WRITE(control->index), &level) != 0)
		return -_mixer_error(mixer, "MIXER_WRITE", 1);
#endif
	return 0;
}


/* mixer_set_control_widget */
static int _set_control_widget_channels(MixerControl2 * control);
static int _set_control_widget_mute(MixerControl2 * control);
static int _set_control_widget_radio(MixerControl2 * control);
static int _set_control_widget_set(MixerControl2 * control);

static int _mixer_set_control_widget(Mixer * mixer, MixerControl2 * control)
{
	String const * type;
	(void) mixer;

	if((type = mixercontrol_get_type(control->control)) == NULL)
		/* XXX report error */
		return -1;
	if(string_compare(type, "channels") == 0)
		return _set_control_widget_channels(control);
	if(string_compare(type, "mute") == 0)
		return _set_control_widget_mute(control);
	if(string_compare(type, "radio") == 0)
		return _set_control_widget_radio(control);
	if(string_compare(type, "set") == 0)
		return _set_control_widget_set(control);
	return -1;
}

static int _set_control_widget_channels(MixerControl2 * control)
{
	gboolean bind = TRUE;
	gdouble value;
	size_t i;
	char buf[16];

	/* unset bind if the channels are no longer synchronized */
	for(i = 1; i < control->un.level.channels_cnt; i++)
		if(control->un.level.channels[i]
				!= control->un.level.channels[i - 1])
		{
			bind = FALSE;
			break;
		}
	if(bind == FALSE)
		if(mixercontrol_set(control->control, "bind", FALSE, NULL) != 0)
			return -1;
	/* set the individual channels */
	for(i = 0; i < control->un.level.channels_cnt; i++)
	{
		snprintf(buf, sizeof(buf), "value%zu", i);
		value = control->un.level.channels[i];
		if(mixercontrol_set(control->control, buf, value, NULL) != 0)
			return -1;
	}
	return 0;
}

static int _set_control_widget_mute(MixerControl2 * control)
{
	/* FIXME implement */
	return -1;
}

static int _set_control_widget_radio(MixerControl2 * control)
{
	return mixercontrol_set(control->control, "value", control->un.mask,
			NULL);
}

static int _set_control_widget_set(MixerControl2 * control)
{
	return mixercontrol_set(control->control, "value", control->un.mask,
				NULL);
}


/* useful */
/* mixer_refresh_control */
static int _mixer_refresh_control(Mixer * mixer, MixerControl2 * control)
{
	if(_mixer_get_control(mixer, control) != 0)
		return -1;
	return _mixer_set_control_widget(mixer, control);
}


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
		for(u = 0; u < mixer->classes_cnt; u++)
			if(mixer->classes[u].hbox != NULL)
				gtk_widget_show(mixer->classes[u].hbox);
		return;
	}
	u = view;
	if(u >= mixer->classes_cnt)
		return;
	for(u = 0; u < mixer->classes_cnt; u++)
		if(mixer->classes[u].hbox == NULL)
			continue;
		else if(u == (size_t)view)
			gtk_widget_show(mixer->classes[u].hbox);
		else
			gtk_widget_hide(mixer->classes[u].hbox);
#endif
}
