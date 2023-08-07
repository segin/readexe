#include <stdio.h>

int main(int argc, char *argv[]) { 
    int i;
    printf("My arguments count is %d elements.\nMy arguments vector contains the following values:\n", argc);
    for(i=0;i<argc;i++) {
        printf(" [%d]: \"%s\"\n", i, argv[i]);
    }
    return 0;
}