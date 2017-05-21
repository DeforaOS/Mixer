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



#include "Mixer/control.h"
#include <System/object.h>


/* MixerControlMute */
/* private */
/* types */
struct _MixerControlPlugin
{
	GtkWidget * widget;

	GtkWidget * mute;
};


/* prototypes */
/* control */
static MixerControlPlugin * _mute_init(String const * type, va_list properties);
static void _mute_destroy(MixerControlPlugin * mute);

static GtkWidget * _mute_get_widget(MixerControlPlugin * mute);

static int _mute_set(MixerControlPlugin * mute, va_list properties);

/* callbacks */
static void _mute_on_toggled(gpointer data);


/* public */
/* variables */
MixerControlDefinition control =
{
	NULL,
	"Mute button",
	NULL,
	_mute_init,
	_mute_destroy,
	_mute_get_widget,
	_mute_set
};


/* private */
/* functions */
/* mute_init */
static MixerControlPlugin * _mute_init(String const * type, va_list properties)
{
	MixerControlPlugin * mute;
	(void) type;

	if((mute = object_new(sizeof(*mute))) == NULL)
		return NULL;
	mute->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	mute->mute = gtk_toggle_button_new();
	g_signal_connect(mute->mute, "toggled", G_CALLBACK(_mute_on_toggled),
			mute);
	gtk_box_pack_start(GTK_BOX(mute->widget), mute->mute, FALSE, TRUE, 0);
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
	g_object_unref(mute->widget);
	object_delete(mute);
}


/* accessors */
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
			gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(mute->mute), value);
		}
		else
			/* FIXME report the error */
			return -1;
	}
	return 0;
}


/* callbacks */
/* mute_on_toggled */
static void _mute_on_toggled(gpointer data)
{
	MixerControlPlugin * mute = data;

	/* FIXME implement */
}
