TARGETS	= $(OBJDIR)Mixer.pot $(OBJDIR)fr.mo
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

$(OBJDIR)Mixer.pot: POTFILES
	./gettext.sh -P "$(PREFIX)" -- "$(OBJDIR)Mixer.pot"

$(OBJDIR)fr.mo: $(OBJDIR)Mixer.pot fr.po
	./gettext.sh -P "$(PREFIX)" -- "$(OBJDIR)fr.mo"

clean:
	$(RM) -- $(Mixer.pot_OBJS) $(fr.mo_OBJS)
	./gettext.sh -c -P "$(PREFIX)" -- "Mixer.pot"
	./gettext.sh -c -P "$(PREFIX)" -- "fr.mo"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)fr.mo"

uninstall:
	./gettext.sh -P "$(DESTDIR)$(PREFIX)" -u -- "fr.mo"

.PHONY: all clean distclean install uninstall
