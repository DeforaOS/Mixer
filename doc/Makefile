TARGETS	= mixer.1 mixer.html
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

mixer.1: mixer.xml
	./docbook.sh -P "$(PREFIX)" -- "mixer.1"

mixer.html: mixer.xml
	./docbook.sh -P "$(PREFIX)" -- "mixer.html"

clean:
	$(RM) -- $(mixer.1_OBJS) $(mixer.html_OBJS)
	./docbook.sh -c -P "$(PREFIX)" -- "mixer.html"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "mixer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "mixer.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "mixer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "mixer.html"

.PHONY: all clean distclean install uninstall
