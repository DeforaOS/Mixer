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

	unsigned int value;

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

static String const * _radio_get_type(MixerControlPlugin * radio);
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
	_radio_get_type,
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
	radio->widget = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(radio->widget),
			GTK_BUTTONBOX_SPREAD);
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
/* radio_get_type */
static String const * _radio_get_type(MixerControlPlugin * radio)
{
	return "radio";
}


/* radio_get_widget */
static GtkWidget * _radio_get_widget(MixerControlPlugin * radio)
{
	return radio->widget;
}


/* radio_set */
static int _set_label(MixerControlPlugin * radio, unsigned int pos,
		String const * label);
static int _set_members(MixerControlPlugin * radio, unsigned int cnt);
static int _set_value(MixerControlPlugin * radio, unsigned int value);
static int _set_value_pos(MixerControlPlugin * radio, unsigned int pos,
		unsigned int value);

static int _radio_set(MixerControlPlugin * radio, va_list properties)
{
	String const * p;
	String const * s;
	unsigned int u;
	unsigned int value;

	while((p = va_arg(properties, String const *)) != NULL)
	{
		if(sscanf(p, "label%u", &u) == 1)
		{
			s = va_arg(properties, String const *);
			if(_set_label(radio, u, s) != 0)
				return -1;
		}
		else if(string_compare(p, "members") == 0)
		{
			u = va_arg(properties, unsigned int);
			if(_set_members(radio, u) != 0)
				return -1;
		}
		else if(string_compare(p, "value") == 0)
		{
			value = va_arg(properties, unsigned int);
			if(_set_value(radio, value) != 0)
				return -1;
		}
		else if(sscanf(p, "value%u", &u) == 1)
		{
			value = va_arg(properties, unsigned int);
			if(_set_value_pos(radio, u, value) != 0)
				return -1;
		}
		else
			/* FIXME report the error */
			return -1;
	}
	return 0;
}

static int _set_label(MixerControlPlugin * radio, unsigned int pos,
		String const * label)
{
	if(pos >= radio->radios_cnt)
		return -1;
	gtk_button_set_label(GTK_BUTTON(radio->radios[pos].widget), label);
	return 0;
}

static int _set_members(MixerControlPlugin * radio, unsigned int cnt)
{
	size_t i;
	MixerControlRadio * p;

	/* delete buttons as required */
	if(radio->radios_cnt >= cnt)
	{
		for(i = cnt; i < radio->radios_cnt; i++)
			g_object_unref(radio->radios[i].widget);
		radio->radios_cnt = cnt;
		return 0;
	}
	if((p = realloc(radio->radios, sizeof(*p) * cnt)) == NULL)
		return -1;
	radio->radios = p;
	for(i = radio->radios_cnt; i < cnt; i++)
	{
		p = &radio->radios[i];
		p->plugin = radio;
		p->value = 0;
		p->widget = gtk_radio_button_new(radio->group);
		gtk_widget_set_sensitive(p->widget, FALSE);
		/* FIXME implement the callback */
		if(radio->group == NULL)
			radio->group = gtk_radio_button_get_group(
					GTK_RADIO_BUTTON(p->widget));
		gtk_container_add(GTK_CONTAINER(radio->widget), p->widget);
	}
	radio->radios_cnt = cnt;
	gtk_widget_show_all(radio->widget);
	return 0;
}

static int _set_value(MixerControlPlugin * radio, unsigned int value)
{
	/* FIXME implement */
	return 0;
}

static int _set_value_pos(MixerControlPlugin * radio, unsigned int pos,
		unsigned int value)
{
	if(pos >= radio->radios_cnt)
		return -1;
	radio->radios[pos].value = value;
	gtk_widget_set_sensitive(radio->radios[pos].widget, (value != 0)
			? TRUE : FALSE);
	return 0;
}


/* callbacks */
/* FIXME implement */
