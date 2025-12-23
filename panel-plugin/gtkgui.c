#include <gtk/gtk.h>
#include "processmanager.h"


static void activate(GtkApplication *app, gpointer user_data) {

    // Pointer handles
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *button;
    
    // Some list of column attributes we're gonna have them on the GTK window
    const char *attributes[] = {
        "Process", "Port", "Internal IP", "Remote IP", 
        "Protocol", "PID", "Action"
    };
    
    // Window properties for the GTK window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Process Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
    // Set window icon
    GError *error = NULL;
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "icons/plug.png", &error);
    if (error != NULL) {
        g_warning("Failed to load icon: %s", error->message);
        g_error_free(error);
    }
    
    // Create scrollable viewport for responsive layout
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);
    
    // Grid like structure to view the following attributes on the GTK screen
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_add(GTK_CONTAINER(scrolled_window), grid);
    
    // Plugging those headers into the table. Headers here will be those group
    // of attributes from the array
    for (int i = 0; i < 7; i++) {
        label = gtk_label_new(attributes[i]);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        
        // Make header attributes bold
        PangoAttrList *attrs = pango_attr_list_new();
        PangoAttribute *attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
        pango_attr_list_insert(attrs, attr);
        gtk_label_set_attributes(GTK_LABEL(label), attrs);
        pango_attr_list_unref(attrs);
        
        gtk_grid_attach(GTK_GRID(grid), label, i, 0, 1, 1);
    }
    
    // Get network processes data
    ProcessList *process_list = get_network_processes();
    int n = process_list->count;
    
    // Add n rows with actual process data
    for (int row = 1; row <= n; row++) {
        ProcessInfo *proc = &process_list->processes[row - 1];
        
        // Column 0: Process Name
        label = gtk_label_new(proc->process_name);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
        
        // Column 1: Port
        char port_str[16];
        g_snprintf(port_str, sizeof(port_str), "%d", proc->local_port);
        label = gtk_label_new(port_str);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, 1, row, 1, 1);
        
        // Column 2: Local IP
        label = gtk_label_new(proc->local_ip);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, 2, row, 1, 1);
        
        // Column 3: Remote IP
        label = gtk_label_new(proc->remote_ip);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, 3, row, 1, 1);
        
        // Column 4: Protocol
        label = gtk_label_new(proc->protocol);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, 4, row, 1, 1);
        
        // Column 5: PID
        char pid_str[32];
        g_snprintf(pid_str, sizeof(pid_str), "%d", proc->pid);
        label = gtk_label_new(pid_str);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_vexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_grid_attach(GTK_GRID(grid), label, 5, row, 1, 1);
        
        // Column 6: Action (Terminate button)
        button = gtk_button_new_with_label("Terminate");
        gtk_widget_set_hexpand(button, TRUE);
        gtk_widget_set_vexpand(button, TRUE);
        g_signal_connect(button, "clicked", 
                        G_CALLBACK(on_terminate_clicked), 
                        GINT_TO_POINTER(proc->pid));
        gtk_grid_attach(GTK_GRID(grid), button, 6, row, 1, 1);
    }
    
    // Clean up process list
    free_process_list(process_list);
    
    gtk_widget_show_all(window);
}

//  This window is spawned from the XFCE panel plugin and is attached
//  * to the panel widget as a transient child. No new GTK application
//  * or main loop is created. The window runs inside XFCE's existing
//  * GTK event loop. 
void create_process_manager_window(GtkWidget *parent) {
    GtkApplication *app = gtk_application_new("com.alpha.processmanager",
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    g_application_run(G_APPLICATION(app), 0, NULL);
    g_object_unref(app);
}

// Standalone test entry point for GUI development. This is just temprory
#ifdef STANDALONE_GUI
int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.alpha.processmanager",
                                               G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
#endif