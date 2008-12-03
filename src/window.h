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
   virtual void on_cboxtSeqnames_changed( void );
   virtual void on_adjDisplayedValueRange_changed( void );
   virtual void on_adjPointerPos_value_changed( void );
   virtual void on_canvasClicked( GdkEventButton * ev, long binLo, long BinHi );

   // dataColorizers:
   std::vector<DataColorizer * > * dataCols;
   unsigned cur_dataCol_idx;

   // Member widgets:
   HilbertCurveDisplay canvas;
   Gtk::Button btnZoomOut4x, btnZoomOut64x, btnCoarser, btnFiner, btnPrev, btnNext;
   Gtk::Button btnOpen, btnClose;
   Gtk::HBox hbox1, hbox2, hbox3, hbox4, hbox5, hbox6, hbox7;
   Gtk::VBox vbox1;
   Gtk::Box * box0;
   Gtk::Table tbl1;
   Gtk::Frame frame1, frame2, frame3, frame4, frame5, frame6, frame7;
   Gtk::Label lblPos, lblValue;
   Ruler rulerPointerPos;
   Ruler rulerDispValueRange;
   Gtk::ComboBoxText cboxtSeqnames;
   Gtk::RadioButtonGroup rbtngLeftMouse;
   Gtk::RadioButton rbtnZoomIn4x;
   Gtk::RadioButton rbtnZoomIn64x, rbtnPlotLin;
};

#endif //HILBERT_WINDOW_H
