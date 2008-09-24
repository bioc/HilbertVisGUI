#include "colorizers.h"
#include <math.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>


void fill_with_default_palette( std::vector< Gdk::Color > & palette )
{
   for( unsigned i = 0; i < palette.size(); i++ ) {
      const double pi = 2 * acos(0.);
      double p = (double) i / palette.size();
      palette[i].set_rgb_p( (1+sin(2*pi*p))/2., (1+cos(2*pi*p))/2., .25 + p/2. );
   }
}


IndexColorizer::IndexColorizer( )
 : palette( 256 )
{
   fill_with_default_palette( palette );
}

Glib::ustring IndexColorizer::get_name( void ) const
{
   return "Index colors";
}

Gdk::Color IndexColorizer::get_bin_color( long bin_start, long bin_size ) const
{
   unsigned colidx;
   if( ( bin_start + bin_size/2. < 0 ) || ( bin_start + bin_size/2. > get_length() ) )
      colidx = 0;
   else
      colidx = (int) nearbyint( 255. * 3. * ( bin_start + bin_size/2. ) / get_length() ) % 256;
   assert( colidx < palette.size() );
   return palette[ colidx ];
}   

long IndexColorizer::get_length( void ) const
{
   return 100 * 512*512;
}   



SimpleDataColorizer::SimpleDataColorizer( DataVector * data_, 
   Glib::ustring name_, std::vector< Gdk::Color > * palette_, 
   Gdk::Color na_color_, std::vector<double> * palette_steps_ )
 : data( data_ ),
   name( name_ ),
   na_color( na_color_ )
{
   palette = palette_;
   palette_steps = palette_steps_;
} 
   
   
Gdk::Color SimpleDataColorizer::get_bin_color( long bin_start, 
      long bin_size ) const
{
   double mx;
   try {
      mx = data->get_bin_value( bin_start, bin_size );
   } catch( naValue e ) {
      return na_color;
   }
   
   unsigned i;
   for( i = 0; i < palette_steps->size(); i++ )
      if( (*palette_steps)[i] >= mx )
         break;

   assert( (unsigned) i < palette->size() );
      
   return (*palette)[ i ];
}


long SimpleDataColorizer::get_length( void ) const
{
   return data->get_length();
}

Glib::ustring SimpleDataColorizer::get_name( void ) const
{
   return name;
}

DataVector * SimpleDataColorizer::get_data( void ) const
{
   return &*data;
}



ThreeChannelColorizer::ThreeChannelColorizer( DataVector * data_red_, 
      DataVector * data_green_, DataVector * data_blue_, 
      Glib::ustring name_, Gdk::Color na_color_ )
 : name( name_ ),
   na_color( na_color_ )
{
   data[0] = data_red_;
   data[1] = data_green_;
   data[2] = data_blue_;
} 
   
ThreeChannelColorizer::~ThreeChannelColorizer( )
{
   for( int i = 0; i < 3; i++ )
      delete data[i];
}   
   
Gdk::Color ThreeChannelColorizer::get_bin_color( long bin_start, 
      long bin_size ) const
{

   double v[ 3 ];
   try {
      for( int i = 0; i < 3; i++ ) {
         if( ! data[i] ) {
	    v[i] = 0;
	    continue;
	 }
         v[i] = data[i]->get_bin_value( bin_start, bin_size );
	 if( v[i] < 0 )
	    v[i] = 0;
	 if( v[i] > 1 )
	    v[i] = 1;
      }
   } catch( naValue e ) {
      return na_color;
   }
   
   Gdk::Color c;
   c.set_rgb_p( v[0], v[1], v[2] );
   
   return c;
}


long ThreeChannelColorizer::get_length( void ) const
{
   return data[0]->get_length();
}

Glib::ustring ThreeChannelColorizer::get_name( void ) const
{
   return name;
}
