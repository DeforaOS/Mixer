DeforaOS Mixer
==============

About Mixer
-----------

Mixer is a volume control application for the DeforaOS desktop.

Mixer is part of the DeforaOS Project, found at https://www.defora.org/.

Documentation
-------------

A manual page for the executable installed is available in the `doc` folder. It
is written in the DocBook-XML format, and needs libxslt and DocBook-XSL to be
installed to be converted to either HTML or man file format.

Compiling Mixer
----------------

Mixer depends on the following components:

 * Gtk+ 2.4 or later, or Gtk+ 3.0 or later
 * DeforaOS libDesktop
 * an implementation of `make`
 * gettext (libintl) for translations
 * DocBook-XSL (for the manual pages)

With these installed, the following command should be enough to compile Mixer on
most systems:

    $ make

The following command will then install Mixer:

    $ make install

To install (or package) Mixer in a different location:

    $ make clean
    $ make PREFIX="/another/prefix" install

Mixer also supports `DESTDIR`, to be installed in a staging directory; for
instance:

    $ make DESTDIR="/staging/directory" PREFIX="/another/prefix" install

On some systems, the Makefiles shipped can be re-generated accordingly thanks to
the DeforaOS configure tool.

The compilation process supports a number of options, such as PREFIX and DESTDIR
for packaging and portability, or OBJDIR for compilation outside of the source
tree.

Distributing Mixer
------------------

DeforaOS Mixer is subject to the terms of the 2-clause BSD license. Please see
the `COPYING` file for more information.
