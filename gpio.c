#include "gpio.h"

int input(int argc, char **argv){
    //gpio [-status] [-help] [-config] = 1 + 3 total

    if(argc == 1) {
        //on off logic
    }

    else {
        // [-status] [-help] [-cfg]
        for(int i = 1; i < argc; i++)
        {
            if(strcasecmp(argv[i], "-h") == 0 || strcasecmp(argv[i], "-help") == 0);
                //general help
            else if(strcasecmp(argv[i], "-s") == 0 || strcasecmp(argv[i], "-status") == 0);
                //pin status
            else if(strcasecmp(argv[i], "-c") == 0 || strcasecmp(argv[i], "-config") == 0);
                //config file
            else {
                printf("\033[31mgpio: invalid option -- '%s'\nTry 'gpio -help for more information.\n",argv[i]);
                return 1;
            }
        }
    }

    return 0;
}

int help() {
    //todo ?
    char *text = "Usage: gpio [OPTION]...\n\nCase-insensitive, both short and long argument notation can be used.\n\n\t-h, -help \tLists commands with additional information.\n\t-s, -status \tDisplays the status of each pin.\n\t-c, -config \tPrints the configuration file.\n\n\033[32mVersion 1.0\thttps://github.com/FRAKTEON/gpio\n";
    printf("%s",text);
    return 0;
}
int status() {
    //todo display status of each pin {3, 5, 7, 29, 31, 26, 24, 21, 19, 23, 32, 33, 8, 10, 36, 11, 12, 35, 38, 40, 15, 16, 18, 22, 37, 13}
    return 0;
}

int config() {
    //todo display the configuration file ???
    return 0;
}

//todo rework functions below

int export_pin(GPIO *p_io){

    if((p_io->fd = open("/sys/class/gpio/export", O_WRONLY)) == -1){
        printf("Unable to open /sys/class/gpio/export\n");
        return EXIT_FAILURE;
    }

    if((write(p_io->fd, p_io->pin, 2)) != 2){
        printf("Error writing to /sys/class/gpio/export\n");
        return EXIT_FAILURE;
    }

    close(p_io->fd);

    return EXIT_SUCCESS;
}

int direction(GPIO *p_io){

    if((p_io->fd = open("/sys/class/gpio/gpio24/direction", O_WRONLY)) == -1){
        printf("Unable to open /sys/class/gpio/gpio24/direction\n");
        return EXIT_FAILURE;
    }

    if ((write(p_io->fd, "out", 3) != 3)){
        printf("Error writing to /sys/class/gpio/gpio24/direction\n");
        return EXIT_FAILURE;
    }

    close(p_io->fd);

    return EXIT_SUCCESS;
}

int toggle(GPIO *p_io){

    if ((p_io->fd = open("/sys/class/gpio/gpio24/value", O_WRONLY)) == 1)
    {
        printf("Unable to open /sys/class/gpio/gpio24/value");
        return EXIT_FAILURE;
    }

    if((write(p_io->fd, p_io->value, 1)) != 1)
    {
        printf("Error writing to /sys/class/gpio/gpio24/value");
        return EXIT_FAILURE;
    }

    close(p_io->fd);
}

int unexport(GPIO *p_io){

    if ((p_io->fd = open("/sys/class/gpio/unexport", O_WRONLY)) == -1) {
        printf("Unable to open /sys/class/gpio/unexport");
        return EXIT_FAILURE;
    }

    if ((write(p_io->fd, p_io->pin, 2)) != 2) {
        printf("Error writing to /sys/class/gpio/unexport");
        return EXIT_FAILURE;
    }

    close(p_io->fd);

    return EXIT_SUCCESS;
}

