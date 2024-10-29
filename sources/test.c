#include <stdio.h>
#include <stdlib.h>

#include "headers/rsa.h"

void testtemps()
{
    clock_t temps_i;
    clock_t temps_f;
    double temps = 0;
    long p1 = 324154;
    long p2;
    int i = 2;
    printf("%ld\n", p1);
    p1 = powl(p1, (long) i);
    printf("%ld\n", p1);

    
    while (temps < 0.02)
    {
        p2 = p1;
        p1 = powl(p1, (long) i);
        printf("p1 = %ld", p1);
        break;

        temps_i = clock();
        if (is_prime_naive(p1)) p1 += 1;
        temps_f = clock();

        temps = (double) (temps_f - temps_i) / CLOCKS_PER_SEC;
        printf("%.3lf s\n", temps);
        i+= 1;
    }
    printf("prime le plus haut : %ld\n", p2);
    
}

int main()
{ 
    testtemps();
    return 0;
}