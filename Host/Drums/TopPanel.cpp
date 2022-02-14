#include "TopPanel.h"

TopPanel::TopPanel()
{
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    init_smf_type_block();
    init_ppqn_block();
    init_time_signature_block();
    init_tempo_block();
}

GtkWidget* TopPanel::get_pointer()
{
    return box;
}

int TopPanel::get_tempo()
{
    return gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
}

int TopPanel::get_ppqn()
{
    return ppqn;
}

void TopPanel::init_tempo_block()
{
    GtkWidget *elem;

    //  Pack a left label
    elem = gtk_label_new("Tempo: ");
    gtk_box_pack_start(GTK_BOX(box), elem, FALSE, FALSE, 0);

    //  Create a spin button
    GtkAdjustment *adjustment = gtk_adjustment_new(120, TEMPO_MIN, TEMPO_MAX, 1, 10, 10);
    spin = gtk_spin_button_new(adjustment, 0, 0);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(spin), GTK_UPDATE_IF_VALID);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spin), TRUE);
    gtk_box_pack_start(GTK_BOX(box), spin, FALSE, FALSE, 5);

    //  Pack a right label
    elem = gtk_label_new(" bpm");
    gtk_box_pack_start(GTK_BOX(box), elem, FALSE, FALSE, 0);
}

void TopPanel::init_ppqn_block()
{
    std::string ppqn_values[] = {"96", "192", "384", "768", "1536", "3072", "6144", "12288", "24576"};

    GtkWidget *elem;

    //  Pack a label to main box
    elem = gtk_label_new("PPQN: ");
    gtk_box_pack_start(GTK_BOX(box), elem, FALSE, FALSE, 0);

    //  Create a dropdown menu
    elem = gtk_menu_new();
    for (int i=0; i<9; i++)
    {
        GtkWidget *item = gtk_menu_item_new_with_label(ppqn_values[i].c_str());
        gtk_menu_shell_append(GTK_MENU_SHELL(elem), item);
        gtk_widget_show(item);
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(gcallback_menu_item_selected), this);
    }

    //  Create a button showing menu and pack it
    button_ppqn = gtk_button_new_with_label(std::to_string(ppqn).c_str());
    g_signal_connect(G_OBJECT(button_ppqn), "clicked", G_CALLBACK(gcallback_show_menu), elem);
    gtk_box_pack_start(GTK_BOX(box), button_ppqn, FALSE, FALSE, 5);
}

void TopPanel::init_time_signature_block()
{
    GtkWidget *out_box, *in_box;
    GtkWidget *elem;

    //  Create inner box with combo/label/combo
    in_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    popo_combo_l = gtk_combo_box_text_new();
    char c[4];
    for (int i = 0; i<TIMESIGN_UPPER_MAX; i++)
    {
        sprintf(c, "%d", i+1);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(popo_combo_l), c);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(popo_combo_l), time_signature_upper-1);
    gtk_box_pack_start(GTK_BOX(in_box), popo_combo_l, FALSE, FALSE, 0);

    elem = gtk_label_new(" / ");
    gtk_box_pack_start(GTK_BOX(in_box), elem, FALSE, FALSE, 0);

    popo_combo_r = gtk_combo_box_text_new();
    for (int i=0; i<TIMESIGN_LOWER_MAX_AS_POWER_2; i++)
    {
        sprintf(c, "%d", 2<<i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(popo_combo_r), c);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(popo_combo_r), time_signature_lower_as_power_2-1);
    gtk_box_pack_start(GTK_BOX(in_box), popo_combo_r, FALSE, FALSE, 0);

    //  Create outer box with inner box and a button
    out_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(out_box), in_box, FALSE, FALSE, 0);

    elem = gtk_button_new_with_label("SET");
    g_signal_connect(G_OBJECT(elem), "clicked", G_CALLBACK(gcallback_popover_set), this);
    gtk_box_pack_start(GTK_BOX(out_box), elem, FALSE, FALSE, 0);

    //  Create a button calling popover
    button_popo = gtk_button_new_with_label( (
                std::to_string(time_signature_upper)
                + "/"
                + std::to_string(1<<time_signature_lower_as_power_2)
            ).c_str() );
    g_signal_connect(G_OBJECT(button_popo), "clicked", G_CALLBACK(gcallback_show_popover), this);

    popover = gtk_popover_new(button_popo);

    //  Fill popover and make everything visible
    gtk_container_add(GTK_CONTAINER(popover), out_box);
    gtk_widget_show_all(out_box);

    //  Add to main box
    gtk_box_pack_start(GTK_BOX(box), button_popo, FALSE, FALSE, 25);
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
    gtk_box_pack_start(GTK_BOX(box), out_box, FALSE, FALSE, 25);
}

void TopPanel::gcallback_show_menu(GtkWidget *button, gpointer data)
{
    GtkWidget *menu = (GtkWidget *) data;
    gtk_menu_popup_at_widget(GTK_MENU(menu), button, GDK_GRAVITY_EAST, GDK_GRAVITY_NORTH_WEST, NULL);
}

void TopPanel::gcallback_menu_item_selected(GtkWidget *item, gpointer data)
{
    TopPanel *ptr = (TopPanel *)data;
    std::string s_ppqn = gtk_menu_item_get_label(GTK_MENU_ITEM(item));
    ptr->ppqn = stoi(s_ppqn);
    gtk_button_set_label(GTK_BUTTON(ptr->button_ppqn), s_ppqn.c_str());
}


void TopPanel::gcallback_show_popover(GtkWidget *button, gpointer data)
{
    gtk_popover_popup(GTK_POPOVER(((TopPanel*)data)->popover));
}

void TopPanel::gcallback_popover_set(GtkWidget *button, gpointer data)
{
    TopPanel *ptr = (TopPanel *)data;
    ptr->time_signature_upper = gtk_combo_box_get_active(GTK_COMBO_BOX(ptr->popo_combo_l))+1;
    ptr->time_signature_lower_as_power_2 = gtk_combo_box_get_active(GTK_COMBO_BOX(ptr->popo_combo_r))+1;
    gtk_popover_popdown(GTK_POPOVER(ptr->popover));
    gtk_button_set_label( GTK_BUTTON(ptr->button_popo),
            ( std::to_string(ptr->time_signature_upper)
              + "/"
              + std::to_string(1<<ptr->time_signature_lower_as_power_2) ).c_str()
    );
}
