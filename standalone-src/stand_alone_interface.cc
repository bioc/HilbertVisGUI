#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <limits>
#include <zlib.h>
#include "display.h"
#include "colorizers.h"
#include "window.h"
#include "simple_regex_pp.h"
#include "data_loading.h"

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

enum binning_style_t { bin_max, bin_min, abs_bin_max, bin_avg };

class StepDataVector : public DataVector {
  protected:
   step_vector< double > * v;
   long full_length;
   binning_style_t binning_style;
   bool own_vector;
  public:
   StepDataVector( step_vector<double> * v_, 
      binning_style_t binning_style_ = bin_max, bool own_vector_ = true  ); 
   virtual ~StepDataVector( );
   virtual double get_bin_value( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
   virtual pair< double, double > get_range( void ) const;
   void set_full_length( long full_length_, bool round_up_to_pow2 = false );
};

class MainWindowWithFileButtons : public MainWindow {
  public:
   MainWindowWithFileButtons( std::vector< DataColorizer * > * dataCols, 
      bool portrait = true );
  protected:
   virtual void on_btnOpen_clicked( void );
   virtual void on_btnClose_clicked( void );
   static void brew_palettes( double max_value, double gamma = 1.0 );
   
   // Global palette information
   static const int shared_palette_size;
   static vector< Gdk::Color > shared_palette;
   static vector< Gdk::Color > shared_palette_neg;
   static vector<double> shared_palette_steps;
   static Gdk::Color shared_na_color;
   
   // only for now:
   friend vector< DataColorizer * > * load_data( vector<string> filenames, bool same_scale, bool pow2 );
   
};

MainWindowWithFileButtons::MainWindowWithFileButtons( 
     std::vector< DataColorizer * > * dataCols, bool portrait) 
  : MainWindow( dataCols, portrait, true )
{
   brew_palettes( 10 );
}  


void MainWindowWithFileButtons::on_btnOpen_clicked( void )
{
   Gtk::FileChooserDialog dialog("Load data file" );
   dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
   dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

   Gtk::FileFilter filt1, filt2, filt3, filt4, filt5;

   filt4.add_pattern("*.gff");
   filt4.add_pattern("*.bed");
   filt4.add_pattern("*.wig");
   filt4.add_pattern("*.map");
   filt4.set_name("All supported file types (GFF, BED/Wiggle, Maq map)");
   dialog.add_filter( filt4 );

   filt1.add_pattern("*.gff");
   filt1.add_pattern("*.gft");
   filt1.set_name("Genomic Feature Files (*.gff, *.gft)");
   dialog.add_filter( filt1 );

   filt2.add_pattern("*.bed");
   filt2.add_pattern("*.wig");
   filt2.set_name("BED and wiggle track files (*.bed, *.wig)");
   dialog.add_filter( filt2 );

   filt3.add_pattern("*.map");
   filt3.set_name("Maq map files (*.map)");
   dialog.add_filter( filt3 );

   filt5.add_pattern("*");
   filt5.set_name("All files");
   dialog.add_filter( filt5 );

   int result = dialog.run();   
   if( result != Gtk::RESPONSE_OK )
      return;
   dialog.hide( );
   
   std::cout << dialog.get_filename() << std::endl;
   
   enum {gff, wig, maq} filetype;
   
   if( regex_match_pp( ".gff", dialog.get_filename(), REG_ICASE ) )
      filetype = gff;
   else if( regex_match_pp( ".bed", dialog.get_filename(), REG_ICASE ) 
         || regex_match_pp( ".wig", dialog.get_filename(), REG_ICASE ) )
      filetype = wig;
   else if( regex_match_pp( ".map", dialog.get_filename(), REG_ICASE ) )
      filetype = maq;
   else {
      Gtk::Dialog typedialog( "Specify file type" );      
      Gtk::Label lbl1( Glib::ustring( "You have choosen to load the file\n" ) +
         Glib::filename_display_basename( dialog.get_filename( ) ) +
         Glib::ustring( ".\n\nThe type of this file cannot be determined from its\n"
         "extension. Please indicate the file format:\n") ); 
      Gtk::RadioButtonGroup rbtng;
      Gtk::RadioButton rbtnGff( rbtng, "Genomic Feature Format (GFF)" );
      Gtk::RadioButton rbtnWig( rbtng, "BED or wiggle track file" );
      Gtk::RadioButton rbtnMaq( rbtng, "Maq map file" );
      typedialog.get_vbox()->add( lbl1 );
      typedialog.get_vbox()->add( rbtnGff );
      typedialog.get_vbox()->add( rbtnWig );
      typedialog.get_vbox()->add( rbtnMaq );
      lbl1.show();      
      rbtnGff.show( );
      rbtnWig.show( );
      rbtnMaq.show( );
      
      typedialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      typedialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
      if( typedialog.run( ) != Gtk::RESPONSE_OK )
         return;
	 
      if( rbtnGff.get_active( ) )
         filetype = gff;
      else if( rbtnWig.get_active( ) )
         filetype = wig;
      else if( rbtnMaq.get_active( ) )
         filetype = maq;
      else abort(); 
      
      typedialog.hide( );	 
   }
   
   get_toplevel()->get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );
   set<string> toc;
   switch( filetype ) {
      case gff:  toc = get_gff_toc( dialog.get_filename( ) ); break;
      case wig: ;
      case maq: ;
   };   
   get_toplevel()->get_window()->set_cursor( );
   
