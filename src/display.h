#ifndef HILBERT_DISPLAY_H
#define HILBERT_DISPLAY_H

#include <cassert>
#include <cmath>
#include <vector>
#include <gtkmm.h>
#include <gtkmm/drawingarea.h>

typedef struct {
   int x, y;
} coord;    

coord hilbert( long long t, int lv );

/* A data colorizer is an adapter between the Hilbert display and the data.
It represnts the full length of the sequence and can be queried for the 
color of a bin (i.e., short section of the sequence that is represented by one 
pixel in the Hilbert), which is specified in sequence coordinates.
Most implementations of this abstract class do so by summarizing the relevant
part of the data, but this is hidden to HilbertCurveDisplay. */

class DataColorizer {
  public:
   virtual ~DataColorizer( );
   virtual Glib::ustring get_name( ) const;
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const = 0;
   virtual long get_length( void ) const = 0;
   
   // The following fields are not used by DataColorizer itself but by
   // MainWindow. (Some refactoring might be in order here.)
   // the next seven fields form the pixmap cache
   Glib::RefPtr< Gdk::Pixmap > pixmap;
   std::vector< long > rev_map_lo;
   std::vector< long > rev_map_hi;
   int zoom_level;
   long zoom_offset;
   int pixel_size_level;  
   int palette_level; // this one only used by stand-alone version
   std::vector< Gtk::Window * > linplot_wins; // this one only used by stand-alone version
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
   const DataColorizer * get_dataCol( ) const;
   long long get_begin( void ) const;
   double get_bin_size( void ) const;
   int get_num_pixels( void ) const;   
   int get_canvas_size_level( void ) const;
   int get_pixel_size_level( void ) const;
   void set_pixel_size_level( int pixel_size_level_);
   int get_zoom_level( void ) const;
   long get_zoom_offset( void ) const;
   void set_zoom( int zoom_level_, long zoom_offset_ );
   int get_palette_level( void ) const;
   void set_palette_level( int palette_level_ );
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
   int palette_level; // this one only used by stand-alone version
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
