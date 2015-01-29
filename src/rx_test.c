#include <stdio.h>
#include <unistd.h>

int main()
{
    char buff[17];
    buff[16] = 0;

    int recv_n = 0;
    FILE *f = fopen("/dev/rs232", "wb+");

    while(recv_n = fread(buff, 1, 16, f))
    {
        puts("something");
        puts(buff);
        sleep(0.1);
    }

    fclose(f);

    return 0;
}
