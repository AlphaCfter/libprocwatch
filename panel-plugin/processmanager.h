#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>

void create_process_manager_window(GtkWidget *parent);

void on_plugin_clicked(GtkWidget *widget, gpointer data);
void on_terminate_clicked(GtkWidget *widget, gpointer data);
gboolean on_hover(GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer data);
#endif
