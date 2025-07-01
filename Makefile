# Use gcc-15 for the latest features.
CC := /usr/local/bin/gcc
CXX := /usr/local/bin/g++

.PHONY: config-dbg
config-dbg : 
	CC=$(CC) \
	CXX=$(CXX) \
	LIBRARY_PATH=\usr\local\lib64:$$LIBRARY_PATH \
	cmake -DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath,/usr/local/lib64" \
	-S ./src -B ./bin

.PHONY: config
config: 
	CC=$(CC) \
	CXX=$(CXX) \
	LIBRARY_PATH=\usr\local\lib64:$$LIBRARY_PATH \
	-DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath,/usr/local/lib64" \
	cmake \
	-DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath,/usr/local/lib64" \
	-S ./src -B ./bin


.PHONY: build
build : 
	cmake --build ./bin

.PHONY: test
test :
	ctest --test-dir ./bin
