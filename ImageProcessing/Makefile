CXX = g++
CXXFLAGS = -O3 -Wall -std=c++14
test : main.cxx
	$(CXX) $(CXXFLAGS) $^ -o $@.out
clean:
	rm -f *.out
convertPPM:
	convert simple_grayscale.ppm simple_grayscale.jpg
	convert simple_sorbel_vertical.ppm simple_sorbel_vertical.jpg
	convert simple_sorbel_horizontal.ppm simple_sorbel_horizontal.jpg
	convert simple_sorbel.ppm simple_sorbel.jpg
	convert expansion.ppm expansion.jpg
	convert reduction.ppm reduction.jpg
	convert simple_maxmin.ppm simple_maxmin.jpg
	convert simple_blackwhite.ppm simple_blackwhite.jpg
	convert average_grayscale.ppm average_grayscale.jpg
	convert average_maxmin.ppm average_maxmin.jpg
	convert average_blackwhite.ppm average_blackwhite.jpg
	convert gauss_grayscale.ppm gauss_grayscale.jpg
	convert gauss_maxmin.ppm gauss_maxmin.jpg
	convert gauss_blackwhite.ppm gauss_blackwhite.jpg
	convert sort_grayscale.ppm sort_grayscale.jpg
	convert sort_maxmin.ppm sort_maxmin.jpg
	convert sort_blackwhite.ppm sort_blackwhite.jpg
	rm -f *.ppm
