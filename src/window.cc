#include <cassert>
#include <iostream>
#include <string>
#include "window.h"
#include "display.h"
#include "error_bell_kludge.h"
#include "colorizers.h"
#include "ruler.h"

MainWindow::MainWindow( std::vector< DataColorizer * > * dataCols_, 
      bool portrait, bool file_buttons, bool palette_bar )
 : dataCols( dataCols_ ),
   canvas( (*dataCols_)[0] ),
   btnZoomOut4x( "Zoom out 4×" ),
   btnZoomOut64x( "Zoom out 64×" ),
   btnCoarser( "Coarser" ),
   btnFiner( "Finer" ),
   btnPrev( "Previous" ),
   btnNext( "Next" ),
   btnOpen( "Load" ),
   btnClose( "Unload" ),
   btnSave( "Save Img" ),
   btnDown( "Darker" ),
   btnUp( "Lighter" ),
   btnAbout( "About" ),
   btnQuit( "Quit" ),
   tbl1( file_buttons ? 2 : 1, 6, true ),
   frame1( "Bin under mouse cursor" ),
   frame2( "Full sequence" ),
   frame3( "Displayed part of sequence" ),
   frame4( "Effect of left mouse button" ),
   frame5( "Zoom out" ),
   frame6( "Pixel size" ),
   frame7( "Displayed data" ),
   frame8( "Color key" ),
   lblPos( "Position: ---" ),
   lblValue( "Value: ---:" ),
   rulerPointerPos( &canvas.get_adjPointerPos() ),
   rulerDispValueRange( &canvas.get_adjDisplayedValueRange() ),
   rbtnZoomIn4x( rbtngLeftMouse, "Zoom in 4×" ),
   rbtnZoomIn64x( rbtngLeftMouse, "Zoom in 64×" ),
   rbtnPlotLin( rbtngLeftMouse, "Linear plot" ),
   paletteBar( )
{

   hbox1.set_homogeneous( true );
   hbox1.pack_start( lblPos );
   hbox1.pack_start( lblValue );
   frame1.add( hbox1 );
   
   hbox2.set_homogeneous( true );
   hbox2.set_spacing( 7 );
   frame2.add( rulerDispValueRange );
   hbox2.pack_start( frame2 );
   frame3.add( rulerPointerPos );
   hbox2.pack_start( frame3 );
   
   hbox3.set_homogeneous( true );
   hbox3.set_spacing( 10 );
   hbox3.pack_start( rbtnZoomIn4x );
   hbox3.pack_start( rbtnZoomIn64x );
   hbox3.pack_start( rbtnPlotLin );
   frame4.add( hbox3 );
   
   hbox6.set_spacing( 7 );
   hbox6.set_homogeneous( true );
   hbox6.pack_start( btnZoomOut4x );
   hbox6.pack_start( btnZoomOut64x );
   frame5.add( hbox6 );
   hbox7.set_spacing( 7 );
   hbox7.set_homogeneous( true );
   hbox7.pack_start( btnCoarser );
   hbox7.pack_start( btnFiner );   
   frame6.add( hbox7 );
   hbox4.set_spacing( 7 );
   hbox4.set_homogeneous( true );
   hbox4.pack_start( frame5 );
   hbox4.pack_start( frame6 ); 

   tbl1.set_col_spacings( 10 );
   tbl1.attach( btnPrev, 0, 1, 0, 1 );
   tbl1.attach( cboxtSeqnames, 1, 5, 0, 1 );
   tbl1.attach( btnNext, 5, 6, 0, 1 );
   if( file_buttons ) {
      tbl1.attach( btnOpen,  0, 1, 1, 2 );
      tbl1.attach( btnClose, 1, 2, 1, 2 );
      tbl1.attach( btnSave,  2, 3, 1, 2 );
      tbl1.attach( btnAbout, 4, 5, 1, 2 );
      tbl1.attach( btnQuit,  5, 6, 1, 2 );
   }
   frame7.add( tbl1 );

   if( palette_bar ) {
      tbl2.set_col_spacings( 10 );
      tbl2.attach( btnDown, 0, 1, 0, 1 );
      tbl2.attach( paletteBar, 1, 5, 0, 1 );
      tbl2.attach( btnUp, 5, 6, 0, 1 );
      frame8.add( tbl2 );
   }
   
   vbox1.set_spacing( 10 );
   vbox1.pack_start( frame7, Gtk::PACK_SHRINK );
   tbl3.set_homogeneous( true );
   tbl3.attach( frame1, 0, 5, 0, 1 );
   vbox1.pack_start( tbl3, Gtk::PACK_SHRINK );
   vbox1.pack_start( hbox2, Gtk::PACK_SHRINK );
   if( palette_bar )
      vbox1.pack_start( frame8, Gtk::PACK_SHRINK );
   vbox1.pack_start( frame4, Gtk::PACK_SHRINK );
   vbox1.pack_start( hbox4, Gtk::PACK_SHRINK );

   if( portrait )
      box0 = manage( new Gtk::VBox() );
   else
      box0 = manage( new Gtk::HBox() );
   box0->set_spacing( 10 );
   box0->pack_start( canvas, Gtk::PACK_SHRINK );
   box0->pack_start( vbox1, Gtk::PACK_SHRINK );
   add( *box0 );
   
   for( unsigned i = 0; i < dataCols->size(); i++ ) {
      //char buf[150];
      //snprintf( buf, 150, "[%d]  %s", i, (*dataCols)[i]->get_name().c_str() );
      cboxtSeqnames.append_text( (*dataCols)[i]->get_name() ); //buf );
   }
   cboxtSeqnames.set_active( 0 );

   btnZoomOut4x.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnZoomOut4x_clicked) );
   btnZoomOut64x.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnZoomOut64x_clicked) );
   btnCoarser.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnCoarser_clicked) );
   btnFiner.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnFiner_clicked) );
   btnPrev.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnPrev_clicked) );
   btnNext.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnNext_clicked) );
   btnOpen.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnOpen_clicked) );
   btnClose.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnClose_clicked) );
   btnSave.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnSave_clicked) );
   btnQuit.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnQuit_clicked) );
   btnAbout.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnAbout_clicked) );
   btnUp.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnUp_clicked) );
   btnDown.signal_clicked().connect( 
      sigc::mem_fun( *this, &MainWindow::on_btnDown_clicked) );
   cboxtSeqnames.signal_changed().connect( 
      sigc::mem_fun( *this, &MainWindow::on_cboxtSeqnames_changed) );
   canvas.get_adjDisplayedValueRange().signal_changed().connect(
      sigc::mem_fun( *this, &MainWindow::on_adjDisplayedValueRange_changed) );
   canvas.get_adjPointerPos().signal_value_changed().connect(
      sigc::mem_fun( *this, &MainWindow::on_adjPointerPos_value_changed) );
   canvas.signal_mouse_clicked().connect(
      sigc::mem_fun( *this, &MainWindow::on_canvasClicked ) );

   set_title( "Hilbert curve data display" );

   show_all_children( );
}

