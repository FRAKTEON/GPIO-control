#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "gpio.h"

int main(int argc, char **argv){

    GPIO io;
    const char *gpio_path = "/sys/class/gpio/gpio";
    struct stat s;

    if(input(argc, argv))
        return EXIT_FAILURE; 

    if(!(stat(gpio_path, &s) == 0 && S_ISDIR(s.st_mode)))
    {
        if(export_pin(&io))
            return EXIT_FAILURE;

        usleep(1500000);
    }

    if(direction(&io))
        return EXIT_FAILURE;

    if(toggle(&io))
       return EXIT_FAILURE; 
    
    if(io.value[0] == '0')
        unexport(&io);
        

    return EXIT_SUCCESS;    
}
