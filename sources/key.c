#include "headers/key.h"

void init_key(Key* key, long val, long n)
{
    key->val = val;
    key->n = n;
}

//                      (s,n)       (u,n)
void init_pair_keys(Key* pKey, Key* sKey, long low_size, long up_size)
{
    long s, u, t;
    long p = random_prime_number(low_size, up_size, 5000);
    long q = random_prime_number(low_size, up_size, 5000);
    while(p == q)
    {
        q = random_prime_number(low_size, up_size, 5000);
    }
    long n = p * q;
    generate_key_values(p, q, &n, &s, &u);
    if (u < 0)
    {
        t = (p - 1) * (q - 1);
        u = u +  t; //on aura toujours s * u mod t = 1
    }
    init_key(pKey, s, n);
    init_key(sKey, u, n);  
}

char* key_to_str(Key* key)
{
    char* s = (char*) malloc(256*sizeof(char));
    if (s == NULL)
    {
        printf("Erreur d allocation (key_to_str)\n");
    }
    sprintf(s, "(%lx,%lx)", key->val, key->n);
    return s;
}

Key* str_to_key(char* str)
{
    long val;
    long n;
    sscanf(str, "(%lx,%lx)", &val, &n);
    Key* key = (Key*) malloc(sizeof(Key));
    if (key == NULL)
    {
        printf("Erreur d allocation (str_to_key)\n");
        return NULL;
    }
    key->val = val;
    key->n = n;

    return key;
}