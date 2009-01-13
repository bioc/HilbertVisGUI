#ifndef HILBERT_LINPLOT_H
#define HILBERT_LINPLOT_H

#include<string>
#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/window.h>
#include "display.h"

class LinPlot : public Gtk::DrawingArea {
  public:
   LinPlot( DataVector * dv_, std::string name_, long int left_, long int right_, double scale_ );      
  protected:
   DataVector * dv;
   std::string name;
   long int left, right;
   double scale;   
   virtual bool on_expose_event( GdkEventExpose* event );
};

class LinPlotWindow : public Gtk::Window {
  public:
   LinPlotWindow( DataVector * dv, std::string name, long int middle, long int width, double scale ); 
  protected:
   LinPlot lp;     
};

#endif //HILBERT_LINPLOT_H
