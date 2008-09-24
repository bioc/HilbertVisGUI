// The method Gtk::Widget::error_bell was added to gtkmm only
// in version 2.12. The following kludge allows to use error
// bell when compiling against older GTK+ 2.4 gtkmm versions by
// simply defining error_bell as alias to GTK+'s C function gdk_beep.

#ifndef HILBERT_ERROR_BELL_KLUDGE_H
#define HILBERT_ERROR_BELL_KLUDGE_H

#if GTKMM_MINOR_VERSION < 12
#include <gtk/gtk.h>
#define error_bell gdk_beep
#endif

#endif //HILBERT_ERROR_BELL_KLUDGE_H
