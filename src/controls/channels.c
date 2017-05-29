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



#include <stdlib.h>
#ifdef DEBUG
# include <stdio.h>
#endif
#include <libintl.h>
#include <System/object.h>
#include "Mixer/control.h"
#define _(string) gettext(string)


/* MixerControlChannels */
/* private */
/* types */
typedef struct _MixerControlChannel
{
	MixerControlPlugin * plugin;

	GtkWidget * widget;
} MixerControlChannel;

struct _MixerControlPlugin
{
	MixerControlPluginHelper * helper;

	GtkWidget * widget;

	unsigned int delta;

	/* channels */
	GtkWidget * hbox;
	MixerControlChannel * channels;
	size_t channels_cnt;

	GtkWidget * bbox;

	/* bind */
	GtkWidget * bind;
	GtkWidget * bind_image;

	/* mute */
	GtkWidget * mute;
#if !GTK_CHECK_VERSION(3, 0, 0)
	GtkWidget * mute_image;
#endif

	gboolean signal;
};


/* prototypes */
/* control */
static MixerControlPlugin * _channels_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties);
static void _channels_destroy(MixerControlPlugin * channels);

static int _channels_get(MixerControlPlugin * channels, va_list properties);

static String const * _channels_get_type(MixerControlPlugin * channels);
static GtkWidget * _channels_get_widget(MixerControlPlugin * channels);

static int _channels_set(MixerControlPlugin * channels, va_list properties);

/* callbacks */
static void _channels_on_bind_toggled(gpointer data);

static void _channels_on_changed(GtkWidget * widget, gpointer data);

#if GTK_CHECK_VERSION(3, 0, 0)
static void _channels_on_mute_notify_active(gpointer data);
#else
static void _channels_on_mute_toggled(gpointer data);
#endif


/* public */
/* variables */
MixerControlDefinition control =
{
	NULL,
	"Channels",
	NULL,
	_channels_init,
	_channels_destroy,
	_channels_get,
	_channels_get_type,
	_channels_get_widget,
	_channels_set
};


/* private */
/* functions */
/* channels_init */
static MixerControlPlugin * _channels_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties)
{
	MixerControlPlugin * channels;
	GtkWidget * hbox;
	GtkWidget * widget;
#if !GTK_CHECK_VERSION(3, 14, 0)
	GtkWidget * align;
#endif
	(void) type;

	if((channels = object_new(sizeof(*channels))) == NULL)
		return NULL;
	channels->helper = helper;
	channels->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_container_set_border_width(GTK_CONTAINER(channels->widget), 4);
	channels->delta = 1;
	channels->channels = NULL;
	channels->channels_cnt = 0;
	channels->hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
#if GTK_CHECK_VERSION(3, 14, 0)
	gtk_widget_set_halign(channels->hbox, GTK_ALIGN_CENTER);
	gtk_box_pack_start(GTK_BOX(channels->widget), channels->hbox, TRUE,
			TRUE, 0);
#else
	align = gtk_alignment_new(0.5, 0.5, 0.0, 1.0);
	gtk_container_add(GTK_CONTAINER(align), channels->hbox);
	gtk_box_pack_start(GTK_BOX(channels->widget), align, TRUE, TRUE, 0);
#endif
	channels->bbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_box_set_homogeneous(GTK_BOX(channels->bbox), TRUE);
	/* bind */
	channels->bind = gtk_toggle_button_new();
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	channels->bind_image = gtk_image_new_from_icon_name("gtk-connect",
			GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), channels->bind_image, FALSE, TRUE, 0);
	widget = gtk_label_new(_("Bind"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_widget_show_all(hbox);
	gtk_container_add(GTK_CONTAINER(channels->bind), hbox);
	gtk_widget_set_no_show_all(channels->bind, TRUE);
	g_signal_connect_swapped(channels->bind, "toggled", G_CALLBACK(
				_channels_on_bind_toggled), channels);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(hbox), channels->bind, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(channels->bbox), hbox, FALSE, TRUE, 0);
	/* mute */
#if GTK_CHECK_VERSION(3, 0, 0)
	channels->mute = gtk_switch_new();
	g_signal_connect_swapped(channels->mute, "notify::active",
			G_CALLBACK(_channels_on_mute_notify_active), channels);
#else
	channels->mute = gtk_toggle_button_new();
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	channels->mute_image = gtk_image_new_from_icon_name(
			"audio-volume-muted", GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), channels->mute_image, FALSE, TRUE, 0);
	widget = gtk_label_new(_("Mute"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_widget_show_all(hbox);
	gtk_container_add(GTK_CONTAINER(channels->mute), hbox);
	g_signal_connect_swapped(channels->mute, "toggled", G_CALLBACK(
				_channels_on_mute_toggled), channels);
#endif
	gtk_widget_set_no_show_all(channels->mute, TRUE);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(hbox), channels->mute, TRUE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(channels->bbox), hbox, FALSE, TRUE, 0);
	gtk_box_pack_end(GTK_BOX(channels->widget), channels->bbox, FALSE, TRUE,
			0);
	channels->signal = FALSE;
	if(_channels_set(channels, properties) != 0)
	{
		_channels_destroy(channels);
		return NULL;
	}
	return channels;
}


