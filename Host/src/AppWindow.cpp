#include "AppWindow_impl.h"

#include "Session2.h"
#include "ResInit.h"

namespace appwin {
    const int WIN_WIDTH = 600;
    const int WIN_HEIGHT = 350;
    const int WIN_BORDER = 10;
    const int CHOISE_PANEL_SPACE_V = 5;
    const int CHOISE_PANEL_SPACE_H = 10;
    const int CHOISE_PANEL_PADDING = 2;
    const int INSTRUMENTS_COUNT = 47;

    const char* INSTRUMENTS_LIST[] {
        "35 Acoustic Bass Drum",
        "36 Bass Drum 1",
        "37 Side Kick",
        "38 Acoustic Snare",
        "39 Hand Clap",
        "40 Electric Snare",
        "41 Low Floor Tom",
        "42 Closed High-Hat",
        "43 High Floor Tom",
        "44 Pedal High Hat",
        "45 Low Tom",
        "46 Open High Hat",
        "47 Low-Mid Tom",
        "48 High-Mid Tom",
        "49 Crash Cymbal 1",
        "50 High Tom",
        "51 Ride Cymbal 1",
        "52 Chinese Cymbal",
        "53 Ride Bell",
        "54 Tambourine",
        "55 Splash Cymbal",
        "56 Cowbell",
        "57 Crash Cymbal 2",
        "58 Vibraslap",
        "59 Ride Cymbal 2",
        "60 High Bongo",
        "61 Low Bongo",
        "62 Mute High Conga",
        "63 Open High Conga",
        "64 Low Conga",
        "65 High Timbale",
        "66 Low Timbale",
        "67 High Agogo",
        "68 Low Agogo",
        "69 Cabasa",
        "70 Maracas",
        "71 Short Whistle",
        "72 Long Whistle",
        "73 Short Guiro",
        "74 Long Guiro",
        "75 Claves",
        "76 High Wood Block",
        "77 Low Wood Block",
        "78 Mute Cuica",
        "79 Open Cuica",
        "80 Mute Triangle",
        "81 Open Triangle"
        };

    GtkWidget *mwindow;
    GtkWidget *button_rec;

    InstrumentsPanel *instrPanel;

    GtkWidget *button_port;

    sess::Session *session;
}
//==========================================================================================================================
void appwin::launch() {
    create_main_window();
    gtk_widget_show_all(mwindow);
}
void appwin::cb_close_mwindow(GtkWidget *widget, gpointer udata) {
    delete session;
    delete instrPanel;
    gtk_main_quit();
}

void appwin::create_main_window() {
    mwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(mwindow), WIN_WIDTH, WIN_HEIGHT);
    gtk_container_set_border_width(GTK_CONTAINER(mwindow), WIN_BORDER);
    gtk_window_set_position(GTK_WINDOW(mwindow), GTK_WIN_POS_CENTER);
    g_signal_connect(G_OBJECT(mwindow), "destroy", G_CALLBACK(cb_close_mwindow), NULL);

    GtkWidget *vbox, *hbox;
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(mwindow), vbox);

    gtk_box_pack_start(GTK_BOX(vbox), create_top_panel(), FALSE, FALSE, 0);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
    instrPanel = new InstrumentsPanel();
    gtk_box_pack_end(GTK_BOX(hbox), instrPanel->widget(), FALSE, TRUE, 5);

    gtk_box_pack_end(GTK_BOX(vbox), create_bottom_panel(), FALSE, FALSE, 0);
}

