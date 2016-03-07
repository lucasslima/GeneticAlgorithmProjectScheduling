/*
 * ga.c
 * example of a Genetic Algorithm for the knapsack problem
 * Copyright (C) 2016 H.G. Santos <haroldo.santos@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <algorithm>
#include "instance.h"
#include <limits>
#include <vector>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

// computes a random number [0,1)
#define DBL_RND (((double)rand()) / (((double)RAND_MAX)+1.0))


typedef struct
{
    /* incidence vector indicating if each element will or not be chosen */
    vector<float> gene;
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

void allocate_memory_population(Instance &inst);

/* fills initial population */
void populate(Instance &inst);

/* binary tournament */
void selectTwoIndividuals( Individual *pop, int *i1, int *i2 );

/* generates new population by uniform crossover */
void applyCrossover(Instance & inst);

void applyMutation(Instance & inst);

void evaluatePopulation(Instance & inst, Individual *population );

int main( int argc, const char **argv )
{
    if (argc<2)
    {
        fprintf( stderr, "enter instance name.\n");
        exit(EXIT_FAILURE);
    }



    /* instance with problem data */
    Instance inst(argv[1]);

    printf("starting GA for project scheduling problem with %d jobs.\n", inst.i->mNumberOfJobs );

    allocate_memory_population(inst);

    for (int i = inst.i->mNumberOfJobs; i < inst.i->mNumberOfJobs * 2; i++)

    populate(inst);

    for (int  gen=1 ; (gen<+generations) ; ++gen )
    {
        evaluatePopulation(inst, currGen);

        /* generates new population by crossover */
        applyCrossover(inst);

        /* mutates */
        applyMutation(inst);

        printf("generation %06d, fitness: (worst, average, best): %14.0f %14.0f %14.0f\n", gen+1, worstFit, avFit, bestFit );
    
        /* swaping populations */
        Individual *temp = currGen;
        currGen = nextGen;
        nextGen = temp;
    }
}

void applyCrossover(Instance &inst)
{
    int i;
    for ( i=0 ; (i<popSize) ; ++i )
    {
        int i1, i2;
        selectTwoIndividuals( currGen, &i1, &i2 );

        int j;
        for ( j=0 ; (j<inst.i->mNumberOfJobs * 2) ; ++j )
                nextGen->gene[j] = (DBL_RND > 0.7) ? currGen->gene[j] : currGen->gene[j];
    }
}

void allocate_memory_population(Instance &inst)
{
    currGen = new Individual[popSize];
    int i;
    for ( i=0 ; (i<popSize) ; ++i ) {
        currGen->gene.resize(inst.i->mNumberOfJobs * 2);
    }

    nextGen = new Individual[popSize];
    for ( i=0 ; (i<popSize) ; ++i ) {
        nextGen->gene.resize(inst.i->mNumberOfJobs * 2);
    }
}
//TODO adicionar prioridades aleatórias para cada tarefa
void populate(Instance &inst)
{
    for (int i=0 ; (i<popSize) ; ++i )
    {
        for (int j=0 ; j<inst.i->mNumberOfJobs ; ++j )
            currGen->gene[j] = DBL_RND;
        for (int j = inst.i->mNumberOfJobs; j < inst.i->mNumberOfJobs * 2; j++)
            currGen->gene[j] = DBL_RND;
    }
}

double evaluateFitness( Instance & inst, const Individual *ind );

void evaluatePopulation(Instance & inst, Individual *population )
{
    bestFit   = -DBL_MAX;
    worstFit  = DBL_MAX;
    avFit     = 0.0;
    int i;
    for ( i=0 ; (i<popSize) ; ++i )
    {
        population[i].fitness = evaluateFitness(inst, &population[i] );
        bestFit = MAX( bestFit, population[i].fitness );
        worstFit = MIN( worstFit, population[i].fitness );
        avFit += population[i].fitness;
    }
    avFit /= ((double)popSize);
}

