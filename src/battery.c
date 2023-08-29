#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MIN_BATTERY     30
#define MAX_ICON_PATH   100
#define MAX_LEN_COMMAND 100
#define MAX_NOTIFY_TEXT 200
#define PROJECT_PATH ""

/* Open File */
FILE *open(char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")) == NULL) {
        perror("Error occured while opening file");
        exit(EXIT_FAILURE);
    }
    return file;
}

/* Call Notify */
void notify_command(char *message, char *icon)
{
    char command[MAX_NOTIFY_TEXT] = "notify-send 'Battery' ";
    char *arg = " --icon='";
    strcat(command, message);
    strcat(command, arg);
    strcat(command, icon);
    strcat(command, "'");
    system(command);
}

/* Create Notification */
void notification(char *notify_message, char *command_message,
                  char *icon, char *file_name, char command[])
{
    notify_command(notify_message, icon);
    strcat(command, command_message);
    strcat(command, file_name);
    system(command);
}

/* Print Percent Battery */
void print(char *status, int perc)
{
    if (strcmp(status, "Charging\n") == 0) {
        printf(" %d%%", perc);
    } else if (strcmp(status, "Full\n") == 0) {
        printf(" %d%%", perc);
    } else {
        if (perc >= 0 && perc <= 20) {
            printf("  %d%%", perc);
            system("systemctl suspend");
        } else if (perc > 20 && perc <= 30) {
            printf("  %d%%", perc);
            system("brightnessctl set 25 > /dev/null");
        } else if (perc > 30 && perc <= 65) {
            printf("  %d%%", perc);
        } else if (perc > 65 && perc <= 85) {
            printf("  %d%%", perc);
        } else {
            printf("  %d%%", perc);
        }
    }
}

/* Condition Notifacation  */
void condition(char *choice, char *status, char *charging, char *discharging,
               char *low, char *name, int perc, char *command)
{
    if (strcmp(choice, status) != 0) {
        if (strcmp(status, "Charging\n") == 0) {
            notification("'Battery Charging!'", "echo Charging > ",
                         charging, name, command);
            system("brightnessctl set 102 > /dev/null");
        } else if (strcmp(choice, "Low\n") != 0 && strcmp(status, "Discharging") == 0){
            notification("'Battery Discharging!'", "echo Discharging > ",
                         discharging, name, command);
        }
    } else if (strcmp(status, "Discharging") == 0 && perc <= MIN_BATTERY) {
        notification("'Battery Low!'", "echo Low > ",
                     low, name, command);
    }
} 

int main()
{
    char *discharging_icon, *charging_icon, *low_icon, *name_choice;

    discharging_icon = (char*)calloc(MAX_ICON_PATH, sizeof(char));
    charging_icon = (char*)calloc(MAX_ICON_PATH, sizeof(char));
    low_icon = (char*)calloc(MAX_ICON_PATH, sizeof(char));
    name_choice = (char*)calloc(MAX_ICON_PATH, sizeof(char));

    /* Add Project_Path To Icon and Name */
    strcat(discharging_icon, PROJECT_PATH);
    strcat(charging_icon, PROJECT_PATH);
    strcat(low_icon, PROJECT_PATH);
    strcat(name_choice, PROJECT_PATH);

    /* Add Full Path */
    strcat(discharging_icon, "/img/discharging.svg");
    strcat(charging_icon, "/img/charging.svg");
    strcat(low_icon, "/img/low.svg");
    strcat(name_choice, "/bat.txt");

    char *name_percent = "/sys/class/power_supply/BAT0/capacity";
    char *name_status = "/sys/class/power_supply/BAT0/status";   
    
    char percent[3], status[12], choice[12];
    FILE *file_percent, *file_status, *file_choice;

    /* Open File */
    file_percent = open(name_percent);
    file_status = open(name_status);
    file_choice = open(name_choice);

    /* Read File */
    fgets(percent, 3, file_percent);
    fgets(status, 12, file_status);
    fgets(choice, 12, file_choice);

    /* Print Percent */
    int bperc = atoi(percent);
    print(status, bperc);

    /* Notifacation */
    char command[MAX_LEN_COMMAND];
    condition(choice, status, charging_icon, discharging_icon,
              low_icon, name_choice, bperc, command);
    
    /* Close File */
    fclose(file_percent);
    fclose(file_status);
    fclose(file_choice);
}