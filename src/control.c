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



#ifdef DEBUG
# include <stdio.h>
#endif
#include <System/object.h>
#include <System/plugin.h>
#include <Desktop.h>
#include "Mixer/control.h"
#include "mixer.h"
#include "control.h"
#include "../config.h"


/* private */
/* types */
struct _MixerControl
{
	Mixer * mixer;

	MixerControlPluginHelper helper;

	String * id;
	Plugin * handle;
	MixerControlDefinition * definition;
	MixerControlPlugin * plugin;
	GtkWidget * widget;

	/* widgets */
	GtkWidget * frame;
	GtkWidget * icon;
	GtkWidget * name;
};


/* prototypes */
/* helper */
static int _mixercontrol_helper_set(MixerControl * control);


/* public */
/* functions */
/* mixercontrol_new */
MixerControl * mixercontrol_new(Mixer * mixer, String const * id,
		String const * icon, String const * name,
		String const * type, ...)
{
	MixerControl * control;
	va_list ap;
	GtkWidget * hbox;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(\"%s\", \"%s\", \"%s\")\n", __func__, id,
			name, type);
#endif
	if((control = object_new(sizeof(*control))) == NULL)
		return NULL;
	control->mixer = mixer;
	control->helper.control = control;
	control->helper.mixercontrol_set = _mixercontrol_helper_set;
	control->id = string_new(id);
	control->handle = plugin_new(LIBDIR, PACKAGE, "controls", type);
	control->definition = NULL;
	control->plugin = NULL;
	control->frame = NULL;
	va_start(ap, type);
	if(control->id == NULL
			|| control->handle == NULL
			|| (control->definition = plugin_lookup(control->handle,
					"control")) == NULL
			|| control->definition->init == NULL
			|| control->definition->destroy == NULL
			|| (control->plugin = control->definition->init(
					&control->helper, type, ap)) == NULL
			|| control->definition->get_widget == NULL
			|| (control->widget = control->definition->get_widget(
					control->plugin)) == NULL)
	{
		va_end(ap);
#ifdef DEBUG
		fprintf(stderr, "DEBUG: %s() => NULL %p %p\n", __func__,
				control->handle, control->definition);
#endif
		mixercontrol_delete(control);
		return NULL;
	}
	va_end(ap);
	/* widgets */
	control->frame = gtk_frame_new(NULL);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
	control->icon = gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_MENU);
	gtk_box_pack_start(GTK_BOX(hbox), control->icon, FALSE, TRUE, 0);
	control->name = gtk_label_new(name);
#if GTK_CHECK_VERSION(3, 0, 0)
	gtk_widget_set_halign(control->name, GTK_ALIGN_START);
#else
	gtk_misc_set_alignment(GTK_MISC(control->name), 0.0, 0.5);
#endif
	gtk_box_pack_start(GTK_BOX(hbox), control->name, TRUE, TRUE, 0);
	gtk_frame_set_label_widget(GTK_FRAME(control->frame), hbox);
	gtk_container_add(GTK_CONTAINER(control->frame), control->widget);
	return control;
}


/* mixercontrol_delete */
void mixercontrol_delete(MixerControl * control)
{
	if(control->plugin != NULL && control->definition->destroy != NULL)
		control->definition->destroy(control->plugin);
	if(control->handle != NULL)
		plugin_delete(control->handle);
	if(control->id != NULL)
		string_delete(control->id);
	object_delete(control);
}


/* accessors */
/* mixercontrol_get */
int mixercontrol_get(MixerControl * control, ...)
{
	int ret;
	va_list ap;

#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s()\n", __func__);
#endif
	if(control->definition->get == NULL)
		return -1;
	va_start(ap, control);
	ret = control->definition->get(control->plugin, ap);
	va_end(ap);
	return ret;
}


/* mixercontrol_get_id */
String const * mixercontrol_get_id(MixerControl * control)
{
	return control->id;
}


/* mixercontrol_get_type */
String const * mixercontrol_get_type(MixerControl * control)
{
	return (control->definition->get_type != NULL)
		? control->definition->get_type(control->plugin) : NULL;
}


/* mixercontrol_get_widget */
GtkWidget * mixercontrol_get_widget(MixerControl * control)
{
	return control->frame;
}


/* mixercontrol_set */
int mixercontrol_set(MixerControl * control, ...)
{
	int ret;
	va_list ap;

	if(control->definition->set == NULL)
		return -1;
	va_start(ap, control);
	ret = control->definition->set(control->plugin, ap);
	va_end(ap);
	return ret;
}


/* mixercontrol_set_icon */
void mixercontrol_set_icon(MixerControl * control, String const * icon)
{
	gtk_image_set_from_icon_name(GTK_IMAGE(control->icon), icon,
			GTK_ICON_SIZE_MENU);
}


/* private */
/* mixercontrol_helper_set */
static int _mixercontrol_helper_set(MixerControl * control)
{
#ifdef DEBUG
	fprintf(stderr, "DEBUG: %s(%p)\n", __func__, (void *)control);
#endif
	return mixer_set(control->mixer, control);
}
