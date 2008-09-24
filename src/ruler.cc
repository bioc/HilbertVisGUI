#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <string>
#include "ruler.h"

std::string int2strB( int a )
{
   static const int nbuf = 300;
   char buf[nbuf];
   snprintf( buf, nbuf, "%d", a );
   std::string s;
   for( int i = strlen(buf)-1; i >= 0; i-- ) {
      s = buf[i] + s;
      if( (strlen(buf)-i) % 3 == 0 && i != 0 )
         s = ',' + s;
   }
   return s;
}

Ruler::Ruler( InvalidableAdjustment * adj_ )
{
   adj = adj_;
   set_size_request( 50, 30 );
   adj->signal_changed().connect( sigc::mem_fun( *this, &Ruler::on_adj_changed) );
   adj->signal_value_changed().connect( sigc::mem_fun( *this, &Ruler::on_adj_changed) );
}

bool Ruler::on_expose_event( GdkEventExpose* event )
{
   Glib::RefPtr< Gdk::GC > gc = Gdk::GC::create( get_window() );
   int width, height;
   get_window()->get_size( width, height );

   Gdk::Color col;
   col.set( "lightgreen" );
   get_window()->get_colormap()->alloc_color( col );
   gc->set_foreground( col );
   get_window()->draw_rectangle( gc, true, 0, 0, width-1, height-1 );   

   col.set( "black" );
   get_window()->get_colormap()->alloc_color( col );
   gc->set_foreground( col );
   get_window()->draw_line( gc, 0, 0, 0, height-1 );   
   get_window()->draw_line( gc, width-1, 0, width-1, height-1 );   
   get_window()->draw_line( gc, 0, height-1, width-1, height-1 );   

   if( adj->is_valid() ) {
      col.set( "red" );
      get_window()->get_colormap()->alloc_color( col );
      gc->set_foreground( col );
      int xl = (int) round( (double) width * ( adj->get_value() - adj->get_lower() ) / 
         ( adj->get_upper() - adj->get_lower() ) );
      int xr = (int) round( (double) width * adj->get_page_size()  / ( adj->get_upper() - adj->get_lower() ) );
      if( xr < 1 )
         xr = 1;
      get_window()->draw_rectangle( gc, true, xl, 0, xr, height-1 );   
   }
   
   col.set( "black" );
   get_window()->get_colormap()->alloc_color( col );
   gc->set_foreground( col );

   Glib::RefPtr< Pango::Layout > pl = create_pango_layout( 
      int2strB( (int) round( adj->get_lower() ) ) );
   int pl_width, pl_height;
   pl->get_pixel_size( pl_width, pl_height );
   get_window()->draw_layout( gc, 3, height - pl_height, pl );

   pl = create_pango_layout( int2strB( (int) round( adj->get_upper() ) ) );
   pl->get_pixel_size( pl_width, pl_height );
   get_window()->draw_layout( gc, width - pl_width - 3, height - pl_height, pl );

   return true;
}

void Ruler::on_adj_changed( void )
{
   queue_draw( );
}

