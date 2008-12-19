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
   virtual void on_btnSave_clicked( void );
   virtual void on_btnDown_clicked( void );
   virtual void on_btnUp_clicked( void );
   virtual void on_btnAbout_clicked( void );
   virtual void on_btnQuit_clicked( void );
   void brew_palettes( int palette_level );
   
   // Global palette information
   const int shared_palette_size;
   vector< Gdk::Color > shared_palette;
   vector< Gdk::Color > shared_palette_neg;
   vector<double> shared_palette_steps;
   Gdk::Color shared_na_color;
   // Global full_length
   long full_length;
};

class BidirColorizer : public SimpleDataColorizer {
  public:
   BidirColorizer( DataVector * data_, Glib::ustring name_, 
      std::vector< Gdk::Color > * pos_palette_, 
      std::vector< Gdk::Color > * neg_palette_, Gdk::Color na_color_, 
      std::vector<double> * palette_steps_ );
   virtual Gdk::Color get_bin_color( long bin_start, long bin_size ) const;
   void set_full_length( long full_length );
  protected:
   std::vector< Gdk::Color > * neg_palette;
};

MainWindowForStandalone::MainWindowForStandalone( 
     std::vector< DataColorizer * > * dataCols, bool portrait) 
  : MainWindow( dataCols, portrait, true ),
    shared_palette_size( 256 ),
    shared_palette( shared_palette_size ),
    shared_palette_neg( shared_palette_size ),
    shared_palette_steps( shared_palette_size-1 ),
    full_length( 0 )
{
   #include "main_icon.c"
   set_default_icon( Gdk::Pixbuf::create_from_inline( -1, main_icon ) ); 
   brew_palettes( 4 );
   rbtnPlotLin.set_sensitive( false );
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
   
   enum {gff, wig, maq, maq_old} filetype;
   
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
   
   if( filetype == maq ) {
      Gtk::Dialog maqtypedialog( "Specify file type" );      
      Gtk::Label lbl1( 
         "\n"
         "The binary format of Maq map files has changed\n"
         "in Maq version 0.7 (released September 2008).\n"
         "Unfortunately, the type of file cannot\n"
         "be determined automatically.\n\n"
         "Please indicate whether the file to be loaded\n"
         "was created with an old version of Maq (before\n"
         "version 0.7) or a new version (0.7 or later).\n" ); 
      maqtypedialog.get_vbox()->add( lbl1 );  
      maqtypedialog.show_all_children();       
      
      maqtypedialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      maqtypedialog.add_button("Old Maq", Gtk::RESPONSE_YES);
      maqtypedialog.add_button("New Maq", Gtk::RESPONSE_NO);
      
      int res = maqtypedialog.run( );
      maqtypedialog.hide( );            
      if( res != Gtk::RESPONSE_YES && res != Gtk::RESPONSE_NO )
         return;

      if( res == Gtk::RESPONSE_YES )
         filetype = maq_old;
   }

   get_toplevel()->get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );
   set<string> toc;
   try{   
      switch( filetype ) {
         case gff:  toc = get_gff_toc( dialog.get_filename( ) ); break;
         case wig:  break;
         case maq:  toc = get_maqmap_toc( dialog.get_filename( ) ); break;
         case maq_old:  toc = get_maqmap_old_toc( dialog.get_filename( ) ); break;
      };   
   } catch( data_loading_exception e ) {   
      Gtk::MessageDialog mdlg( string( "The file " ) + 
         Glib::filename_display_basename( dialog.get_filename( ) ) +
         " could not be loaded due to the following error: " + e,
         false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true );
      mdlg.run();
      get_toplevel()->get_window()->set_cursor( );
      return;
   }
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
   seqdialog.hide( );
   
   int min_qual = 0;
   if( filetype == maq or filetype == maq_old ) {
      Gtk::Dialog minqdialog( "Minimum alignment quality" );      
      Gtk::Label lbl1( 
         "\n"
         "Do you want to disregard read below a certain\n"
         "alignment quality threshold?\n\n"
         "If so, indicate the minimum alignment quality\n"
         "here. (Recommended threshold values are\n"
         "10 or 15.)\n\n"
         "A threshold of 0 means no filtering.\n\n" );
         
      Gtk::TextView tv;
      char buf[50] = "0";
      tv.get_buffer( )->set_text( buf );
      minqdialog.get_vbox()->add( lbl1 );
      minqdialog.get_vbox()->add( tv );
      minqdialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
      minqdialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
      minqdialog.show_all_children( );
      bool valid_input = true;
      do {
         if( minqdialog.run( ) != Gtk::RESPONSE_OK )
            return;
         try{
            min_qual = from_string<long int>( tv.get_buffer( )->get_text() );
         } catch( conversion_failed_exception e ) {
            valid_input = false;
         }
         if( min_qual < 0 )
            valid_input = false;
         if( !valid_input ) {
            Gtk::MessageDialog mdlg( "Please enter a non-negative integer number.", 
               false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true );
            mdlg.run();
         }
      } while( !valid_input );
   }   

   get_toplevel()->get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );  
   get_toplevel()->get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );  
   step_vector<double> * sv;
   string seqname = lvt.get_text( lvt.get_selected()[0] );
   try{ 
      switch( filetype ) {   
         case gff:  sv = load_gff_data( dialog.get_filename( ), seqname ); break;
         case wig:  Gtk::MessageDialog( "Loading of Wiggle or BED files is not yet implemented, sorry.", 
               false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true ).run(); return;
         case maq:  sv = load_maqmap_data( dialog.get_filename( ), seqname, min_qual ); break;
         case maq_old:  sv = load_maqmap_old_data( dialog.get_filename( ), seqname, min_qual ); break;
      }
   } catch( data_loading_exception e ) {
      Gtk::MessageDialog mdlg( string( "The file " ) + 
         Glib::filename_display_basename( dialog.get_filename( ) ) +
         " could not be loaded due to the following error: " + e,
         false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, false );
      mdlg.run();
      return;
   }
   get_toplevel()->get_window()->set_cursor( );
      
   #ifdef ASK_FOR_FULL_LENGTH
      bool valid_input = true;
      long ans = -17;         
      do{ 
         Gtk::Dialog lendialog( "Enter length" );      
         Gtk::Label lbllen( "\n"
             "The length of the loaded sequence\n"
             "is not provided by the file. Please\n"
             "enter it manually. If you want to\n"
             "use the maximum index found in the\n"
             "file as length, just press 'Ok'.\n" ); 
         Gtk::TextView tv;
         char buf[50];
         snprintf( buf, 50, "%ld", sv->max_index );
         tv.get_buffer( )->set_text( buf );
         lendialog.get_vbox()->add( lbllen );
         lendialog.get_vbox()->add( tv );
         lendialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
         lendialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
         lendialog.show_all_children( );
         if( lendialog.run( ) != Gtk::RESPONSE_OK )
            return;
         try{
            ans = from_string<long int>( tv.get_buffer( )->get_text() );
         } catch( conversion_failed_exception e ) {
            valid_input = false;
         }
         if( ans <= 0 )
            valid_input = false;
         if( !valid_input ) {
            Gtk::MessageDialog mdlg( "Please enter a positive integer number.", 
               false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true );
            mdlg.run();
         }
      } while( !valid_input );
      long old_maxindex = sv->max_index;
      sv->max_index = ans;
      sv->set_value( old_maxindex, ans, 0 );
   #endif

   addColorizer( new BidirColorizer( new StepDataVector( sv, abs_bin_max ), 
      Glib::filename_display_basename( dialog.get_filename( ) ) + ": " + seqname, 
      &shared_palette, &shared_palette_neg, shared_na_color, &shared_palette_steps ) );   
   cboxtSeqnames.set_active( dataCols->size()-1 );

   if( sv->max_index > full_length )
   {
      full_length = sv->max_index;
      for( std::vector<DataColorizer*>::iterator i = dataCols->begin();
            i != dataCols->end(); i++ ) 
         if( dynamic_cast<BidirColorizer*>(*i) )
            dynamic_cast<BidirColorizer*>(*i)->set_full_length( full_length );
   }     
     
}

