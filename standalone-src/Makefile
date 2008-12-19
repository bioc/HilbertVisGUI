GTKMM_CFLAGS = `pkg-config gtkmm-2.4 --cflags`
GTKMM_LIBS   = `pkg-config gtkmm-2.4 --libs`
GLIB_CFLAGS  = `pkg-config glib-2.0 --cflags`
GLIB_LIBS    = `pkg-config glib-2.0 --libs`
LIBZ         = -lz

RSRC=../src/

CFLAGS = -O2 -iquote ${RSRC} -g -DHILBERTVIS_VERSION=\"$(HILBERTVIS_VERSION)\"

CC = g++ ${CFLAGS} 

HILBERTVIS_VERSION = `cat VERSION`

all: hilbert conv2bwc

hilbert: stand_alone_interface.o  colorizers.o \
   display.o ruler.o window.o simple_regex_pp.o data_loading.o	
	${CC} -o hilbert stand_alone_interface.o colorizers.o \
	display.o ruler.o window.o simple_regex_pp.o data_loading.o \
	${LIBZ} ${GTKMM_LIBS}
	
conv2bwc: conv2bwc.o simple_regex_pp.o	
	${CC} -o conv2bwc conv2bwc.o simple_regex_pp.o \
	${LIBZ} ${GLIB_LIBS}
	
conv2bwc.o: conv2bwc.cc simple_regex_pp.h
	${CC} -c conv2bwc.cc ${GLIB_CFLAGS}

simple_regex_pp.o: simple_regex_pp.cc
	${CC} -c $<

data_loading.o: data_loading.cc step_vector.h
	${CC} -c $<

stand_alone_interface.o: stand_alone_interface.cc  \
   ${RSRC}/colorizers.h ${RSRC}/display.h ${RSRC}/ruler.h \
   ${RSRC}/window.h data_loading.h step_vector.h main_icon.c VERSION
	${CC} -c stand_alone_interface.cc ${GTKMM_CFLAGS}
	
%.o: ${RSRC}/%.cc
	${CC} -c -o $@ $< ${GTKMM_CFLAGS} 	

main_icon.c: main_icon.png
	gdk-pixbuf-csource main_icon.png --name main_icon --raw >main_icon.c
	
clean:
	rm -f *.o hilbert conv2bwc main_icon.c