//TODO the function must return the total makespan
//Uma pra cada membro da população, uma tabela deve ser calculada
//Através do metodo de geração de schedules do artigo.

double evaluateFitness(Instance &inst, const Individual *ind )
{
    double f = 0;

    vector<int> active;
    vector<int> e;
    vector<int> scheduled;
    vector<int> gama; // Γo from the article
    vector< vector<int> > RD;
    RD.resize(inst.i->desiredDueDate);
    for (auto &t : RD){
        t.resize(inst.mRenewableResourceAvailability.size());
        t.insert(t.end(), inst.mRenewableResourceAvailability.begin(),inst.mRenewableResourceAvailability.end());
    }
    int g = 0;
    active.push_back(0);
    gama.push_back(0);
    scheduled.push_back(0);
    while (scheduled.size() < inst.i->mNumberOfJobs){
        for (int j = 0; j < inst.i->mNumberOfJobs; j++){
            if ( scheduled.size() != 1) {
                for (int i = 0; i < inst.i->precedences[j].size(); i++) {
                    if (inst.i->f[i] <= g + ind->gene[inst.i->mNumberOfJobs + i])
                        e.push_back(j);
                }
            }else{
                vector<int> diference;
                for (int s = 0; s < scheduled.size(); s++){
                    if (std::find(scheduled.begin(),scheduled.end(),s) == scheduled.end()){
                        for (int i = 0; i < inst.i->precedences[j].size(); i++) {
                            if (inst.i->f[i] <= g + ind->gene[inst.i->mNumberOfJobs + i])
                                diference.push_back(j);
                        }
                    }

                }
                e.insert(e.end(),diference.begin(), diference.end());
            }
        }
        auto geneIt = std::max_element(ind->gene.begin(), ind->gene.end() );
        int j = std::distance(ind->gene.begin(), geneIt);
        //Compute earliest finish (precedence only)
        vector<int> precedencesEarliestFinish;
        for (auto &p : inst.i->precedences[j])
            precedencesEarliestFinish.push_back(inst.i->f[p]);
        inst.i->earliestJobCompletion[j] = *std::max_element(precedencesEarliestFinish.begin(), precedencesEarliestFinish.end()) + inst.i->jobDuration[j];

        vector<int> possibletimes;
        for (int t = inst.i->earliestJobCompletion[j] - inst.i->jobDuration[j]; t < inst.i->desiredDueDate;t++){

            if (std::find(gama.begin(),gama.end(),t) != gama.end() ){
                for (int td = t; td <= t + inst.i->jobDuration[j]; td++){
                    bool possible = true;
                    for (int r = 0; r < inst.mRenewableResourceAvailability.size(); r++){
                        if (inst.i->resourceRequirement[j][r] > RD[td][r]) {
                            possible = false;
                            break;
                        }
                    }
                    possibletimes.push_back(td);
                }
            }
        }
        inst.i->f[j] = *std::min_element(possibletimes.begin(),possibletimes.end());
        scheduled.push_back(j);
        gama.push_back(inst.i->f[j]);
        g++;
        for (int i = 0; i < inst.i->mNumberOfJobs; i++){
            if (inst.i->f[i] - inst.i->jobDuration[i] <= g and g < inst.i->f[i]){
                active.push_back(i);
            }
        }
        for (int i = inst.i->f[j] - inst.i->jobDuration[j]; i < inst.i->f[i]; i++){
            for (int r = 0; r < inst.mRenewableResourceAvailability.size(); r++){
                for (int a = 0 ; a < active.size(); a++)
                    RD[i][r] = inst.mRenewableResourceAvailability[r] - inst.i->resourceRequirement[a][r];
            }
        }
    }

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

void applyMutation(Instance &inst)
{
    int i,j;
    for ( i=0 ; (i<popSize) ; ++i )
        for ( j=0 ; (j<inst.i->mNumberOfJobs) ; ++j )
            if ( DBL_RND <= mtRate )
                currGen[i].gene[j] = DBL_RND;
}

