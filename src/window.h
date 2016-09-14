/* $Id$ */
/* Copyright (c) 2015 Pierre Pronchery <khorben@defora.org> */
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

/* accessors */
gboolean mixerwindow_get_fullscreen(MixerWindow * mixer);
void mixerwindow_set_fullscreen(MixerWindow * mixer, gboolean fullscreen);

/* useful */
void mixerwindow_about(MixerWindow * mixer);
void mixerwindow_properties(MixerWindow * mixer);

void mixerwindow_show(MixerWindow * mixer);
void mixerwindow_show_all(MixerWindow * mixer);
void mixerwindow_show_class(MixerWindow * mixer, char const * name);

#endif /* !MIXER_WINDOW_H */
