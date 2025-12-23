#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>

// Structure to hold process information from the 'ss' command
typedef struct {
    char *local_ip;
    int local_port;
    char *remote_ip;
    char *protocol;
    int pid;
    char *process_name;
} ProcessInfo;

// Structure to hold array of processes
typedef struct {
    ProcessInfo *processes;
    int count;
} ProcessList;

void create_process_manager_window(GtkWidget *parent);
ProcessList* get_network_processes();
void free_process_list(ProcessList *list);

void on_plugin_clicked(GtkWidget *widget, gpointer data);
void on_terminate_clicked(GtkWidget *widget, gpointer data);
gboolean on_hover(GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer data);
unsigned int get_open_ports_count();
gboolean update_open_ports_label(GtkLabel *label);

#endif
