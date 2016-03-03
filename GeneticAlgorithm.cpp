/*
 * ga.c
 * example of a Genetic Algorithm for the knapsack problem
 * Copyright (C) 2016 H.G. Santos <haroldo.santos@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "instance.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// computes a random number [0,1)
#define DBL_RND (((double)rand()) / (((double)RAND_MAX)+1.0))

/* instance with problem data */
Instance *inst;

//Todo Change individual
typedef struct
{
    /* incidence vector indicating if each element will or not be chosen */
    char *iv;
    double fitness;
} Individual;

/* parameters of the algorithm */
int popSize     =  10000;  /* population size */
float mtRate    =   0.01;  /* mutation rate */
int generations =    250;  /* number of generations */

/* statistics about the population, best, worst and average */
double bestFit;
double worstFit;
double avFit;

/* individuals are stored as binary vectors   
 * population of solutions, space for current and next generation  */

Individual *currGen;
Individual *nextGen;

void allocate_memory_population();

/* fills initial population */
void populate();

/* binary tournament */
void selectTwoIndividuals( Individual *pop, int *i1, int *i2 );

/* generates new population by uniform crossover */
void applyCrossover();

void applyMutation();

void evaluatePopulation( Individual *population );

int main( int argc, const char **argv )
{
    if (argc<2)
    {
        fprintf( stderr, "enter instance name.\n");
        exit(EXIT_FAILURE);
    }

    inst = readInstance( argv[1] );
    printf("starting GA for knapsack problem with %d items.\n", inst->n );

    allocate_memory_population();

    populate();
    

    for (int  gen=1 ; (gen<+generations) ; ++gen )
    {
        evaluatePopulation(currGen);

        /* generates new population by crossover */
        applyCrossover();

        /* mutates */
        applyMutation();

        printf("generation %06d, fitness: (worst, average, best): %14.0f %14.0f %14.0f\n", gen+1, worstFit, avFit, bestFit );
    
        /* swaping populations */
        Individual *temp = currGen;
        currGen = nextGen;
        nextGen = temp;
    }
}

void applyCrossover()
{
    int i;
    for ( i=0 ; (i<popSize) ; ++i )
    {
        int i1, i2;
        selectTwoIndividuals( currGen, &i1, &i2 );

        /* creating new individual by uniform crossover */
        int j;
        for ( j=0 ; (j<inst->n) ; ++j )
            nextGen[i].iv[j] =  (rand()%2==0) ? currGen[i1].iv[j] : currGen[i2].iv[j];
    }
}

void allocate_memory_population()
{
    currGen = new Individual[popSize];
    int i;
    for ( i=0 ; (i<popSize) ; ++i )
        currGen[i].iv = new char[inst->n];

    nextGen = new Individual[popSize];
    for ( i=0 ; (i<popSize) ; ++i )
        nextGen[i].iv = new char[inst->n];
}

void populate()
{
    int i;
    for ( i=0 ; (i<popSize) ; ++i )
    {
        int j;
        for ( j=0 ; (j<inst->n) ; ++j )
            currGen[i].iv[j] = rand()%2;
    }
}

double evaluateFitness( const Individual *ind );

void evaluatePopulation( Individual *population )
{
    bestFit   = -DBL_MAX;
    worstFit  = DBL_MAX;
    avFit     = 0.0;
    int i;
    for ( i=0 ; (i<popSize) ; ++i )
    {
        population[i].fitness = evaluateFitness( &population[i] );
        bestFit = MAX( bestFit, population[i].fitness );
        worstFit = MIN( worstFit, population[i].fitness );
        avFit += population[i].fitness;
    }
    avFit /= ((double)popSize);
}

//TODO  change fitness
double evaluateFitness( const Individual *ind )
{
    double f = 0;

    double totalWeight = 0.0;
    int i;
    for ( i=0 ; (i<inst->n) ; ++i )
    {
        totalWeight += ind->iv[i]*inst->weight[i];
        f += ind->iv[i]*inst->profit[i];  /* profit */
    }

    const double excess = MAX( totalWeight - inst->capacity, 0 );
    
    /* penalty for excess */
    f -= excess * 10000.0;

    return f;
}

/* selects two individuals, each one is selected by binary tournament */
void selectTwoIndividuals( Individual *pop, int *i1, int *i2 )
{
    {
        int s1 = rand()%popSize;
        int s2 = rand()%popSize;

        if (pop[s1].fitness > pop[s2].fitness)
            *i1 = s1;
        else
            *i1 = s2;
    }

    do
    {
        int s1 = rand()%popSize;
        int s2 = rand()%popSize;

        if (pop[s1].fitness > pop[s2].fitness)
            *i2 = s1;
        else
            *i2 = s2;
    
    } while (*i2==*i1);
}

void applyMutation()
{
    int i,j;
    for ( i=0 ; (i<popSize) ; ++i )
        for ( j=0 ; (j<inst->n) ; ++j )
            if ( DBL_RND <= mtRate )
                currGen[i].iv[j] = rand()%2;
}