/* channels_destroy */
static void _channels_destroy(MixerControlPlugin * channels)
{
	g_object_unref(channels->widget);
	object_delete(channels);
}


/* accessors */
static int _channels_get(MixerControlPlugin * channels, va_list properties)
{
	String const * p;
	gboolean * b;
	double * value;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	while((p = va_arg(properties, String const *)) != NULL)
		if(string_compare(p, "bind") == 0)
		{
			b = va_arg(properties, gboolean *);
			*b = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
						channels->bind));
		}
		else if(string_compare(p, "value") == 0)
		{
			if(channels->channels_cnt == 0)
				return -1;
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
							channels->bind))
					== FALSE)
				return -1;
			value = va_arg(properties, double *);
			*value = gtk_range_get_value(GTK_RANGE(
						channels->channels[0].widget));
			*value = (*value * 255.0) / 100.0;
		}
		else if(sscanf(p, "value%zu", &i) == 1)
		{
			if(i >= channels->channels_cnt)
				return -1;
			value = va_arg(properties, double *);
			*value = gtk_range_get_value(GTK_RANGE(
						channels->channels[i].widget));
			*value = (*value * 255.0) / 100.0;
		}
		/* FIXME implement the rest */
		else
			return -1;
	return 0;
}


/* channels_get_type */
static String const * _channels_get_type(MixerControlPlugin * channels)
{
	return "channels";
}


/* channels_get_widget */
static GtkWidget * _channels_get_widget(MixerControlPlugin * channels)
{
	return channels->widget;
}


/* channels_set */
static void _set_bind(MixerControlPlugin * channels, gboolean bind);
static int _set_channels(MixerControlPlugin * channels, size_t cnt);
static void _set_delta(MixerControlPlugin * channels, unsigned int delta);
static void _set_mute(MixerControlPlugin * channels, gboolean mute);
static void _set_value(MixerControlPlugin * channels, gdouble value);
static void _set_value_channel(MixerControlPlugin * channels,
		size_t channel, gdouble value);

static int _channels_set(MixerControlPlugin * channels, va_list properties)
{
	String const * p;
	gboolean b;
	GtkSizeGroup * group;
	unsigned int u;
	gdouble value = 0.0;
	size_t zu;

	while((p = va_arg(properties, String const *)) != NULL)
	{
		if(string_compare(p, "bind") == 0)
		{
			b = va_arg(properties, gboolean);
			_set_bind(channels, b);
		}
		else if(string_compare(p, "channels") == 0)
		{
			zu = va_arg(properties, size_t);
			if(_set_channels(channels, zu) != 0)
				return -1;
		}
		else if(string_compare(p, "delta") == 0)
		{
			u = va_arg(properties, unsigned int);
			_set_delta(channels, u);
		}
		else if(string_compare(p, "mute") == 0)
		{
			b = va_arg(properties, gboolean);
			_set_mute(channels, b);
		}
		else if(string_compare(p, "show-bind") == 0)
		{
			value = va_arg(properties, gboolean);
			value ? gtk_widget_show(channels->bind)
				: gtk_widget_hide(channels->bind);
		}
		else if(string_compare(p, "show-mute") == 0)
		{
			value = va_arg(properties, gboolean);
			value ? gtk_widget_show(channels->mute)
				: gtk_widget_hide(channels->mute);
		}
		else if(string_compare(p, "value") == 0)
		{
			value = va_arg(properties, gdouble);
			_set_value(channels, value);
		}
		else if(sscanf(p, "value%zu", &zu) == 1)
		{
			value = va_arg(properties, gdouble);
			_set_value_channel(channels, zu, value);
		}
		else if(string_compare(p, "vgroup") == 0)
		{
			group = va_arg(properties, GtkSizeGroup *);
			gtk_size_group_add_widget(group, channels->bbox);
		}
		else
			/* FIXME report the error */
			return -1;
	}
	return 0;
}

static void _set_bind(MixerControlPlugin * channels, gboolean bind)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(channels->bind), bind);
}

