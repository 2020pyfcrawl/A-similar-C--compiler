#include<stdio.h>

//__int16_t printi(__int16_t X);

__int16_t printi(__int16_t X){
    fprintf(stderr, "%d\n", (__int16_t)X);
    return 0;
}
//int main(){}