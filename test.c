#include <stdio.h>

int main() {
    int a;
    float b;
    float c;
    a=1;
    b=2.5;
    c = a + b;
    {
        int a = 10;
        {
            int a = 20;
            printf("%d\n", a);
        }
        int b = 2;
    }
    printf("%f\n", c);
    return 0;
}