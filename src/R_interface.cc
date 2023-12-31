#include <cassert>
#include <vector>
#include <set>
#include <iostream>
#include <string>
#include <limits>
#include <cmath>
#include "display.h"
#include "window.h"
#include "colorizers.h"

#ifdef MSWINDOWS
  #include <windows.h>
#endif

#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Arith.h>

#ifndef MSWINDOWS
  #include <gdk/gdkx.h>
  #include <R_ext/eventloop.h>
#endif  

#include "R_env_prot.h"


enum binning_mode { maximum, minimum, absmax, average };

template< class data_el_t> 
class RDataVector : public DataVector {
  public:
   RDataVector( SEXP data_, long full_length_, binning_mode bmode_, bool pad_with_zeros_=false );
   virtual ~RDataVector( );
   virtual double get_bin_value( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
   virtual SEXP get_data( void ) const;
  protected:
   SEXP data;
   long full_length;  
   binning_mode bmode; 
   bool pad_with_zeros;
};

template< class data_el_t> 
class RRleDataVector : public RDataVector<data_el_t> {
  public:
   RRleDataVector( SEXP data_, long full_length_, binning_mode bmode_, bool pad_with_zeros_=false );
   virtual double get_bin_value( long bin_start, long bin_size ) const;
  protected:
   SEXP values, lengths;
   inline data_el_t get_interval_value( int ivIdx ) const;
   mutable int i_start, pos_start;
};

class MainWindowForR : public MainWindow {
  public:
   MainWindowForR( std::vector< DataColorizer * > * dataCols, bool portrait,
      SEXP plot_callback_, std::vector< Gdk::Color > * palette_, 
      std::vector< double> * palette_steps_, bool palette_bar = false );
   ~MainWindowForR( );
  protected:
   SEXP plot_callback;
   std::vector< Gdk::Color > * palette;   
   std::vector< double > * palette_steps;
   virtual void on_canvasClicked( GdkEventButton * ev, long binLo, long binHi );
   virtual void on_hide( void );
};

class MainWindowForRForBidir : public MainWindowForR {
  public:
   MainWindowForRForBidir( std::vector< DataColorizer * > * dataCols, bool portrait,
      SEXP plot_callback_, std::vector< Gdk::Color > * palette_, std::vector< Gdk::Color > * palette_neg_,
      std::vector< double > * palette_steps_, double max_palette_value_ );
   ~MainWindowForRForBidir( );
  protected:
   std::vector< Gdk::Color > * palette_neg;   
   std::vector< double > * palette_steps;
   double max_palette_value;
   virtual void on_btnUp_clicked( void );
   virtual void on_btnDown_clicked( void );   
   void set_palette_level( double palette_level );
   Gtk::Frame frame0b;
};

template< class data_el_t >
RDataVector<data_el_t>::RDataVector( SEXP data_, long full_length_, binning_mode bmode_, bool pad_with_zeros_ ) 
{
   data = data_;
   full_length = full_length_;
   bmode = bmode_;
   pad_with_zeros = pad_with_zeros_;
   env_protect( data );
}

template< class data_el_t >
RDataVector<data_el_t>::~RDataVector( ) {
   env_unprotect( data );
}
   
template<>
double RDataVector<int>::get_bin_value( long bin_start, long bin_size ) const
{
   assert( bin_size > 0);
   if( bin_start >= Rf_length( data ) ) {
      if( pad_with_zeros )
         return 0;
      else
         throw naValue( ); 
   }
   long max_idx = bin_start + bin_size;
   if( max_idx > Rf_length( data ) )
      max_idx = Rf_length( data );
   //if( bin_start >= max_idx )  // What was this for?
   //   bin_start = max_idx - 1;
   bool first = true;
   int val;
   long count = 0;   
   for( long i = bin_start; i < max_idx; i++ ) {
      if( R_IsNA( INTEGER(data)[i] ) )
         continue;
      if( first ) {
         val = INTEGER(data)[i];
	 first = false;
	 count++;	 
      } else {
         switch( bmode ) {
	    case maximum:
	       if( INTEGER(data)[i] > val )
	          val = INTEGER(data)[i];
	       break;
	    case minimum:
	       if( INTEGER(data)[i] < val )
	          val = INTEGER(data)[i];
	       break;
	    case absmax:
	       if( abs(INTEGER(data)[i]) > abs(val) )
	          val = INTEGER(data)[i];
	       break;
	    case average:
	       val += INTEGER(data)[i];
	       count++;
	       break;
	    default:
	       Rprintf( "Internal error: Unknown binning mode %d.\n", bmode );
	 }
      }
   }         
   if( first ) {
      if( pad_with_zeros )
         return 0;
      else
         throw naValue( ); 
   }
   if( bmode != absmax )
      return val;
   else
      return (double) val / count;
}


template<>
double RDataVector<double>::get_bin_value( long bin_start, long bin_size ) const
{
   assert( bin_size > 0);
   if( bin_start >= Rf_length( data ) ) {
      if( pad_with_zeros )
         return 0;
      else
         throw naValue( ); 
   }
   long max_idx = bin_start + bin_size;
   if( max_idx > Rf_length( data ) )
      max_idx = Rf_length( data );
   //if( bin_start >= max_idx )  // What was this for?
   //   bin_start = max_idx - 1;
   bool first = true;
   double val;
   long count = 0;
   for( long i = bin_start; i < max_idx; i++ ) {
      if( R_IsNA( REAL(data)[i] ) || R_IsNaN( REAL(data)[i] ) )
         continue;
      if( first ) {
         val = REAL(data)[i];
	 first = false;
	 count++;
      } else {
         switch( bmode ) {
	    case maximum:
	       if( REAL(data)[i] > val )
	          val = REAL(data)[i];
	       break;
	    case minimum:
	       if( REAL(data)[i] < val )
	          val = REAL(data)[i];
	       break;
	    case absmax:
	       if( fabs(REAL(data)[i]) > fabs(val) )
	          val = REAL(data)[i];
	       break;
	    case average:
	       val += REAL(data)[i];
	       count++;
	       break;
	    default:
	       Rprintf( "Internal error: Unknown binning mode %d.\n", bmode );
	 }
      }
   }         
   if( first ) {
      if( pad_with_zeros )
         return 0;
      else
         throw naValue( ); 
   }
   if( bmode == absmax )
      val /= count;
   return val;
}

template< class data_el_t >
long RDataVector<data_el_t>::get_length( void ) const
{
   return full_length;
}

template< class data_el_t >
SEXP RDataVector<data_el_t>::get_data( void ) const
{
   return data;
}

template< class data_el_t >
RRleDataVector<data_el_t>::RRleDataVector( SEXP data_, long full_length_, binning_mode bmode_,
      bool pad_with_zeros_ ) 
 : RDataVector<data_el_t>( data_, full_length_, bmode_, pad_with_zeros_ )
{
   values  = R_do_slot( this->data, Rf_install("values") );
   lengths = R_do_slot( this->data, Rf_install("lengths") );
   i_start = 0;
   pos_start = 0;
}

template< class data_el_t>
double RRleDataVector<data_el_t>::get_bin_value( long bin_start, long bin_size ) const
{
   if( pos_start + INTEGER(lengths)[i_start] >= bin_start ) {
      i_start = 0;
      pos_start = 0;
   }
   int pos = pos_start;
   data_el_t mx = std::numeric_limits<data_el_t>::min();
   data_el_t mn = std::numeric_limits<data_el_t>::max();
   int i;
   for( i = i_start; i < LENGTH(values); i++ ) {
      if( pos + INTEGER(lengths)[i] >= bin_start ) {
         if( get_interval_value( i ) > mx)
            mx = get_interval_value( i );
         if( get_interval_value( i ) < mn )
            mn = get_interval_value( i );
      } 
      pos += INTEGER(lengths)[i];
      if( pos > bin_start + bin_size )
         break;
   }
   if( i >= LENGTH(values) ) {
      if( this->pad_with_zeros )
         return 0;
      else
         throw naValue( ); 
   }
   
   i_start = i - 1;
   pos_start = pos - INTEGER(lengths)[i] - INTEGER(lengths)[i-1];
   switch( this->bmode ) {
      case maximum:
         return mx;
      case minimum:
         return mn;
      case absmax:
         return std::abs(mx) > std::abs(mn) ? mx : mn;
      case average:
         Rf_error( "Binning mode not yet supported!" );
      default:
	 Rprintf( "Internal error: Unknown binning mode %d.\n", this->bmode );
         return 0;
   }
}

template<>
inline int RRleDataVector<int>::get_interval_value( int ivIdx ) const
{
   return INTEGER(values)[ivIdx];
}

template<>
inline double RRleDataVector<double>::get_interval_value( int ivIdx ) const
{
   return REAL(values)[ivIdx];
}


extern "C" void gtk_loop_iter( void * userData )
{
   while( Gtk::Main::events_pending() )
      Gtk::Main::iteration();
}

#ifdef MSWINDOWS

