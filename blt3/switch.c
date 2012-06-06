#include <stdio.h>

int main(int argc, char** argv)
{
    switch(argc) {
        case 0:
            printf("0\n");
            break;
        case 1:
            printf("one\n");
            break;
        case 2:
            printf("second\n");
            break;
        case 3:
            printf("third\n");
            break;
        case 4:
            printf("four\n");
            break;
        case 5:
            printf("five\n");
            break;
    }
}
