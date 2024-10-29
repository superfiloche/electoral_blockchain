#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

#ifndef RSA_H
#define RSA_H

int is_prime_naive (long p);
long test();
long modpow_naive (long a, long m, long n);
int modpow(long a, long m, long n);
void test2(long (*fct)(long, long, long), int (*fct2)(long, long, long), FILE *fd);
int witness(long a, long b, long d, long p);
long rand_long(long low, long up);
int is_prime_miller (long p, int k);
long random_prime_number (int low_size, int up_size, int k);
long extended_gcd (long s, long t, long *u, long *v);
void generate_key_values (long p, long q, long* n, long* s, long* u);
long* encrypt (char* chaine, long s, long n);
char* decrypt (long* crypted, int size, long u, long n);

#endif