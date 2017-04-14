#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(void)
{
    //Message to be sent
    char message[] = {0xFF, 0x01, 0x01};

    //change baud
    system("stty -F /dev/ttyUSB0 9600 cs8 -cstopb"); 

    int fd;

    fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_SYNC);

    write(fd, message, 3);

    sleep(1);
    message[3] = 0x00;

    write(fd, message, 3);

}