   // This code is taken from Michael Lawrence's RGtk2 package,
   // from file Rgtk.c. It starts a synchronized thread to run
   // the GTK+ event loop in, as the Revent loop is not exposed
   // in the API of R for Windows.

   #define HWND_MESSAGE                ((HWND)-3)

   #define RGTK2_ITERATE WM_USER + 101

   DWORD WINAPI R_gtk_thread_proc( LPVOID lpParam ) 
   {
      while( true ) {
         PostMessage( (HWND)lpParam, RGTK2_ITERATE, 0, 0 );
         Sleep( 20 );
      }
      return 0;
   }

   LRESULT CALLBACK R_gtk_win_proc( HWND hwnd, UINT message, 
      WPARAM wParam, LPARAM lParam )
   {
      if( message == RGTK2_ITERATE ) {
         gtk_loop_iter( NULL );
         return 1;
      }
      return DefWindowProc(hwnd, message, wParam, lParam);
   }

#endif


// The command line arguments are needed to initialize GTK+
char ** argv;
int argc;

// This is the instance of the gtkmm main loop
Gtk::Main * the_kit = NULL;

// This keeps track of open windows in order to close them after unloading
std::set< MainWindowForR * > all_open_windows;

#ifndef SO_NAME
#error Define a preprocessor macro SO_NAME with the name of the output file \
for the shared object to be loaded by R.
#endif

#define MACRO_CONCAT_AUX( a, b ) a ## b
#define SYMBOL_CONCAT( a, b ) MACRO_CONCAT_AUX( a, b )
#define STRINGIFY( a ) #a

extern "C" void SYMBOL_CONCAT( R_init_, SO_NAME ) (DllInfo * winDll) 
{   

   // Instatiate GTK (we do this twice, once to check whether it works
   // and the for real
   #ifndef MSWINDOWS
      if( ! gtk_init_check( &argc, &argv) ) {
      Rprintf( "\n | Cannot connect to an X display. Most functionality of \n"
         " | HilbertVisGUI will be unavailable. Make sure that the DISPLAY\n"
         " | environment variable is set properly.\n\n" );
      Rf_warning( "Cannot connect to X display." );
      return;
      }
   #endif
   the_kit = new Gtk::Main( argc, argv, true );
           
   // Hook up into R's event loop
   #ifndef MSWINDOWS

      addInputHandler( R_InputHandlers, ConnectionNumber(GDK_DISPLAY()), gtk_loop_iter, -1 );

   #else
   
      // Taken from Michael's code as well:

      /* Create a dummy window for receiving messages */
      LPCTSTR cls = "HilbertCurveDisplay";
      HINSTANCE instance = GetModuleHandle( NULL );
      WNDCLASS wndclass = { 0, R_gtk_win_proc, 0, 0, instance, NULL, 0, 0, NULL, cls };
      RegisterClass( &wndclass );
      HWND win = CreateWindow( cls, NULL, 0, 1, 1, 1, 1, HWND_MESSAGE, NULL, instance, NULL);

      /* Create a thread that will post messages to our window on this thread */
      HANDLE thread = CreateThread( NULL, 0, R_gtk_thread_proc, win, 0, NULL );
      SetThreadPriority( thread, THREAD_PRIORITY_IDLE );
            
   #endif   
}

extern "C" void SYMBOL_CONCAT( R_unload_, SO_NAME ) (DllInfo * winDll) 
{
   #ifndef MSWINDOWS
      removeInputHandler( &R_InputHandlers, 
         getInputHandler( R_InputHandlers, ConnectionNumber(GDK_DISPLAY()) ) );
   #else
   
      // Strictly speaking, I should add code here to delete the dummy window and the thread.
      // But neither Michael nor Oleg bothered to write an unload function and so I won't, either,
      // especially as my Windows knowledge is way to rusty to remember how to do it.
      
   #endif
   while( ! all_open_windows.empty() ) {
      MainWindowForR * w = *all_open_windows.begin();
      w->hide( );  // this calls delete in turn, and the destructor erases it from all_open_windows
   }
   gtk_loop_iter( NULL );  // process the remaining events, especially the hidings
   delete the_kit;
}

RDataVector<double> * create_normal_or_Rle_RDataVector( SEXP data, long full_length, binning_mode bmode )
{
   if( Rf_isReal( data ) )
      return new RDataVector<double>( data, full_length, bmode, true );
   else if( Rf_isObject( data ) && Rf_inherits( data, "Rle" ) &&
         Rf_isReal( R_do_slot( data, Rf_install("values") ) ) ) 
      return new RRleDataVector<double>( data, full_length, bmode, true );
   else 
      Rf_error( "Illegal data vector (must be a numeric vector or a numeric Rle vector)." );
}

extern "C" SEXP R_display_hilbert_3channel( SEXP dataRed, SEXP dataGreen, SEXP dataBlue, 
      SEXP naColorR, SEXP fullLength, SEXP portrait) 
{
   Gdk::Color na_color;
   na_color.set_rgb_p( INTEGER(naColorR)[0] / 255., 
         INTEGER(naColorR)[1] / 255., INTEGER(naColorR)[2] / 255. );
        
   DataColorizer * col = new ThreeChannelColorizer( 
      create_normal_or_Rle_RDataVector( dataRed,   INTEGER(fullLength)[0], maximum ),
      create_normal_or_Rle_RDataVector( dataGreen, INTEGER(fullLength)[0], maximum ),
      create_normal_or_Rle_RDataVector( dataBlue,  INTEGER(fullLength)[0], maximum ),
      "multi-channel data", na_color );
   std::vector< DataColorizer * > * dataCols = new std::vector< DataColorizer * >( );
   dataCols->push_back( col );
   MainWindowForR * window = new MainWindowForR( dataCols, LOGICAL(portrait)[0], 
      R_NilValue, NULL, NULL );
   window->show( );
   window->raise( );
   while( Gtk::Main::events_pending() )
      Gtk::Main::iteration();   
     
   return R_NilValue;
}

extern "C" SEXP R_display_hilbert( SEXP args) 
{
   if( ! Rf_isPairList( args ) )
      Rf_error( "R_display_hilbert: Must be called with .External." );
      
   #ifndef MSWINDOWS
      if( ! GDK_DISPLAY() ) 
         Rf_error( "R_display_hilbert: X display unavailable." );
   #endif 
      
   SEXP arg = CDR( args );
   SEXP plot_callback = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isNull(plot_callback) || Rf_isFunction(plot_callback) ) )
      Rf_error( "R_display_hilbert: Argument 'plot_callback' must be a callback function or NULL." );
   SEXP seqnames = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isString(seqnames) ) )
      Rf_error( "R_display_hilbert: Argument 'seqnames' must be a vector of strings." );   
   SEXP paletteR = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isInteger(paletteR) && ( Rf_length(paletteR) % 3 == 0 ) ) )
      Rf_error( "R_display_hilbert: Argument 'paletteR' must be a 3-row matrix of integers." );   
   SEXP palette_negR = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isInteger(palette_negR) && ( Rf_length(palette_negR) % 3 == 0 ) ) )
      Rf_error( "R_display_hilbert: Argument 'palette_negR' must be a 3-row matrix of integers." );   
   SEXP naColorR = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isInteger(paletteR) && ( Rf_length(naColorR) == 3 ) ) )
      Rf_error( "R_display_hilbert: Argument 'naColorR' must be 3 integers." );   
   SEXP max_palette_valueR = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isReal(max_palette_valueR) && ( Rf_length(max_palette_valueR) == 1 ) ) )
      Rf_error( "R_display_hilbert: Argument 'max_palette_valueR' must be a scalar numeric value." );   
   SEXP full_lengths = CAR( arg ); arg = CDR( arg );
   if( !( (full_lengths == R_NilValue) || Rf_isInteger(full_lengths) ) )
      Rf_error( "R_display_hilbert: Argument 'full_lengths' must be NULL or a vector of integers." );   
   SEXP portrait = CAR( arg ); arg = CDR( arg );
   if( !Rf_isLogical( portrait ) )
      Rf_error( "R_display_hilbert: Argument 'portrait' must be a logical." );   


   std::vector< Gdk::Color > * palette = new std::vector< Gdk::Color >( Rf_length(paletteR) / 3 );
   for( unsigned i = 0; i < palette->size(); i++ )
      (*palette)[i].set_rgb_p( INTEGER(paletteR)[3*i] / 255., 
         INTEGER(paletteR)[3*i+1] / 255., INTEGER(paletteR)[3*i+2] / 255. );

   std::vector< Gdk::Color > * palette_neg = new std::vector< Gdk::Color >( Rf_length(palette_negR) / 3 );
   for( unsigned i = 0; i < palette_neg->size(); i++ )
      (*palette_neg)[i].set_rgb_p( INTEGER(palette_negR)[3*i] / 255., 
         INTEGER(palette_negR)[3*i+1] / 255., INTEGER(palette_negR)[3*i+2] / 255. );
         
   Gdk::Color na_color;
   na_color.set_rgb_p( INTEGER(naColorR)[0] / 255., 
         INTEGER(naColorR)[1] / 255., INTEGER(naColorR)[2] / 255. );
        
   std::vector< double > * palette_steps = new std::vector< double >( palette->size() - 1 );
   double palette_level = log( REAL( max_palette_valueR )[ 0 ] ) / log( 10. ) * 4;
   for( int i = 0; i < palette_steps->size(); i++ )
      (*palette_steps)[i] = REAL( max_palette_valueR )[ 0 ] / palette->size() * (i+1);
   // The for loop above is copied from MainWindowForRForBidir::set_palette_level,
   // which is bad; it should not appear twice

        
   std::vector< DataColorizer * > * dataCols = new std::vector< DataColorizer * >( );
   int i = 0;
   while( arg != R_NilValue ) {      
      if( !( Rf_isInteger( CAR(arg) ) || Rf_isReal( CAR(arg) ) ||
            ( Rf_isObject( CAR(arg) ) && Rf_inherits( CAR(arg), "Rle" ) ) ) 
            || ( i >= Rf_length( seqnames ) ) ) {
         for( int j = 0; j < i; j++ )
            delete (*dataCols)[j];
         delete dataCols;
         char buf[300];
         snprintf( buf, 300, i < Rf_length( seqnames ) ? 
            "R_display_hilbert: Data vector #%d is not a vector of integers or reals notan Rle object." :
            "R_display_hilbert: Data vector #%d does not have a name in second argument.", i+1 );
         Rf_error( buf );
      }
      Glib::ustring name = CHAR(STRING_ELT( seqnames, i ));
      long fl = ( ( full_lengths != R_NilValue ) && ( i < Rf_length(full_lengths) ) && 
            ( INTEGER(full_lengths)[i] != R_NaInt ) ) ? 
            INTEGER(full_lengths)[i] : Rf_length( CAR(arg) );
      DataVector * datavec;
      binning_mode bmode = absmax;
      if( Rf_isInteger( CAR(arg) ) )             
         datavec = new RDataVector<int>( CAR(arg), fl, bmode );
      else if( Rf_isReal( CAR(arg) ) )                            
         datavec = new RDataVector<double>( CAR(arg), fl, bmode );
      else if( Rf_isObject( CAR(arg) ) && Rf_inherits( CAR(arg), "Rle" ) ) {
         if( Rf_isInteger( R_do_slot( CAR(arg), Rf_install("values") ) ) )
            datavec = new RRleDataVector<int>( CAR(arg), fl, bmode );
         else if( Rf_isReal( R_do_slot( CAR(arg), Rf_install("values") ) ) )
            datavec = new RRleDataVector<double>( CAR(arg), fl, bmode );
         else {
            for( int j = 0; j < i; j++ )
               delete (*dataCols)[j];
            delete dataCols;
            Rf_error( "R_hilbert_display: Can only deal with Rle objects of type integer or real." );
         }
      } else
         Rf_error( "R_hilbert_display: internal error: got confused about argument type" );
      dataCols->push_back( new BidirColorizer( datavec, name, palette, palette_neg,
         na_color, palette_steps ) );
      i++;
      arg = CDR( arg );
   }   
   
   MainWindowForRForBidir * window = new MainWindowForRForBidir( dataCols, LOGICAL(portrait)[0], plot_callback, 
      palette, palette_neg, palette_steps, REAL( max_palette_valueR )[ 0 ] );
      
   window->show( );
   window->raise( );

   while( Gtk::Main::events_pending() )
      Gtk::Main::iteration();   
     
   return R_NilValue;
}



