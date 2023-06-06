#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct gpio {
    char *pin;
    int fd;
}GPIO;

int input(int argc, char **argv);
int help();
int status();
int config();
int export_pin(GPIO *p_io);
int direction(GPIO *p_io);
int toggle(GPIO *p_io);
int unexport(GPIO *p_io);