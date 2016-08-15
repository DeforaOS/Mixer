DeforaOS Mixer
==============

About Mixer
-----------

Mixer is a volume control application for the DeforaOS desktop.


Compiling Mixer
----------------

Mixer depends on the following components:

 * DeforaOS libDesktop
 * DocBook-XSL (for the manual pages)

With GCC, this should then be enough to compile Mixer:

    $ make

To install (or package) Mixer in a different location:

    $ make DESTDIR="/staging/directory" PREFIX="/another/prefix" install


Documentation
-------------

Manual pages for each of the executables installed are available in the `doc`
folder. They are written in the DocBook-XML format, and need libxslt and
DocBook-XSL to be installed to be converted to either HTML or man file format.
