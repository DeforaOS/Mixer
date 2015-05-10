/* $Id$ */
/* Copyright (c) 2015 Pierre Pronchery <khorben@defora.org> */
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



#ifndef MIXER_WINDOW_H
# define MIXER_WINDOW_H

# include "mixer.h"


/* MixerWindow */
/* public */
/* types */
typedef struct _MixerWindow MixerWindow;


/* functions */
MixerWindow * mixerwindow_new(char const * device, MixerLayout layout,
		gboolean embedded);
void mixerwindow_delete(MixerWindow * mixer);

/* useful */
void mixerwindow_about(MixerWindow * mixer);
void mixerwindow_properties(MixerWindow * mixer);

void mixerwindow_show(MixerWindow * mixer);
void mixerwindow_show_all(MixerWindow * mixer);
void mixerwindow_show_class(MixerWindow * mixer, char const * name);

#endif /* !MIXER_WINDOW_H */
