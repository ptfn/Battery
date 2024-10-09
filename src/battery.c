#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MAX_LEN_PATH        1024
#define MAX_LEN_COMMAND     200
#define MAX_NOTIFY_TEXT     200

typedef struct 
{
    FILE    *file;
    int     perc;
    char    new[5];
} Light;

const char *icon[] = {"", "", "", "", "", "", ""};
const char *text[] = {"C", "F", "D", "D", "D", "D", "D"};
static int min_battery_perc = 30;

/* Open File */
FILE *open(char *fname, char *mode)
{
    FILE *file;
    if ((file = fopen(fname, mode)) == NULL) {
        perror("Error occured while opening file");
        exit(EXIT_FAILURE);
    }
    return file;
}

/* Call Notify 
void notify_command(char *message, char *icon)
{
    char command[MAX_NOTIFY_TEXT] = "notify-send 'Battery' ";
    char *arg = " --icon='";
    strcat(command, message);
    strcat(command, arg);
    strcat(command, icon);
    strcat(command, "'");
    system(command);
}*/

/* Create Notification
void notification(char *notify_message, char *command_message,
                  char *icon, char *file_name, char command[])
{
    notify_command(notify_message, icon);
    strcat(command, command_message);
    strcat(command, file_name);
    system(command);
}*/

/* Print Percent Battery */
void print(char *status, int perc, const char **icon, Light bright)
{
    if (strcmp(status, "Charging\n") == 0) {
        printf("%s %d%%", icon[0], perc);
    } else if (strcmp(status, "Full\n") == 0) {
        printf("%s %d%%", icon[1], perc);
    } else {
        if (perc >= 0 && perc <= 20) {
            printf("%s %d%%", icon[2], perc);
            system("systemctl suspend");
        } else if (perc > 20 && perc <= min_battery_perc) {
            printf("%s %d%%", icon[3], perc);
            sprintf(bright.new, "%d", bright.perc);
            fputs(bright.new, bright.file);
        } else if (perc > min_battery_perc && perc <= 65) {
            printf("%s %d%%", icon[4], perc);
        } else if (perc > 65 && perc <= 85) {
            printf("%s %d%%", icon[5], perc);
        } else {
            printf("%s %d%%", icon[6], perc);
        }
    }
}

void clock(char *energy, char *rate, char* full, char *status)
{
    int ienergy, irate, ifull, second, minute, hour;
    double all = 0;
    
    ienergy = atoi(energy) / 1000;
    irate = atoi(rate) / 1000;
    ifull = atoi(full) / 1000;

    if (irate > 0) {
        if (strcmp(status, "Charging\n") == 0)
            all = (((float)ifull - (float)ienergy) / (float)irate) * 3600;
        else
            all = ((float)ienergy / (float)irate) * 3600;
    }
    
    second = (int)all % 60;
    minute = (int)(all / 60) % 60;
    hour = all/3600;

    printf(" %d%d:%d%d:%d%d", hour / 10, hour % 10
                                    , minute / 10, minute % 10
                                    , second / 10, second % 10);
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
    } else if (strcmp(status, "Discharging") == 0 && perc <= min_battery_perc) {
        notification("'Battery Low!'", "echo Low > ",
                     low, name, command);
    }
} 

int main(int argc, char **argv)
{
    char project_path[1024];
    bool anotify, atime, aicon;
    int c;

    anotify = atime = aicon = false;

    while ((c = getopt(argc, argv, "m:tnip:")) != -1) {
        switch (c) {
            case 'p':
                sprintf(project_path, "%s", optarg);
                break;

            case 'n':
                anotify = true;
                break;

            case 't':
                atime = true;
                break;

            case 'm':
                min_battery_perc = atoi(optarg);
                break;

            case 'i':
                aicon = true;
                break;
        }
    }

    
    /* Init Variables */
    char *discharging_icon, *charging_icon, *low_icon;

    if (anotify) {
        /* Alloc Icon */
        discharging_icon = (char*)calloc(MAX_LEN_PATH, sizeof(char));
        charging_icon = (char*)calloc(MAX_LEN_PATH, sizeof(char));
        low_icon = (char*)calloc(MAX_LEN_PATH, sizeof(char));

        /* Add Project_Path To Icon and Name */
        strcat(discharging_icon, project_path);
        strcat(charging_icon, project_path);
        strcat(low_icon, project_path);

        /* Add Full Path */
        strcat(discharging_icon, "/img/discharging.svg");
        strcat(charging_icon, "/img/charging.svg");
        strcat(low_icon, "/img/low.svg");
    }

    /* Init Stat File */
    char *name_choice = (char*)calloc(MAX_LEN_PATH, sizeof(char));
    strcat(name_choice, project_path);
    strcat(name_choice, "/bat.txt"); // Create meself file
    
    /* Icon Battery */
    
    /* Name Files Battery */
    char *name_percent = "/sys/class/power_supply/BAT0/capacity";
    char *name_status = "/sys/class/power_supply/BAT0/status";   
    char *name_rate = "/sys/class/power_supply/BAT0/power_now";
    char *name_energy = "/sys/class/power_supply/BAT0/energy_now";
    char *name_full = "/sys/class/power_supply/BAT0/energy_full";
    char *name_now = "/sys/class/backlight/intel_backlight/brightness";
    char *name_max = "/sys/class/backlight/intel_backlight/max_brightness";
    
    /* Init Variables For Files Battery */
    char percent[4], status[12], choice[12], rate[9], energy[9], full[9], now[5], max[5];
    FILE *file_percent, *file_status, *file_choice, *file_rate, *file_energy, *file_full, *file_now, *file_max;

    /* Open File */
    file_percent = open(name_percent, "r");
    file_status = open(name_status, "r");
    file_choice = open(name_choice, "r");
    file_rate = open(name_rate, "r");
    file_energy = open(name_energy, "r");
    file_full = open(name_full, "r");
    file_now = open(name_now, "w+");
    file_max = open(name_max, "r");

    /* Read File */
    fgets(percent, 4, file_percent);
    fgets(status, 12, file_status);
    fgets(choice, 12, file_choice);
    fgets(rate, 9, file_rate);
    fgets(energy, 9, file_energy);
    fgets(full, 9, file_full);
    fgets(now, 5 ,file_now);
    fgets(max, 5, file_max);

    /* Print Percent */
    int bperc = atoi(percent);
    Light bright = {
        .file = file_now,
        .perc = (int)(atoi(max)/100*20)
    };
    print(status, bperc, aicon ? icon : text, bright);

    /* Time Battery */
    if (atime)
        clock(energy, rate, full, status);

    /* Notifacation */
    char command[MAX_LEN_COMMAND];

    /* Condition Notify */
    if (anotify)
        condition(choice, status, charging_icon, discharging_icon,
              low_icon, name_choice, bperc, command);
    
    /* Close File */
    fclose(file_percent);
    fclose(file_status);
    fclose(file_choice);
    fclose(file_rate);
    fclose(file_energy);
    fclose(file_full);
    fclose(file_now);
    fclose(file_max);
}
