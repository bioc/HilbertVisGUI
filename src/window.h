#ifndef HILBERT_WINDOW_H
#define HILBERT_WINDOW_H

#include <vector>
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>
#include "display.h"
#include "ruler.h"


class MainWindow : public Gtk::Window {
  
  public:
   MainWindow( std::vector< DataColorizer * > * dataCols, bool portrait = true,
      bool fileButtons = false );
      
   void addColorizer( DataColorizer * dcol );
   DataColorizer * removeCurrentColorizer( );

  protected:
   // Signal handlers:
   virtual void on_btnZoomOut4x_clicked( void );
   virtual void on_btnZoomOut64x_clicked( void );
   virtual void on_btnCoarser_clicked( void );
   virtual void on_btnFiner_clicked( void );
   virtual void on_btnPrev_clicked( void );
   virtual void on_btnNext_clicked( void );
   virtual void on_btnOpen_clicked( void );
   virtual void on_btnClose_clicked( void );
   virtual void on_btnSave_clicked( void );
   virtual void on_btnDown_clicked( void );
   virtual void on_btnUp_clicked( void );
   virtual void on_btnAbout_clicked( void );
   virtual void on_btnQuit_clicked( void );
   virtual void on_cboxtSeqnames_changed( void );
   virtual void on_adjDisplayedValueRange_changed( void );
   virtual void on_adjPointerPos_value_changed( void );
   virtual void on_canvasClicked( GdkEventButton * ev, long binLo, long BinHi );

   // dataColorizers:
   std::vector<DataColorizer * > * dataCols;

   // Member widgets:
   HilbertCurveDisplay canvas;
   Gtk::Button btnZoomOut4x, btnZoomOut64x, btnCoarser, btnFiner, btnPrev, btnNext;
   Gtk::Button btnOpen, btnClose, btnSave, btnDown, btnUp, btnAbout, btnQuit;
   Gtk::HBox hbox1, hbox2, hbox3, hbox4, hbox5, hbox6, hbox7, hbox8;
   Gtk::VBox vbox1;
   Gtk::Box * box0;
   Gtk::Table tbl1, tbl2;
   Gtk::Frame frame1, frame2, frame3, frame4, frame5, frame6, frame7, frame8;
   Gtk::Label lblPos, lblValue;
   Ruler rulerPointerPos;
   Ruler rulerDispValueRange;
   Gtk::ComboBoxText cboxtSeqnames;
   Gtk::RadioButtonGroup rbtngLeftMouse;
   Gtk::RadioButton rbtnZoomIn4x;
   Gtk::RadioButton rbtnZoomIn64x, rbtnPlotLin;
   PaletteBar paletteBar;
};

#endif //HILBERT_WINDOW_H
