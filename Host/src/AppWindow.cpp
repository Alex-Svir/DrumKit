#include "AppWindow_impl.h"

#include "Session2.h"
#include "ResInit.h"

#include <fstream>
#include <ctime>

namespace appwin {
    const int WIN_WIDTH = 600;
    const int WIN_HEIGHT = 350;
    const int WIN_BORDER = 10;
    const int CHOISE_PANEL_SPACE_V = 5;
    const int CHOISE_PANEL_SPACE_H = 10;
    const int CHOISE_PANEL_PADDING = 2;
    const int INSTRUMENTS_COUNT = 47;

    const int TEMPO_INIT = 120;
    const int TEMPO_MAX = 310;
    const int TEMPO_MIN = 30;
    const int TIMESIGN_UPPER_MAX = 128;
    const int TIMESIGN_LOWER_POW2_MAX = 8;
    const int TIMESIGN_UPPER_INIT = 4;
    const int TIMESIGN_LOWER_POW2_INIT = 2;
    const int PPQN_INIT = 768;

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

    const int ppqn_values[] = {96, 192, 384, 768, 1536, 3072, 6144, 12288, 24576};

    GtkWidget *mwindow;

    GtkWidget *button_rec;
    SmfTypeSwitch *smfSw;
    PpqnSwitch *ppqnSw;
    TimeSignatureSwitch *timeSign;
    TempoSwitch *tempoSw;

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
    delete smfSw;
    delete ppqnSw;
    delete timeSign;
    delete tempoSw;
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

    tempoSw = new TempoSwitch();
    gtk_box_pack_end(GTK_BOX(box), tempoSw->widget(), FALSE, FALSE, 0);
    timeSign = new TimeSignatureSwitch();
    gtk_box_pack_end(GTK_BOX(box), timeSign->widget(), FALSE, FALSE, 25);
    ppqnSw = new PpqnSwitch();
    gtk_box_pack_end(GTK_BOX(box), ppqnSw->widget(), FALSE, FALSE, 0);
    smfSw = new SmfTypeSwitch();
    gtk_box_pack_end(GTK_BOX(box), smfSw->widget(), FALSE, FALSE, 25);

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
    if (session) {
        if (session->stop(on_rec_session_ended)) {
            gtk_button_set_label(GTK_BUTTON(button_rec), "REC");
            return;
        }
    } else session = new sess::RecordSession();

    sess::RecordSession *rs = dynamic_cast<sess::RecordSession*>(session);
    rs->port(gtk_button_get_label(GTK_BUTTON(button_port)));
    session->start(on_rec_session_started);

return;

    if (!session)
        session = new sess::RecordSession();
    if (session->stop(on_rec_session_ended)) {
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
void appwin::cb_timesign_popup(GtkWidget *widget, gpointer) {
    timeSign->popup();
}
void appwin::cb_timesign_set(GtkWidget *widget, gpointer) {
    timeSign->set();
}

void appwin::cb_ppqn_selected(GtkWidget *widget, gpointer udata) {
    const gchar *c_ppqn = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));
    ppqnSw->set(c_ppqn);
}

void appwin::cb_show_menu(GtkWidget *widget, gpointer udata) {
    gtk_menu_popup_at_widget(GTK_MENU(udata), widget, GDK_GRAVITY_EAST, GDK_GRAVITY_NORTH_WEST, NULL);
}
//==========================================================================================================================
void appwin::on_rec_session_started(void*) {
    gtk_button_set_label(GTK_BUTTON(button_rec), "SAVE");
}

void appwin::on_rec_session_ended(void *arg) {
    g_main_context_invoke(NULL, on_rec_session_ended_handler, arg);
}

std::string generate_default_file_name() {
    time_t tt = time(0);
    tm *tnow = localtime(&tt);
    char buf[25];
    sprintf(buf, "%d_%02d_%02d_%02d_%02d_%02d.midi", tnow->tm_year + 1900,
                                                tnow->tm_mon + 1,
                                                tnow->tm_mday,
                                                tnow->tm_hour,
                                                tnow->tm_min,
                                                tnow->tm_sec);
    return buf;
}

