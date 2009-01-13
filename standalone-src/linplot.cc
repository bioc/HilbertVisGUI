#include "linplot.h"
#include "ruler.h"

LinPlot::LinPlot( DataVector * dv_, std::string name_, long int left_, long int right_, double scale_ ) 
 : dv( dv_ ), name( name_ ), left( left_ ), right( right_ ), scale( scale_ )
{
}

bool LinPlot::on_expose_event( GdkEventExpose * event )
{
   Glib::RefPtr< Gdk::GC > gc = Gdk::GC::create( get_window() );
   int width, height;
   get_window()->get_size( width, height );

   Gdk::Color col;
   col.set( "white" );
   get_window()->get_colormap()->alloc_color( col );
   gc->set_foreground( col );
   get_window()->draw_rectangle( gc, true, 0, 0, width-1, height-1 );   
   
   col.set_grey_p( .2 );
   get_window()->get_colormap()->alloc_color( col );
   gc->set_foreground( col );
   long bin_size = round( (right-left) / width );
   if( bin_size < 1 )
      bin_size = 1;
   for( int x = 0; x < width; x++ ) {
      long int xx = round( left + double( x ) / width * (right-left) );
      get_window()->draw_line( gc, x, height/2, x, 
         height/2 - dv->get_bin_value( xx, bin_size ) / scale * height/2 );   
   }
   
   col.set( "blue" );
   get_window()->get_colormap()->alloc_color( col );
   gc->set_foreground( col );
   get_window()->draw_line( gc, 0, height/2, width-1, height/2 );   
   
   Glib::RefPtr< Pango::Layout > pl = create_pango_layout( 
      int2strB( left ) );
   int pl_width, pl_height;
   pl->get_pixel_size( pl_width, pl_height );
   get_window()->draw_layout( gc, 3, height/2 - pl_height, pl );
   
   pl = create_pango_layout( int2strB( right ) );
   pl->get_pixel_size( pl_width, pl_height );
   get_window()->draw_layout( gc, width - pl_width - 3, height/2 - pl_height, pl );

   pl = create_pango_layout( double2strB( scale ) );
   pl->get_pixel_size( pl_width, pl_height );
   get_window()->draw_layout( gc, 3, 3, pl );

   pl = create_pango_layout( double2strB( -scale ) );
   pl->get_pixel_size( pl_width, pl_height );
   get_window()->draw_layout( gc, 3, height - pl_height - 3, pl );

   pl = create_pango_layout( name );
   pl->get_pixel_size( pl_width, pl_height );
   get_window()->draw_layout( gc, width - pl_width - 3, height - pl_height - 3, pl );

   return true;
}

LinPlotWindow::LinPlotWindow( DataVector * dv, std::string name, long int middle, long int width, double scale )
 : lp( dv, name, middle - width/2, middle + width/2, scale )
{
  set_default_size( 280, 200 );
  add( lp );
  show_all_children( );
}

