#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <limits>
#include <zlib.h>
#include "display.h"
#include "colorizers.h"
#include "window.h"

using namespace std;

class StdDataVector : public DataVector {
  protected:
   vector< double > * v;
   long full_length;
  public:
   StdDataVector( vector<double> * v_ ); 
   virtual double get_bin_value( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
   virtual pair< double, double > get_range( void ) const;
   void set_full_length( long full_length_, bool round_up_to_pow2 = false );
};

class BwcDataVector : public StdDataVector {
  protected:
   string name;
   BwcDataVector( vector<double> * v_, string name_, double min_, double max_ );
   double min, max;
  public:
   virtual ~BwcDataVector( );
   static BwcDataVector * new_BwcDataVector_from_file( const string bwc_file_name );
   const string & get_name( ) const;
   virtual pair< double, double > get_range( void ) const;
};   


StdDataVector::StdDataVector( vector<double> * v_ )
 : v(v_)
{ 
   full_length = v->size();
}   

double StdDataVector::get_bin_value( long bin_start, long bin_size ) const
{
   if( bin_start + bin_size > v->size() )
      throw naValue();   
   double res = -numeric_limits<double>::max();
   for( long i = bin_start; i < bin_start + bin_size && i < v->size(); i++ )
      if( (*v)[i] > res )
         res = (*v)[i];
   return res;
}

long StdDataVector::get_length( void ) const
{
   return full_length;
}

pair< double, double > StdDataVector::get_range( void ) const 
{
   double min = numeric_limits<double>::max();
   double max = numeric_limits<double>::min();
   for( long i = 0; i < v->size(); i++ ) {
      if( (*v)[i] < min )
         min = (*v)[i];
      if( (*v)[i] > max )
         max = (*v)[i];
   }
   return pair<double,double>( min, max );	
}

void StdDataVector::set_full_length( long full_length_, bool round_up_to_pow2 )
{
   full_length = full_length_;
   
   if( round_up_to_pow2 ) {
      int i;
      for( i = 0; i < 64; i++ )
         if( !( (unsigned) full_length >> i ) )
	    break;
      full_length = 1UL << i;
   }  
}

BwcDataVector::BwcDataVector( vector<double> * v_, string name_, double min_, 
      double max_ )
 : name( name_ ), min( min_ ), max( max_ ),
   StdDataVector( v_ )
{
}

BwcDataVector::~BwcDataVector(  )
{
   delete v;
}

BwcDataVector * BwcDataVector::new_BwcDataVector_from_file( const string bwc_file_name )
{   
   char buf[ 200 ] = "invalid";
   gzFile bwc_file = gzopen( bwc_file_name.c_str(), "rb" );
   if( ! bwc_file ) {
      cerr << "Failed to open file " << bwc_file_name << endl;
      exit( 1);
   }     
   cout << "Reading file " << bwc_file_name << "... ";
   cout.flush();
   gzgets( bwc_file, buf, 200 );
   if( strncmp( buf, "BWC1\n", 200 ) != 0 ) {
      cerr << "File " << bwc_file_name << " is not a valid bwc file." << endl;
      exit( 1);
   }     
   guint32 chrlen;
   gzread( bwc_file, &chrlen, sizeof( guint32 ) );
   // read name and remove trailing \n:
   gzgets( bwc_file, buf, 200 ); 
   buf[ strlen(buf)-1 ] = '\0';
   
   vector< double > * v = new vector< double >( chrlen );
   double min = numeric_limits<double>::max();
   double max = numeric_limits<double>::min();

   guint32 pos = 0;
   while( ! gzeof( bwc_file ) ) {
      gdouble val;
      guint32 rep;
      gzread( bwc_file, &val, sizeof( gdouble ) );
      gzread( bwc_file, &rep, sizeof( guint32 ) );
      guint32 to = pos + rep;
      if( to > v->size() ) {
         cerr << "File " << bwc_file_name << " contains erroneous size or position data.\n";
	 exit( 1);
      }
      for( ; pos < to; pos++ )
         (*v)[pos] = val;
      if( val < min )
         min = val;
      if( val > max )
         max = val;
   }
   gzclose( bwc_file );
   cout << "Done.\n";
   cout.flush();
   
   return new BwcDataVector( v, string( buf ), min, max );
}
  
const string & BwcDataVector::get_name( ) const
{
   return name;
}

pair< double, double > BwcDataVector::get_range( void ) const 
{
   return pair<double,double>( min, max );
}


vector< DataColorizer * > * load_data( vector<string> filenames, bool same_scale, bool pow2 )
{
   // Load the data:
   vector< BwcDataVector * > bdv_vec;
   for( vector<string>::iterator filename = filenames.begin(); 
         filename != filenames.end(); filename++ )
      bdv_vec.push_back( BwcDataVector::new_BwcDataVector_from_file( *filename ) );
      
   if( same_scale ) {
      long max_length = 0;
      for( vector<BwcDataVector*>::iterator bdv = bdv_vec.begin();
            bdv != bdv_vec.end(); bdv++ )
	 if( (*bdv)->get_length() > max_length)
	    max_length = (*bdv)->get_length();
      for( vector<BwcDataVector*>::iterator bdv = bdv_vec.begin();
            bdv != bdv_vec.end(); bdv++ )
	 (*bdv)->set_full_length( max_length, pow2 );
      cout << max_length << endl;
   } else if( pow2 ) 
      for( vector<BwcDataVector*>::iterator bdv = bdv_vec.begin();
            bdv != bdv_vec.end(); bdv++ )
	 (*bdv)->set_full_length( (*bdv)->get_length(), true );
      
   // Get range:
   double min = numeric_limits<double>::max();
   double max = numeric_limits<double>::min();
   for( vector<BwcDataVector*>::iterator bdv = bdv_vec.begin();
         bdv != bdv_vec.end(); bdv++ ) {
      pair<double,double> r = (*bdv)->get_range();
      if( r.first < min )
         min = r.first;
      if( r.second > max )
         max = r.second;
   }
   if( min < 0 ) {
      cerr << "Warning: Negative data values found. The currently implemented binning "
         "function will display these incorrectly.\n";
      min = 0;
   }
	 
   // Construct palette:
   vector< Gdk::Color > * palette = new vector< Gdk::Color >;
   Gdk::Color col;
   // first quarter: white to red
   for( int i = 0; i < 25; i++ ) {
      col.set_rgb_p( 1, 1-(i/24.), 1-(i/24.) );
      palette->push_back( col );
   }
   // remaining quarters: red to blue
   for( int i = 0; i < 75; i++ ) {
      col.set_rgb_p( 1-(i/74.), 0, i/74. );
      palette->push_back( col );
   }
   
   // Construct palette steps:
   vector<double> * palette_steps = new vector<double>( palette->size() - 1 );
   for( int i = 0; i < palette_steps->size(); i++ )
      (*palette_steps)[i] = min + (max-min)/palette->size() * (i+1);
   printf( "Palette: pure white = %.3g  (minimum value)\n", min );
   printf( "         pure red   = %.3g\n", (*palette_steps)[24] );
   printf( "         pure blue  = %.3g  (maximum value)\n", max );
      
   // NA color:
   col.set_grey_p( .5 );
   
   // Construct the data colorizers:
   vector< DataColorizer * > * dataCols = new vector< DataColorizer * >;
   for( vector<BwcDataVector*>::iterator bdv = bdv_vec.begin();
         bdv != bdv_vec.end(); bdv++ )
      dataCols->push_back( new SimpleDataColorizer( 
         *bdv, (*bdv)->get_name(), palette, col, palette_steps ) );
   return dataCols;   
}

int main( int argc, char *argv[] )
{
   cout << "Hilbert curve display of wiggle data.\n";
   cout << "(c) Simon Anders, European Bioinformatics Institute (EMBL-EBI)\n";
   cout << "sanders@fs.tum.de, version 0.99.3, date 2008-08-20\n";      
   cout << "Released under GNU General Public License version 3\n\n";      

   bool same_scale = true;
   bool pow2 = false;
   char opt;
   while( (opt = getopt(argc, argv, "sph") ) != -1 )
      switch( opt ) {
         case 's':
	    same_scale = false;
	    break;
         case 'p':
	    pow2 = true;
	    break;
	 case '?':
	    exit( 1 );
	 case 'h':
	    cout << "Usage: hilbert [options] <bwc file> [ <bwc file> ... ]\n\n"
	    "Options:\n"
	    "   -s: Scale all vectors such that they use the full plotting area.\n"
	    "       The default is to pad grey area to the end of smaller vector\n"
	    "       such that all vectors are displayed in the same scale.\n"
	    "   -p: Pad grey area to the end of all vectors such that their length\n"
	    "       becomes a power of 2. This ensures that individual vector elements\n"
	    "       become squares in high zoom.\n"
	    "   -h: Display this help message.\n";
	    exit( 0 );
      }
   Gtk::Main kit(argc, argv);
   vector<string> filenames;
   if( argc == 1 ) {
      cerr << "Call with a list of bwc files as command-line arguments.\n";
      exit( 1);
   }
   for( int i = optind; i < argc; i++ )
      filenames.push_back( argv[i] );
   vector< DataColorizer * > * dataCols = load_data( filenames, same_scale, pow2 );
   cout << "Preparing display...\n"; cout.flush();
   MainWindow win( dataCols );   
   Gtk::Main::run( win );   
}      

