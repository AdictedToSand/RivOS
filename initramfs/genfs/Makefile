CXX := g++ # Change for system

CXX_FLAGS := -std=c++23 -fno-strict-aliasing
INPUT_FILES := src/main.cpp
OUTPUT_FILE := build/genfs

OUTPUT_FS_FILE := out.txt
INP_FS_FILE := in/

all:
	$(CXX) --version
	-mkdir build

build: 
	$(CXX) $(CXX_FLAGS) $(INPUT_FILES) -o $(OUTPUT_FILE)

mv_exec: 
	mv $(OUTPUT_FILE) genfs

clean:
	rm -rf build
	rm genfs

run:
	@echo '--- RUNNING EXECUTABLE ---: ./genfs $(INP_FS_FILE) $(OUTPUT_FS_FILE)'
	@./genfs $(INP_FS_FILE) $(OUTPUT_FS_FILE) 

mr: all build mv_exec run 
	@echo '--- RivFs dump ---'
	@make pd

pd: 
	python3 rivfsdata.py $(OUTPUT_FS_FILE)

hd:
	hexdump -C $(OUTPUT_FS_FILE) 

.PHONY: all build
	