void MainWindow::addColorizer( DataColorizer * dcol )
{
   //char buf[50];
   if( dynamic_cast<EmptyColorizer*>( (*dataCols)[0] ) ) {
      (*dataCols)[ 0 ] = dcol;
      //snprintf( buf, 35, "[%d] %s", 0, dcol->get_name().c_str() );
      cboxtSeqnames.clear_items( );
      cboxtSeqnames.append_text( dcol->get_name().c_str() ); //buf );
      cboxtSeqnames.set_active( 0 );
   } else {
      dataCols->push_back( dcol );
      //snprintf( buf, 35, "[%d] %s", dataCols->size()-1, dcol->get_name().c_str() );
      cboxtSeqnames.append_text( dcol->get_name().c_str() ); //buf );
   }
}

DataColorizer * MainWindow::removeCurrentColorizer( )
{
   int pos = cboxtSeqnames.get_active_row_number();
   DataColorizer * dcol = (*dataCols)[ pos ];
   dataCols->erase( dataCols->begin() + pos );
   
   if( dataCols->size() > 0 ) {
      cboxtSeqnames.remove_text( cboxtSeqnames.get_active_text( ) );
      cboxtSeqnames.set_active( 0 );
   } else {
      dataCols->push_back( new EmptyColorizer() );
      cboxtSeqnames.clear_items( );
      cboxtSeqnames.append_text( (*dataCols)[0]->get_name() );
      cboxtSeqnames.set_active( 0 );
   } 
   
   return dcol;
}


