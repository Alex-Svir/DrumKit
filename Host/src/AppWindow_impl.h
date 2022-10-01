#ifndef __APPWINDOW_IMPL__H__
#define __APPWINDOW_IMPL__H__

#include "AppWindow.h"
#include <vector>
#include <gtk/gtk.h>

namespace appwin {
    void create_main_window();
    GtkWidget* create_top_panel();
    GtkWidget* create_bottom_panel();

    void cb_close_mwindow(GtkWidget*, gpointer);

    void cb_btn_rec(GtkWidget*, gpointer);
    void cb_btn_port(GtkWidget*, gpointer);
    void cb_menu_port_selected(GtkWidget*, gpointer);

    void on_rec_session_started(void*);

    class InstrumentsPanel {
        struct Instrument {GtkWidget *combo;};
        GtkWidget *root;
        GtkWidget *instr_box;
        std::vector<Instrument> rows;
    public:
        InstrumentsPanel();
        GtkWidget* widget() {return root;}
        void add(int);
    };
}

#endif
