#include "headers/rsa.h"
#include "headers/block.h"

int is_prime_naive (long p)
{
    if (p % 2 == 0)
    {
        return 0;
    }
    int i = 3;
    while (i < p)
    {
        if (p % i == 0) return 0;
        i++;
    }
    return 1;
}
//Complexite O(n)

long test()
{
    clock_t temps_initial;
    clock_t temps_final;
    double temps = 0;
    long p1 = 3200000;
    long p2;
    int i = 2;

    while (temps < 2)
    {
        p2 = p1;
        p1 = powl(p1,(long) i);
        
        temps_initial = clock();
        if (is_prime_naive(p1))
        temps_final = clock(); 

        temps = (temps_final - temps_initial)/CLOCKS_PER_SEC;
        printf("%lf\n", temps);
        i += 1;
    }
    return p2;
}
//29 a 30 bits


long modpow_naive (long a, long m, long n)
{
    long res = 1;
    for (int i = 0; i < m; i++)
    {
        res = (res * a) % n;
    }
    return res;
}
//Complexite O(m)


int modpow(long a, long m, long n)
{
    long res = 1;
    long i = m;
    while(i > 0)
    {
        if ((i % 2) == 1) 
            res = (res * a) % n;
        a = (a * a) % n;
        i = (long) (i / 2);
    }
    return (int) res;
}

void test2(long (*fct)(long, long, long), int (*fct2)(long, long, long), FILE *fd)
{
    srand(time(NULL));
	clock_t temps_initial ;
	clock_t temps_final ;
	double temps_cpu1, temps_cpu2;
    long aa = (((long)rand() * (long)(10000 - 10) / (long)RAND_MAX)) + 10;
    long mm = (((long)rand() * (long)(10000 - 10) / (long)RAND_MAX)) + 10;
    long nn = (((long)rand() * (long)(10000 - 10) / (long)RAND_MAX)) + 10;

    for (int i = 0; i < 50; i++ )
    {
        printf("\n __--NOUVEAU TEST--__ :\n");
        aa = aa + 1000000;
        mm = mm + 1000000;
        nn = nn + 1000000;
        printf("a : %ld\nm : %ld\nn : %ld\n",aa,mm,nn);

        temps_initial = clock();
	    printf("Résultat 1 = %ld\n",fct(aa,mm,nn));
	    temps_final = clock();
        

        temps_cpu1 = ((double) (temps_final - temps_initial)) / CLOCKS_PER_SEC;

        temps_initial = clock();
	    printf("Résultat 2 = %d\n",fct2(aa,mm,nn));
	    temps_final = clock();
    
	    temps_cpu2 = ((double) (temps_final - temps_initial)) / CLOCKS_PER_SEC;

        /*temps_initial = clock();
	    printf("Résultat 3 = %d\n",fct3(aa,mm,nn));
	    temps_final = clock();
        

        temps_cpu3 = ((double) (temps_final - temps_initial)) / CLOCKS_PER_SEC;*/

	    fprintf(fd, "%ld %ld %ld %f %f \n", aa, mm, nn, temps_cpu1, temps_cpu2);
        puts("");
    }
}

int witness(long a, long b, long d, long p)
{
    long x = modpow(a, d, p);
    if (x == 1)
        return 0;
    for (long i = 0; i < b; i++)
    {
        if(x == p - 1)
            return 0;
        x = modpow(x,2,p);
    }
    return 1;
}

long rand_long(long low, long up)
{
    return rand() % (up - low + 1) + low;
}

int is_prime_miller (long p, int k) 
{
    if (p == 2)
        return 1; 
    if (!(p & 1) || p <= 1) 
        return 0; //on verifie que p est impair et different de 1 return 0;

    //on determine b et d :
    long b = 0;
    long d = p - 1;
    while (!(d & 1))
    { //tant que d n’est pas impair
        d = d/2;
        b=b+1; 
    }
    // On genere k valeurs pour a, et on teste si c’est un temoin :
    long a;
    int i;
    for(i = 0;i<k;i++)
    {
        a = rand_long(2, p-1);
        if (witness(a,b,d,p))
            return 0;
    }
    return 1; 
}
/*
Pour tout entier p non premier quelconque, 3/4 valeurs entre 2 et p-1 sont
des temoins de Miller pour p. (p - 3) / 4 valeurs ne sont pas temoins de p.
*/

long random_prime_number (int low_size, int up_size, int k)
{
    if (low_size > up_size)
    {
        printf("Probleme de borne : borne inferieure > borne superieure.\nInversion des bornes.\n");
        int temp = low_size;
        low_size = up_size;
        up_size = temp;
    }

    long alea = rand_long(powl(2, low_size), powl(2, up_size + 1) - 1);
    
    int test = is_prime_miller(alea, k);

    //printf("Nombre aleatoire : %ld\nTest de Miller-Rabin : %d (0 si faux, sinon 1)\n\n", alea, test);

    if (test != 1)
    {
        return random_prime_number(low_size, up_size, k);
    }
    else
    {
        return alea;
    }
}

long extended_gcd (long s, long t, long *u, long *v)
{
    if (s == 0)
    {
        *u = 0;
        *v = 1;
        return t;
    }
    long uPrim , vPrim ;
    long gcd = extended_gcd (t % s, s, &uPrim , & vPrim );
    *u = vPrim - (t / s) * uPrim;
    *v = uPrim;
    return gcd;
}

void generate_key_values (long p, long q, long* n, long* s, long* u)
{
    *n = (p * q);
    long t = (p - 1) * (q - 1);
    *s = rand_long(0, t);
    long v;
    long gcd = extended_gcd(*s, t, u, &v);
    if (gcd != 1) generate_key_values(p, q, n, s, u);
}

long* encrypt (char* chaine, long s, long n)
{
    int i = 0;
    long* res = (long *) malloc(strlen(chaine) * sizeof(long));
    while (chaine[i] != '\0')
    {
        res[i] = (long) modpow(chaine[i], s, n);
        i++;
    }
    return res;
}

char* decrypt (long* crypted, int size, long u, long n)
{
    int i = 0;
    char* res = (char *) malloc(size + 1 * sizeof(char));
    while (i < size)
    {
        res[i] = (char) modpow(crypted[i], u, n);
        i++;
    }
    res[i] = '\0';
    return res;
}