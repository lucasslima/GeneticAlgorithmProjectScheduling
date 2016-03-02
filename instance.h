#ifndef INSTANCE_H
#define INSTANCE_H

typedef struct
{
   int n;        /* nr. de items         */
   int *weight;  /* weight of each item  */
   int *profit;  /* profit of each item  */
   int capacity; /* capacity of knapsack */
} Instance;

/* cria um objeto instância e preenche com dados do arquivo */
Instance *readInstance( const char *fileName ); 

#endif