void MainWindowForStandalone::on_btnClose_clicked( void )
{
   removeCurrentColorizer();
}

void MainWindowForStandalone::on_btnSave_clicked( void )
{
   Gtk::FileChooserDialog dialog("Save displayed image as PNG file",
      Gtk::FILE_CHOOSER_ACTION_SAVE );
   dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
   dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
   dialog.set_do_overwrite_confirmation();

   Gtk::FileFilter filt1, filt2;

   filt1.add_pattern("*.png");
   filt1.set_name("Portable Networks Graphics (PNG) format");
   dialog.add_filter( filt1 );

   filt2.add_pattern("*");
   filt2.set_name("All files");
   dialog.add_filter( filt2 );
   
   int result = dialog.run();   
   if( result != Gtk::RESPONSE_OK )
      return;
   dialog.hide( );

   std::string filename = dialog.get_filename( );
   if( filename.substr( filename.length()-4 ) != ".png" ) {
      filename += ".png";
      if( Glib::file_test( filename, Glib::FILE_TEST_EXISTS ) ) {
         Gtk::MessageDialog mdlg( string( "The file " ) + 
            Glib::filename_display_basename( dialog.get_filename( ) ) +
            " already exists.\nDo you want to overwrite it?", 
              false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK_CANCEL, true );
         if( mdlg.run() != Gtk::RESPONSE_OK )
            return;
      }
   }

   int width, height;
   canvas.get_dataCol()->pixmap->get_size( width, height );
   Glib::RefPtr<Gdk::Pixbuf> pb = Gdk::Pixbuf::create( 
      canvas.get_dataCol()->pixmap->get_image( 0, 0, width, height ), 
      0, 0, width, height );
   pb->save( filename, "png" );
}

