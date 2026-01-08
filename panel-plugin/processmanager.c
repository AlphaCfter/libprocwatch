#include "processmanager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *CMD = "sudo -n /usr/local/bin/panel-plugin/helpers/ss-network-viewer.sh";
static const char *KILLER_HELPER = "sudo /usr/local/bin/panel-plugin/helpers/port-trace-kill.sh %d";
static const char *FALLBACK_CMD = "ss -tupln";
static const char *FILES[] = {
        "/proc/net/tcp",
        "/proc/net/tcp6",
        "/proc/net/udp",
        "/proc/net/udp6"
    };

// Function to get al those network processes from 'ss' binary
ProcessList* get_network_processes() {
    FILE *fp;
    char buffer[1024];
    ProcessList *list = g_malloc0(sizeof(ProcessList));
    unsigned int capacity = 50;
    list->processes = g_malloc0(capacity * sizeof(ProcessInfo));
    list->count = 0;
    list->auth_failed = FALSE;

    g_message("DEBUG: get_network_processes() called");
    
    /* Execute ss command via sudo using wrapper script
    * The wrapper ignores all arguments and always runs: 
    * 'ss -tupln'
    * This prevents argument injection attacks
    * Use -n flag to fail immediately if sudo requires a password
    */
    gboolean retry_without_sudo = FALSE;
    
    g_message("DEBUG: About to run command: %s", CMD);
    
    fp = popen(CMD, "r");
    if (fp == NULL) {
        g_warning("Failed to run sudo ss command");
        list->auth_failed = TRUE;
        return list;
    }
    
    g_message("DEBUG: Command executed, reading output...");

    // Skips header line
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        // If we get no output, authentication was likely cancelled
        int status = pclose(fp);
        if (status != 0) {
            g_warning("Authentication cancelled or failed (exit code: %d) - retrying without sudo", WEXITSTATUS(status));
            retry_without_sudo = TRUE;
        } else {
            return list;
        }
    }

    // If authentication failed, retry with ss command without sudo
    if (retry_without_sudo) {
        g_message("DEBUG: Retrying with ss command without sudo");
        
        fp = popen(FALLBACK_CMD, "r");
        if (fp == NULL) {
            g_warning("Failed to run ss command without sudo");
            list->auth_failed = TRUE;
            return list;
        }
        
        // Skip header line for non-sudo command
        if (fgets(buffer, sizeof(buffer), fp) == NULL) {
            pclose(fp);
            list->auth_failed = TRUE;
            return list;
        }
        
        // Mark that we're running in rootless mode
        list->auth_failed = TRUE;
    }

    // Parses each line of the command
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (list->count >= capacity) {
            capacity *= 2;
            list->processes = g_realloc(list->processes, capacity * sizeof(ProcessInfo));
        }

        ProcessInfo *info = &list->processes[list->count];
        char netid[16], state[32], local_addr[256], peer_addr[256], users[512];
        
        // Parses ss output format: Netid State Recv-Q Send-Q Local_Address:Port Peer_Address:Port Process
        int parsed = sscanf(buffer, "%15s %*s %*s %*s %255s %255s %511[^\n]",
                           netid, local_addr, peer_addr, users);
        
        if (parsed < 3) {
            continue;
        }

        // Extracts protocol
        info->protocol = g_strdup(netid);
        
        // Extracts local IP and ports
        info->local_port = 0;
        char *last_colon = strrchr(local_addr, ':');
        if (last_colon) {
            info->local_port = atoi(last_colon + 1);
            *last_colon = '\0';
        }
        // Removes brackets from IPv6 addresses
        if (local_addr[0] == '[') {
            char *close_bracket = strchr(local_addr, ']');
            if (close_bracket) {
                *close_bracket = '\0';
                info->local_ip = g_strdup(local_addr + 1);
            } else {
                info->local_ip = g_strdup(local_addr);
            }
        } else {
            info->local_ip = g_strdup(local_addr);
        }
        
        // Extracts remote IP and removes the ports associated
        last_colon = strrchr(peer_addr, ':');
        if (last_colon) {
            *last_colon = '\0';
        }

        // Removes brackets from IPv6 addresses
        if (peer_addr[0] == '[') {
            char *close_bracket = strchr(peer_addr, ']');
            if (close_bracket) {
                *close_bracket = '\0';
                info->remote_ip = g_strdup(peer_addr + 1);
            } else {
                info->remote_ip = g_strdup(peer_addr);
            }
        } else {
            info->remote_ip = g_strdup(peer_addr);
        }
        
        /* Extracts PID and process name from users field.
         * This would be in the format given below
         * Format: users:(("process_name",pid=1234,fd=5))
        */
        info->pid = 0;
        info->process_name = g_strdup("");
        
        if (parsed >= 4 && users[0] != '\0') {
            char *pid_start = strstr(users, "pid=");
            if (pid_start) {
                info->pid = atoi(pid_start + 4);
            }
            
            char *name_start = strstr(users, "((\"");
            if (name_start) {
                name_start += 3;
                char *name_end = strchr(name_start, '"');
                if (name_end) {
                    int name_len = name_end - name_start;
                    info->process_name = g_strndup(name_start, name_len);
                }
            }
        }
        
        list->count++;
    }

    pclose(fp);
    g_message("DEBUG: Finished parsing, found %d processes", list->count);
    return list;
}

