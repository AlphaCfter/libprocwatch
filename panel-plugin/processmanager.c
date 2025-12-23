#include "processmanager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Function to get al those network processes from 'ss' binary
ProcessList* get_network_processes() {
    FILE *fp;
    char buffer[1024];
    ProcessList *list = g_malloc0(sizeof(ProcessList));
    int capacity = 50;
    list->processes = g_malloc0(capacity * sizeof(ProcessInfo));
    list->count = 0;

    // Execute pkexec ss -tupln binary with the polkit ofcs
    const char *cmd = "pkexec ss -tupln 2>/dev/null";
    
    fp = popen(cmd, "r");
    if (fp == NULL) {
        g_warning("Failed to run pkexec ss command");
        return list;
    }

    // Skip header line
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        pclose(fp);
        return list;
    }

    // Parse each line of the command
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (list->count >= capacity) {
            capacity *= 2;
            list->processes = g_realloc(list->processes, capacity * sizeof(ProcessInfo));
        }

        ProcessInfo *info = &list->processes[list->count];
        char netid[16], state[32], local_addr[256], peer_addr[256], users[512];
        
        // Parse ss output format: Netid State Recv-Q Send-Q Local_Address:Port Peer_Address:Port Process
        int parsed = sscanf(buffer, "%15s %*s %*s %*s %255s %255s %511[^\n]",
                           netid, local_addr, peer_addr, users);
        
        if (parsed < 3) {
            continue;
        }

        // Extract protocol
        info->protocol = g_strdup(netid);
        
        // Extract local IP and ports
        info->local_port = 0;
        char *last_colon = strrchr(local_addr, ':');
        if (last_colon) {
            info->local_port = atoi(last_colon + 1);
            *last_colon = '\0';
        }
        // Remove brackets from IPv6 addresses
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
        
        // Extract remote IP and removes the ports associated
        last_colon = strrchr(peer_addr, ':');
        if (last_colon) {
            *last_colon = '\0';
        }

        // Remove brackets from IPv6 addresses
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
        
        /* Extract PID and process name from users field.
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
