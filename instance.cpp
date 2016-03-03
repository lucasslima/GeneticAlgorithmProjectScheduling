#include "instance.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

Instance *readInstance( const char *fileName )
{
   Instance *inst = new Instance();
   
   FILE *f = fopen( fileName, "r" );
   if ( f ==NULL )
   {
      fprintf( stderr, "fileName %s not found.\n", fileName );
      abort();
   }
   
   fscanf( f, "%d", &(inst->n) );
   fscanf( f, "%d", &(inst->capacity) );

   inst->weight = (int*) malloc( sizeof(int)*inst->n );
   inst->profit = (int*) malloc( sizeof(int)*inst->n );

   int i;
   for ( i=0; (i<inst->n) ; i++ ) 
      fscanf( f, "%d", &(inst->weight[i]) );
   for ( i=0; (i<inst->n) ; i++ ) 
      fscanf( f, "%d", &(inst->profit[i]) );

   fclose( f );
  
   return inst;
}
