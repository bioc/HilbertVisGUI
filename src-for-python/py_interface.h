#ifndef PY_INTERFACE_H
#define PY_INTERFACE_H

struct color {
   int red, green, blue;
};

typedef color ( *callback_handler_type ) ( long bin_start, long bin_size, void *pyfunc );
extern callback_handler_type callback_handler_global;

void displayHilbertVis( void * pyfunc );

#endif //PY_INTERFACE_H
