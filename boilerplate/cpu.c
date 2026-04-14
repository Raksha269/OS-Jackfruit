#include <stdio.h>

int main() {
    long long i = 0;
    while (1) {
        i++;
        if (i % 100000000 == 0) {
            printf("CPU running: %lld\n",i);
        }
    }
    return 0;
}
