#include "processmanager.h"

void on_plugin_clicked(GtkWidget *widget, gpointer data)
{
    XfcePanelPlugin *plugin = XFCE_PANEL_PLUGIN(data);
    create_process_manager_window(GTK_WIDGET(plugin));
}

void on_terminate_clicked(GtkWidget *widget, gpointer data)
{
    
}

gboolean on_hover(GtkWidget *widget,
                                 gint x,
                                 gint y,
                                 gboolean keyboard_mode,
                                 GtkTooltip *tooltip,
                                 gpointer user_data)
{
    gtk_tooltip_set_text(tooltip,
        "ProcWatch\nProcesses: 128\nTCP: 34");

    return TRUE;
}
