COMPILER = g++
FLAGS    = -Wall -std=c++11


all:
	mkdir -p build
	$(COMPILER) $(FLAGS) geometry.hpp -o build/geometry.o
	$(COMPILER) $(FLAGS) model.hpp    -o build/model.o  
	$(COMPILER) $(FLAGS) tgaimage.hpp -o build/image.o
	$(COMPILER) $(FLAGS) main.cpp     -o main  

clean:
	-rm -rf build
	-rm -f *.tga