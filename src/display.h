#ifndef HILBERT_DISPLAY_H
#define HILBERT_DISPLAY_H

#include <assert.h>
#include <math.h>
#include <vector>
#include <gtkmm.h>
#include <gtkmm/drawingarea.h>

typedef struct {
   int x, y;
} coord;    

coord hilbert( long long t, int lv );

class DataColorizer {
  public:
   virtual ~DataColorizer( );
   virtual Glib::ustring get_name( ) const;
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const = 0;
   virtual long get_length( void ) const = 0;
   // the next six fields form the pixmap cache
   Glib::RefPtr< Gdk::Pixmap > pixmap;
   std::vector< long > rev_map_lo;
   std::vector< long > rev_map_hi;
   int zoom_level;
   long zoom_offset;
   int pixel_size_level;  
};


class DataVector {
  public:
   virtual ~DataVector( ) {};
   virtual double get_bin_value( long bin_start, long bin_size ) const = 0;
   virtual long get_length( void ) const = 0;
};

class naValue {};

class InvalidableAdjustment : public Gtk::Adjustment {
  public:
   InvalidableAdjustment( void );
   bool is_valid( void ) const;
   void set_valid( bool v );
  protected:
   bool valid;
};


class HilbertValueError : public Glib::Exception {
   virtual Glib::ustring what ( ) const;
};

class HilbertCurveDisplay : public Gtk::DrawingArea {
  public:
   HilbertCurveDisplay( DataColorizer * dataCol_, 
      int pixel_size_level_ = 1, int canvas_size_level_ = 9);
   void set_dataCol( DataColorizer * dataCol_ );
   long long get_begin( void ) const;
   double get_bin_size( void ) const;
   int get_num_pixels( void ) const;   
   int get_canvas_size_level( void ) const;
   int get_pixel_size_level( void ) const;
   void set_pixel_size_level( int pixel_size_level_);
   int get_zoom_level( void ) const;
   long get_zoom_offset( void ) const;
   void set_zoom( int zoom_level_, long zoom_offset_ );
   const DataColorizer * get_dataCol( void ) const;
   InvalidableAdjustment & get_adjDisplayedValueRange( void );
   InvalidableAdjustment & get_adjPointerPos( void );
   sigc::signal< void, GdkEventButton *, long, long > signal_mouse_clicked( void );
   const int canvas_size_level;
  protected:
   virtual void on_realize( );
   virtual bool on_expose_event( GdkEventExpose * event );
   virtual bool on_button_press_event( GdkEventButton * event );
   virtual bool on_motion_notify_event( GdkEventMotion * event );
   virtual bool on_leave_notify_event( GdkEventCrossing * event );
   virtual void fill_pixmap( void );
   void set_adjPointerPos( void );
   void set_adjDisplayedValueRange( void );
   int pixel_size_level;
   DataColorizer * dataCol;
   int zoom_level;
   long zoom_offset;
   InvalidableAdjustment adjDisplayedValueRange;
   InvalidableAdjustment adjPointerPos;
   sigc::signal< void, GdkEventButton *, long, long > sgn_MouseClicked;
};


inline long long HilbertCurveDisplay::get_begin( void ) const
// returns the Hilbert curve index of the first displayed bin
// To get the data vector index  of the beginning of the bin, multiply by get_bin_size().
{
   assert( (long long) zoom_offset << ( 2 * (canvas_size_level-pixel_size_level) ) >= 0 );
   return (long long) zoom_offset << ( 2 * (canvas_size_level-pixel_size_level) );
}

inline double HilbertCurveDisplay::get_bin_size( void ) const
// returns the number of data points represented by one pixel
{
   return double( dataCol->get_length() ) / ( get_num_pixels() * exp( log(2.) * (2.*zoom_level)) );
}   

inline int HilbertCurveDisplay::get_num_pixels( void ) const
// returns the number of pixels displayed on the canvas
{
   return 1 << ( 2 * ( canvas_size_level - pixel_size_level ) );
}


#endif // HILBERT_DISPLAY_H
