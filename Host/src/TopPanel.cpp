#include "TopPanel.h"

/// Constructor
TopPanel::TopPanel(options *optns)
{
    this->optns = optns;

    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(box), 2);

    init_rec_button();
    init_tempo_block();
    init_time_signature_block();
    init_ppqn_block();
    init_smf_type_block();
}

//! Returns root element
/// \return pointer to the root element
GtkWidget* TopPanel::get_pointer()
{
    return box;
}

/*!
    Returns RECORD button pointer
*/
GtkWidget* TopPanel::get_rec_button()
{
    return button_rec;
}

void TopPanel::init_rec_button()
{
    button_rec = gtk_button_new_with_label("REC");
    gtk_box_pack_start(GTK_BOX(box), button_rec, FALSE, FALSE, 2);
}

void TopPanel::init_tempo_block()
{
    GtkWidget *elem;

    //  Pack a right label
    elem = gtk_label_new("bpm");
    gtk_box_pack_end(GTK_BOX(box), elem, FALSE, FALSE, 5);

    //  Create a spin button
    GtkAdjustment *adjustment = gtk_adjustment_new(optns->tempo, TEMPO_MIN, TEMPO_MAX, 1, 10, 10);
    elem = gtk_spin_button_new(adjustment, 0, 0);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(elem), GTK_UPDATE_IF_VALID);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(elem), TRUE);
    gtk_box_pack_end(GTK_BOX(box), elem, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(elem), "value-changed", G_CALLBACK(gcallback_tempo_changed), optns);

    //  Pack a left label
    elem = gtk_label_new("Tempo:");
    gtk_box_pack_end(GTK_BOX(box), elem, FALSE, FALSE, 5);
}

void TopPanel::init_ppqn_block()
{
    int ppqn_values[] = {96, 192, 384, 768, 1536, 3072, 6144, 12288, 24576};
    char str[8];

    GtkWidget *elem;

    //  Create a dropdown menu
    elem = gtk_menu_new();
    for (int i=0; i<9; i++)
    {
        sprintf(str, "%d", ppqn_values[i]);
        GtkWidget *item = gtk_menu_item_new_with_label(str);

        gtk_menu_shell_append(GTK_MENU_SHELL(elem), item);
        gtk_widget_show(item);
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(gcallback_menu_item_selected), this);
    }

    //  Create a button showing menu and pack it
    sprintf(str, "%d", optns->ppqn);
    button_ppqn = gtk_button_new_with_label(str);

    g_signal_connect(G_OBJECT(button_ppqn), "clicked", G_CALLBACK(gcallback_show_menu), elem);
    gtk_box_pack_end(GTK_BOX(box), button_ppqn, FALSE, FALSE, 5);

    //  Pack a label to main box
    elem = gtk_label_new("PPQN:");
    gtk_box_pack_end(GTK_BOX(box), elem, FALSE, FALSE, 0);
}

void TopPanel::init_time_signature_block()
{
    //  Create a button calling popover
    gchar *label = optns->get_timesign_string();
    button_timesign = gtk_button_new_with_label(label);
    g_free(label);
    popover.create(button_timesign, gcallback_popover_set, this);
    popover.set_combos(optns->time_signature_upper, optns->time_signature_lower_as_power_2);
    g_signal_connect(G_OBJECT(button_timesign), "clicked", G_CALLBACK(gcallback_show_popover), this);

    //  Add to main box
    gtk_box_pack_end(GTK_BOX(box), button_timesign, FALSE, FALSE, 25);
}

void TopPanel::init_smf_type_block()
{
    GtkWidget *out_box, *in_box;
    GtkWidget *elem;

    //  Create inner box with radois
    in_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    elem = gtk_radio_button_new_with_label(NULL, "0");
    gtk_box_pack_start(GTK_BOX(in_box), elem, FALSE, FALSE, 0);

    elem = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(elem), "1");
    gtk_box_pack_start(GTK_BOX(in_box), elem, FALSE, FALSE, 0);

    //  Create outer box with label and radios above
    out_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    elem = gtk_label_new("SMF type:");
    gtk_box_pack_start(GTK_BOX(out_box), elem, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(out_box), in_box, TRUE, FALSE, 0);

    //  Pack in main box
    gtk_box_pack_end(GTK_BOX(box), out_box, FALSE, FALSE, 25);
}

