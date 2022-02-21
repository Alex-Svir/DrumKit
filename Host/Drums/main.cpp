#include <iostream>
#include <thread>
#include <gtk/gtk.h>
#include "TopPanel.h"
#include "ChoisePanel.h"
#include "BottomPanel.h"
#include "Session.h"

void gcallback_window_close(GtkWidget*, gpointer);
void gcallback_button_rec(GtkWidget*, gpointer);
void notify_session_started(gpointer ptr);

Session ses;

int main(int argc, char* argv[])
{
    std::cout << "START!" << std::endl;

    GtkWidget *window;
    GtkWidget *form, *frame, *box;

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
    ChoisePanel center;
    TopPanel top;
    BottomPanel bottom;

    //  Session parameters
    struct params prms;
    prms.center = &center;
    prms.top = &top;
    prms.bottom = &bottom;

    //  Top frame
    frame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(form), frame, FALSE, FALSE, 0);

    //  Top panel
    gtk_container_add(GTK_CONTAINER(frame), top.get_pointer());
    g_signal_connect(G_OBJECT(top.get_rec_button()), "clicked", G_CALLBACK(gcallback_button_rec), &prms);

    //  Central area
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(form), box, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(box), center.get_pointer(), FALSE, TRUE, 5);

    //  Bottom frame
    frame = gtk_frame_new(NULL);
    gtk_box_pack_end(GTK_BOX(form), frame, FALSE, FALSE, 0);

    //  Bottom panel
    gtk_container_add(GTK_CONTAINER(frame), bottom.get_pointer());

    //  Other parameters
    prms.func = &notify_session_started;
    prms.arg = (gpointer) top.get_rec_button();

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
    struct params *prms = (struct params*) data;
    if (ses.ready_for_stop())
    {
        ses.stop();
        gtk_button_set_label(GTK_BUTTON(button), "REC");
        prms->bottom->drop_filename();
    }
    else if (ses.ready_for_start())
    {
        std::thread th(threadfunc_record, prms);
        th.detach();
    }
}

gboolean set_button_recording_state(gpointer ptr)
{
    GtkWidget *button = (GtkWidget *)ptr;
    gtk_button_set_label(GTK_BUTTON(button), "STOP");
    return FALSE;
}

void notify_session_started(gpointer ptr)
{
    g_main_context_invoke(NULL, set_button_recording_state, ptr);
}
