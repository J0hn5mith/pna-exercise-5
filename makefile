OS := $(shell uname)
ifeq ($(OS), LINUX)
PLATFORM=1
else
PLATFORM=0
endif

ifeq ($(PLATFORM), 1)
CCOMPILER=gcc-5
CFLAGS=-c -Wall
else
CCOMPILER=fccpx
#CFLAGS=-Kfast,openmp -Nsrc 
CFLAGS=-c -Kfast,openmp
endif

LDFLAGS=-Kfast,openmp
COMMON_SOURCES=matrix.c utils.c
BUILD_SOURCES= main.c $(COMMON_SOURCES)
BUILD_OBJECTS=$(BUILD_SOURCES:.cpp=.o)
TEST_SOURCES= test.c $(COMMON_SOURCES)
TEST_OBJECTS=$(TEST_SOURCES:.c=.o)
EXECUTABLE=app


all: compile link run clean

compile: $(BUILD_SOURCES)

$(EXECUTABLE): $(BUILD_OBJECTS)
	$(CCOMPILER) $(LDFLAGS) $(BUILD_OBJECTS) -o $@

.cpp.o:
	$(CCOMPILER) $(CFLAGS) $< -o $@

link: $(EXECUTABLE)

run:
	./$(EXECUTABLE)

clean:
	-rm *.o
