/* $Id$ */
/* Copyright (c) 2017-2018 Pierre Pronchery <khorben@defora.org> */
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



#include <libintl.h>
#include <System/object.h>
#include "Mixer/control.h"
#define _(string) gettext(string)


/* MixerControlMute */
/* private */
/* types */
struct _MixerControlPlugin
{
	MixerControlPluginHelper * helper;

	GtkWidget * widget;

	GtkWidget * mute;
#if !GTK_CHECK_VERSION(3, 0, 0)
	GtkWidget * mute_image;
#endif
};


/* prototypes */
/* control */
static MixerControlPlugin * _mute_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties);
static void _mute_destroy(MixerControlPlugin * mute);

static int _mute_get(MixerControlPlugin * mute, va_list properties);

static String const * _mute_get_type(MixerControlPlugin * mute);
static GtkWidget * _mute_get_widget(MixerControlPlugin * mute);

static int _mute_set(MixerControlPlugin * mute, va_list properties);

/* callbacks */
#if GTK_CHECK_VERSION(3, 0, 0)
static void _mute_on_notify_active(gpointer data);
#else
static void _mute_on_toggled(gpointer data);
#endif


/* public */
/* variables */
MixerControlDefinition control =
{
	NULL,
	"Mute button",
	NULL,
	_mute_init,
	_mute_destroy,
	_mute_get,
	_mute_get_type,
	_mute_get_widget,
	_mute_set
};


/* private */
/* functions */
/* mute_init */
static MixerControlPlugin * _mute_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties)
{
	MixerControlPlugin * mute;
#if !GTK_CHECK_VERSION(3, 0, 0)
	GtkWidget * hbox;
	GtkWidget * widget;
#endif
	(void) type;

	if((mute = object_new(sizeof(*mute))) == NULL)
		return NULL;
	mute->helper = helper;
	mute->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_container_set_border_width(GTK_CONTAINER(mute->widget), 4);
#if GTK_CHECK_VERSION(3, 0, 0)
	mute->mute = gtk_switch_new();
	g_signal_connect_swapped(mute->mute, "notify::active",
			G_CALLBACK(_mute_on_notify_active), mute);
#else
	mute->mute = gtk_toggle_button_new();
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	mute->mute_image = gtk_image_new_from_icon_name("audio-volume-high",
			GTK_ICON_SIZE_BUTTON);
	gtk_box_pack_start(GTK_BOX(hbox), mute->mute_image, FALSE, TRUE, 0);
	widget = gtk_label_new(_("Mute"));
	gtk_box_pack_start(GTK_BOX(hbox), widget, TRUE, TRUE, 0);
	gtk_container_add(GTK_CONTAINER(mute->mute), hbox);
	g_signal_connect_swapped(mute->mute, "toggled",
			G_CALLBACK(_mute_on_toggled), mute);
#endif
	gtk_box_pack_end(GTK_BOX(mute->widget), mute->mute, FALSE, TRUE, 0);
	if(_mute_set(mute, properties) != 0)
	{
		_mute_destroy(mute);
		return NULL;
	}
	return mute;
}


/* mute_destroy */
static void _mute_destroy(MixerControlPlugin * mute)
{
	object_delete(mute);
}


/* accessors */
/* mute_get */
static int _mute_get(MixerControlPlugin * mute, va_list properties)
{
	String const * p;
	gboolean * b;

	while((p = va_arg(properties, String const *)) != NULL)
		if(string_compare(p, "value") == 0)
		{
			b = va_arg(properties, gboolean *);
# if GTK_CHECK_VERSION(3, 0, 0)
			*b = gtk_switch_get_active(GTK_SWITCH(mute->mute))
				? 0 : 1; /* XXX assumes 1 is "off" */
# else
			*b = gtk_toggle_button_get_active(
					GTK_TOGGLE_BUTTON(mute->mute))
				? 1 : 0; /* XXX assumes 0 is "off" */
# endif
		}
		/* FIXME implement the rest */
		else
			return -1;
	return 0;
}


/* mute_get_type */
static String const * _mute_get_type(MixerControlPlugin * mute)
{
	(void) mute;

	return "mute";
}


/* mute_get_widget */
static GtkWidget * _mute_get_widget(MixerControlPlugin * mute)
{
	return mute->widget;
}


/* mute_set */
static int _mute_set(MixerControlPlugin * mute, va_list properties)
{
	String const * p;
	gboolean value;

	while((p = va_arg(properties, String const *)) != NULL)
	{
		if(string_compare(p, "value") == 0)
		{
			value = va_arg(properties, gboolean);
#if GTK_CHECK_VERSION(3, 0, 0)
			gtk_switch_set_active(GTK_SWITCH(mute->mute), value);
#else
			gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(mute->mute), value);
#endif
		}
		else
			/* FIXME report the error */
			return -1;
	}
	return 0;
}


/* callbacks */
#if GTK_CHECK_VERSION(3, 0, 0)
/* mute_on_notify_active */
static void _mute_on_notify_active(gpointer data)
{
	MixerControlPlugin * mute = data;

	mute->helper->mixercontrol_set(mute->helper->control);
}
#else
/* mute_on_toggled */
static void _mute_on_toggled(gpointer data)
{
	MixerControlPlugin * mute = data;

	gboolean active;

	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mute->mute));
	gtk_image_set_from_icon_name(GTK_IMAGE(mute->mute_image),
			active ? "audio-volume-muted" : "audio-volume-high",
			GTK_ICON_SIZE_BUTTON);
	mute->helper->mixercontrol_set(mute->helper->control);
}
#endif
