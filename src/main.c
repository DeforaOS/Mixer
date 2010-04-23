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
#include "mixer.h"
#include "../config.h"
#define _(string) gettext(string)


/* constants */
#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif
#ifndef LOCALEDIR
# define LOCALEDIR	DATADIR "/locale"
#endif


/* functions */
/* usage */
static int _usage(void)
{
	fputs(_("Usage: mixer [-H|-V][-d device]\n"
"  -H	Show the classes next to each other\n"
"  -V	Show the classes on top of each other\n"
"  -d	The mixer device to use\n"), stderr);
	return 1;
}


/* main */
int main(int argc, char * argv[])
{
	int o;
	char const * device = NULL;
	MixerOrientation mo = MO_HORIZONTAL;
	Mixer * mixer;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	while((o = getopt(argc, argv, "HVd:")) != -1)
		switch(o)
		{
			case 'H':
				mo = MO_HORIZONTAL;
				break;
			case 'V':
				mo = MO_VERTICAL;
				break;
			case 'd':
				device = optarg;
				break;
			default:
				return _usage();
		}
	if(optind != argc)
		return _usage();
	gtk_init(&argc, &argv);
	if((mixer = mixer_new(device, mo)) == NULL)
		return 2;
	gtk_main();
	mixer_delete(mixer);
	return 0;
}
