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
#include <System/object.h>
#include "Mixer/control.h"


/* MixerControlRadio */
/* private */
/* types */
typedef struct _MixerControlRadio
{
	MixerControlPlugin * plugin;

	GtkWidget * widget;
} MixerControlRadio;

struct _MixerControlPlugin
{
	GtkWidget * widget;

	GSList * group;
	MixerControlRadio * radios;
	size_t radios_cnt;
};


/* prototypes */
/* control */
static MixerControlPlugin * _radio_init(String const * type,
		va_list properties);
static void _radio_destroy(MixerControlPlugin * radio);

static GtkWidget * _radio_get_widget(MixerControlPlugin * radio);

static int _radio_set(MixerControlPlugin * radio, va_list properties);

/* callbacks */


/* public */
/* variables */
MixerControlDefinition control =
{
	NULL,
	"Radio buttons",
	NULL,
	_radio_init,
	_radio_destroy,
	_radio_get_widget,
	_radio_set
};


/* private */
/* functions */
/* radio_init */
static MixerControlPlugin * _radio_init(String const * type, va_list properties)
{
	MixerControlPlugin * radio;
	(void) type;

	if((radio = object_new(sizeof(*radio))) == NULL)
		return NULL;
	radio->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
	gtk_container_set_border_width(GTK_CONTAINER(radio->widget), 4);
	radio->group = NULL;
	radio->radios = NULL;
	radio->radios_cnt = 0;
	if(_radio_set(radio, properties) != 0)
	{
		_radio_destroy(radio);
		return NULL;
	}
	return radio;
}


/* radio_destroy */
static void _radio_destroy(MixerControlPlugin * radio)
{
	free(radio->radios);
	g_object_unref(radio->widget);
	object_delete(radio);
}


/* accessors */
/* radio_get_widget */
static GtkWidget * _radio_get_widget(MixerControlPlugin * radio)
{
	return radio->widget;
}


/* radio_set */
static int _set_label(MixerControlPlugin * radio, guint pos,
		String const * label);
static void _set_value(MixerControlPlugin * radio, guint value);

static int _radio_set(MixerControlPlugin * radio, va_list properties)
{
	String const * p;
	String const * s;
	unsigned int u;
	guint value;

	while((p = va_arg(properties, String const *)) != NULL)
	{
		if(string_compare(p, "value") == 0)
		{
			value = va_arg(properties, guint);
			_set_value(radio, value);
		}
		else if(sscanf(p, "label%u", &u) == 1)
		{
			s = va_arg(properties, String const *);
			if(_set_label(radio, u, s) != 0)
				return -1;
		}
		else
			/* FIXME report the error */
			return -1;
	}
	return 0;
}

static int _set_label(MixerControlPlugin * radio, guint pos,
		String const * label)
{
	guint i;
	MixerControlRadio * p;

	if(pos >= radio->radios_cnt)
	{
		if((p = realloc(radio->radios, sizeof(*p) * (pos + 1))) == NULL)
			return -1;
		radio->radios = p;
	}
	for(i = radio->radios_cnt; i < pos; i++)
	{
		radio->radios[i].plugin = radio;
		/* FIXME set the correct label */
		radio->radios[i].widget = gtk_radio_button_new_with_label(
				radio->group, label);
		/* FIXME implement the callback */
		if(radio->group == NULL)
			radio->group = gtk_radio_button_get_group(
					GTK_RADIO_BUTTON(
						radio->radios[i].widget));
		gtk_box_pack_start(GTK_BOX(radio->widget),
				radio->radios[i].widget, FALSE, TRUE, 0);
	}
	radio->radios_cnt = pos;
	gtk_widget_show_all(radio->widget);
	return 0;
}

static void _set_value(MixerControlPlugin * radio, guint value)
{
	/* FIXME implement */
}


/* callbacks */
/* FIXME implement */
