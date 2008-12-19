#include "display.h"
#include <iostream>
#include "error_bell_kludge.h"

// ****************************************
// ***** Calculation of Hilbert curve *****
// ****************************************

coord hilbert( long long t, int lv ) 
{
   long long q, r;
   coord c;
   if( lv == 0 )
      return (coord) { 0, 0 };
   else {
      r = 1ll << (2*(lv-1));
      q = t / r ;      
      c = hilbert( t % r, lv - 1 );
      switch( q ) {
         case 0: { return (coord) { c.y, c.x }; }
         case 1: { return (coord) { c.x, c.y + ( 1l << (lv-1) ) }; }
         case 2: { return (coord) { c.x + ( 1l << (lv-1) ), c.y + ( 1l << (lv-1) ) }; }
         case 3: { return (coord) { (1l<<lv) - 1 - c.y, ( 1l << (lv-1) ) - 1 - c.x }; }
         default: abort( );
      }
   }
   return (coord) {-1,-1}; /* dummy statement, never reached */
}      


// *******************************
// ***** class DataColorizer *****
// *******************************

DataColorizer::~DataColorizer( )
{
}

Glib::ustring DataColorizer::get_name( ) const
{
   return "unnamed";
}   


// ***************************************
// ***** class InvalidableAdjustment *****
// ***************************************


InvalidableAdjustment::InvalidableAdjustment( void )
 : Gtk::Adjustment( 0, 0, 0, 0, 0),
   valid( false )
{
}

bool InvalidableAdjustment::is_valid( void ) const
{
   return valid;
}

void InvalidableAdjustment::set_valid( bool v )
{
   valid = v;
   value_changed();
}   

// ***********************************
// ***** class HilbertValueError *****
// ***********************************


Glib::ustring HilbertValueError::what ( ) const
{
   return "Illegal value passed to a HilbertCurveDisplay widget.";
}



// *************************************
// ***** class HilbertCurveDisplay *****
// *************************************


HilbertCurveDisplay::HilbertCurveDisplay( DataColorizer * dataCol_,
      int pixel_size_level_, int canvas_size_level_  )
 : canvas_size_level( canvas_size_level_),
   pixel_size_level( pixel_size_level_)
{
   dataCol = dataCol_;
   set_zoom( 0, 0 );
   set_size_request( 1l << canvas_size_level, 1l << canvas_size_level );
   add_events( Gdk::BUTTON_PRESS_MASK | Gdk::POINTER_MOTION_MASK | 
      Gdk::POINTER_MOTION_HINT_MASK | Gdk::LEAVE_NOTIFY_MASK );
}

void HilbertCurveDisplay::set_dataCol( DataColorizer * dataCol_ )
{
   dataCol = dataCol_;
   if( !get_window() )
      return;
   if( !dataCol->pixmap || dataCol->zoom_level != zoom_level || 
         dataCol->zoom_offset != zoom_offset || 
	 dataCol->pixel_size_level != pixel_size_level ||
	 dataCol->palette_level != palette_level ) {
      fill_pixmap();
   }
   queue_draw( );
   set_adjDisplayedValueRange( );
}

void HilbertCurveDisplay::on_realize( )
{
   Gtk::DrawingArea::on_realize();
   assert( get_window() );
   get_window()->set_cursor( Gdk::Cursor(Gdk::TCROSS) );
   fill_pixmap( );  
}   


bool HilbertCurveDisplay::on_expose_event( GdkEventExpose* event )
{
   assert( dataCol->pixmap );
   Glib::RefPtr< Gdk::GC > gc = Gdk::GC::create( get_window() );
   get_window()->draw_drawable( gc, dataCol->pixmap, 0, 0, 0, 0 );
   return true;   
}   

bool HilbertCurveDisplay::on_button_press_event( GdkEventButton * event )
{
   if( event->x <= 0 || event->y <= 0 )
      return false;
   if( event->x >= (1l<<canvas_size_level) || event->y >= (1l<<canvas_size_level) )
      return false;
   int rev_map_idx = (long(event->x) << canvas_size_level) | long(event->y);

   sgn_MouseClicked( event, dataCol->rev_map_lo[ rev_map_idx ], 
      dataCol->rev_map_hi[ rev_map_idx ] );

   return true;
}

void HilbertCurveDisplay::set_adjPointerPos( void )
{
   if( !get_window() ) {
      adjPointerPos.set_valid( false );
      return;
   }
   int x, y;
   Gdk::ModifierType mask;
   get_window()->get_pointer( x, y, mask);

   if( x < 0 || y < 0 || x >= (1l<<canvas_size_level) || y >= (1l<<canvas_size_level) ) {
      adjPointerPos.set_valid( false );
   } else {
      const int rev_map_idx = ((long) x << canvas_size_level) | y;
      adjPointerPos.set_value( dataCol->rev_map_lo[rev_map_idx] );
      adjPointerPos.set_valid ( true );   
   }
}

void HilbertCurveDisplay::set_adjDisplayedValueRange( void )
{
   adjDisplayedValueRange.set_lower( 0 );
   adjDisplayedValueRange.set_upper( dataCol->get_length() );
   adjDisplayedValueRange.set_page_size( get_num_pixels() * get_bin_size() );
   adjDisplayedValueRange.set_valid( true );   
   adjPointerPos.set_lower( get_begin() * get_bin_size() );
   adjPointerPos.set_upper( (get_begin() + get_num_pixels()) * get_bin_size() );
   adjPointerPos.set_page_size( get_bin_size() > 1 ? get_bin_size() : 1 );
   adjDisplayedValueRange.set_value( get_begin() * get_bin_size() );
   set_adjPointerPos( );   
}

