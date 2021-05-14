default:
	c++ -std=c++11 $$(pkg-config --cflags --libs opencv4) main.cpp -o image-detector
	./image-detector

clean:
	rm image-detector
