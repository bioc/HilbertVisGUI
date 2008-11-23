#include <assert.h>
#include <vector>
#include <set>
#include <iostream>
#include <string>
#include "display.h"
#include "window.h"
#include "colorizers.h"

// For handling the event loop under MS Windows I have first used the kludge that 
// Oleg Sklyar found for EBImage for now. Following Michael Lawrence's advice I have
// changed this now to the same style that he used in RGtk2. 
// I think this works corectly now but by defining the macro OLEGS_KLUDGE I can
// switch back to the other implementation:
//#define OLEGS_KLUDGE

#ifdef MSWINDOWS
  #ifdef OLEGS_KLUDGE
     // The following is a kludge that I took over from Oleg's code in EBImage.
     // It seems that the eventloop API is lacking in R for Windows and he
     // uses this symbol instead to hook in, which, however, seems to be defined
     // only if the RGUI application is used, not in command-line R!
     extern  __declspec(dllimport) void (* R_tcldo) ();
  #else
     #include <windows.h>
  #endif
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


enum binning_mode { maximum, average };

template< class data_el_t> 
class RDataVector : public DataVector {
  public:
   RDataVector( SEXP data_, long full_length_, binning_mode bmode_ );
   virtual ~RDataVector( );
   virtual double get_bin_value( long bin_start, long bin_size ) const;
   virtual long get_length( void ) const;
   virtual SEXP get_data( void ) const;
  protected:
   SEXP data;
   long full_length;  
   binning_mode bmode; 
};

class MainWindowForR : public MainWindow {
  public:
   MainWindowForR( std::vector< DataColorizer * > * dataCols, bool portrait,
      SEXP plot_callback_, std::vector< Gdk::Color > * palette_, std::vector< double> * palette_steps_ );
   ~MainWindowForR( );
  protected:
   SEXP plot_callback;
   std::vector< Gdk::Color > * palette;   
   std::vector< double > * palette_steps;
   virtual void on_canvasClicked( GdkEventButton * ev, long binLo, long binHi );
   virtual void on_hide( void );
};

template< class data_el_t >
RDataVector<data_el_t>::RDataVector( SEXP data_, long full_length_, binning_mode bmode_ ) 
{
   data = data_;
   full_length = full_length_;
   bmode = bmode_;
   env_protect( data );
}

template< class data_el_t >
RDataVector<data_el_t>::~RDataVector( ) {
   env_unprotect( data );
}
   
template<>   
double RDataVector<int>::get_bin_value( long bin_start, long bin_size ) const
{
   assert( bmode == maximum );
   assert( bin_size > 0);
   long max_idx = bin_start + bin_size;
   if( max_idx >= Rf_length( data ) )
      throw naValue();
   if( bin_start >= max_idx )
      bin_start = max_idx - 1;
   int mx = -INT_MAX;
   for( long i = bin_start; i < max_idx; i++ ) {
      if( INTEGER(data)[i] == R_NaInt)
         throw naValue();
      if( INTEGER(data)[i] > mx )
         mx = INTEGER(data)[i];
   }	 
   assert( mx > -INT_MAX );
   return mx;
}

