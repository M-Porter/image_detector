default:
	c++ -std=c++11 $$(pkg-config --cflags --libs opencv4) main.cpp -o image-detector
	./image-detector "$(IN)"

clean:
	rm image-detector

example:
	c++ -std=c++11 $$(pkg-config --cflags --libs opencv4) main.cpp -o image-detector
	./image-detector "test_images/nhl_pens.png"