MainWindowForR::MainWindowForR( std::vector< DataColorizer * > * dataCols, bool portrait,
      SEXP plot_callback_, std::vector< Gdk::Color > * palette_, std::vector< double> * palette_steps_,
      bool palette_bar )
 : MainWindow( dataCols, portrait, false, palette_bar )
{
   plot_callback = plot_callback_;
   palette = palette_;
   palette_steps = palette_steps_;   
   all_open_windows.insert( this );   
}

MainWindowForR::~MainWindowForR( )
{
   for( unsigned i = 0; i < dataCols->size(); i++ )
      delete (*dataCols)[i];   
   delete dataCols;
   delete palette;
   delete palette_steps;
   all_open_windows.erase( this );
}

void MainWindowForR::on_hide( void )
{
   MainWindow::on_hide( );
   delete this;  // Is this allowed?? If no: better use a signal to the kit?
}

void MainWindowForR::on_canvasClicked( GdkEventButton * ev, long binLo, long binHi )
{
   if( ev->type == GDK_BUTTON_PRESS && ev->button == 1 && rbtnPlotLin.get_active( ) ) {
      if( ! Rf_isFunction( plot_callback ) ) {
         Gtk::MessageDialog( "You must supply an R callback function to use the 'linear plot' feature.", 
            false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true ).run( );
         return;
      }

      get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );
      get_toplevel()->get_window()->set_cursor( Gdk::Cursor(Gdk::WATCH) );

      SEXP info, infonames, a;
      Rf_protect( info = Rf_allocVector( VECSXP,  7) );
      for( int i = 0; i < 6; i++ ) {
         Rf_protect( a = Rf_allocVector( INTSXP, 1 ) );
         switch( i ) {
            case 0: INTEGER(a)[0] = 1 + binLo; break;
            case 1: INTEGER(a)[0] = 1 + ( binLo + binHi ) / 2; break;
            case 2: INTEGER(a)[0] = 1 + binHi; break;
            case 3: INTEGER(a)[0] = 1 + (int) round( canvas.get_adjPointerPos().get_lower() ); break;
            case 4: INTEGER(a)[0] = 1 + (int) round( canvas.get_adjPointerPos().get_upper() ); break;
            case 5: INTEGER(a)[0] = cboxtSeqnames.get_active_row_number() + 1; break;
         }
         SET_VECTOR_ELT( info, i, a );
      }
      Rf_protect( a = Rf_allocVector( STRSXP, 1 ) );
      SEXP chr;
      Rf_protect( chr = Rf_mkChar( canvas.get_dataCol()->get_name().c_str() ) );
      SET_STRING_ELT( a, 0, chr );
      SET_VECTOR_ELT( info, 6, a );
      
      Rf_protect( infonames = Rf_allocVector( STRSXP, 7 ) );
      for( int i = 0; i < 7; i++ ) {
         switch( i ) {
            case 0: chr = Rf_mkChar( "binLo" );   break;
            case 1: chr = Rf_mkChar( "bin" );     break;
            case 2: chr = Rf_mkChar( "binHi" );   break;
            case 3: chr = Rf_mkChar( "dispLo" );  break;
            case 4: chr = Rf_mkChar( "dispHi" );  break;
            case 5: chr = Rf_mkChar( "seqIdx" );  break;
            case 6: chr = Rf_mkChar( "seqName" ); break;
         }
         Rf_protect( chr );
         SET_STRING_ELT( infonames, i, chr );
      }
      Rf_namesgets( info, infonames );

      SEXP call, data;
      DataVector * dv = ((SimpleDataColorizer*) canvas.get_dataCol())->get_data();
      Rf_protect( data = ((RDataVector<int>*) dv)->get_data() ); 
         /* I hope it's ok to specialize to 'int' here althought it might also be 'double'. */
      Rf_protect( call = Rf_lang3( plot_callback, data, info ) );
      Rf_eval( call, R_GlobalEnv );
      Rf_unprotect( 19 );

      get_window()->set_cursor( Gdk::Cursor(Gdk::TCROSS) );
      get_toplevel()->get_window()->set_cursor( );
   } else
      MainWindow::on_canvasClicked( ev, binLo, binHi );
}


