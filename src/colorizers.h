#ifndef HILBERT_COLORIZERS_H
#define HILBERT_COLORIZERS_H

#include <memory>
#include <vector>
#include <gtkmm.h>
#include "display.h"


void fill_with_default_palette( std::vector< Gdk::Color > & palette );


class IndexColorizer : public DataColorizer {
  public:
   IndexColorizer( );
   virtual Glib::ustring get_name( void ) const;
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
  protected:
   std::vector< Gdk::Color > palette;
};


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

class EmptyColorizer : public DataColorizer {
  public:
   virtual Glib::ustring get_name( ) const;
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
};


#endif //HILBERT_COLORIZERS_H
