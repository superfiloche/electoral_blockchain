#include <stdlib.h>
#include "rsa.h"
#include "key.h"

#ifndef SIGNATURE_H
#define SIGNATURE_H

typedef struct _signature{
    long* content;
    int size;
}Signature;

typedef struct _protected{
    Key* pubk;
    char* mess;
    Signature* sgn;
}Protected;

Signature* init_signature(long* content, int size);
Signature* sign(char* mess, Key* key);
char *signature_to_str(Signature * sgn);
Signature *str_to_signature(char *str);
Protected* init_protected(Key* pKey, char* mess, Signature* sgn);
int verify(Protected* pr);
char* protected_to_str(Protected* pro);
Protected* str_to_protected(char* stop);
void freeprotected(Protected* pr);

#endif