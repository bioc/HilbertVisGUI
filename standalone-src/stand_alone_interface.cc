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

class MainWindowForStandalone : public MainWindow {
  public:
   MainWindowForStandalone( std::vector< DataColorizer * > * dataCols, 
      bool portrait = true );
  protected:
   virtual void on_btnOpen_clicked( void );
   virtual void on_btnClose_clicked( void );
   void brew_palettes( double max_value, double gamma = 1.0 );
   
   // Global palette information
   const int shared_palette_size;
   vector< Gdk::Color > shared_palette;
   vector< Gdk::Color > shared_palette_neg;
   vector<double> shared_palette_steps;
   Gdk::Color shared_na_color;
};

class BidirColorizer : public SimpleDataColorizer {
  public:
   BidirColorizer( DataVector * data_, Glib::ustring name_, 
      std::vector< Gdk::Color > * pos_palette_, 
      std::vector< Gdk::Color > * neg_palette_, Gdk::Color na_color_, 
      std::vector<double> * palette_steps_ );
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
  protected:
   std::vector< Gdk::Color > * neg_palette;
};

MainWindowForStandalone::MainWindowForStandalone( 
     std::vector< DataColorizer * > * dataCols, bool portrait) 
  : MainWindow( dataCols, portrait, true ),
    shared_palette_size( 256 ),
    shared_palette( shared_palette_size ),
    shared_palette_neg( shared_palette_size ),
    shared_palette_steps( shared_palette_size-1 )
{
   brew_palettes( 10 );
}  


void MainWindowForStandalone::on_btnOpen_clicked( void )
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
   string seqname = lvt.get_text( lvt.get_selected()[0] );
   try{ 
      sv = load_gff_data( dialog.get_filename( ), seqname );
   } catch( ... ) {
      Gtk::MessageDialog mdlg( string( "The file " ) + 
         Glib::filename_display_basename( dialog.get_filename( ) ) +
	 " could not be loaded.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true );
      mdlg.run();
   }
     
   addColorizer( new BidirColorizer( new StepDataVector( sv, abs_bin_max ), 
      Glib::filename_display_basename( dialog.get_filename( ) ) + ": " + seqname, 
      &shared_palette, &shared_palette_neg, shared_na_color, &shared_palette_steps ) );   
   cboxtSeqnames.set_active( dataCols->size()-1 );
      
}

void MainWindowForStandalone::on_btnClose_clicked( void )
{
   error_bell();
}

void MainWindowForStandalone::brew_palettes( double max_value, double gamma )
{
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


BidirColorizer::BidirColorizer( DataVector * data_, 
      Glib::ustring name_, 
      std::vector< Gdk::Color > * pos_palette_, 
      std::vector< Gdk::Color > * neg_palette_, Gdk::Color na_color_, 
      std::vector<double> * palette_steps_ )
 : SimpleDataColorizer( data_, name_, pos_palette_, na_color_, 
      palette_steps_ ),
   neg_palette( neg_palette_ )   
{
}
   
Gdk::Color BidirColorizer::get_bin_color( long bin_start, long bin_size ) const
{
   double m;
   try {
      m = data->get_bin_value( bin_start, bin_size );
   } catch( naValue e ) {
      return na_color;
   }
   
   unsigned i;
   for( i = 0; i < palette_steps->size(); i++ )
      if( (*palette_steps)[i] >= abs(m) )
         break;

   assert( (unsigned) i < palette->size() );
      
   return m >= 0 ? (*palette)[ i ] : (*neg_palette)[ i ];

}


int main( int argc, char *argv[] )
{
   cout << "Hilbert curve display of wiggle data.\n";
   cout << "(c) Simon Anders, European Bioinformatics Institute (EMBL-EBI)\n";
   cout << "sanders@fs.tum.de, version 0.99.3, date 2008-08-20\n";      
   cout << "Released under GNU General Public License version 3\n\n";      

   Gtk::Main kit(argc, argv);
   std::vector< DataColorizer * > dataCols;
   dataCols.push_back( new EmptyColorizer() );
   MainWindowForStandalone win( &dataCols, false );   
   Gtk::Main::run( win );   
}      
