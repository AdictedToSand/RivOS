CXX := g++ # Change for system

CXX_FLAGS := -std=c++23 -fno-strict-aliasing
INPUT_FILES := src/main.cpp
OUTPUT_FILE := build/genfs

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
	@echo '--- RUNNING EXECUTABLE ---: ./genfs in/ out.txt'
	@./genfs in/ out.txt

mr: all build mv_exec run 
	@echo '--- RivFs dump ---'
	@make pd

pd: 
	python3 rivfsdata.py

hd:
	hexdump -C out.txt

.PHONY: all build
	
