#ifndef __APPWINDOW_IMPL__H__
#define __APPWINDOW_IMPL__H__

#include "AppWindow.h"
#include "Midi2.h"
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
    void cb_timesign_popup(GtkWidget*, gpointer);
    void cb_timesign_set(GtkWidget*, gpointer);
    void cb_ppqn_selected(GtkWidget*, gpointer);
    void cb_show_menu(GtkWidget*, gpointer);

    void on_rec_session_started(void*);
    void on_rec_session_ended(void*);

    std::string generate_timeshift_str(int,int);

    class InstrumentsPanel {
        struct Instrument {GtkWidget *combo;};
        GtkWidget *root;
        GtkWidget *instr_box;
        std::vector<Instrument> rows;
    public:
        InstrumentsPanel();
        GtkWidget* widget() {return root;}
        void add(int);
        midi::InstrumentsMap instrumentsMap();
    };

    class TempoSwitch {
        GtkWidget *root;
        GtkAdjustment *adjustment;
    public:
        TempoSwitch();
        GtkWidget* widget() {return root;}
        int tempo();
    };

    class TimeSignatureSwitch {
        GtkWidget *root;
        GtkWidget *popover, *combo_l, *combo_r;
    public:
        TimeSignatureSwitch();
        GtkWidget* widget() {return root;}
        void popup();
        void set();
    };

    class PpqnSwitch {
        GtkWidget *root, *ppqn_btn;
        int value;
    public:
        PpqnSwitch();
        GtkWidget* widget() {return root;}
        void set(const gchar*);
        int ppqn() {return value;}
    };

    class SmfTypeSwitch {
        GtkWidget *root;
    public:
        SmfTypeSwitch();
        GtkWidget* widget() {return root;}
    };
}

#endif
