# Compiler und Flags
CXX = g++
CC = gcc
CXXFLAGS_RELEASE = -std=c++17 -Wall -O2
CXXFLAGS_DEBUG = -std=c++17 -Wall -g
CFLAGS_RELEASE = -Wall -O2
CFLAGS_DEBUG = -Wall -g
CPPFLAGS = -I../civetweb/include
CIVETWEB_DEFINES = -DNO_SSL

# Benötigte Bibliotheken
LIBS = -lcurl -ljsoncpp -pthread -ldl

# CivetWeb (liegt im Ordner eine Ebene darüber)
CIVETWEB_SRC = ../civetweb/src/civetweb.c
CIVETWEB_OBJ = civetweb.o

# Zielname des Executables
TARGET_RELEASE = PiceaToLoxone
TARGET_DEBUG = PiceaToLoxoneD

# Quellcodedateien (Program.cpp ist deine main)
SOURCES = Program.cpp Config.cpp Logger.cpp PiceaAPI.cpp LoxoneAPI.cpp

# Automatisch generierte Objektdateien
OBJECTS = $(SOURCES:.cpp=.o) $(CIVETWEB_OBJ)

# Standardziel
all: release

# Release-Build (optimiert, ohne Debug-Informationen)
release: CXXFLAGS = $(CXXFLAGS_RELEASE)
release: CFLAGS = $(CFLAGS_RELEASE)
release: $(TARGET_RELEASE)

# Debug-Build (mit Debug-Informationen)
debug: CXXFLAGS = $(CXXFLAGS_DEBUG)
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(TARGET_DEBUG)

$(TARGET_RELEASE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_RELEASE) $(OBJECTS) $(LIBS)

$(TARGET_DEBUG): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_DEBUG) $(OBJECTS) $(LIBS)

# Regel zum Kompilieren der C++-Quellcodedateien
%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# CivetWeb separat als C kompilieren
$(CIVETWEB_OBJ): $(CIVETWEB_SRC)
	$(CC) $(CPPFLAGS) $(CIVETWEB_DEFINES) $(CFLAGS) -c $(CIVETWEB_SRC) -o $(CIVETWEB_OBJ)

# Clean-Ziel: Löscht alle Objektdateien und das Executable
clean:
	rm -f $(TARGET_RELEASE) $(TARGET_DEBUG) $(SOURCES:.cpp=.o) $(CIVETWEB_OBJ)

.PHONY: all clean release debug
