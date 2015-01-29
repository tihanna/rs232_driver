#include <string.h>
#include <stdio.h>

int main()
{
    char buff[17];
    int n;

    FILE *f = fopen("/dev/ttyUSB0", "wb+");

    do {
        printf("> ");
        n = scanf("%s", buff);
        memset(buff + strlen(buff), '_', 15 - strlen(buff));

        //puts(buff);

        fwrite( buff, 1, 16, f);

    } while (n != 0);

    fclose(f);
    return 0;
}
