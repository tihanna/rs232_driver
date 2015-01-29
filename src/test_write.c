#include <stdio.h>

int main()
{
    FILE *f = fopen("/dev/rs232_mod", "w");
    fputs("hello", f);
    fclose(f);
    return 0;
}
