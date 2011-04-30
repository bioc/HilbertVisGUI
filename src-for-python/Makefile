hilbertvis.so: colorizers.o display.o ruler.o window.o hilbert.o py_interface.o hilbertvis.o
	g++ -shared -o $@ $^ `python-config --libs` `pkg-config gtkmm-2.4 --libs`

colorizers.o: colorizers.cc
	g++ -c -o $@ $< `pkg-config gtkmm-2.4 --cflags` -fPIC

display.o: display.cc
	g++ -c -o $@ $< `pkg-config gtkmm-2.4 --cflags` -fPIC

ruler.o: ruler.cc
	g++ -c -o $@ $< `pkg-config gtkmm-2.4 --cflags` -fPIC

window.o: window.cc
	g++ -c -o $@ $< `pkg-config gtkmm-2.4 --cflags` -fPIC

hilbertvis.cpp: hilbertvis.pyx
	cython --cplus hilbertvis.pyx

hilbertvis.o: hilbertvis.cpp
	g++ -c -o $@ $< `python-config --cflags` -w  -fPIC

py_interface.o: py_interface.cc
	g++ -c -o $@ $< `python-config --cflags` `pkg-config gtkmm-2.4 --cflags`  -fPIC

clean:
	rm -f hilbertvis.so colorizers.o display.o ruler.o window.o hilbert.cpp hilbertvis.o py_interface.o
