/* $Id$ */
/* Copyright (c) 2018 Pierre Pronchery <khorben@defora.org> */
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


/* MixerControlTemplate */
/* private */
/* types */
struct _MixerControlPlugin
{
	MixerControlPluginHelper * helper;
};


/* prototypes */
/* control */
static MixerControlPlugin * _template_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties);
static void _template_destroy(MixerControlPlugin * template);

static int _template_get(MixerControlPlugin * template, va_list properties);

static String const * _template_get_type(MixerControlPlugin * template);
static GtkWidget * _template_get_widget(MixerControlPlugin * template);

static int _template_set(MixerControlPlugin * template, va_list properties);


/* public */
/* variables */
MixerControlDefinition control =
{
	NULL,
	"Template control",
	NULL,
	_template_init,
	_template_destroy,
	_template_get,
	_template_get_type,
	_template_get_widget,
	_template_set
};


/* private */
/* functions */
/* template_init */
static MixerControlPlugin * _template_init(MixerControlPluginHelper * helper,
		String const * type, va_list properties)
{
	MixerControlPlugin * template;

	if((template = object_new(sizeof(*template))) == NULL)
		return NULL;
	template->helper = helper;
	/* TODO implement */
	return template;
}


/* template_destroy */
static void _template_destroy(MixerControlPlugin * template)
{
	/* TODO implement */
	object_delete(template);
}


/* accessors */
/* template_get */
static int _template_get(MixerControlPlugin * template, va_list properties)
{
	/* TODO implement */
	return -1;
}


/* template_get_type */
static String const * _template_get_type(MixerControlPlugin * template)
{
	/* TODO implement */
	return "template";
}


/* template_get_widget */
static GtkWidget * _template_get_widget(MixerControlPlugin * template)
{
	/* TODO implement */
	return NULL;
}


/* template_set */
static int _template_set(MixerControlPlugin * template, va_list properties)
{
	/* TODO implement */
	return -1;
}