void define_output_file_name(std::string& filename) {
    GtkFileChooserNative *chooser_native = gtk_file_chooser_native_new("Save New File",
                                                            NULL,
                                                            GTK_FILE_CHOOSER_ACTION_SAVE,
                                                            NULL,
                                                            NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(chooser_native);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);

    static std::string path;

    if (path.empty())
        gtk_file_chooser_set_current_name(chooser, generate_default_file_name().c_str());
    else
        gtk_file_chooser_set_filename(chooser, (path + generate_default_file_name()).c_str());

    gint result = gtk_native_dialog_run(GTK_NATIVE_DIALOG(chooser_native));

    if (result == GTK_RESPONSE_ACCEPT) {
        gchar *dir = gtk_file_chooser_get_current_folder(chooser);
        path = dir;
        g_free(dir);
        if (path.back() != '/') path.push_back('/');
        filename = gtk_file_chooser_get_filename(chooser);
    }

    g_object_unref(chooser_native);
}

gboolean appwin::on_rec_session_ended_handler(void *arg) {
    midi::RawMidiRecord *record = (midi::RawMidiRecord*)arg;
    midi::MidiSong *song = midi::create_song(record, tempoSw->tempo(), ppqnSw->ppqn(), instrPanel->instrumentsMap());
    delete record;

    if (song) {
        std::string filename;
        define_output_file_name(filename);
        if (!filename.empty()) {
            std::fstream fs_midi(filename, std::fstream::in | std::fstream::out | std::fstream::trunc);
            midi::write_song(fs_midi, song);
            fs_midi.close();
        }
        delete song;
    }
    return FALSE;
}
//==========================================================================================================================
std::string appwin::generate_timeshift_str(int upper, int lower_pow2) {
    return std::to_string(upper) + "/" + std::to_string(1 << lower_pow2);
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

midi::InstrumentsMap appwin::InstrumentsPanel::instrumentsMap() {
    uint8_t *instrs = new uint8_t[rows.size()];
    uint8_t *dest = instrs;
    for (std::vector<Instrument>::const_iterator src = rows.cbegin(); src != rows.cend(); src++, dest++) {
        *dest = uint8_t(std::stoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(src->combo))));
    }
    return midi::InstrumentsMap(instrs, rows.size());
}
//--------------------------------------------------------------------------------------------------------------------------
appwin::TempoSwitch::TempoSwitch() {
    root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *elem = gtk_label_new("bpm");
    gtk_box_pack_end(GTK_BOX(root), elem, FALSE, FALSE, 5);
    //  Create a spin button
    adjustment = gtk_adjustment_new(TEMPO_INIT, TEMPO_MIN, TEMPO_MAX, 1, 10, 10);
    elem = gtk_spin_button_new(adjustment, 0, 0);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(elem), GTK_UPDATE_IF_VALID);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(elem), TRUE);
    gtk_box_pack_end(GTK_BOX(root), elem, FALSE, FALSE, 0);
    //g_signal_connect(G_OBJECT(elem), "value-changed", G_CALLBACK(gcallback_tempo_changed), optns);
            //  [void gcallback_tempo_changed(GtkSpinButton *spin_button, gpointer data);]
    //  Pack a left label
    elem = gtk_label_new("Tempo:");
    gtk_box_pack_end(GTK_BOX(root), elem, FALSE, FALSE, 5);
}
int appwin::TempoSwitch::tempo() {
    return gtk_adjustment_get_value(adjustment);
}
//--------------------------------------------------------------------------------------------------------------------------
appwin::TimeSignatureSwitch::TimeSignatureSwitch() {
    root = gtk_button_new_with_label(generate_timeshift_str(TIMESIGN_UPPER_INIT, TIMESIGN_LOWER_POW2_INIT).c_str());
    g_signal_connect(G_OBJECT(root), "clicked", G_CALLBACK(cb_timesign_popup), NULL);

    char c[4];
    combo_l = gtk_combo_box_text_new();
    for (int i = 0; i < TIMESIGN_UPPER_MAX; i++) {
        sprintf(c, "%d", i + 1);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_l), c);
    }
    combo_r = gtk_combo_box_text_new();
    for (int i = 0; i < TIMESIGN_LOWER_POW2_MAX; i++) {
        sprintf(c, "%d", 2 << i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_r), c);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_l), TIMESIGN_UPPER_INIT - 1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_r), TIMESIGN_LOWER_POW2_INIT - 1);

    GtkWidget *elem, *out_box, *in_box;
    //  Create inner box with combo/label/combo
    in_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(in_box), combo_l, FALSE, FALSE, 0);
    elem = gtk_label_new(" / ");
    gtk_box_pack_start(GTK_BOX(in_box), elem, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(in_box), combo_r, FALSE, FALSE, 0);
    //  Create outer box with inner box and a button
    out_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(out_box), in_box, FALSE, FALSE, 0);
    elem = gtk_button_new_with_label("SET");
    gtk_box_pack_start(GTK_BOX(out_box), elem, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(elem), "clicked", G_CALLBACK(cb_timesign_set), NULL);
    //  Fill popover and make everything visible
    popover = gtk_popover_new(root);
    gtk_container_add(GTK_CONTAINER(popover), out_box);
    gtk_widget_show_all(out_box);
}

