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



#include <System/object.h>
#include "Mixer/control.h"


/* MixerControlSet */
/* private */
/* types */
typedef struct _MixerControlSet
{
	GtkWidget * widget;
} MixerControlSet;

struct _MixerControlPlugin
{
	GtkWidget * widget;

	MixerControlSet * sets;
	size_t sets_cnt;
};


/* prototypes */
static MixerControlPlugin * _set_init(String const * type, va_list properties);
static void _set_destroy(MixerControlPlugin * set);

static GtkWidget * _set_get_widget(MixerControlPlugin * set);

static int _set_set(MixerControlPlugin * set, va_list properties);


/* public */
/* variables */
MixerControlDefinition control =
{
	NULL,
	"Set of values",
	NULL,
	_set_init,
	_set_destroy,
	_set_get_widget,
	_set_set
};


/* private */
/* functions */
/* set_init */
static MixerControlPlugin * _set_init(String const * type, va_list properties)
{
	MixerControlPlugin * set;
	(void) type;

	if((set = object_new(sizeof(*set))) == NULL)
		return NULL;
	set->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
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
	g_object_unref(set->widget);
	object_delete(set);
}


/* accessors */
/* set_get_widget */
static GtkWidget * _set_get_widget(MixerControlPlugin * set)
{
	return set->widget;
}


/* set_set */
static void _set_value(MixerControlPlugin * set, guint value);

static int _set_set(MixerControlPlugin * set, va_list properties)
{
	String const * p;
	guint value;

	while((p = va_arg(properties, String const *)) != NULL)
	{
		if(string_compare(p, "value") == 0)
		{
			value = va_arg(properties, guint);
			_set_value(set, value);
		}
		else
			/* FIXME report the error */
			return -1;
	}
	return 0;
}

static void _set_value(MixerControlPlugin * set, guint value)
{
	/* FIXME implement */
}
