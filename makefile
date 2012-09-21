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

COMPILER_DIR = src/compiler/

$(PROG): $(OBJS)
	$(CC) $(OBJS) -o $(PROG) $(LIBS)

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)

compiler-demo: $(COMPILER_DIR)summ.yy.cc $(COMPILER_DIR)parse.cc src/main.cpp
	$(CC) -Wall src/compiler/main.cpp $(COMPILER_DIR)compiler.cpp src/bytecode.cpp $(COMPILER_DIR)parse.cc $(COMPILER_DIR)summ.yy.cc -o compiler-demo

$(COMPILER_DIR)summ.yy.cc: $(COMPILER_DIR)summ.l
	flex -i -o $(COMPILER_DIR)summ.yy.cc $(COMPILER_DIR)summ.l

$(COMPILER_DIR)parse.cc: src/compiler/summ.y
	cd $(COMPILER_DIR) && bisonc++ --filenames=summparse summ.y

all:
	$(PROG)

clean:
	$(RM) $(PROG) compiler-demo $(OBJS) $(COMPILER_DIR)*.o $(COMPILER_DIR)summparsebase.h $(COMPILER_DIR)summparse.ih $(COMPILER_DIR)parse.cc $(COMPILER_DIR)summ.yy.cc
