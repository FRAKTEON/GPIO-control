#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "gpio.h"

#define MAX_BYTES 11
#define DIRECTION_INDEX 7
#define VALUE_INDEX 9
#define FILE_SIZE 308

void signal_handler(int signal, siginfo_t *info, void *context) {}

/*Main program functions*/
int init_struct(struct gpio gpio[]) {
    /*Function that initializes gpio struct array with data from save file*/
    int fd;

    if (access("./gpio.cfg", F_OK) == -1) {
        create_save_file();
        printf("*debug Save file successfully created\n");
    }

    if ((fd = open("./gpio.cfg", O_RDWR)) == -1) {
        fprintf(stderr, "*debug Unable to open save file\n");
        return EXIT_FAILURE;
    }

    char buffer[MAX_BYTES];

    for (int i = 0; i < MAX_PINS; i++) {
        if (read(fd, buffer, 11) == -1) {
            fprintf(stderr, "*debug Error reading from save file\n");
            return EXIT_FAILURE;
        }

        if ((sscanf(buffer, "%s %d %d", gpio[i].name, &gpio[i].direction, &gpio[i].value)) == 0) {
            fprintf(stderr, "*debug Save file data is corrupted\n");
        }
    }

    close(fd);
    return 0;
}

int interface(struct gpio *gpio) {
    /*Function that provides an interface menu to configure GPIO*/
    struct termios defterm; /*Default terminal settings*/
    unsigned char key[3] = {0}; /*Reading 3 bytes from input*/
    int index = 0; /*0 to MAX_PINS - 1 (28 GPIO in total)*/

    /*Set terminal buffering and echo to off*/
    tcgetattr(STDIN_FILENO, &defterm);
    set_terminal(defterm);

    /*ctrl+c signal protection*/
    struct sigaction sa;
    sa.sa_sigaction = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /*Process input*/
    unsigned char state = 0; /*Variable used as an exit condition for the main loop*/

    while (!state) {
        display_menu(gpio, index);

        if (read(STDIN_FILENO, key, 3) == -1) {
            fprintf(stderr, "*debug Error reading from STDIN or CTRL+C signal used\n");
            reset_terminal(defterm);
            cfg_cancel(gpio);
            return EXIT_FAILURE;
        }

        if (key[0] == '\033') {
            switch (key[2]) {
                case UP_ARROW:
                    index = (index - 1 + MAX_PINS) % MAX_PINS;
                    break;
                case DOWN_ARROW:
                    index = (index + 1) % MAX_PINS;
                    break;
            }
        } else
            switch (key[0]) {
                case ENTER:
                    gpio[index].value ^= 1;
                    if ((gpio[index].value) ? gpio_export(&gpio[index]) : gpio_unexport(&gpio[index])) {
                        fprintf(stderr, "*debug (un)export operation error\n");
                        state = 1;
                    }
                    break;
                case DIRECTION:
                    gpio[index].direction ^= 1;
                    if (gpio[index].value) {
                        if (gpio_direction(&gpio[index]))
                            state = 1;
                    }
                    break;
                case QUIT:
                    if (cfg_save(gpio)) {
                        printf("*debug Save operation failed\n");
                    }
                    state = 1;
                    break;
                case CANCEL:
                    if (cfg_cancel(gpio)) {
                        printf("*debug Cancel operation failed\n");
                    }
                    state = 1;
                    break;
            }

        key[2] = 0; /*Reset key[2] to prevent unwanted behavior*/
        if (!state)printf("\033[%dF", MAX_PINS + 3); /*Move cursor to override current text*/
    }

    /*Set terminal back to the default state*/
    reset_terminal(defterm);

    return 0;
}

int cfg_save(struct gpio *gpio) {
    /*Function that saves current configuration of gpio*/
    int fd;

    if ((fd = open("./gpio.cfg", O_WRONLY)) == -1)
        printf("*debug Unable to open save file\n");

    for (int i = 0; i < MAX_PINS; ++i) {
        dprintf(fd, "%s %d %d\n", gpio[i].name, gpio[i].direction, gpio[i].value);
    }

    close(fd);

    return 0;
}

