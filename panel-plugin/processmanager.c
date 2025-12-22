#include "processmanager.h"
#include <stdlib.h>
#include <stdio.h>

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

unsigned int get_open_ports_count() {
    FILE *fp;
    char buffer[128];
    int open_ports = 0;

    const char *cmd = "awk 'NR>1 {split($2, a, \":\"); port = strtonum(\"0x\" a[2]); if (FILENAME ~ /tcp/ && $4 == \"0A\") ports[port]=1; if (FILENAME ~ /udp/ && port != 0) ports[port]=1;} END {print length(ports)}' /proc/net/tcp /proc/net/tcp6 /proc/net/udp /proc/net/udp6";

    fp = popen(cmd, "r");
    if (fp == NULL) {
        g_warning("Failed to run awk command");
        return -1;
    }

    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        open_ports = atoi(buffer);
    }

    pclose(fp);
    return open_ports;
}

gboolean update_open_ports_label(GtkLabel *label) {
    unsigned int open_ports = get_open_ports_count();
    char *markup = g_strdup_printf("<span foreground=\"#00aa00\">• Online: %d</span>", open_ports);
    gtk_label_set_markup(label, markup);
    g_free(markup);

    return TRUE;
}
