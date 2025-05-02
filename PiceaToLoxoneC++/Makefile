# Compiler und Flags
CXX = g++
CC = gcc

# Release- und Debug-Flags mit korrektem Include für civetweb + NO_SSL
CXXFLAGS_RELEASE = -std=c++17 -Wall -O2 -I../civetweb/include -DNO_SSL
CXXFLAGS_DEBUG = -std=c++17 -Wall -g -I../civetweb/include -DNO_SSL
CFLAGS = -Wall -O2 -I../civetweb/include -DNO_SSL

# Benötigte Bibliotheken
LIBS = -lcurl -ljsoncpp -pthread

# Zielname des Executables
TARGET_RELEASE = PiceaToLoxone
TARGET_DEBUG = PiceaToLoxoneD

# Quellcodedateien (Program.cpp ist dein main)
CPP_SOURCES = Program.cpp Config.cpp PiceaAPI.cpp LoxoneAPI.cpp
C_SOURCES = ../civetweb/src/civetweb.c

# Automatisch generierte Objektdateien
OBJECTS = $(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)

# Standardziel
all: release

# Release-Build (optimiert, ohne Debug-Informationen)
release: CXXFLAGS = $(CXXFLAGS_RELEASE)
release: $(TARGET_RELEASE)

# Debug-Build (mit Debug-Informationen)
debug: CXXFLAGS = $(CXXFLAGS_DEBUG)
debug: $(TARGET_DEBUG)

$(TARGET_RELEASE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_RELEASE) $(OBJECTS) $(LIBS)

$(TARGET_DEBUG): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET_DEBUG) $(OBJECTS) $(LIBS)

# Regel zum Kompilieren der C++-Quellcodedateien
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regel zum Kompilieren der C-Quellcodedateien (z.B. civetweb)
../civetweb/src/%.o: ../civetweb/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean-Ziel: Löscht alle Objektdateien und das Executable
clean:
	rm -f $(TARGET_RELEASE) $(TARGET_DEBUG) $(OBJECTS)
