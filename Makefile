# commented out and removed from recipe so that no strange error can happen.
# move these back when I want to use the new gcc again.
#CC=$(CC) \
#CXX=$(CXX) \
#LIBRARY_PATH=\usr\local\lib64:$$LIBRARY_PATH \
#-DCMAKE_EXE_LINKER_FLAGS="-Wl,-rpath,/usr/local/lib64" \
	
# Use gcc-15 for the latest features.
#CC := /usr/local/bin/gcc
#CXX := /usr/local/bin/g++

.PHONY: config-dbg
config-dbg : 
	cmake -DCMAKE_BUILD_TYPE=Debug -S ./src -B ./bin
	

.PHONY: config
config: 
	cmake -S ./src -B ./bin

.PHONY: build
build : 
	cmake --build ./bin

.PHONY: test
test :
	ctest --test-dir ./bin
