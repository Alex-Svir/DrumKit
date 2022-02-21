#ifndef __TOPPANEL__H__
#define __TOPPANEL__H__

#define TEMPO_MAX 310
#define TEMPO_MIN 30
#define TIMESIGN_UPPER_MAX 128
#define TIMESIGN_LOWER_MAX_AS_POWER_2 8
#define DEFAULT_PPQN 768

#include <gtk/gtk.h>
#include <cstdio>
#include <string>

class TimeSignaturePopover
{
    GtkWidget *popover, *combo_l, *combo_r;
public:
    void create(GtkWidget *base_button, void (*callback)(GtkWidget*, gpointer), gpointer argument);
    void set_combos(int numerator, int denominator);
    GtkWidget* get_pointer();
    int get_numerator();
    int get_denominator();
};

class TopPanel
{
    GtkWidget *box;
    GtkWidget *button_rec;
    GtkWidget *spin;
    GtkWidget *button_ppqn;
    GtkWidget *button_timesign;
    TimeSignaturePopover popover;

    int ppqn = DEFAULT_PPQN;
    int time_signature_upper = 4;
    int time_signature_lower_as_power_2 = 2;

    void init_rec_button();
    void init_smf_type_block();
    void init_ppqn_block();
    void init_time_signature_block();
    void init_tempo_block();
    const char* get_timesign_string();

    static void gcallback_show_menu(GtkWidget *button, gpointer data);
    static void gcallback_menu_item_selected(GtkWidget *item, gpointer data);
    static void gcallback_show_popover(GtkWidget *button, gpointer data);
    static void gcallback_popover_set(GtkWidget *button, gpointer data);
public:
    TopPanel();
    GtkWidget* get_pointer();
    GtkWidget* get_rec_button();
    int get_tempo();
    int get_ppqn();
};

#endif