bool HilbertCurveDisplay::on_motion_notify_event( GdkEventMotion * event )
{
   set_adjPointerPos();
   return true;   
}

bool HilbertCurveDisplay::on_leave_notify_event( GdkEventCrossing * event )
{
   adjPointerPos.set_valid( false );
   return true;   
}

void HilbertCurveDisplay::fill_pixmap( void )
{
   assert( zoom_offset >= 0 && zoom_offset < 1l << (2*zoom_level) );
   if( get_toplevel() && get_toplevel()->get_window() ) {
      get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );
      get_toplevel()->get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );
   }
   assert( dataCol );
   dataCol->pixmap = Gdk::Pixmap::create ( 
      get_window(), 1 << canvas_size_level, 1 << canvas_size_level );
   long num_phys_pixels = 1l << (2*canvas_size_level);
   dataCol->rev_map_lo.resize( num_phys_pixels );
   dataCol->rev_map_hi.resize( num_phys_pixels );
   Glib::RefPtr< Gdk::GC > gc = Gdk::GC::create( get_window() );
   for( long long i = get_begin(); i < get_begin() + get_num_pixels(); i++ ) {
      assert( i >= 0 );
      long lo = (long) round( get_bin_size() * i );
      long hi = (long) round( get_bin_size() * (i+1) );
      Gdk::Color col = dataCol->get_bin_color( lo, hi==lo ? 1: hi - lo );
      gc->set_rgb_fg_color( col );
      coord c = hilbert( i, canvas_size_level + zoom_level - pixel_size_level);
      c.x = (c.x * (1<<pixel_size_level)) & ( (1<<canvas_size_level) - 1 );
      c.y = (c.y * (1<<pixel_size_level)) & ( (1<<canvas_size_level) - 1 );
      assert( ( c.x >= 0 ) && ( c.y >= 0 ) );
      assert( ( c.x + (1<<pixel_size_level) - 1 < (1<<canvas_size_level) ) 
           && ( c.y + (1<<pixel_size_level) - 1 < (1<<canvas_size_level) ) );
      dataCol->pixmap->draw_rectangle( gc, true, c.x, c.y, 
         1<<pixel_size_level, 1<<pixel_size_level ); 
      assert( dataCol->rev_map_lo.size() == dataCol->rev_map_hi.size() );
      for( int xx = 0; xx < 1<<pixel_size_level; xx++)
         for( int yy = 0; yy < 1<<pixel_size_level; yy++) {
	    unsigned rev_map_idx = ((c.x+xx) << canvas_size_level) | (c.y+yy);
	    assert( rev_map_idx < dataCol->rev_map_lo.size() );
	    dataCol->rev_map_lo[ rev_map_idx ] = lo;
	    dataCol->rev_map_hi[ rev_map_idx ] = hi;
	 }
   }
   dataCol->zoom_level = zoom_level;
   dataCol->zoom_offset = zoom_offset;
   dataCol->pixel_size_level = pixel_size_level;
   dataCol->palette_level = palette_level;
   if( get_toplevel() && get_toplevel()->get_window() ) {
      get_window()->set_cursor( Gdk::Cursor(Gdk::TCROSS) );
      get_toplevel()->get_window()->set_cursor( );
   }
}

int HilbertCurveDisplay::get_canvas_size_level( void ) const
{
   return canvas_size_level;
}

int HilbertCurveDisplay::get_pixel_size_level( void ) const
{
   return pixel_size_level;
}   

void HilbertCurveDisplay::set_pixel_size_level( int pixel_size_level_)
{
   if( (pixel_size_level_ < 0) || (pixel_size_level_ > canvas_size_level - 1) )
      throw HilbertValueError();
   pixel_size_level = pixel_size_level_;
   fill_pixmap( );
   set_adjDisplayedValueRange( );
   queue_draw( );
}

int HilbertCurveDisplay::get_palette_level( void ) const
{
   return palette_level;
}   

void HilbertCurveDisplay::set_palette_level( int palette_level_ )
{
   palette_level = palette_level_;
   if( !get_window() || !is_visible() )
      return;
   fill_pixmap( );
   queue_draw( );
}


int HilbertCurveDisplay::get_zoom_level( void ) const
{
   return zoom_level;
}

long HilbertCurveDisplay::get_zoom_offset( void ) const
{
   return zoom_offset;
}   

void HilbertCurveDisplay::set_zoom( int zoom_level_, long zoom_offset_ )
{
   if( zoom_level_ < 0 || zoom_offset_ < 0 || zoom_offset_ >= 1l << (2*zoom_level_) )
      throw HilbertValueError( );
   if ( ( 1l << (2*(canvas_size_level + zoom_level_))) <= 0 )
      throw HilbertValueError( );   
   zoom_level = zoom_level_;
   zoom_offset = zoom_offset_;
   if( get_window() ) {
      fill_pixmap();
      queue_draw( );
   }
   set_adjDisplayedValueRange( );
}

InvalidableAdjustment & HilbertCurveDisplay::get_adjDisplayedValueRange( void )
{
   return adjDisplayedValueRange;
}          

InvalidableAdjustment & HilbertCurveDisplay::get_adjPointerPos( void ) 
{
   return adjPointerPos;
}

sigc::signal< void, GdkEventButton *, long, long > HilbertCurveDisplay::signal_mouse_clicked( void )
{
   return sgn_MouseClicked;
}

const DataColorizer * HilbertCurveDisplay::get_dataCol( void ) const
{
   return dataCol;
}   
