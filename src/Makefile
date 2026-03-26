CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Isrc
LDFLAGS  = -lpthread

# Target tanpa viewer
all: bin/voxelizer

bin/voxelizer: src/main.cpp src/*.hpp | bin
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp $(LDFLAGS)

# Target dengan viewer (butuh GLFW)
viewer: bin/voxelizer_viewer

bin/voxelizer_viewer: src/main.cpp src/*.hpp | bin
	$(CXX) $(CXXFLAGS) -DENABLE_VIEWER -o $@ src/main.cpp \
		$(LDFLAGS) -lGL -lGLU -lglfw

bin:
	mkdir -p bin

clean:
	rm -rf bin

# Windows (MinGW) - tanpa viewer
windows:
	$(CXX) $(CXXFLAGS) -o bin/voxelizer.exe src/main.cpp -lpthread -lws2_32

# Windows dengan viewer
windows-viewer:
	$(CXX) $(CXXFLAGS) -DENABLE_VIEWER -o bin/voxelizer_viewer.exe src/main.cpp \
		-lpthread -lws2_32 -lGL -lGLU -lglfw3

.PHONY: all viewer clean windows windows-viewer