template<>
double RDataVector<double>::get_bin_value( long bin_start, long bin_size ) const
{
   assert( bmode == maximum );
   assert( bin_size > 0);
   long max_idx = bin_start + bin_size;
   if( max_idx >= Rf_length( data ) )
      throw naValue( );
   if( bin_start >= max_idx )
      bin_start = max_idx - 1;
   double mx = R_NegInf;
   for( long i = bin_start; i < max_idx; i++ ) {
      if( R_IsNA( REAL(data)[i] ) || R_IsNaN( REAL(data)[i] ) )
         throw naValue( );
      if( REAL(data)[i] > mx )
         mx = REAL(data)[i];
   }	 
   assert( mx > -INT_MAX );
   return mx;
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

extern "C" void gtk_loop_iter( void * userData )
{
   while( Gtk::Main::events_pending() )
      Gtk::Main::iteration();
}

#ifdef MSWINDOWS

   #ifdef OLEGS_KLUDGE
   
   extern "C" void gtk_loop_iter_no_args( void )
   {
      gtk_loop_iter( NULL );
   }  
   
   #else

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
   // Instatiate GTK (only if it ahs not yet been instantiated)
   the_kit = new Gtk::Main( argc, argv, true );
	   
   // Hook up into R's event loop
   #ifndef MSWINDOWS

      if( ! GDK_DISPLAY() )
         Rf_error( "Cannot connect to X display." );
      addInputHandler( R_InputHandlers, ConnectionNumber(GDK_DISPLAY()), gtk_loop_iter, -1 );

   #else
   
      #ifdef OLEGS_KLUDGE
   
      // This is taken from Oleg's code in EBImage. It hooks the event handler
      // into the symbol defined above. Really a strange hack, I should check whether
      // there is no better way.
      R_tcldo = gtk_loop_iter_no_args;

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

extern "C" SEXP R_display_hilbert( SEXP args) 
{
   if( ! Rf_isPairList( args ) )
      Rf_error( "R_display_hilbert: Must be called with .External." );
      
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
   SEXP naColorR = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isInteger(paletteR) && ( Rf_length(naColorR) == 3 ) ) )
      Rf_error( "R_display_hilbert: Argument 'naColorR' must be 3 integers." );   
   SEXP palette_stepsR = CAR( arg ); arg = CDR( arg );
   if( !( Rf_isNumeric(palette_stepsR) && ( Rf_length(palette_stepsR) == Rf_length(paletteR) / 3 - 1 ) ) )
      Rf_error( "R_display_hilbert: Argument 'palette_stepsR' must be a numerical vector of length one"
         " less than the number of colors." );  
   SEXP full_lengths = CAR( arg ); arg = CDR( arg );
   if( !( (full_lengths == R_NilValue) || Rf_isInteger(full_lengths) ) )
      Rf_error( "R_display_hilbert: Argument 'full_lengths' must be NULL or a vector of integers." );   
   SEXP portrait = CAR( arg ); arg = CDR( arg );
   if( !Rf_isLogical( portrait ) )
      Rf_error( "R_display_hilbert: Argument 'portrait' must be a logical." );   


   std::vector< Gdk::Color > * palette = new std::vector< Gdk::Color >( Rf_nrows(paletteR) );
   for( unsigned i = 0; i < palette->size(); i++ )
      (*palette)[i].set_rgb_p( INTEGER(paletteR)[3*i] / 255., 
         INTEGER(paletteR)[3*i+1] / 255., INTEGER(paletteR)[3*i+2] / 255. );
	 
   std::vector< double > * palette_steps = new std::vector< double >( Rf_length( palette_stepsR ) );
   for( unsigned i = 0; i < palette_steps->size(); i++ )
      (*palette_steps)[i] = REAL(palette_stepsR)[i];

   Gdk::Color na_color;
   na_color.set_rgb_p( INTEGER(naColorR)[0] / 255., 
         INTEGER(naColorR)[1] / 255., INTEGER(naColorR)[2] / 255. );
	
   std::vector< DataColorizer * > * dataCols = new std::vector< DataColorizer * >( );
   int i = 0;
   while( arg != R_NilValue ) {
      if( !( Rf_isInteger( CAR(arg) ) || Rf_isReal( CAR(arg) ) ) 
            || ( i >= Rf_length( seqnames ) ) ) {
         for( int j = 0; j < i; j++ )
            delete (*dataCols)[j];
	 delete dataCols;
         char buf[300];
         snprintf( buf, 300, i < Rf_length( seqnames ) ? 
	    "R_display_hilbert: Data vector #%d is not a vector of integers." :
	    "R_display_hilbert: Data vector #%d does not have a name in second argument.", i+1 );
         Rf_error( buf );
      }
      Glib::ustring name = CHAR(STRING_ELT( seqnames, i ));
      long fl = ( ( full_lengths != R_NilValue ) && ( i < Rf_length(full_lengths) ) && 
	    ( INTEGER(full_lengths)[i] != R_NaInt ) ) ? 
	    INTEGER(full_lengths)[i] : Rf_length( CAR(arg) );
      DataVector * datavec;
      binning_mode bmode = maximum;
      if( Rf_isInteger( CAR(arg) ) ) 	    
         datavec = new RDataVector<int>( CAR(arg), fl, bmode );
      else  	     
         datavec = new RDataVector<double>( CAR(arg), fl, bmode );
      dataCols->push_back( new SimpleDataColorizer( datavec, name, palette, 
         na_color, palette_steps ) );
      i++;
      arg = CDR( arg );
   }   
   
   MainWindowForR * window = new MainWindowForR( dataCols, LOGICAL(portrait)[0],plot_callback, 
      palette, palette_steps );
   window->show( );
   window->raise( );

   while( Gtk::Main::events_pending() )
      Gtk::Main::iteration();   
     
   return R_NilValue;
}


extern "C" SEXP R_display_hilbert_3channel( SEXP dataRed, SEXP dataGreen, SEXP dataBlue, 
      SEXP naColorR, SEXP fullLength, SEXP portrait) 
{
   Gdk::Color na_color;
   na_color.set_rgb_p( INTEGER(naColorR)[0] / 255., 
         INTEGER(naColorR)[1] / 255., INTEGER(naColorR)[2] / 255. );
	
   std::vector< DataColorizer * > * dataCols = new std::vector< DataColorizer * >( );
   dataCols->push_back( new ThreeChannelColorizer( 
      new RDataVector<double>( dataRed,   INTEGER(fullLength)[0], maximum ),
      new RDataVector<double>( dataGreen, INTEGER(fullLength)[0], maximum ),
      new RDataVector<double>( dataBlue,  INTEGER(fullLength)[0], maximum ),
      "multi-channel data", na_color ) );
   MainWindowForR * window = new MainWindowForR( dataCols, LOGICAL(portrait)[0], 
      R_NilValue, NULL, NULL );
   window->show( );
   window->raise( );
   while( Gtk::Main::events_pending() )
      Gtk::Main::iteration();   
     
   return R_NilValue;
}



MainWindowForR::MainWindowForR( std::vector< DataColorizer * > * dataCols, bool portrait,
      SEXP plot_callback_, std::vector< Gdk::Color > * palette_, std::vector< double> * palette_steps_ )
 : MainWindow( dataCols, portrait )
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


