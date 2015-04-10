imagedir = $(pkgdatadir)/images
localedir = $(datadir)/locale
LDADD = @LIBINTL@

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DIMAGEDIR="\"$(imagedir)\""

AM_CPPFLAGS += $(ALSA_CFLAGS) $(GTK2_CFLAGS) $(GLIB_CFLAGS) $(X11_CFLAGS)

bin_PROGRAMS = retrovol

retrovol_SOURCES = alsa_classes.cpp alsa_classes.h config_settings.cpp config_settings.h config_window.cpp config_window.h eggtrayicon.c eggtrayicon.h main.cpp main.h retro_slider.cpp retro_slider.h gettext.h

retrovol_LDADD = $(ALSA_LIBS) $(GTK2_LIBS) $(GLIB_LIBS) $(X11_LIBS)

