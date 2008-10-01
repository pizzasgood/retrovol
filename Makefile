BINNAME = retrovol
OBJS = main.o retro_slider.o alsa_classes.o


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

main.o: main.cpp retro_slider.h alsa_classes.h
	$(PP) main.cpp $(CFLAGS) $(INCLUDE)

retro_slider.o: retro_slider.c retro_slider.h
	$(CC) retro_slider.c $(CFLAGS) $(INCLUDE)

alsa_classes.o: alsa_classes.cpp alsa_classes.h
	$(PP) alsa_classes.cpp $(CFLAGS) $(INCLUDE)

all: rvol

clean:
	rm -f $(BINNAME) *.o

test: all
	./$(BINNAME)
