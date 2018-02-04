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



#include <stdlib.h>
#include <System/object.h>
#include <Desktop.h>
#include "Mixer/control.h"


/* MixerControlSet */
/* private */
/* types */
typedef struct _MixerControlSet
{
	MixerControlPlugin * plugin;

	unsigned int value;

	GtkWidget * widget;
} MixerControlSet;

struct _MixerControlPlugin
{
	MixerControlPluginHelper * helper;

	GtkWidget * widget;

	MixerControlSet * sets;
	size_t sets_cnt;
};


/* prototypes */
/* control */
static MixerControlPlugin * _set_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties);
static void _set_destroy(MixerControlPlugin * set);

static int _set_get(MixerControlPlugin * set, va_list properties);

static String const * _set_get_type(MixerControlPlugin * set);
static GtkWidget * _set_get_widget(MixerControlPlugin * set);

static int _set_set(MixerControlPlugin * set, va_list properties);

/* callbacks */
static void _set_on_toggled(gpointer data);


/* public */
/* variables */
MixerControlDefinition control =
{
	NULL,
	"Set of values",
	NULL,
	_set_init,
	_set_destroy,
	_set_get,
	_set_get_type,
	_set_get_widget,
	_set_set
};


/* private */
/* functions */
/* set_init */
static MixerControlPlugin * _set_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties)
{
	MixerControlPlugin * set;
	(void) type;

	if((set = object_new(sizeof(*set))) == NULL)
		return NULL;
	set->helper = helper;
	set->widget = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(set->widget),
			GTK_BUTTONBOX_SPREAD);
	gtk_container_set_border_width(GTK_CONTAINER(set->widget), 4);
	set->sets = NULL;
	set->sets_cnt = 0;
	if(_set_set(set, properties) != 0)
	{
		_set_destroy(set);
		return NULL;
	}
	return set;
}


/* set_destroy */
static void _set_destroy(MixerControlPlugin * set)
{
	free(set->sets);
	object_delete(set);
}


/* accessors */
/* set_get */
static unsigned int _get_value(MixerControlPlugin * set);

static int _set_get(MixerControlPlugin * set, va_list properties)
{
	String const * p;
	unsigned int * u;

	while((p = va_arg(properties, String const *)) != NULL)
		if(string_compare(p, "value") == 0)
		{
			u = va_arg(properties, unsigned int *);
			*u = _get_value(set);
		}
		else
			/* FIXME implement the rest */
			return -1;
	return 0;
}

static unsigned int _get_value(MixerControlPlugin * set)
{
	unsigned int ret = 0;
	size_t i;

	for(i = 0; i < set->sets_cnt; i++)
		ret |= gtk_toggle_button_get_active(
				GTK_TOGGLE_BUTTON(set->sets[i].widget))
			? set->sets[i].value : 0;
	return ret;
}


/* set_get_type */
static String const * _set_get_type(MixerControlPlugin * set)
{
	return "set";
}


/* set_get_widget */
static GtkWidget * _set_get_widget(MixerControlPlugin * set)
{
	return set->widget;
}


/* set_set */
static int _set_label(MixerControlPlugin * set, unsigned int pos,
		String const * label);
static int _set_members(MixerControlPlugin * set, unsigned int cnt);
static int _set_value(MixerControlPlugin * set, unsigned int value);
static int _set_value_pos(MixerControlPlugin * set, unsigned int pos,
		unsigned int value);

static int _set_set(MixerControlPlugin * set, va_list properties)
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
			if(_set_label(set, u, s) != 0)
				return -1;
		}
		else if(string_compare(p, "members") == 0)
		{
			u = va_arg(properties, unsigned int);
			if(_set_members(set, u) != 0)
				return -1;
		}
		else if(string_compare(p, "value") == 0)
		{
			value = va_arg(properties, unsigned int);
			if(_set_value(set, value) != 0)
				return -1;
		}
		else if(sscanf(p, "value%u", &u) == 1)
		{
			value = va_arg(properties, unsigned int);
			if(_set_value_pos(set, u, value) != 0)
				return -1;
		}
		else
			/* FIXME report the error */
			return -1;
	}
	return 0;
}

static int _set_label(MixerControlPlugin * set, unsigned int pos,
		String const * label)
{
	if(pos >= set->sets_cnt)
		return -1;
	gtk_button_set_label(GTK_BUTTON(set->sets[pos].widget), label);
	return 0;
}

static int _set_members(MixerControlPlugin * set, unsigned int cnt)
{
	size_t i;
	MixerControlSet * p;

	/* delete buttons as required */
	if(set->sets_cnt >= cnt)
	{
		for(i = cnt; i < set->sets_cnt; i++)
			g_object_unref(set->sets[i].widget);
		set->sets_cnt = cnt;
		return 0;
	}
	if((p = realloc(set->sets, sizeof(*p) * cnt)) == NULL)
		return -1;
	set->sets = p;
	for(i = set->sets_cnt; i < cnt; i++)
	{
		p = &set->sets[i];
		p->plugin = set;
		p->value = 0;
		p->widget = gtk_check_button_new();
		gtk_widget_set_sensitive(p->widget, FALSE);
		g_signal_connect_swapped(p->widget, "toggled", G_CALLBACK(
					_set_on_toggled), set);
		gtk_container_add(GTK_CONTAINER(set->widget), p->widget);
	}
	set->sets_cnt = cnt;
	gtk_widget_show_all(set->widget);
	return 0;
}

static int _set_value(MixerControlPlugin * set, unsigned int value)
{
	size_t i;

	for(i = 0; i < set->sets_cnt; i++)
		gtk_toggle_button_set_active(
				GTK_TOGGLE_BUTTON(set->sets[i].widget),
				(set->sets[i].value & value) ? TRUE : FALSE);
	return 0;
}

static int _set_value_pos(MixerControlPlugin * set, unsigned int pos,
		unsigned int value)
{
	if(pos >= set->sets_cnt)
		return -1;
	set->sets[pos].value = value;
	gtk_widget_set_sensitive(set->sets[pos].widget, (value != 0)
			? TRUE : FALSE);
	return 0;
}


/* callbacks */
/* set_on_toggled */
static void _set_on_toggled(gpointer data)
{
	MixerControlPlugin * set = data;

	set->helper->mixercontrol_set(set->helper->control);
}
