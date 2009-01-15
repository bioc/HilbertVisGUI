#ifndef HILBERT_COLORIZERS_H
#define HILBERT_COLORIZERS_H

#include <memory>
#include <vector>
#include <gtkmm.h>
#include "display.h"

/* See display.h for an explanation of colorizers and the declaration of
their abstract base class. */

void fill_with_default_palette( std::vector< Gdk::Color > & palette );

/* The index colorizer simply paints a rainbow-style pattern that traces
the Hilbert curve. meant for demonstration and test purposes only. */
class IndexColorizer : public DataColorizer {
  public:
   IndexColorizer( );
   virtual Glib::ustring get_name( void ) const;
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
  protected:
   std::vector< Gdk::Color > palette;
};

/* A SimpleDataColorizer holds a DataVector and a palette (complete with
palette steps, i.e. ladder of values corresponding to the palette colour), 
as well as a name. The bin value is calculated by taking the maximum of the
relevant part of the DataVector, which is the converted to a color from the 
palette. */

class SimpleDataColorizer : public DataColorizer {
  public:
   SimpleDataColorizer( DataVector * data_, Glib::ustring name_, 
      std::vector< Gdk::Color > * palette_, Gdk::Color na_color_, 
      std::vector<double> * palette_steps_ );
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
   virtual Glib::ustring get_name( ) const;
   DataVector * get_data( void ) const;
  protected:
   std::auto_ptr<DataVector> data;
   Glib::ustring name;
   std::vector< Gdk::Color > * palette;
   Gdk::Color na_color;
   std::vector<double> * palette_steps;
};

/* A ThreeChannelColorizer takes three DataVectors, which have to be normalized
to [0,1]. The bin maxima in each vector control one color channel. */

class ThreeChannelColorizer : public DataColorizer {
  public:
   ThreeChannelColorizer( DataVector * data_red_, DataVector * data_green_,
      DataVector * data_blue_, Glib::ustring name_, Gdk::Color na_color_ );
   virtual ~ThreeChannelColorizer( );      
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
   virtual Glib::ustring get_name( ) const;
  protected:
   DataVector * data[ 3 ];
   Glib::ustring name;
   Gdk::Color na_color;
   std::vector<double> * palette_steps;
};

/* The EmptyColorizer always reports gray as bin color. */

class EmptyColorizer : public DataColorizer {
  public:
   virtual Glib::ustring get_name( ) const;
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
};

/* The BidirColorizer works as the SimpleDataColorizer but has two
palettes, one for positive and one for negative values. Depending on
whether the maximum or the absolute value of the minimum is larger, either
the former or the later palette determines the bin color. */

class BidirColorizer : public SimpleDataColorizer {
  public:
   BidirColorizer( DataVector * data_, Glib::ustring name_, 
      std::vector< Gdk::Color > * pos_palette_, 
      std::vector< Gdk::Color > * neg_palette_, Gdk::Color na_color_, 
      std::vector<double> * palette_steps_ );
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   //void set_full_length( long full_length );
  protected:
   std::vector< Gdk::Color > * neg_palette;
};




#endif //HILBERT_COLORIZERS_H