MainWindowForRForBidir::MainWindowForRForBidir( std::vector< DataColorizer * > * dataCols, 
      bool portrait, SEXP plot_callback_, std::vector< Gdk::Color > * palette_, 
      std::vector< Gdk::Color > * palette_neg_, std::vector< double > * palette_steps_, double max_palette_value_ )
 : MainWindowForR( dataCols, portrait, plot_callback_, palette_, NULL, true ),
   palette_neg( palette_neg_ ),
   palette_steps( palette_steps_ ),
   max_palette_value( max_palette_value_ )
{
   paletteBar.set_palettes( max_palette_value, palette, palette_neg );

   // Add a save button (this would fit better in the parent constructor)
   frame0b.set_label(" ");
   frame0b.add( btnSave );
   tbl3.attach( frame0b, 5, 6, 0, 1 );
   btnSave.show();
   frame0b.show();
}

MainWindowForRForBidir::~MainWindowForRForBidir( )
{
   delete palette_neg;
}

void MainWindowForRForBidir::on_btnDown_clicked( void )
{
   double palette_level = log( max_palette_value) / log( 10. ) * 4;
   set_palette_level( palette_level - 1 );
}

void MainWindowForRForBidir::on_btnUp_clicked( void )
{
   double palette_level = log( max_palette_value) / log( 10. ) * 4;
   set_palette_level( palette_level + 1 );
}

