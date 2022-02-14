#include "BottomPanel.h"

BottomPanel::BottomPanel()
{
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    port_button = gtk_button_new_with_label("SELECT PORT");

    DIR *dir = opendir("/dev");
    struct dirent *file;
    while ((file = readdir(dir)) != NULL)
    {
        if (strstr(file->d_name, "ttyUSB") == file->d_name)
        {
            char dev[262] = "/dev/";
            gtk_button_set_label(GTK_BUTTON(port_button), strcat(dev, file->d_name));
        }
    }
    closedir(dir);

    g_signal_connect(G_OBJECT(port_button), "clicked", G_CALLBACK(gcallback_port_button_clicked), this);
    gtk_box_pack_start(GTK_BOX(box), port_button, FALSE, FALSE, 2);

    file_button = gtk_button_new_with_label("FILE NAME");
    g_signal_connect(G_OBJECT(file_button), "clicked", G_CALLBACK(gcallback_file_button), this);
    gtk_box_pack_end(GTK_BOX(box), file_button, FALSE, FALSE, 2);
}

GtkWidget* BottomPanel::get_pointer()
{
    return box;
}

std::string BottomPanel::get_file_name()
{
    if (filename != "") return filename;
    time_t _time = time(0);
    struct tm *_tm = localtime(&_time);
    char c[24];
    strftime(c, 24, "%F-%H-%M-%S.mid", _tm);
    return std::string(c);
}

std::string BottomPanel::get_file_name_with_path()
{
    if (filename != "") return path + filename;
    return path + get_file_name();
}

void BottomPanel::gcallback_file_button(GtkWidget *button, gpointer data)
{
    BottomPanel *ptr = (BottomPanel *)data;
    GtkFileChooserNative *chooser_native = gtk_file_chooser_native_new("Save New File",
                                                            NULL,
                                                            GTK_FILE_CHOOSER_ACTION_SAVE,
                                                            NULL,
                                                            NULL);

    GtkFileChooser *chooser = GTK_FILE_CHOOSER(chooser_native);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    if (ptr->path != "") gtk_file_chooser_set_current_folder(chooser, ptr->path.c_str());
    gtk_file_chooser_set_current_name(chooser, ptr->get_file_name().c_str());

    gint result = gtk_native_dialog_run(GTK_NATIVE_DIALOG(chooser_native));
    if (result == GTK_RESPONSE_ACCEPT)
    {
        ptr->set_file_name_and_path(gtk_file_chooser_get_filename(chooser));
        gtk_button_set_label(GTK_BUTTON(ptr->file_button), ptr->get_file_name_with_path().c_str());
    }

    g_object_unref(chooser_native);
}

void BottomPanel::gcallback_port_button_clicked(GtkWidget *button, gpointer data)
{
    GtkWidget *menu = gtk_menu_new();

    DIR *dir = opendir("/dev");
    struct dirent *file;
    while ((file = readdir(dir)) != NULL)
    {
        if (strstr(file->d_name, "ttyUSB") == file->d_name)
        {
            char dev[262] = "/dev/";
            GtkWidget *item = gtk_menu_item_new_with_label(strcat(dev,file->d_name));
            gtk_menu_shell_prepend(GTK_MENU_SHELL(menu), item);
            gtk_widget_show(item);
            g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(gcallback_port_menu_activated), data);
        }
    }
    closedir(dir);
    gtk_menu_popup_at_widget(GTK_MENU(menu), button, GDK_GRAVITY_EAST, GDK_GRAVITY_SOUTH_WEST, NULL);
}

void BottomPanel::gcallback_port_menu_activated(GtkWidget *item, gpointer data)
{
    BottomPanel *ptr = (BottomPanel *)data;
    gtk_button_set_label(GTK_BUTTON(ptr->port_button), gtk_menu_item_get_label(GTK_MENU_ITEM(item)));
}

void BottomPanel::drop_filename()
{
    filename = "";
    gtk_button_set_label(GTK_BUTTON(file_button), "FILE NAME");
}

void BottomPanel::set_file_name_and_path(char *filename)
{
    std::string str(filename);
    int index = str.find_last_of('/') + 1;
    path = str.substr(0, index);
    this->filename = str.substr(index);
    index = this->filename.find_last_of('.');
    if (index > -1)
    {
        std::string ext = this->filename.substr(index);
        for (int i=0; i<ext.length(); i++)
            ext[i] = tolower(ext[i]);
        if (ext == ".mid" || ext == ".midi") return;

    }
    this->filename = this->filename + ".mid";
}

std::string BottomPanel::get_port_name()
{
    return std::string(gtk_button_get_label(GTK_BUTTON(port_button)));
}