void MainWindow::on_btnZoomOut4x_clicked( void )
{
   try{
      if( canvas.get_zoom_level() > 0)
         canvas.set_zoom( canvas.get_zoom_level() - 1, 
            canvas.get_zoom_offset() >> 2 );
      else
         error_bell( );
   } catch( HilbertValueError e ) {
      error_bell( );
   }
}

void MainWindow::on_btnZoomOut64x_clicked( void )
{
   if( canvas.get_zoom_level() == 0 ) {
      error_bell();
   } else if( canvas.get_zoom_level() <= 3 ) { 
      canvas.set_zoom( 0, 0 );
   } else {
      canvas.set_zoom( canvas.get_zoom_level() - 3, canvas.get_zoom_offset() >> 6 );
   }
}

void MainWindow::on_btnCoarser_clicked( void )
{
   try {
      canvas.set_pixel_size_level( canvas.get_pixel_size_level() + 1 );
   } catch( HilbertValueError e ) {
      error_bell();
   }
}

void MainWindow::on_btnFiner_clicked ( void )
{
   try {
      canvas.set_pixel_size_level( canvas.get_pixel_size_level() - 1 );
   } catch( HilbertValueError e ) {
      error_bell();
   }
}

void MainWindow::on_btnPrev_clicked( void )
{
  if( cboxtSeqnames.get_active_row_number() > 0 )
     cboxtSeqnames.set_active( cboxtSeqnames.get_active_row_number() - 1 );
  else
     error_bell ();
}

void MainWindow::on_btnNext_clicked( void )
{
  if( (unsigned) cboxtSeqnames.get_active_row_number() < dataCols->size()-1 )
     cboxtSeqnames.set_active( cboxtSeqnames.get_active_row_number() + 1 );
  else
     error_bell ();
}

void MainWindow::on_btnOpen_clicked( void )
{
}

void MainWindow::on_btnClose_clicked( void )
{
}

void MainWindow::on_btnSave_clicked( void )
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
         Gtk::MessageDialog mdlg( std::string( "The file " ) + 
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

void MainWindow::on_btnDown_clicked( void )
{
}

void MainWindow::on_btnUp_clicked( void )
{
}

void MainWindow::on_btnAbout_clicked( void )
{
}

void MainWindow::on_btnQuit_clicked( void )
{
}

void MainWindow::on_cboxtSeqnames_changed( void )
{
   if( cboxtSeqnames.get_active_row_number() >= 0 )
      canvas.set_dataCol( (*dataCols)[ cboxtSeqnames.get_active_row_number() ] );
}

void MainWindow::on_adjDisplayedValueRange_changed( void )
{
};

void MainWindow::on_adjPointerPos_value_changed( void )
{
   const InvalidableAdjustment & adj = canvas.get_adjPointerPos();
   if( adj.is_valid( ) ) {
      lblPos.set_text( "Position: " + int2strB( (int) ( adj.get_value() + adj.get_page_size() / 2 ) ) );
      lblValue.set_text( "Value: xxx" );
   } else {
      lblPos.set_text( "Position: ---" );
      lblValue.set_text( "Value: ---" );
   }
      
}

void MainWindow::on_canvasClicked( GdkEventButton * ev, long binLo, long binHi )
{
   try{ 
      if( ev->type == GDK_BUTTON_PRESS && ev->button == 1 ) {
         if( rbtnZoomIn4x.get_active( ) ) {
            int quadrant = int( 4 * ( ( (binLo+binHi)/2/canvas.get_bin_size() - 
               canvas.get_begin() ) / canvas.get_num_pixels() ) );
            canvas.set_zoom( canvas.get_zoom_level() + 1, 
               ( canvas.get_zoom_offset() << 2 ) | quadrant );         
         } else if( rbtnZoomIn64x.get_active( ) ) {
            int tant = int( 64 * ( ( (binLo+binHi)/2/canvas.get_bin_size() - 
               canvas.get_begin() ) / canvas.get_num_pixels() ) );
            canvas.set_zoom( canvas.get_zoom_level() + 3, 
               ( canvas.get_zoom_offset() << 6 ) | tant );         
         } else if( rbtnPlotLin.get_active( ) ) {
            // plot lin
         }
      }
   } catch (HilbertValueError e) {
      error_bell();
   }
}
