#ifndef HILBERT_RULER_H
#define HILBERT_RULER_H

#include <gtkmm.h>
#include <gtkmm/drawingarea.h>
#include "display.h"

std::string int2strB( int a );

class Ruler : public Gtk::DrawingArea {
  public:
   Ruler( InvalidableAdjustment * adj_ );
  protected:
   InvalidableAdjustment * adj;
   virtual bool on_expose_event( GdkEventExpose* event );
   virtual void on_adj_changed( void );
};

class PaletteBar : public Gtk::DrawingArea {
  public:
   PaletteBar( );
   void set_palettes( double max_value_, std::vector< Gdk::Color > * palette_, 
      std::vector< Gdk::Color > * neg_palette_ = NULL );
  protected: 
   double max_value;
   std::vector< Gdk::Color > * palette;
   std::vector< Gdk::Color > * neg_palette;
   virtual bool on_expose_event( GdkEventExpose* event );
};

#endif // HILBERT_RULER_H
