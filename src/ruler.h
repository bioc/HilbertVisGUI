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

#endif // HILBERT_RULER_H
