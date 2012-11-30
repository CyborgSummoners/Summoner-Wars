CC = g++
CFLAGS = -Wall -c

# - tobbi libraryt majd ide kell hozzaadni.
# - fontos a sorrend: sfml-system kell az sfml-windownak, aki pedig kell
# az sfml-graphicsnak
# - sfml-graphics-nak kell freetype.
# - sfml-audio-nak kell libsndfile es openal.
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

RM = /bin/rm -f

SOURCES := $(wildcard src/*.cpp) $(wildcard src/compiler/*.cpp) src/compiler/summ.yy.cc src/compiler/parse.cc
SOURCES := $(filter-out src/konzoltest.cpp, $(SOURCES))
OBJECTS := $(patsubst src/%.cpp,obj/%.o,$(SOURCES))
OBJECTS := $(patsubst src/%.cc,obj/%.occ,$(OBJECTS))
COMPILER_SOURCES := $(wildcard src/compiler/*.cpp) src/compiler/summ.yy.cc src/compiler/parse.cc src/bytecode.cpp src/interpreter.cpp
COMPILER_OBJECTS := $(patsubst src/%.cpp,obj/%.o,$(COMPILER_SOURCES))
COMPILER_OBJECTS := $(patsubst src/%.cc,obj/%.occ,$(COMPILER_OBJECTS))
COMPILER_OBJECTS := $(filter-out obj/main.o,$(COMPILER_OBJECTS)) obj/compiler/main-compiler.o

PROG = sumwar

$(PROG): $(OBJECTS)
	$(CC) -o $(PROG) $(OBJECTS) $(LIBS)

obj/%.o: src/%.cpp | obj obj/compiler src/compiler/parse.cc
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

obj/%.occ: src/%.cc | obj obj/compiler
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)

obj/compiler/compiler.o: src/compiler/parse.cc

obj/compiler/main-compiler.o: src/main.cpp
	$(CC) $(CFLAGS) -o obj/compiler/main-compiler.o -DINTERPRETER_DEMO src/main.cpp

obj:
	mkdir -p obj

obj/compiler:
	mkdir -p obj/compiler

compiler-demo: $(COMPILER_OBJECTS) src/compiler/parse.cc
	$(CC) -Wall $(COMPILER_OBJECTS) -o compiler-demo

konzoltest: src/konzoltest.cpp src/terminal.cpp src/terminal.hpp
	$(CC) -Wall src/konzoltest.cpp src/terminal.cpp -o konzoltest

src/compiler/summ.yy.cc: src/compiler/summ.l
	flex -i -o src/compiler/summ.yy.cc src/compiler/summ.l

src/compiler/parse.cc: src/compiler/summ.y src/compiler/summ.yy.cc
	cd src/compiler && bisonc++ --filenames=summparse summ.y

all:
	$(PROG)

clean:
	$(RM) $(sort $(PROG) compiler-demo $(OBJECTS) $(COMPILER_OBJECTS) src/compiler/summparsebase.h src/compiler/parse.cc src/compiler/summ.yy.cc)