GtkWidget* appwin::create_top_panel() {
    GtkWidget *frame = gtk_frame_new(NULL);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), box);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);

    button_rec = gtk_button_new_with_label("REC");
    gtk_box_pack_start(GTK_BOX(box), button_rec, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(button_rec), "clicked", G_CALLBACK(cb_btn_rec), NULL);

    return frame;
}
GtkWidget* appwin::create_bottom_panel() {
    GtkWidget *frame = gtk_frame_new(NULL);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);
    gtk_container_add(GTK_CONTAINER(frame), box);

    button_port = gtk_button_new_with_label("SELECT PORT");
    gtk_box_pack_start(GTK_BOX(box), button_port, FALSE, FALSE, 2);
    g_signal_connect(G_OBJECT(button_port), "clicked", G_CALLBACK(cb_btn_port), NULL);

    resin::OpenDirectory opdir("/dev");
    struct dirent *file;
    //const char *const condev = "/dev/";
    char dev[262] = "/dev/";
    const int condevlen = strlen("/dev/");
    while ((file = opdir.next()) != NULL) {
        if (strstr(file->d_name, "ttyUSB") == file->d_name)
            strcpy(dev + condevlen, file->d_name);
    }
    if (strlen(dev) > condevlen)
        gtk_button_set_label(GTK_BUTTON(button_port), dev);

    return frame;
}

//==========================================================================================================================
void appwin::cb_btn_rec(GtkWidget *widget, gpointer udata) {
    if (!session)
        session = new sess::RecordSession();
    if (session->stop()) {
        gtk_button_set_label(GTK_BUTTON(button_rec), "REC");
    } else {
        sess::RecordSession *rs = dynamic_cast<sess::RecordSession*>(session);
        rs->port(gtk_button_get_label(GTK_BUTTON(button_port)));
        session->start(on_rec_session_started);
    }
}
void appwin::cb_btn_port(GtkWidget *widget, gpointer udata) {
    GtkWidget *menu = gtk_menu_new();
    resin::OpenDirectory opendir("/dev");
    struct dirent *file;
    char dev[262] = "/dev/";
    const int condevlen = strlen("/dev/");
    while ((file = opendir.next()) != NULL) {
        if (strstr(file->d_name, "ttyUSB") == file->d_name) {
            strcpy(dev + condevlen, file->d_name);
            GtkWidget *item = gtk_menu_item_new_with_label(dev);
            gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), item);
            g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(cb_menu_port_selected), NULL);
        }
    }
    gtk_widget_show_all(menu);
    gtk_menu_popup_at_widget(GTK_MENU(menu), widget, GDK_GRAVITY_EAST, GDK_GRAVITY_SOUTH_WEST, NULL);
}
void appwin::cb_menu_port_selected(GtkWidget *widget, gpointer udata) {
    gtk_button_set_label(GTK_BUTTON(button_port), gtk_menu_item_get_label(GTK_MENU_ITEM(widget)));
}
//==========================================================================================================================
void appwin::on_rec_session_started(void*) {
    gtk_button_set_label(GTK_BUTTON(button_rec), "STOP");
    //g_main_context_invoke(NULL, set_button_recording_state, ptr);
    //return FALSE;
}
//==========================================================================================================================
appwin::InstrumentsPanel::InstrumentsPanel() {
    root = gtk_frame_new("Channels");

    GtkWidget *swin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(swin), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(root), swin);

    instr_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, CHOISE_PANEL_SPACE_V);
    gtk_container_add(GTK_CONTAINER(swin), instr_box);

    for (int i = 0; i < 16; i++) add(i);
}

void appwin::InstrumentsPanel::add(int idx) {
    GtkWidget *row, *label, *combo;
    char title[16];
    sprintf(title, "Channel %d", idx);
    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, CHOISE_PANEL_SPACE_H);
    label = gtk_label_new(title);
    gtk_box_pack_start(GTK_BOX(row), label, FALSE, FALSE, CHOISE_PANEL_PADDING);
    combo = gtk_combo_box_text_new();
    for (int i = 0; i < INSTRUMENTS_COUNT; i++)
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), INSTRUMENTS_LIST[i]);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), idx % INSTRUMENTS_COUNT);
    gtk_box_pack_end(GTK_BOX(row), combo, FALSE, FALSE, CHOISE_PANEL_PADDING);

    rows.push_back({combo});
    gtk_box_pack_start(GTK_BOX(instr_box), row, FALSE, FALSE, CHOISE_PANEL_PADDING);
}
