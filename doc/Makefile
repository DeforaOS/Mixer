TARGETS	= $(OBJDIR)mixer.1 $(OBJDIR)mixer.html
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

$(OBJDIR)mixer.1: mixer.xml
	./docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)mixer.1"

$(OBJDIR)mixer.html: manual.css.xml mixer.css.xml mixer.xml
	./docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)mixer.html"

clean:
	$(RM) -- $(mixer.1_OBJS) $(mixer.html_OBJS)
	./docbook.sh -c -P "$(PREFIX)" -- "mixer.1"
	./docbook.sh -c -P "$(PREFIX)" -- "mixer.html"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)mixer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)mixer.html"

uninstall:
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "mixer.1"
	./docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "mixer.html"

.PHONY: all clean distclean install uninstall
