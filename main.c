#include <stdlib.h>
#include <string.h>
#include "gpio.h"
#include "stdio.h"

int main(int argc, char **argv) {
    struct gpio gpio[MAX_PINS];

    init_struct(gpio);

    if (argc == 1) {
        interface(gpio);
    } else {
        for (int i = 1; i < argc; i++) {
            if (strcasecmp(argv[i], "-h") == 0 || strcasecmp(argv[i], "--help") == 0)
                help();
            else if (strcasecmp(argv[i], "-s") == 0 || strcasecmp(argv[i], "--status") == 0)
                status();
            else if (strcasecmp(argv[i], "-r") == 0 || strcasecmp(argv[i], "--restore") == 0)
                cfg_restore(gpio);
            else {
                unknown(argv[i]);
                return 1;
            }
        }
    }

    return EXIT_SUCCESS;
}
