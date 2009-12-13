/* $Id$ */
/* Copyright (c) 2009 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Desktop Mixer */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#ifndef MIXER_MIXER_H
# define MIXER_MIXER_H

# include <gtk/gtk.h> /* XXX should not be necessary */


/* Mixer */
/* types */
typedef struct _Mixer Mixer;


/* functions */
Mixer * mixer_new(char const * device);
void mixer_delete(Mixer * mixer);

/* accessors */
int mixer_set_enum(Mixer * mixer, GtkWidget * widget);
int mixer_set_value(Mixer * mixer, GtkWidget * widget, gdouble value);

/* useful */
void mixer_about(Mixer * mixer);
void mixer_properties(Mixer * mixer);

void mixer_show(Mixer * mixer, int view);
void mixer_show_all(Mixer * mixer);
void mixer_show_class(Mixer * mixer, char const * name);

#endif /* !MIXER_MIXER_H */
