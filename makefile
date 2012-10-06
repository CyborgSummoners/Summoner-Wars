CC = g++
CFLAGS = -Wall -c

# - tobbi libraryt majd ide kell hozzaadni.
# - fontos a sorrend: sfml-system kell az sfml-windownak, aki pedig kell
# az sfml-graphicsnak
# - sfml-graphics-nak kell freetype.
# - sfml-audio-nak kell libsndfile es openal.
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

RM = /bin/rm -f

SOURCES := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)
OBJECTS := $(patsubst src/%.cpp,obj/%.o,$(SOURCES))
COMPILER_SOURCES := $(wildcard src/compiler/*.cpp)

IDEMO_SOURCES := $(COMPILER_SOURCES) src/interpreter.cpp src/bytecode.cpp src/main.cpp

PROG = sumwar

$(PROG): $(OBJECTS)
	$(CC) -o $(PROG) $(OBJECTS) $(LIBS)

obj/%.o: src/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $< $(LIBS)

compiler-demo: src/compiler/summ.yy.cc src/compiler/parse.cc $(IDEMO_SOURCES)
	$(CC) -Wall $(IDEMO_SOURCES) src/compiler/parse.cc src/compiler/summ.yy.cc -DINTERPRETER_DEMO -o compiler-demo

compiler-demo-debug: src/compiler/summ.yy.cc src/compiler/parse.cc $(IDEMO_SOURCES)
	$(CC) -Wall $(IDEMO_SOURCES) src/compiler/parse.cc src/compiler/summ.yy.cc  -DINTERPRETER_DEMO -DDEBUG_MACROS_ENABLED -o compiler-demo

src/compiler/summ.yy.cc: src/compiler/summ.l
	flex -i -o src/compiler/summ.yy.cc src/compiler/summ.l

src/compiler/parse.cc: src/compiler/summ.y
	cd src/compiler && bisonc++ --filenames=summparse summ.y

all:
	$(PROG)

clean:
	$(RM) $(PROG) compiler-demo $(OBJECTS) src/compiler/*.o src/compiler/summparsebase.h src/compiler/summparse.ih src/compiler/parse.cc src/compiler/summ.yy.cc
