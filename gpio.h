#ifndef GPIO_CONTROL_GPIO_H
#define GPIO_CONTROL_GPIO_H
#endif
#include "termios.h"
#define MAX_PINS 28

struct gpio {
    char name[8];
    int direction,
    value;
};

enum keys {
    ENTER = 10,
    DIRECTION = 'd',
    QUIT = 'q',
    CANCEL = 'c',
    UP_ARROW = 65,
    DOWN_ARROW = 66
};

/*Main program functions*/
int init_struct(struct gpio gpio[]);
int interface(struct gpio *gpio);
int cfg_save(struct gpio *gpio);
int cfg_cancel(struct gpio *gpio);
int cfg_restore(struct gpio *gpio);

/*System gpio functions*/
int gpio_export(struct gpio *gpio);
int gpio_direction(struct gpio *gpio);
int gpio_toggle(struct gpio *gpio);
int gpio_unexport(struct gpio *gpio);

/*Utility functions*/
int set_terminal(struct termios new);
int reset_terminal(struct termios def);
int display_menu(struct gpio *gpio, int index);
int create_save_file(void);
int help(void);
int status(void);
int unknown(char *str);