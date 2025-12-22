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
        "Process ID", "Internal IP", "Remote IP", 
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
    for (int i = 0; i < 6; i++) {
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
    
    // Add 7 rows of empty cells. Later it would be replaced with n number of
    // available but for now it would be 7 just for the sake of views
    for (int row = 1; row <= 7; row++) {
        // Fill in empty Process ID, Internal IP, Remote IP, Protocol, PID columns
        for (int col = 0; col < 5; col++) {
            label = gtk_label_new("");
            gtk_widget_set_halign(label, GTK_ALIGN_START);
            gtk_widget_set_hexpand(label, TRUE);
            gtk_widget_set_vexpand(label, TRUE);
            gtk_widget_set_margin_start(label, 5);
            gtk_widget_set_margin_end(label, 5);
            gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
        }
        
        // Action column which would have the terminate button
        button = gtk_button_new_with_label("Terminate");
        gtk_widget_set_hexpand(button, TRUE);
        gtk_widget_set_vexpand(button, TRUE);
        g_signal_connect(button, "clicked", 
                        G_CALLBACK(on_terminate_clicked), 
                        GINT_TO_POINTER(row));
        gtk_grid_attach(GTK_GRID(grid), button, 5, row, 1, 1);
    }
    
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