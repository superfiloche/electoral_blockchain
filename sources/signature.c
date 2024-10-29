#include "headers/signature.h"

Signature* init_signature(long* content, int size)
{
    Signature* ns = (Signature*) malloc(sizeof(Signature));
    if (ns == NULL)
    {
        printf("Erreur d allocation (init_signature)\n");
        return NULL;
    }
    ns->content = content;
    ns->size    = size;
    return ns;
}

Signature* sign(char* mess, Key* key)
{
    return (init_signature(encrypt(mess, key->val, key->n), strlen(mess)));
}

char *signature_to_str(Signature * sgn){
    char* result = malloc (10*sgn->size*sizeof(char));
    result[0]= '#' ;
    int pos = 1;
    char buffer[156];
    for(int i=0; i<sgn->size; i++){
        sprintf(buffer ,"%lx" , sgn->content[i]);
        for(int j = 0; j < (int) strlen(buffer) ; j++){
            result[pos] = buffer[j];
            pos = pos +1;
        }
        result[pos] = '#' ;
        pos = pos+1;
    }
    result[pos] = '\0' ;
    result = realloc(result , (pos+1)*sizeof(char)) ;
    return result;
}

Signature *str_to_signature(char *str){
    int len = strlen(str);
    long *content = (long*)malloc(sizeof(long)*len) ;
    int num = 0;
    char buffer [256];
    int pos = 0;
    for(int i=0; i<len ; i++){
        if (str[i] != '#'){
            buffer [pos] = str[i];
            pos = pos+1;
        }else{
            if(pos != 0){
                buffer[pos] = '\0';
                sscanf(buffer , "%lx" , &(content[num])) ;
                num = num + 1;
                pos = 0;
            }
        }
    }
    content = realloc(content , num*sizeof(long));
    return init_signature(content , num);
}

Protected* init_protected(Key* pKey, char* mess, Signature* sgn)
{
    Protected* npro = (Protected*) malloc(sizeof(Protected));
    if(npro == NULL)
    {
        printf("Erreur d allocation (init_protected)\n");
        return NULL;
    }

    npro->mess = strdup(mess);
    npro->pubk = pKey;
    npro->sgn  = sgn;

    return npro;
}

int verify(Protected* pr)
{
    char* s = decrypt(pr->sgn->content, strlen(pr->mess), pr->pubk->val, pr->pubk->n);
    int i = (int) strcmp(pr->mess, s);
    free(s);
    if (i != 0) return 0;
    return 1;
}

char* protected_to_str(Protected* pro)
{
    char* ptos = (char*) malloc(256 * sizeof(char));
    if(ptos == NULL)
    {
        printf("Erreur d allocation (protected_to_str)\n");
        return NULL;
    }
    char* temp = signature_to_str(pro->sgn);
    char* temp2 = key_to_str(pro->pubk);
    sprintf(ptos, "%s , %s , %s\n", temp2, pro->mess, temp);
    free(temp);
    free(temp2);
    return ptos;
}


Protected* str_to_protected(char* stop)
{
    char Key_str[256];
    char message[256];
    char sgn_str[256];

    if (sscanf(stop, "%s , %s , %s", Key_str, message, sgn_str) != 3)
    {
        printf("Mauvais nombre d'arguments (str_to_protected)\n");
        return NULL;
    }
    Key *pKey = str_to_key(Key_str);
    Signature *sign = str_to_signature(sgn_str);
    Protected* pr = init_protected(pKey, message, sign);
    return pr;
}

void freeprotected(Protected* pr)
{
    free(pr->mess);
    free(pr->pubk);
    free(pr->sgn->content);
    free(pr->sgn);
    free(pr);
}