// Function to free up and clean opened processes
void free_process_list(ProcessList *list) {
    if (list == NULL) return;
    
    for (int i = 0; i < list->count; i++) {
        g_free(list->processes[i].local_ip);
        g_free(list->processes[i].remote_ip);
        g_free(list->processes[i].protocol);
        g_free(list->processes[i].process_name);
    }
    
    g_free(list->processes);
    g_free(list);
}

void on_plugin_clicked(GtkWidget *widget, gpointer data)
{
    g_message("DEBUG: on_plugin_clicked called!");
    XfcePanelPlugin *plugin = XFCE_PANEL_PLUGIN(data);
    g_message("DEBUG: About to call create_process_manager_window");
    create_process_manager_window(GTK_WIDGET(plugin));
    g_message("DEBUG: Returned from create_process_manager_window");
}

void on_terminate_clicked(GtkWidget *widget, gpointer data)
{
    if (data == NULL)
        return;

    int pid = GPOINTER_TO_INT(data);
    if (pid <= 0) {
        g_warning("Invalid PID: %d", pid);
        return;
    }

    // Gets the toplevel window from the button widget
    GtkWidget *window = gtk_widget_get_toplevel(widget);
    if (!GTK_IS_WINDOW(window)) {
        window = NULL;
    }

    // Simple check: Verify process exists before trying to kill it
    // kill(pid, 0) returns 0 if process exists, -1 if it doesn't
    if (kill(pid, 0) != 0) {
        char *markup = g_strdup_printf("Process %d does not exist or is not accessible", pid);
        display_gtk(window, "Process Not Found", markup, "OK", NULL);
        g_free(markup);
        return;
    }

    // Try to terminate the program by sending SIGTERM to kill it gracefully
    if (kill(pid, SIGTERM) == 0) {
        g_message("Sent SIGTERM to process %d", pid);

        // Give it a moment to terminate gracefully
        g_usleep(100000); // 100ms

        // Check if process still exists
        if (kill(pid, 0) == 0) {
            gboolean status = display_gtk_printf(window, 
                "Unable to kill gracefully", "Kill Process", "Cancel", "SIGTERM was issued to PID %d but it still exists, should it kill forcefully?",pid);
            if (status) {
                if (kill(pid, SIGKILL) == -1) {
                    g_warning("Failed to send SIGKILL to process %d: %s",
                             pid, g_strerror(errno));
                } else {
                    g_message("Sent SIGKILL to process %d", pid);
                }
            }
        }
    } else {
        // If EPERM (no permission), try sudo helper for root processes
        if (errno == EPERM) {
            g_message("Process %d requires elevated privileges, using sudo helper", pid);
            
            char cmd[256];
            snprintf(cmd, sizeof(cmd), KILLER_HELPER, pid);
            
            int status = system(cmd);
            if (status == 0) {
                g_message("Successfully killed root process %d via sudo helper", pid);
            } else {
                g_warning("Failed to kill root process %d (exit status: %d)", 
                         pid, WEXITSTATUS(status));
            }
        } else {
            g_warning("Failed to send SIGTERM to process %d: %s",
                     pid, g_strerror(errno));
        }
    }
}

/**
 * display_gtk:
 * @parent: (nullable): Parent window for the dialog
 * @window_title: Title of the dialog (must not be %NULL)
 * @markup: Message text to display (must not be %NULL)
 * @button1_label: (nullable): Label for the confirmation button
 * @button2_label: (nullable): Label for the cancellation button
 *
 * Displays a modal question dialog.
 *
 * Only one dialog may be active at a time. If a dialog is already
 * open, it is presented and the function returns %FALSE.
 *
 * At least one of @button1_label or @button2_label must be non-%NULL.
 * Violating these preconditions causes the function to return %FALSE
 * and log a critical warning.
 *
 * Returns: %TRUE if the confirmation button was activated, %FALSE otherwise
 */