int cfg_cancel(struct gpio *gpio) {
    /*Function that reverts gpio to its previous state*/
    int fd;

    if ((fd = open("./gpio.cfg", O_RDWR)) == -1) {
        fprintf(stderr, "*debug Unable to open save file\n");
        return EXIT_FAILURE;
    }

    char buffer[MAX_BYTES];

    for (int i = 0; i < MAX_PINS; i++) {
        if (read(fd, buffer, 11) == -1) {
            fprintf(stderr, "*debug Error reading from save file\n");
            return EXIT_FAILURE;
        }

        if (gpio[i].direction != (int) (buffer[DIRECTION_INDEX] - '0')) {
            gpio[i].direction = (int) (buffer[DIRECTION_INDEX] - '0');
            if (gpio_direction(&gpio[i]))
                return EXIT_FAILURE;
        }

        if (gpio[i].value != (int) (buffer[VALUE_INDEX] - '0')) {
            gpio[i].value = (int) (buffer[VALUE_INDEX] - '0');
            if ((gpio[i].value) ? gpio_export(&gpio[i]) : gpio_unexport(&gpio[i])) {
                fprintf(stderr, "*debug (un)export operation error\n");
                return EXIT_FAILURE;
            }
        }
    }
    close(fd);
    printf("*debug Successfully reverted any changes\n");
    return 0;
}

