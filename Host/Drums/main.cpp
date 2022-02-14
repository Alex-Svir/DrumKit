#include <iostream>
#include <thread>
#include <gtk/gtk.h>
#include "TopPanel.h"
#include "ChoisePanel.h"
#include "BottomPanel.h"
#include "Session.h"

void gcallback_window_close(GtkWidget*, gpointer);
void gcallback_button_rec(GtkWidget*, gpointer);
void set_button_recording_state(gpointer);

Session ses;

int main(int argc, char* argv[])
{
    std::cout << "START!" << std::endl;

    GtkWidget *window;
    GtkWidget *form, *frame, *box;
    GtkWidget *button_rec;

    gtk_init(&argc, &argv);

    //  Create the main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 350);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gcallback_window_close), NULL);

    //  Form
    form = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), form);

    //  Panels' content
    ChoisePanel cp;
    TopPanel tp;
    BottomPanel bp;

    //  Session parameters
    struct params prms;
    prms.cp = &cp;
    prms.tp = &tp;
    prms.bp = &bp;

    //  Top frame
    frame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(form), frame, FALSE, FALSE, 0);

    //  Top panel
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);
    gtk_container_add(GTK_CONTAINER(frame), box);

    button_rec = gtk_button_new_with_label("REC");
    g_signal_connect(G_OBJECT(button_rec), "clicked", G_CALLBACK(gcallback_button_rec), &prms);
    gtk_box_pack_start(GTK_BOX(box), button_rec, FALSE, FALSE, 2);

    gtk_box_pack_end(GTK_BOX(box), tp.get_pointer(), FALSE, TRUE, 5);

    //  Central area
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(form), box, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(box), cp.get_pointer(), FALSE, TRUE, 5);

    //  Bottom frame
    frame = gtk_frame_new(NULL);
    gtk_box_pack_end(GTK_BOX(form), frame, FALSE, FALSE, 0);

    //  Bottom panel
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);
    gtk_container_add(GTK_CONTAINER(frame), box);
    gtk_box_pack_end(GTK_BOX(box), bp.get_pointer(), TRUE, TRUE, 0);

    //  Other parameters
    prms.func = &set_button_recording_state;
    prms.arg = (gpointer) button_rec;

    //  Start
    gtk_widget_show_all(window);
    gtk_main();

    /*while (true)
    {
        std::string in_str;
        std::cin >> in_str;
        if (in_str == "q")
            break;
    }*/
    return 0;
}

void threadfunc_record(struct params *prms)
{
    ses.start(prms);
}

void gcallback_window_close(GtkWidget *window, gpointer data)
{
    ses.stop();
    gtk_main_quit();
}

void gcallback_button_rec(GtkWidget *button, gpointer data)
{
    if (ses.ready_for_stop())
    {
        ses.stop();
        gtk_button_set_label(GTK_BUTTON(button), "REC");
        ((struct params *)data)->bp->drop_filename();
        ((struct params *)data)->arg = (gpointer) button;
    }
    else if (ses.ready_for_start())
    {
        std::thread th(threadfunc_record, (struct params *)data);
        th.detach();
    }
}

void set_button_recording_state(gpointer ptr)
{
    GtkWidget *button = (GtkWidget *)ptr;
    gtk_button_set_label(GTK_BUTTON(button), "STOP");
}