void MainWindowForRForBidir::set_palette_level( double palette_level )
{
   max_palette_value = exp( palette_level / 4. * log( 10. ) );
   for( int i = 0; i < palette_steps->size(); i++ )
      (*palette_steps)[i] = max_palette_value / palette->size() * (i+1);
   paletteBar.set_palettes( max_palette_value, palette, palette_neg );
   canvas.set_palette_level( palette_level );
}

extern "C" SEXP dotsapplyR( SEXP args ) {
   SEXP fun = CADR( args );
   if( ! Rf_isFunction(fun) )
      Rf_error( "dotsapply: First argument must be a function." );
   SEXP env = CADDR( args );
   if( ! Rf_isEnvironment(env) )
      Rf_error( "dotsapply: Second argument must be an environment." );
   int num = 0;
   SEXP dots0 = CDR( CDR( CDR( args ) ) );
   for( SEXP dots = dots0; dots != R_NilValue; dots = CDR(dots) )
      num++;
      
   SEXP res;
   Rf_protect( res = Rf_allocVector( VECSXP, num ) );
   int i = 0;
   for( SEXP dots = dots0; dots != R_NilValue; dots = CDR(dots) ) {
      SEXP call;
      Rf_protect( call = Rf_lang2( fun, CAR( dots ) ) );
      SET_VECTOR_ELT( res, i, Rf_eval( call, env ) );
      Rf_unprotect( 1 );
      i++;
   }
   Rf_unprotect( 1 );
   return res;
}


