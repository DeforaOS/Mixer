DeforaOS Mixer
==============

About Mixer
-----------

Mixer is a volume control application for the DeforaOS desktop.

Documentation
-------------

A manual page for the executable installed is available in the `doc` folder. It
is written in the DocBook-XML format, and needs libxslt and DocBook-XSL to be
installed to be converted to either HTML or man file format.

Compiling Mixer
----------------

Mixer depends on the following components:

 * DeforaOS libDesktop
 * DocBook-XSL (for the manual pages)

With GCC, this should then be enough to compile Mixer:

    $ make

On some systems, the Makefiles shipped can be re-generated accordingly thanks to
the DeforaOS configure tool.

The compilation process supports a number of options, such as PREFIX and DESTDIR
for packaging and portability, or OBJDIR for compilation outside of the source
tree.

For instance, to install (or package) Mixer in a different location:

    $ make DESTDIR="/staging/directory" PREFIX="/another/prefix" install

Distributing Panel
------------------

DeforaOS Mixer is subject to the terms of the 2-clause BSD license. Please see
the `COPYING` file for more information.