void appwin::TimeSignatureSwitch::popup() {
    gtk_popover_popup(GTK_POPOVER(popover));
}
void appwin::TimeSignatureSwitch::set() {
    gtk_popover_popdown(GTK_POPOVER(popover));
    gtk_button_set_label(GTK_BUTTON(root), generate_timeshift_str(
        gtk_combo_box_get_active(GTK_COMBO_BOX(combo_l)) + 1,
        gtk_combo_box_get_active(GTK_COMBO_BOX(combo_r)) + 1
    ).c_str());
}
//--------------------------------------------------------------------------------------------------------------------------
appwin::PpqnSwitch::PpqnSwitch() : value(PPQN_INIT) {
    root = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    char str[8];
    //  Create a dropdown menu
    GtkWidget *elem = gtk_menu_new();
    for (int i = 0; i < 9; i++) {
        sprintf(str, "%d", ppqn_values[i]);
        GtkWidget *item = gtk_menu_item_new_with_label(str);
        gtk_menu_shell_append(GTK_MENU_SHELL(elem), item);
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(cb_ppqn_selected), NULL);
    }
    gtk_widget_show_all(elem);
    //  Create a button showing menu and pack it
    sprintf(str, "%d", value);
    ppqn_btn = gtk_button_new_with_label(str);
    gtk_box_pack_end(GTK_BOX(root), ppqn_btn, FALSE, FALSE, 5);
    g_signal_connect(G_OBJECT(ppqn_btn), "clicked", G_CALLBACK(cb_show_menu), elem);
    elem = gtk_label_new("PPQN:");
    gtk_box_pack_end(GTK_BOX(root), elem, FALSE, FALSE, 0);
}

void appwin::PpqnSwitch::set(const gchar* lbl) {
    value = atoi(lbl);
    gtk_button_set_label(GTK_BUTTON(ppqn_btn), lbl);
}
//--------------------------------------------------------------------------------------------------------------------------
appwin::SmfTypeSwitch::SmfTypeSwitch() {
    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *elem = gtk_label_new("SMF type:");
    gtk_box_pack_start(GTK_BOX(root), elem, FALSE, FALSE, 0);
    //  Create inner box with radois
    GtkWidget *in_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(root), in_box, TRUE, FALSE, 0);
    elem = gtk_radio_button_new_with_label(NULL, "0");
    gtk_box_pack_start(GTK_BOX(in_box), elem, FALSE, FALSE, 0);
    elem = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(elem), "1");
    gtk_box_pack_start(GTK_BOX(in_box), elem, FALSE, FALSE, 0);
}
