CC = g++
CFLAGS = -Wall -c

# - tobbi libraryt majd ide kell hozzaadni.
# - fontos a sorrend: sfml-system kell az sfml-windownak, aki pedig kell 
# az sfml-graphicsnak
# - sfml-graphics-nak kell freetype.
# - sfml-audio-nak kell libsndfile es openal.
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

RM = /bin/rm -f

#Ehhez a sorhoz kell hozzaadni a forrasokbol keszult objectfileok helyet. 
OBJS = obj/main.o

PROG = sumwar

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LIBS)

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

all:
	$(PROG)

clean:
	$(RM) $(PROG) $(OBJS)
