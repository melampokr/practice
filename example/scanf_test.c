#include <stdio.h>



int main(void)
{
    int h, c, t, l;
    char test[100];

    scanf("%d:%d:%d:%d %s", &h, &c, &t, &l, test);

    printf("%d:%d:%d:%d\n", h, c, t, l);
    printf("%s\n", test);
}
