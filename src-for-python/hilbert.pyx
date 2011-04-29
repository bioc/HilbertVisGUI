import sys, traceback

cdef extern from "py_interface.h":

   cdef cppclass color:
      int red, green, blue

   ctypedef color ( *callback_handler_type ) ( long bin_start, long bin_size, void *pyfunc )
   cdef callback_handler_type callback_handler_global
   
   void displayHilbertVis( void * pyfunc )
   

cdef color callback_handler( long bin_start, long bin_size, void * pyfunc ):
   cdef int red, green, blue
   cdef color c
   try:
      res = (<object> pyfunc)( bin_start, bin_size )   
   except:
      sys.stderr.write( "Exception occured in callback function %s:\n" % 
         (<object> pyfunc).__name__ )
      sys.stderr.write( traceback.format_exc() )
      red, green, blue = 0, 0, 0
   try:
      red, green, blue = res
   except:
      sys.stderr.write( "Exception occured in processing return value of callback function %s:\n" % 
         (<object> pyfunc).__name__ )     
      sys.stderr.write( traceback.format_exc() )          
      red, green, blue = 0, 0, 0
   c.red = red
   c.green = green
   c.blue = blue
   return c

def simple_coloring( bin_start, bin_size ):
   return ((bin_start//100) % 256, (bin_start//30) % 256, (bin_start//10) % 256 )

def test():
   print "Flump"
   displayHilbertVis( <void*> simple_coloring )
   
callback_handler_global = callback_handler

test()   
