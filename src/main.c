/* $Id$ */
/* Copyright (c) 2010 Pierre Pronchery <khorben@defora.org> */
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
