#include <stdio.h>
#include <set>
#include "window.h"
#include "colorizers.h"

#include "py_interface.h"

callback_handler_type callback_handler_global = NULL;

class PyCallbackColorizer : public DataColorizer {
  public:
   PyCallbackColorizer( const void * pyfunc_, long length_, const char * name_ );
   virtual Glib::ustring get_name( void ) const;
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
  protected:
   const Glib::ustring name;
   const void * pyfunc;
   long length;
};

class MainWindowForPython : public MainWindow {
  public:
   MainWindowForPython( std::vector< DataColorizer * > * dataCols, bool portrait );
   ~MainWindowForPython( );
  protected:
   virtual void on_canvasClicked( GdkEventButton * ev, long binLo, long binHi );
   virtual void on_hide( void );
};


PyCallbackColorizer::PyCallbackColorizer( const void * pyfunc_, long length_, const char * name_ )
  : name( name_ ), pyfunc( pyfunc_ ), length( length_ )
{
}

Glib::ustring PyCallbackColorizer::get_name( void ) const 
{
   return name;
}

long PyCallbackColorizer::get_length( void ) const
{
   return length;
}

Gdk::Color PyCallbackColorizer::get_bin_color( long bin_start, long bin_size ) const
{
   color col = callback_handler_global( bin_start, bin_size, const_cast< void* >( pyfunc ) );
   Gdk::Color col2;
   col2.set_rgb_p( col.red/256., col.green/256., col.blue/256. );
   return col2;
}

// The command line arguments are needed to initialize GTK+
char ** argv;
int argc;

void displayHilbertVis( void * pyfunc )
{
   printf( "FoO\n" );
   Gtk::Main kit(argc, argv);
   PyCallbackColorizer colorizer ( pyfunc, 100000, "Test1" );
   std::vector< DataColorizer * > dataCols;
   dataCols.push_back( &colorizer );   
   MainWindow win( &dataCols, false, false, false );
   Gtk::Main::run( win );
}


