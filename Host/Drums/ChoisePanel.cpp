#include "ChoisePanel.h"

ChoisePanel::ChoisePanel()
{
    GtkWidget *row, *table, *elem;

    //  Create a table of Channels
    table = gtk_box_new(GTK_ORIENTATION_VERTICAL, CHOISE_PANEL_SPACE_V);
    char label[16];
    for (int i = 0; i < CHOISE_PANEL_ROWS; i++)
    {
        row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, CHOISE_PANEL_SPACE_H);

        sprintf(label, "Channel %d", i);
        elem = gtk_label_new(label);
        gtk_box_pack_start(GTK_BOX(row), elem, FALSE, FALSE, CHOISE_PANEL_PADDING);

        combo[i] = gtk_combo_box_text_new();
        for (int k = 0; k < INSTRUMENTS_COUNT; k++)
        {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo[i]), instruments[k]);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo[i]), i % INSTRUMENTS_COUNT);
        gtk_box_pack_end(GTK_BOX(row), combo[i], FALSE, FALSE, CHOISE_PANEL_PADDING);

        gtk_box_pack_start(GTK_BOX(table), row, FALSE, FALSE, CHOISE_PANEL_PADDING);
    }

    //  Table => Scroll => Frame
    elem = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(elem), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(elem), table);

    frame = gtk_frame_new("Channels");
    gtk_container_add(GTK_CONTAINER(frame), elem);
}

GtkWidget* ChoisePanel::get_pointer()
{
    return frame;
}

void ChoisePanel::get_notes_map(uint8_t *buf, int len)
{
    int co = len > CHOISE_PANEL_ROWS ? CHOISE_PANEL_ROWS : len;
    std::stringstream ss;
    int t;
    for (int i=0; i<co; i++)
    {
        ss.str(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo[i])));
        ss >> t;
        buf[i] = t;
    }
    if (co < len)
        for (int i=co; i<len; i++)
            buf[i] = 0;
}
