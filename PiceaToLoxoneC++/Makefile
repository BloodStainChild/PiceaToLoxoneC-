# Compiler und Flags
CXX = g++
CXXFLAGS_RELEASE = -std=c++17 -Wall -O2
CXXFLAGS_DEBUG = -std=c++17 -Wall -g

# Benötigte Bibliotheken
LIBS = -lcurl -ljsoncpp -pthread

# Zielname des Executables
TARGET_RELEASE = PiceaToLoxone
TARGET_DEBUG = PiceaToLoxoneD

# Quellcodedateien (Program.cpp ist deine main)
SOURCES = Program.cpp Config.cpp PiceaAPI.cpp LoxoneAPI.cpp

# Automatisch generierte Objektdateien
OBJECTS = $(SOURCES:.cpp=.o)

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

# Regel zum Kompilieren der Quellcodedateien
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean-Ziel: Löscht alle Objektdateien und das Executable
clean:
	rm -f $(TARGET_RELEASE) $(TARGET_DEBUG) $(OBJECTS)

.PHONY: all clean release debug
