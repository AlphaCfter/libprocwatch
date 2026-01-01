#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>

/**
 * ProcessInfo:
 *
 * Holds information about a single network-related process.
 *
 * All string members are dynamically allocated and owned by the
 * #ProcessInfo instance. They must be freed when the structure
 * is destroyed.
 *
 * @local_ip: Local IP address as a string
 * @local_port: Local port number
 * @remote_ip: Remote IP address as a string
 * @protocol: Network protocol in use (e.g. "tcp", "udp")
 * @pid: Process ID
 * @process_name: Name of the process
 */
typedef struct {
    char *local_ip;
    int   local_port;
    char *remote_ip;
    char *protocol;
    int   pid;
    char *process_name;
} ProcessInfo;

/**
 * ProcessList:
 *
 * Container for a dynamically allocated list of network-related processes.
 *
 * @processes: Pointer to an array of #ProcessInfo structures.
 *             The array is allocated dynamically and must be freed by
 *             free_process_list().
 *
 * @count: Number of valid entries in @processes.
 *
 * @auth_failed: Set to %TRUE if authentication failed or was cancelled
 *               by the user while retrieving process information.
 */
typedef struct {
    ProcessInfo *processes;
    int count;
    gboolean auth_failed;
} ProcessList;

/**
 * create_process_manager_window:
 * @parent: (nullable): The parent GtkWidget, typically the main application window.
 *
 * Creates and displays the process manager window. This window allows the user
 * to view and manage running processes.
 */
void create_process_manager_window(GtkWidget *parent);

/**
 * get_network_processes:
 *
 * Retrieves a list of processes that are utilizing network resources.
 *
 * Returns: (transfer full): A pointer to a ProcessList structure containing
 * the network processes. The caller is responsible for freeing the memory
 * using free_process_list().
 */
ProcessList* get_network_processes(void);

/**
 * free_process_list:
 * @list: (nullable): A pointer to the ProcessList structure to be freed.
 *
 * Frees the memory allocated for a ProcessList structure.
 */
void free_process_list(ProcessList *list);

/**
 * on_plugin_clicked:
 * @widget: The GtkWidget that triggered the event.
 * @data: (nullable): User data passed to the callback function.
 *
 * Callback function that is triggered when the plugin is clicked.
 */
void on_plugin_clicked(GtkWidget *widget, gpointer data);

/**
 * on_terminate_clicked:
 * @widget: The GtkWidget that triggered the event.
 * @data: (nullable): User data passed to the callback function.
 *
 * Callback function that is triggered when the terminate button is clicked.
 * Typically used to terminate a selected process.
 */
void on_terminate_clicked(GtkWidget *widget, gpointer data);

/**
 * on_hover:
 * @widget: The GtkWidget being hovered over.
 * @x: The x-coordinate of the hover event.
 * @y: The y-coordinate of the hover event.
 * @keyboard_mode: A gboolean indicating if the hover was triggered via keyboard navigation.
 * @tooltip: The GtkTooltip to be displayed.
 * @data: (nullable): User data passed to the callback function.
 *
 * Callback function that is triggered when the user hovers over a widget.
 * Used to display a tooltip with additional information.
 *
 * Returns: A gboolean indicating whether the tooltip was successfully updated.
 */
gboolean on_hover(GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer data);

/**
 * get_open_ports_count:
 *
 * Retrieves the number of open network ports.
 *
 * Returns: The count of open network ports as an unsigned integer.
 */
unsigned int get_open_ports_count(void);

/**
 * update_open_ports_label:
 * @label: A GtkLabel widget to be updated with the count of open ports.
 *
 * Updates the given GtkLabel with the current count of open network ports.
 *
 * Returns: A gboolean indicating whether the label was successfully updated.
 */
gboolean update_open_ports_label(GtkLabel *label);

/**
 * display_gtk:
 * A helper function which displays a simple GTK dialog box to display errors
 * or statements since g_print isn't allowed on the production. g_print uses
 * the stdout and gtk_warnings to throw errors latching onto a terminal window which 
 * is active
 * 
 * @parent: A GtkWidget to depict a parent so the child window can latch itself
 * @window_title: A string of GTK window title to be displayed onto the dialouge box
 * @markup: A string of GTK window which displays the body text
 * @button1_label: A string of GTK label for the buttons1
 * @button2_label: A string of GTK label for the button2
 * 
 * Returns: A gboolean value weather these buttons were clicked
 */
gboolean display_gtk(GtkWidget* parent, const char* window_title,
                    const char* markup, const char* button1_label,
                    const char* button2_label);

#endif