   Gtk::Dialog seqdialog( "Choose sequences" );      
   Gtk::Label lblseq( Glib::ustring( "The following sequences are found in\n" ) +
       Glib::filename_display_basename( dialog.get_filename( ) ) +
       Glib::ustring( ".\n\nChose the sequence you wish to display.\n") ); 
   Gtk::ListViewText lvt( 1 );
   Gtk::ScrolledWindow scrwnd;
   scrwnd.set_size_request( 150, 400 );
   lvt.set_column_title( 0, "Sequences:" );
   for( set<string>::const_iterator i = toc.begin(); i != toc.end(); i++ )
      lvt.append_text( *i );
   seqdialog.get_vbox()->add( lblseq );
   scrwnd.add ( lvt );
   seqdialog.get_vbox()->add( scrwnd );
   lblseq.show( );      
   scrwnd.show( );
   lvt.show( );      
      
   seqdialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
   seqdialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
   if( seqdialog.run( ) != Gtk::RESPONSE_OK )
      return;

   step_vector<double> * sv;
   string seqname =lvt.get_text( lvt.get_selected()[0] );
   try{ 
      sv = load_gff_data( dialog.get_filename( ), seqname );
   } catch( ... ) {
      Gtk::MessageDialog mdlg( string( "The file " ) + 
         Glib::filename_display_basename( dialog.get_filename( ) ) +
	 " could not be loaded.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true );
      mdlg.run();
   }
     
   addColorizer( new SimpleDataColorizer( new StepDataVector( sv ), 
      Glib::filename_display_basename( dialog.get_filename( ) ) + ": " + seqname, 
      &shared_palette, shared_na_color, &shared_palette_steps ) );   
      
}

void MainWindowWithFileButtons::on_btnClose_clicked( void )
{
   error_bell();
}

const int MainWindowWithFileButtons::shared_palette_size = 256;
vector< Gdk::Color > MainWindowWithFileButtons::shared_palette( shared_palette_size );
vector< Gdk::Color > MainWindowWithFileButtons::shared_palette_neg( shared_palette_size );
vector<double> MainWindowWithFileButtons::shared_palette_steps( shared_palette_size-1 );
Gdk::Color MainWindowWithFileButtons::shared_na_color;

void MainWindowWithFileButtons::brew_palettes( double max_value, double gamma )
{
   cout << "Brewing palette." << endl;
   
   // Construct palette:
   Gdk::Color col;
   for( int i = 0; i < shared_palette_size; i++ ) {
      double x = exp( gamma * log( i / (double) shared_palette_size ) );
      // positive: white to red
      shared_palette[i].set_rgb_p( 1., 1.-x, 1.-x );
      // positive: white to blue
      shared_palette_neg[i].set_rgb_p( 1.-x, 1.-x, 1. );
   }
   
   // Construct palette steps:
   const int min_value = 0;
   for( int i = 0; i < shared_palette_steps.size(); i++ )
      shared_palette_steps[i] = min_value + (max_value-min_value)/shared_palette.size() * (i+1);

   // NA color:
   shared_na_color.set_grey_p( .5 );

}


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


StepDataVector::StepDataVector( step_vector<double> * v_, 
      binning_style_t binning_style_, bool own_vector_ )
 : v(v_), binning_style( binning_style_ ), own_vector( own_vector_ )
{ 
   full_length = v->max_index;
}   

StepDataVector::~StepDataVector( )
{
   if( own_vector )
      delete v;
}

inline double max( double a, double b )
{
   return a > b ? a : b;
}

inline double min( double a, double b )
{
   return a < b ? a : b;
}


double StepDataVector::get_bin_value( long bin_start, long bin_size ) const
{
   // and binning_style needs respect
   if( bin_start + bin_size > v->max_index )
      throw naValue();   
      
   switch( binning_style ) {
      case bin_max:
         return v->get_max( bin_start, bin_start+bin_size-1 );
      case bin_min:
         return v->get_min( bin_start, bin_start+bin_size-1 );
      case abs_bin_max: {
         pair<double,double> mm = v->get_minmax( bin_start, bin_start+bin_size-1 );
	 return (-mm.first) > mm.second ? mm.first : mm.second;
      }
      case bin_avg: {
         cerr << "average binning not yet implemented\n";
	 abort( );
      }
      default:
         abort( );
   }
}


long StepDataVector::get_length( void ) const
{
   return full_length;
}

pair< double, double > StepDataVector::get_range( void ) const 
{
   // This needs to be optimized
   double min = numeric_limits<double>::max();
   double max = numeric_limits<double>::min();
   for( long i = 0; i <= v->max_index; i++ ) {
      if( (*v)[i] < min )
         min = (*v)[i];
      if( (*v)[i] > max )
         max = (*v)[i];
   }
   return pair<double,double>( min, max );	
}

void StepDataVector::set_full_length( long full_length_, bool round_up_to_pow2 )
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
	 
   /*
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
   */
   
   // Construct the data colorizers:
   vector< DataColorizer * > * dataCols = new vector< DataColorizer * >;
   for( vector<BwcDataVector*>::iterator bdv = bdv_vec.begin();
         bdv != bdv_vec.end(); bdv++ )
      dataCols->push_back( new SimpleDataColorizer( 
         *bdv, (*bdv)->get_name(), &MainWindowWithFileButtons::shared_palette, 
	 MainWindowWithFileButtons::shared_na_color, 
	 &MainWindowWithFileButtons::shared_palette_steps ) );
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
   MainWindowWithFileButtons win( dataCols, false );   
   Gtk::Main::run( win );   
}      

