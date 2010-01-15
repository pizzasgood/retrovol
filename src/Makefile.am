imagedir = $(pkgdatadir)/images
localedir = $(datadir)/locale
LDADD = @LIBINTL@

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DIMAGEDIR="\"$(imagedir)\""

INCLUDES = `pkg-config --cflags gtk+-2.0`

bin_PROGRAMS = retrovol

retrovol_SOURCES = alsa_classes.cpp alsa_classes.h config_settings.cpp config_settings.h config_window.cpp config_window.h eggtrayicon.c eggtrayicon.h main.cpp main.h retro_slider.cpp retro_slider.h gettext.h

#retrovol_LDADD = -lasound `pkg-config --libs gtk+-2.0`
retrovol_LDADD = -lasound -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lpangocairo-1.0 -lpango-1.0 -lcairo -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0

