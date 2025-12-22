#include <libxfce4panel/libxfce4panel.h>
#include <gtk/gtk.h>
#include "processmanager.h"

static void hello_construct(XfcePanelPlugin *plugin)
{
    GtkWidget *label = gtk_label_new(NULL);
    GtkWidget *button;

    button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

    unsigned int open_ports = get_open_ports_count();
    char *markup = g_strdup_printf("<span foreground=\"#00aa00\">• Online: %d</span>", open_ports);
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);

    gtk_container_add(GTK_CONTAINER(button), label);

    g_signal_connect(button, "clicked", 
                     G_CALLBACK(on_plugin_clicked), 
                     plugin);

    gtk_widget_set_has_tooltip(button, TRUE);

    g_signal_connect(button,
        "query-tooltip",
        G_CALLBACK(on_hover),
        NULL);

    gtk_container_add(GTK_CONTAINER(plugin), button);
    gtk_widget_show_all(GTK_WIDGET(plugin));

    // Set up periodic updates every 1 second
    g_timeout_add(1000, (GSourceFunc)update_open_ports_label, GTK_LABEL(label));
}

XFCE_PANEL_PLUGIN_REGISTER(hello_construct);