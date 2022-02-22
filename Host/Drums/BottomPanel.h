#ifndef __BOTTOMPANEL__H__
#define __BOTTOMPANEL__H__

#include <string>
#include <ctime>
#include <gtk/gtk.h>

class BottomPanel
{
    GtkWidget *box;
    GtkWidget *file_button;
    GtkWidget *port_button;
    std::string filename = "";
    std::string path = "";

    static void gcallback_file_button(GtkWidget *button, gpointer data);
    static void gcallback_port_button_clicked(GtkWidget *button, gpointer data);
    static void gcallback_port_menu_activated(GtkWidget *item, gpointer data);
    void set_file_name_and_path(char *filename);
    std::string get_file_name();
public:
    BottomPanel();
    /// Returns root element pointer
    GtkWidget* get_pointer();
    std::string get_port_name();
    std::string get_file_name_with_path();
    void drop_filename();
};

#endif
