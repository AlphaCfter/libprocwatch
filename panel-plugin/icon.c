#include <libxfce4panel/libxfce4panel.h>
#include <gtk/gtk.h>

static void hello_construct(XfcePanelPlugin *plugin)
{
    GtkWidget *label = gtk_label_new(NULL);

    gtk_label_set_markup(GTK_LABEL(label),
    "<span foreground=\"#00aa00\">• Online</span> "
    "<span foreground=\"#ff0000\" weight=\"bold\">• Offline</span>");


    gtk_container_add(GTK_CONTAINER(plugin), label);
    gtk_widget_show_all(GTK_WIDGET(plugin));

}

XFCE_PANEL_PLUGIN_REGISTER(hello_construct);