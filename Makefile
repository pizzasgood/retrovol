BINNAME = retrovol
OBJS = main.o config_settings.o retro_slider.o alsa_classes.o eggtrayicon.o


INCLUDE = `pkg-config --cflags gtk+-2.0`
LIBDIR  =  `pkg-config --libs gtk+-2.0`

LIBRARIES = -lasound

CFLAGS = -Wall -c

LFLAGS = -Wall

OPT = -O2
CC = gcc $(OPT)
PP = g++ $(OPT)

rvol: $(OBJS)
	$(PP) $(LFLAGS) $(OBJS) -o $(BINNAME) $(LIBDIR) $(LIBRARIES)

main.o: main.cpp main.h config_settings.h retro_slider.h alsa_classes.h eggtrayicon.h
	$(PP) main.cpp $(CFLAGS) $(INCLUDE)
	
eggtrayicon.o: eggtrayicon.h
	$(CC) eggtrayicon.c $(CFLAGS) $(INCLUDE)

config_settings.o: config_settings.cpp config_settings.h retro_slider.h alsa_classes.h
	$(PP) config_settings.cpp $(CFLAGS) $(INCLUDE)

retro_slider.o: retro_slider.cpp retro_slider.h
	$(PP) retro_slider.cpp $(CFLAGS) $(INCLUDE)

alsa_classes.o: alsa_classes.cpp alsa_classes.h
	$(PP) alsa_classes.cpp $(CFLAGS) $(INCLUDE)

all: rvol

clean:
	rm -f $(BINNAME) *.o

test: all
	./$(BINNAME)
