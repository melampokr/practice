#include <stdio.h>

#define  BUFF_SIZE   1024

int main( void)
{
    char  buff[BUFF_SIZE];
    FILE *fp;
    char cmd[BUFF_SIZE];

    sprintf(cmd, "lsscsi | grep %s | sed 's/.//' |awk '{split($1,arr,\":\"); printf(\"%%s\", arr[1]);}'", "/dev/sdb");
    printf("%s\n", cmd);

    fp = popen(cmd, "r");
    if ( NULL == fp)
    {
        perror( "popen() 실패");
        return -1;
    }

    while( fgets( buff, BUFF_SIZE, fp) )
        printf( "%s", buff);

    pclose( fp);

    return 0;
}
