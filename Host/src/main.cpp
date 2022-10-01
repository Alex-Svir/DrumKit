#include <iostream>
#include <gtk/gtk.h>
#include "AppWindow.h"

int main(int argc, char** argv) {
    gtk_init(&argc, &argv);
    appwin::launch();
    gtk_main();

    return 0;
}