gboolean
display_gtk(GtkWidget   *parent,
            const char  *window_title,
            const char  *markup,
            const char  *button1_label,
            const char  *button2_label)
{
    static GtkWidget *active_window = NULL;

    /*
     * This means that some or the other GTK window is still active
     * and isnt terminated. It strictly enforces that only one instance
     * i.e the parent window and one child GTK window is present. If
     * attempted to spawn more than one window, it focuses the open GTK
     * window rather than spawning another one
     */
    if (active_window) {
        gtk_window_present(GTK_WINDOW(active_window));
        return FALSE;
    }

    /*
     * Atleast ONE button should be non NULL, else there would be no point
     * of having this GTK dialog box at all. RETURNS a FALSE value raising
     * a critical statement (maybe at journalctl --user)
     */
    g_return_val_if_fail(parent == NULL || GTK_IS_WINDOW(parent), FALSE);
    g_return_val_if_fail(window_title != NULL, FALSE);
    g_return_val_if_fail(markup != NULL, FALSE);
    g_return_val_if_fail(button1_label || button2_label, FALSE);

    /*
     * Create a modal question dialog without buttons.
     * The dialog is transient for @parent and displays @markup.
     * Buttons will be added later becuase its dynamic
     */
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(parent),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_NONE,
        NULL
    );

    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), markup);
    gtk_window_set_title(GTK_WINDOW(dialog), window_title);

    if (button1_label)
        gtk_dialog_add_button(GTK_DIALOG(dialog),
                              button1_label,
                              GTK_RESPONSE_ACCEPT);

    if (button2_label)
        gtk_dialog_add_button(GTK_DIALOG(dialog),
                              button2_label,
                              GTK_RESPONSE_CANCEL);

    /* Tracks and latches the current window to prevent GTK spam */
    active_window = dialog;

    /* Destroys when user closes the window and releases the active_window */
    g_signal_connect(dialog,
                     "destroy",
                     G_CALLBACK(gtk_widget_destroyed),
                     &active_window);

    /* Grabs the response from the user */
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

    return response == GTK_RESPONSE_ACCEPT;
}

gboolean display_gtk_printf(GtkWidget *widget, const char *title, 
                        const char *button1, const char *button2,
                        const char *format, ...) {
    va_list args;
    va_start(args, format);
    char *markup = g_strdup_vprintf(format, args);
    va_end(args);
    
    gboolean result = display_gtk(widget, title, markup, button1, button2);
    g_free(markup);
    return result;
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

// Parse /proc/net files directly without privileges
// Returns number of unique listening ports
unsigned int get_open_ports_count() {
    
    // Use a simple array to track unique ports (max 65536)
    // For efficiency, use a hash set approach with bit array
    unsigned char port_bitmap[8192] = {0};
    int unique_ports = 0;
    
    for (int file_idx = 0; file_idx < 4; file_idx++) {
        FILE *fp = fopen(FILES[file_idx], "r");
        if (fp == NULL) {
            g_warning("Could not open %s", FILES[file_idx]);
            continue;
        }
        
        char line[512];
        // Skip header line
        if (fgets(line, sizeof(line), fp) == NULL) {
            fclose(fp);
            continue;
        }
        
        // Parse each connection line
        while (fgets(line, sizeof(line), fp) != NULL) {
            unsigned long local_addr, remote_addr;
            unsigned int local_port, remote_port, state;
            
            // Format: sl local_address rem_address st tx_queue rx_queue ...
            // local_address format: XXXXXXXX:XXXX (hex IP:hex port)
            int parsed = sscanf(line, "%*d: %lx:%x %lx:%x %x",
                              &local_addr, &local_port, 
                              &remote_addr, &remote_port, &state);
            
            if (parsed < 5) {
                continue;
            }
            
            // For TCP: Only count LISTEN state (0A in hex = 10 in decimal)
            // For UDP: Count all non-zero ports
            gboolean should_count = FALSE;
            
            if (strstr(FILES[file_idx], "tcp")) {
                should_count = (state == 0x0A && local_port > 0);
            } else if (strstr(FILES[file_idx], "udp")) {
                should_count = (local_port > 0);
            }
            
            if (should_count && local_port < 65536) {
                // Check if this port is already counted
                int byte_idx = local_port / 8;
                int bit_idx = local_port % 8;
                
                if (!(port_bitmap[byte_idx] & (1 << bit_idx))) {
                    // Mark this port as seen
                    port_bitmap[byte_idx] |= (1 << bit_idx);
                    unique_ports++;
                }
            }
        }
        
        fclose(fp);
    }
    
    return unique_ports;
}

gboolean update_open_ports_label(GtkLabel *label) {
    unsigned int open_ports = get_open_ports_count();
    char *markup = g_strdup_printf("<span foreground=\"#00aa00\">• Online: %d</span>", open_ports);
    gtk_label_set_markup(label, markup);
    g_free(markup);

    return TRUE;
}