int cfg_restore(struct gpio *gpio) {
    /*Function to restore the default settings, use if something went wrong*/
    printf("Are you sure to restore default settings? (y/n): ");
    int c = getc(stdin);
    if (c == 'n' || c == 'N') {
        return EXIT_SUCCESS;
    } else if (c == 'y' || c == 'Y') {

        char path[] = "/sys/class/gpio/gpio00";

        for (int i = 0; i < MAX_PINS; ++i) {
            path[20] = gpio[i].name[4];
            path[21] = gpio[i].name[5];

            if (access(path, F_OK) == 0) {
                printf("Unexporting %s", gpio[i].name);
                gpio_unexport(gpio);
            }
        }

        remove("./gpio.cfg");
        create_save_file();
    } else {
        fprintf(stderr, "*debug Unknown input\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/*System gpio functions*/

int gpio_export(struct gpio *gpio) {
    int fd;

    if ((fd = open("/sys/class/gpio/export", O_WRONLY)) == -1) {
        fprintf(stderr, "*debug Unable to open /sys/class/gpio/export\n");
        return EXIT_FAILURE;
    }

    if ((write(fd, (gpio->name) + 4, 2)) != 2) {
        fprintf(stderr, "*debug Error writing to /sys/class/gpio/export\n");
        return EXIT_FAILURE;
    }

    close(fd);

    return gpio_direction(gpio);
}

int gpio_direction(struct gpio *gpio) {
    int fd;

    char path[] = "/sys/class/gpio/gpio00/direction";
    path[20] = gpio->name[4], path[21] = gpio->name[5];

    if ((fd = open(path, O_WRONLY)) == -1) {
        printf("*debug Unable to open %s\n", path);
        return EXIT_FAILURE;
    }

    if ((gpio->direction) ? (write(fd, "in", 2) != 2) : (write(fd, "out", 3) != 3)) {
        printf("*debug Error writing to %s\n", path);
        return EXIT_FAILURE;
    }

    close(fd);

    return gpio_toggle(gpio);
}

int gpio_toggle(struct gpio *gpio) {
    int fd;

    char path[] = "/sys/class/gpio/gpio00/value\n";
    path[20] = gpio->name[4], path[21] = gpio->name[5];

    if ((fd = open(path, O_WRONLY)) == -1) {
        printf("*debug Unable to open %s\n", path);
        return EXIT_FAILURE;
    }

    if ((write(fd, (gpio->value) ? "1" : "0", 1)) != 1) {
        printf("*debug Error writing to %s\n", path);
        return EXIT_FAILURE;
    }

    close(fd);

    return EXIT_SUCCESS;
}

int gpio_unexport(struct gpio *gpio) {
    int fd;

    if ((fd = open("/sys/class/gpio/unexport", O_WRONLY)) == -1) {
        printf("*debug Unable to open /sys/class/gpio/unexport\n");
        return EXIT_FAILURE;
    }

    if ((write(fd, (gpio->name) + 4, 2)) != 2) {
        printf("*debug Error writing to /sys/class/gpio/unexport\n");
        return EXIT_FAILURE;
    }

    close(fd);

    return EXIT_SUCCESS;
}

/*Utility functions*/
int set_terminal(struct termios new) {
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    printf("\033[?25l"); /*Escape sequence to hide cursor*/

    return EXIT_SUCCESS;
}

int reset_terminal(struct termios def) {
    tcsetattr(STDIN_FILENO, TCSANOW, &def);
    printf("\033[?25h"); /*Escape sequence to show cursor*/
    return EXIT_SUCCESS;
}

int display_menu(struct gpio *gpio, int index) {

    printf("  \033[35;7m  BCM    DIRECTION   STATE \033[0m\n");
    for (int i = 0; i < MAX_PINS; i++) {
        (i == index) ? printf("\033[36m> GPIO-\033[1;36;4m%s\033[0m", gpio[i].name + 4) : printf("  GPIO-%s",
                                                                                                 gpio[i].name + 4);
        (gpio[i].direction) ? printf("\033[37;1m  -- IN  --  \033[0m") : printf("\033[37;1m  -- OUT --  \033[0m");
        (gpio[i].value == 0) ? printf("\033[1;31m[ OFF ]\033[0m\n") : printf("\033[1;32m[ ON  ]\033[0m\n");
    }

    puts("\n\033[37;7mQ\033[0m Save and exit    "
         "\033[37;7mC\033[0m Cancel changes    "
         "\033[37;7mD\033[0m Direction    "
         "\033[37;7mENTER\033[0m Toggle    "
         "\033[37;7m↑↓ \033[0m Move cursor");

    return 0;
}

int create_save_file() {
    int fd;
    if ((fd = open("./gpio.cfg", O_CREAT | O_WRONLY)) == -1) {
        fprintf(stderr, "*debug Error creating save file\n");
        return fd;
    }

    char *content = "gpio02 0 0\n"
                    "gpio03 0 0\n"
                    "gpio04 0 0\n"
                    "gpio17 0 0\n"
                    "gpio27 0 0\n"
                    "gpio22 0 0\n"
                    "gpio10 0 0\n"
                    "gpio09 0 0\n"
                    "gpio11 0 0\n"
                    "gpio00 0 0\n"
                    "gpio05 0 0\n"
                    "gpio06 0 0\n"
                    "gpio13 0 0\n"
                    "gpio19 0 0\n"
                    "gpio26 0 0\n"
                    "gpio14 0 0\n"
                    "gpio15 0 0\n"
                    "gpio18 0 0\n"
                    "gpio23 0 0\n"
                    "gpio24 0 0\n"
                    "gpio25 0 0\n"
                    "gpio08 0 0\n"
                    "gpio07 0 0\n"
                    "gpio01 0 0\n"
                    "gpio12 0 0\n"
                    "gpio16 0 0\n"
                    "gpio20 0 0\n"
                    "gpio21 0 0\n";

    write(fd, content, FILE_SIZE);
    close(fd);
    return fd;
}

int help(void) {
    char *text = "Usage: igpio [OPTION]...\n"
                 "igpio is a simple program that provides text based interface for (un)exporting individual\n"
                 "gpio pins and changing their state.\n\n"
                 "\t-h, --help \tlists commands and additional information.\n"
                 "\t-s, --status \tdisplays the status of each pin.\n"
                 "\t-r, --restore \trestores the default settings (unexports every)."
                 "\n\n";
    printf("%s", text);
    return 0;
}

int status(void) {
    printf("TBD\n");
    return 0;
}

int unknown(char *str) {
    printf("unknown option: '%s'\n"
           "usage: gpio [--help | -h] [--status | -s]\n", str);
    return 0;
}
