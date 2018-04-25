#include <stdio.h>

void convert(unsigned long int* x) {
    unsigned long int cur = 0;
    for(int i = 0; i < 3; ++i) {
        if ((1 << i) & *x)
            cur |= (1 << i);
    }
    for(int i = 3; i < 14; ++i) {
        if ((1 << i) & *x)
            cur |= (1 << i + 17);
    }
    for(int i = 14; i < 20; ++i) {
        if ((1 << i) & *x)
            cur |= (1 << i);
    }
    for(int i = 20; i < 31; ++i) {
        if ((1 << i) & *x)
            cur |= (1 << i - 17);
    }
    for(int i = 31; i < 32; ++i) {
        if ((1 << i) & *x)
            cur |= (1 << i);
    }
    *x = cur;
}

int main() {
    unsigned long int a;
    scanf("%ld", &a);
    convert(&a);
    printf("%ld", a);
    return 0;
}