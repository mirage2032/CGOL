NAME = CGOL

CC = g++
CFLAGS = -Wall
LIBS = -lSDL2main -lSDL2
SRCPATH = src
OBJPATH = build/obj
BINPATH = build/bin
OBJ = $(addprefix $(OBJPATH)/, cgol.o)

INSTALLPATH = /usr/bin/$(NAME)
EXENAME = $(NAME)
ifeq ($(OS),Windows_NT)
    EXENAME := $(NAME).exe
    INSTALLPATH := C:$(HOMEPATH)/$(NAME).exe
endif

.PHONY: clean

$(OBJPATH)/%.o: $(SRCPATH)/%.cpp
	@mkdir -p $(OBJPATH)
	$(CC) -c -o $@ $< $(CFLAGS)

all: build

build: $(OBJ)
	@mkdir -p $(BINPATH)
	$(CC) -o $(BINPATH)/$(EXENAME) $^ -O3 $(CFLAGS) $(LIBS)

build-debug: $(OBJ)
	@mkdir -p $(BINPATH)
	$(CC) -o $(BINPATH)/debug-$(EXENAME) $^ -g3 $(CFLAGS) $(LIBS)

clean:
	@rm -rf build

run: build $(BINPATH)/$(EXENAME)
	$(BINPATH)/$(EXENAME)

run-debug: build-debug $(BINPATH)/debug-$(EXENAME)
	$(BINPATH)/debug-$(EXENAME)

install: $(BINPATH)/$(EXENAME)
	cp $(BINPATH)/$(EXENAME) $(INSTALLPATH)

uninstall: $(BINPATH)/$(EXENAME)
	rm $(BINPATH)/$(EXENAME)