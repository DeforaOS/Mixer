/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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



#include <unistd.h>
#include <stdio.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "window.h"
#include "../config.h"
#define _(string) gettext(string)


/* constants */
#ifndef PROGNAME
# define PROGNAME	"mixer"
#endif
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* prototypes */
static int _mixer(char const * device, MixerLayout layout, gboolean embedded);

static int _error(char const * message, int ret);
static int _usage(void);


/* functions */
/* mixer */
static int _mixer(char const * device, MixerLayout layout, gboolean embedded)
{
	MixerWindow * mixer;

	if((mixer = mixerwindow_new(device, layout, embedded)) == NULL)
		return 2;
	gtk_main();
	mixerwindow_delete(mixer);
	return 0;
}


/* error */
static int _error(char const * message, int ret)
{
	fputs(PROGNAME ": ", stderr);
	perror(message);
	return ret;
}


/* usage */
static int _usage(void)
{
	fprintf(stderr, _("Usage: %s [-H|-T|-V][-d device][-x]\n"
"  -H	Show the classes next to each other\n"
"  -T	Show the classes in separate tabs\n"
"  -V	Show the classes on top of each other\n"
"  -d	The mixer device to use\n"
"  -x	Enable embedded mode\n"), PROGNAME);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * device = NULL;
	MixerLayout layout = ML_TABBED;
	gboolean embedded = FALSE;

	if(setlocale(LC_ALL, "") == NULL)
		_error("setlocale", 1);
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	gtk_init(&argc, &argv);
	while((o = getopt(argc, argv, "HTVd:x")) != -1)
		switch(o)
		{
			case 'H':
				layout = ML_HORIZONTAL;
				break;
			case 'T':
				layout = ML_TABBED;
				break;
			case 'V':
				layout = ML_VERTICAL;
				break;
			case 'd':
				device = optarg;
				break;
			case 'x':
				embedded = TRUE;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	return (_mixer(device, layout, embedded) == 0) ? 0 : 2;
}
