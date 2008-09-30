BINNAME = retrovol
OBJS = main.o retro_slider.o


#INCLUDE = -I/usr/X11R7/include/
INCLUDE = `pkg-config --cflags gtk+-2.0`
#LIBDIR  =  -L/usr/local/lib -L/usr/X11R7/lib
LIBDIR  =  `pkg-config --libs gtk+-2.0`

LIBRARIES = 

CFLAGS = -Wall -c

LFLAGS = -Wall

OPT = -O2
CC = gcc $(OPT)
PP = g++ $(OPT)

rvol: $(OBJS)
	$(PP) $(LFLAGS) $(OBJS) -o $(BINNAME) $(LIBDIR) $(LIBRARIES)

main.o: main.cpp retro_slider.c
	$(PP) main.cpp $(CFLAGS) $(INCLUDE)

retro_slider.o: retro_slider.c
	$(CC) retro_slider.c $(CFLAGS) $(INCLUDE)

all: rvol

clean:
	rm -f $(BINNAME) *.o

test: all
	./$(BINNAME)