static int _set_channels(MixerControlPlugin * channels, size_t cnt)
{
	size_t i;
	MixerControlChannel * p;
	gboolean signal = channels->signal;

	/* delete channels as required */
	if(channels->channels_cnt >= cnt)
	{
		for(i = cnt; i < channels->channels_cnt; i++)
			g_object_unref(channels->channels[i].widget);
		if((channels->channels_cnt = cnt) < 2)
			gtk_widget_hide(channels->bind);
		else
			gtk_widget_show(channels->bind);
		return 0;
	}
	if((p = realloc(channels->channels, sizeof(*p) * cnt)) == NULL)
		return -1;
	channels->channels = p;
	channels->signal = TRUE;
	for(i = channels->channels_cnt; i < cnt; i++)
	{
		p = &channels->channels[i];
		p->plugin = channels;
		p->widget = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL,
				0.0, 100.0, channels->delta);
		gtk_range_set_inverted(GTK_RANGE(p->widget), TRUE);
		gtk_range_set_value(GTK_RANGE(p->widget), 0.0);
		g_signal_connect(p->widget, "value-changed", G_CALLBACK(
					_channels_on_changed), p);
		gtk_box_pack_start(GTK_BOX(channels->hbox), p->widget, TRUE,
				TRUE, 0);
	}
	channels->signal = signal;
	if((channels->channels_cnt = cnt) < 2)
		gtk_widget_hide(channels->bind);
	else
		gtk_widget_show(channels->bind);
	return 0;
}

static void _set_delta(MixerControlPlugin * channels, unsigned int delta)
{
	size_t i;

	channels->delta = delta;
	for(i = 0; i < channels->channels_cnt; i++)
		gtk_range_set_increments(
				GTK_RANGE(channels->channels[i].widget), delta,
				delta);
}

static void _set_mute(MixerControlPlugin * channels, gboolean mute)
{
# if GTK_CHECK_VERSION(3, 0, 0)
	gtk_switch_set_active(GTK_SWITCH(channels->mute), mute);
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(channels->mute), mute);
#endif
}

static void _set_value(MixerControlPlugin * channels, gdouble value)
{
	size_t i;

	for(i = 0; i < channels->channels_cnt; i++)
		_set_value_channel(channels, i, value);
}

static void _set_value_channel(MixerControlPlugin * channels,
		size_t channel, gdouble value)
{
	gboolean signal = channels->signal;

	channels->signal = TRUE;
	if(channel < channels->channels_cnt)
		gtk_range_set_value(
				GTK_RANGE(channels->channels[channel].widget),
				(value * 100.0) / 255.0);
	channels->signal = signal;
}


/* callbacks */
/* channels_on_bind_toggled */
static void _channels_on_bind_toggled(gpointer data)
{
	MixerControlPlugin * channels = data;
	gboolean active;

	active = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(channels->bind));
	gtk_image_set_from_icon_name(GTK_IMAGE(channels->bind_image),
			active ? "gtk-connect" : "gtk-disconnect",
			GTK_ICON_SIZE_BUTTON);
}


/* channels_on_changed */
static void _channels_on_changed(GtkWidget * widget, gpointer data)
{
	MixerControlChannel * channel = data;
	MixerControlPlugin * channels = channel->plugin;
	gdouble value;
	size_t i;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(channels->signal == TRUE)
		return;
	channels->signal = TRUE;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(channels->bind)))
	{
		value = gtk_range_get_value(GTK_RANGE(widget));
		if(channels->bind)
			for(i = 0; i < channels->channels_cnt; i++)
			{
				widget = channels->channels[i].widget;
				gtk_range_set_value(GTK_RANGE(widget), value);
			}
	}
	channels->helper->mixercontrol_set(channels->helper->control);
	channels->signal = FALSE;
}


#if GTK_CHECK_VERSION(3, 0, 0)
/* channels_on_mute_notify_active */
static void _channels_on_mute_notify_active(gpointer data)
{
	MixerControlPlugin * channels = data;

	channels->helper->mixercontrol_set(channels->helper->control);
}
#else
/* channels_on_mute_toggled */
static void _channels_on_mute_toggled(gpointer data)
{
	MixerControlPlugin * channels = data;
	gboolean active;

	active = gtk_toggle_button_get_active(
			GTK_TOGGLE_BUTTON(channels->mute));
	gtk_image_set_from_icon_name(GTK_IMAGE(channels->mute_image),
			active ? "audio-volume-muted" : "audio-volume-high",
			GTK_ICON_SIZE_BUTTON);
	channels->helper->mixercontrol_set(channels->helper->control);
}
#endif
