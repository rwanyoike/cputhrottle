#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc_, char **argv_)
{
    int x = 1;

    while(x < 255) 
    {
        //	fprintf(stderr, "Hello.\n");
        x++;
        if(255 == x)
        {
            x = 0;
        }
    }

    return 0;
}
