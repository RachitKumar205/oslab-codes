#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>

void read_process_info(const char* pid) {
    char path[256];
    char line[256];
    char name[256];
    char state;
    FILE *fp;

    // Read process name and state from /proc/[pid]/status
    snprintf(path, sizeof(path), "/proc/%s/status", pid);
    fp = fopen(path, "r");
    if (fp) {
        uid_t uid = -1;

        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "Name:", 5) == 0) {
                sscanf(line, "Name: %s", name);
            }
            else if (strncmp(line, "State:", 6) == 0) {
                sscanf(line, "State: %c", &state);
            }
            else if (strncmp(line, "Uid:", 4) == 0) {
                sscanf(line, "Uid: %d", &uid);
            }
        }
        fclose(fp);

        // Get username from UID
        struct passwd *pw = getpwuid(uid);
        const char *username = pw ? pw->pw_name : "unknown";

        printf("PID: %s\n", pid);
        printf("Name: %s\n", name);
        printf("State: %c\n", state);
        printf("User: %s\n", username);
        printf("------------------------\n");
    }
}

int main() {
    DIR *dir;
    struct dirent *ent;

    // Open /proc directory
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("Cannot open /proc");
        return 1;
    }

    printf("Running Processes:\n");
    printf("------------------------\n");

    // Read all entries in /proc
    while ((ent = readdir(dir)) != NULL) {
        // Check if the entry is a number (PID)
        if (ent->d_name[0] >= '0' && ent->d_name[0] <= '9') {
            read_process_info(ent->d_name);
        }
    }

    closedir(dir);
    return 0;
}