void MainWindowForStandalone::on_btnDown_clicked( void )
{
   brew_palettes( canvas.get_palette_level() - 1 );
}

void MainWindowForStandalone::on_btnUp_clicked( void )
{
   brew_palettes( canvas.get_palette_level() + 1 );
}

void MainWindowForStandalone::on_btnAbout_clicked( void )
{
   Gtk::MessageDialog mdlg( 
      "<big><b>HilbertVis</b></big>\n"
      "<i>version " HILBERTVIS_VERSION ", built on " __DATE__ "</i>\n"
      "\n"
      "A tool to visualize extremely long data vectors.\n"
      "\n" 
      "Author: Simon Anders, EMBL-EBI, <tt>anders@fs.tum.de</tt>\n" 
      "\n"
      "For more information, see:\n"
      "<b><tt>   http://www.ebi.ac.uk/~anders/hilbert</tt></b>\n"
      "\n"
      "This is free software, licensed with the\n"
      "GNU General Public License, version 3.\n"
      "<small>See http://www.gnu.org/licenses/gpl-3.0-standalone.html\n"
      "for the text of the license</small>\n",
      true, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true );
   mdlg.run();
}

void MainWindowForStandalone::on_btnQuit_clicked( void )
{
   hide();
}

void MainWindowForStandalone::brew_palettes( int palette_level )
{
   const double gamma = 1.0;
   // get max_value
   
   double max_value = exp( ( palette_level / 4 ) * log( 10 ) );
   if( palette_level % 4 == 1 )
      max_value *= 1.8;
   else if( palette_level % 4 == 2 )
      max_value *= 3.2;
   else if( palette_level % 4 == 3 )
      max_value *= 5.6;
   else if( palette_level % 4 == -1 )
      max_value /= 1.8;
   else if( palette_level % 4 == -2 )
      max_value /= 3.2;
   else if( palette_level % 4 == -3 )
      max_value /= 5.6;

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

   paletteBar.set_palettes( max_value, &shared_palette, &shared_palette_neg );
   canvas.set_palette_level( palette_level );
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

void BidirColorizer::set_full_length( long full_length )
{
   if( dynamic_cast<StepDataVector*>( get_data() ) ) {
      if( dynamic_cast<StepDataVector*>( get_data() )->get_length() != full_length ) {
         pixmap.clear();  
         dynamic_cast<StepDataVector*>( get_data() )->set_full_length( full_length );
      }
   }
   else {
      std::cerr << "Warning: BidirColorizer::set_full_length used although data is not of\n";
      std::cerr << "  class StepDataVector.\n";
      // The whole full_length stuff should be moved from the data vector to the colorizer
   }
} 

int main( int argc, char *argv[] )
{
   Gtk::Main kit(argc, argv);
   std::vector< DataColorizer * > dataCols;
   dataCols.push_back( new EmptyColorizer() );
   MainWindowForStandalone win( &dataCols, false );   
   Gtk::Main::run( win );   
}      
