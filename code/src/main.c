#include <stdio.h>
#include "test_i2c.h"

int main (void) {
    int a = 0;
    a++;
    a++;
    a++;
    printf("Hello world\n");
    i2c_test();
    return 0;
}