void TopPanel::gcallback_show_menu(GtkWidget *button, gpointer data)
{
    GtkWidget *menu = (GtkWidget *) data;
    gtk_menu_popup_at_widget(GTK_MENU(menu), button, GDK_GRAVITY_EAST, GDK_GRAVITY_NORTH_WEST, NULL);
}

void TopPanel::gcallback_menu_item_selected(GtkWidget *item, gpointer data)
{
    TopPanel *ptr = (TopPanel *)data;
    const gchar* c_ppqn = gtk_menu_item_get_label(GTK_MENU_ITEM(item));
    ptr->optns->ppqn = atoi(c_ppqn);
    gtk_button_set_label(GTK_BUTTON(ptr->button_ppqn), c_ppqn);
}

void TopPanel::gcallback_show_popover(GtkWidget *button, gpointer data)
{
    gtk_popover_popup(GTK_POPOVER(((TopPanel*)data)->popover.get_pointer()));
}

void TopPanel::gcallback_tempo_changed(GtkSpinButton *spin_button, gpointer data)
{
    ((options*) data)->tempo = gtk_spin_button_get_value_as_int(spin_button);
}

void TopPanel::gcallback_popover_set(GtkWidget *button, gpointer data)
{
    TopPanel *ptr = (TopPanel *)data;
    ptr->optns->time_signature_upper = ptr->popover.get_numerator();
    ptr->optns->time_signature_lower_as_power_2 = ptr->popover.get_denominator();
    gtk_popover_popdown(GTK_POPOVER(ptr->popover.get_pointer()));
    gchar *label = ptr->optns->get_timesign_string();
    gtk_button_set_label( GTK_BUTTON(ptr->button_timesign), label);
    g_free(label);
}

void TimeSignaturePopover::create(GtkWidget *base_button, void (*callback)(GtkWidget*, gpointer), gpointer argument)
{
    GtkWidget *out_box, *in_box;
    GtkWidget *elem;

    //  Create inner box with combo/label/combo
    in_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    combo_l = gtk_combo_box_text_new();
    char c[4];
    for (int i = 0; i<TIMESIGN_UPPER_MAX; i++)
    {
        sprintf(c, "%d", i+1);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_l), c);
    }
    gtk_box_pack_start(GTK_BOX(in_box), combo_l, FALSE, FALSE, 0);

    elem = gtk_label_new(" / ");
    gtk_box_pack_start(GTK_BOX(in_box), elem, FALSE, FALSE, 0);

    combo_r = gtk_combo_box_text_new();
    for (int i=0; i<TIMESIGN_LOWER_MAX_AS_POWER_2; i++)
    {
        sprintf(c, "%d", 2<<i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_r), c);
    }
    gtk_box_pack_start(GTK_BOX(in_box), combo_r, FALSE, FALSE, 0);

    //  Create outer box with inner box and a button
    out_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(out_box), in_box, FALSE, FALSE, 0);

    elem = gtk_button_new_with_label("SET");
    g_signal_connect(G_OBJECT(elem), "clicked", G_CALLBACK(callback), argument);
    gtk_box_pack_start(GTK_BOX(out_box), elem, FALSE, FALSE, 0);

    popover = gtk_popover_new(base_button);

    //  Fill popover and make everything visible
    gtk_container_add(GTK_CONTAINER(popover), out_box);
    gtk_widget_show_all(out_box);
}

GtkWidget* TimeSignaturePopover::get_pointer()
{
    return popover;
}

void TimeSignaturePopover::set_combos(int numerator, int denominator_as_power_2)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_l), numerator-1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_r), denominator_as_power_2-1);
}

int TimeSignaturePopover::get_numerator()
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(combo_l))+1;
}

int TimeSignaturePopover::get_denominator()
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(combo_r))+1;
}
