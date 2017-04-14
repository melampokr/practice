#include <errno.h>
#include <stdio.h>
#include <fcntl.h> 
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

int set_interface_attribs (int fd, int speed, int parity)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        perror("tcgetattr");
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
    // enable reading
    tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
    tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        perror("tcsetattr");
        return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block)
{
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        perror ("tggetattr");
        return;
    }

    tty.c_cc[VMIN]  = should_block ? 1 : 0;
    tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
        perror ("tcsetattr");
}

int main(void)
{
    char *portname = "/dev/ttyUSB0";
    int fd;
    char relay;
    char reply;
    char input[100];
    int size;

    fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
    {
        printf ("error %d opening %s: %s", errno, portname, strerror (errno));
        return 0;
    }

    set_interface_attribs (fd, 9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (fd, 0);  // set no blocking
    printf("serial Initialize\n");
    sleep(1);

    printf("relay ientify : ");
    relay = 0x50;  //relay identify command
    write (fd, &relay, 1);
    size = read (fd, &reply, 1);
    if (size < 0)
        printf("read fail\n");
    else
        printf("%s(0x%02X)\n",
                (reply == 0xAB) ? "ICSE012A" :
                (reply == 0xAD) ? "ICSE013A" :
                (reply == 0xAC) ? "ICSE014A" : "INVALID ID" , reply);

    sleep(1);

    printf("relay Initialize\n");
    relay = 0x51;  //relay initialize command
    write (fd, &relay, 1);
    sleep(1);

    while (1)
    {
        printf("input : ");
        gets(input);

        if(input[0] == 'q')
            break;

        relay = (char) atoi(input);

        printf("input 0x%02x\n", relay);

        write (fd, &relay, 1);
        usleep ((7 + 25) * 100);
    }

    relay = 0;
    write (fd, &relay, 1); //All relay OFF

    printf("bye~\n");
}
