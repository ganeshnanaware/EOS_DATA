#include "stdio.h"
#include "stdlib.h"


static int check_global = 4;
int uninitialized;
int zero_global = 0;



int main()
{
    int *mallocvari;
    mallocvari = (int*)malloc(sizeof(int));
    
    if(mallocvari == NULL)
    {
        printf("memory not allocated\n");
        exit(1);
    }

    int localvariable = 6;

    printf("Adress of global initialized : %p\n", &check_global);
    printf("Adress of unitialized global variable : %p\n", &uninitialized);
    printf("Adress of zero global : %p\n", &zero_global);
    printf("Adress of local variabl : %p\n", &localvariable);
    printf("Adress of malloc head section : %p\n", mallocvari);
    printf("Adress of code section : %p\n", main);

    printf("Printing values\n");
    printf("global variable : %d\n", check_global);
    printf("uninitialized global variable : %d\n", uninitialized);
    printf("zero global variable : %d\n", zero_global);
    printf("local variable : %d\n", localvariable);
    printf("Malloc variable : %d\n", *mallocvari);

    while(1);
    return 0;

}