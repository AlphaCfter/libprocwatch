#include <gtk/gtk.h>
#include "processmanager.h"

// Structure to hold refresh timer context
typedef struct {
    GtkWidget *grid;
    guint timer_id;
} RefreshContext;

// Function to populate/refresh grid with process data
static void refresh_process_grid(GtkWidget *grid) {
    GtkWidget *label;
    GtkWidget *button;
    
    g_message("DEBUG: refresh_process_grid called");
    
    // Validate grid
    if (!GTK_IS_GRID(grid)) {
        g_warning("Invalid grid widget");
        return;
    }
    
    // Remove all rows except header (row 0)
    GList *children = gtk_container_get_children(GTK_CONTAINER(grid));
    GList *to_destroy = NULL;
    
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        GtkWidget *widget = GTK_WIDGET(iter->data);
        gint row = 0;
        gtk_container_child_get(GTK_CONTAINER(grid), widget, "top-attach", &row, NULL);
        if (row > 0) {  // Keep header row (row 0)
            to_destroy = g_list_prepend(to_destroy, widget);
        }
    }
    g_list_free(children);
    
    // Destroy old widgets
    for (GList *iter = to_destroy; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(to_destroy);
    
    // Fetch fresh network process data
    g_message("DEBUG: About to fetch network processes");
    ProcessList *process_list = get_network_processes();
    int n = process_list->count;
    
    g_message("DEBUG: Got %d processes to display", n);
    
    // Populate grid with fresh data
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
        
        // Column 6: Terminate button
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
    
    // Show all new widgets
    gtk_widget_show_all(grid);
}

// Timer callback for auto-refresh
static gboolean refresh_timer_callback(gpointer user_data) {
    GtkWidget *grid = GTK_WIDGET(user_data);
    
    if (!GTK_IS_GRID(grid)) {
        return G_SOURCE_REMOVE;  // Stop timer if grid is destroyed
    }
    
    refresh_process_grid(grid);
    return G_SOURCE_CONTINUE;  // Keep timer running
}

// Cleanup on window destroy
static void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    RefreshContext *ctx = (RefreshContext *)user_data;
    
    if (ctx) {
        if (ctx->timer_id > 0) {
            g_source_remove(ctx->timer_id);
        }
        g_free(ctx);
    }
}

static void activate(GtkApplication *app, gpointer user_data) {

    g_message("DEBUG: activate called! app=%p", app);
    
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
    
    // Create refresh context
    RefreshContext *ctx = g_malloc0(sizeof(RefreshContext));
    
    // Window properties for the GTK window
    // When called from panel plugin, app is NULL, so create a regular window
    if (app != NULL) {
        window = gtk_application_window_new(app);
    } else {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    }
    gtk_window_set_title(GTK_WINDOW(window), "Port Trace - Network Monitor");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 500);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
    // Connect destroy signal for cleanup
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), ctx);
    
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
    
    // Store grid reference in context
    ctx->grid = grid;
    
    // Initial population with process data
    refresh_process_grid(grid);
    
    // Set up auto-refresh timer (3 seconds)
    ctx->timer_id = g_timeout_add_seconds(3, refresh_timer_callback, grid);
    
    gtk_widget_show_all(window);
}

//  This window is spawned from the XFCE panel plugin and is attached
//  * to the panel widget as a transient child. No new GTK application
//  * or main loop is created. The window runs inside XFCE's existing
//  * GTK event loop. 
void create_process_manager_window(GtkWidget *parent) {
    g_message("DEBUG: create_process_manager_window called!");
    
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    
    // Create refresh context
    RefreshContext *ctx = g_malloc0(sizeof(RefreshContext));
    
    // Create a regular toplevel window (not application window)
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_message("DEBUG: Window created: %p", window);
    
    gtk_window_set_title(GTK_WINDOW(window), "Port Trace - Network Monitor");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 500);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    
    // Connect destroy signal for cleanup
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), ctx);
    
    // Create scrollable viewport
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);
    
    // Create grid
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_add(GTK_CONTAINER(scrolled_window), grid);
    
    // Add column headers
    const char *attributes[] = {
        "Process", "Port", "Internal IP", "Remote IP", 
        "Protocol", "PID", "Action"
    };
    
    for (int i = 0; i < 7; i++) {
        label = gtk_label_new(attributes[i]);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_widget_set_hexpand(label, TRUE);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        
        // Make header bold
        PangoAttrList *attrs = pango_attr_list_new();
        PangoAttribute *attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
        pango_attr_list_insert(attrs, attr);
        gtk_label_set_attributes(GTK_LABEL(label), attrs);
        pango_attr_list_unref(attrs);
        
        gtk_grid_attach(GTK_GRID(grid), label, i, 0, 1, 1);
    }
    
    // Store grid reference
    ctx->grid = grid;
    
    // Initial population
    g_message("DEBUG: About to do initial refresh");
    refresh_process_grid(grid);
    
    // Set up auto-refresh timer (3 seconds)
    ctx->timer_id = g_timeout_add_seconds(3, refresh_timer_callback, grid);
    g_message("DEBUG: Timer set up with ID: %u", ctx->timer_id);
    
    // Show the window
    gtk_widget_show_all(window);
    g_message("DEBUG: Window shown!");
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