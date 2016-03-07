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
#include <string>
#include <limits>
#include <vector>
#include <set>

using namespace std;
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// computes a random number [0,1)
#define DBL_RND (((double)rand()) / (((double)RAND_MAX) + 1.0))

typedef struct {
  /* incidence vector indicating if each element will or not be chosen */
  vector<float> gene;
  int fitness;
} Individual;

/* parameters of the algorithm */
int popSize = 10000;   /* population size */
float mtRate = 0.01;   /* mutation rate */
int generations = 250; /* number of generations */

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
void selectTwoIndividuals(Individual *pop, int *i1, int *i2);

/* generates new population by uniform crossover */
void applyCrossover(Instance &inst);

void applyMutation(Instance &inst);

void evaluatePopulation(Instance &inst, Individual *population);

int main(int argc, const char **argv) {
  if (argc < 2) {
    fprintf(stderr, "enter instance name.\n");
    exit(EXIT_FAILURE);
  }

  /* instance with problem data */

  Instance inst(argv[1]);

  printf("starting GA for project scheduling problem with %d jobs.\n",
         inst.i->mNumberOfJobs);

  allocate_memory_population(inst);

  for (int i = inst.i->mNumberOfJobs; i < inst.i->mNumberOfJobs * 2; i++)

    populate(inst);

  for (int gen = 1; (gen < +generations); ++gen) {
    evaluatePopulation(inst, currGen);

    /* generates new population by crossover */
    applyCrossover(inst);

    /* mutates */
    applyMutation(inst);

    printf("generation %06d, fitness: (worst, average, best): %14.0f %14.0f "
           "%14.0f\n",
           gen + 1, worstFit, avFit, bestFit);

    /* swaping populations */
    Individual *temp = currGen;
    currGen = nextGen;
    nextGen = temp;
  }
}

void applyCrossover(Instance &inst) {
  int i;
  for (i = 0; (i < popSize); ++i) {
    int i1, i2;
    selectTwoIndividuals(currGen, &i1, &i2);

    int j;
    for (j = 0; (j < inst.i->mNumberOfJobs * 2); ++j)
      nextGen[i].gene[j] =
          (DBL_RND > 0.7) ? currGen->gene[j] : currGen->gene[j];
  }
}

void allocate_memory_population(Instance &inst) {
  currGen = new Individual[popSize];
  int i;
  for (i = 0; (i < popSize); ++i) {
    currGen[i].gene.resize(inst.i->mNumberOfJobs * 2);
  }

  nextGen = new Individual[popSize];
  for (i = 0; (i < popSize); ++i) {
    nextGen[i].gene.resize(inst.i->mNumberOfJobs * 2);
  }
}
// TODO adicionar prioridades aleatórias para cada tarefa
void populate(Instance &inst) {
  for (int i = 0; (i < popSize); ++i) {
    for (int j = 0; j < inst.i->mNumberOfJobs; ++j)
      currGen[i].gene[j] = DBL_RND;
    for (int j = inst.i->mNumberOfJobs; j < inst.i->mNumberOfJobs * 2; j++)
      currGen[i].gene[j] = DBL_RND * 1.5 * inst.i->desiredDueDate;
  }
}

int evaluateFitness(const Instance &inst, const Individual *ind);

void evaluatePopulation(Instance &inst, Individual *population) {
  bestFit = -DBL_MAX;
  worstFit = DBL_MAX;
  avFit = 0.0;
  int i;
  for (i = 0; (i < popSize); ++i) {
    population[i].fitness = evaluateFitness(inst, &population[i]);
    bestFit = MAX(bestFit, population[i].fitness);
    worstFit = MIN(worstFit, population[i].fitness);
    avFit += population[i].fitness;
  }
  avFit /= ((double)popSize);
}

// TODO the function must return the total makespan
// Uma pra cada membro da população, uma tabela deve ser calculada
// Através do metodo de geração de schedules do artigo.

int evaluateFitness(const Instance &inst, const Individual *ind) {
  vector<int> active, gama, scheduled; // Line 1: sets definition.
  set<int> e;
  active.push_back(0);
  gama.push_back(0);
  scheduled.push_back(0);
  int g = 0;
  int t = 0;
  // Line 2
  vector<vector<int>> RD;
  RD.resize(inst.i->desiredDueDate);
  for (auto &t : RD) {
    t.insert(t.end(), inst.mRenewableResourceAvailability.begin(),
             inst.mRenewableResourceAvailability.end());
  }
  // Line 3
  while (scheduled.size() < inst.i->mNumberOfJobs) {
    vector<int> diference;
    for (int job = 0; job < inst.i->mNumberOfJobs; job++) {
      if (std::find(scheduled.begin(), scheduled.end(), job) ==
          scheduled.end()) {
        bool feasible = true;
        for (int p = 0; p < inst.i->precedences[job].size(); p++) {
          if (inst.i->f[inst.i->precedences[job][p]] <=
              t + ind->gene[inst.i->mNumberOfJobs + p])
            feasible = true;
          else {
            feasible = false;
            break;
          }
        }
        if (feasible)
          diference.push_back(job);
      }
    }
    e.insert(diference.begin(), diference.end());
    while (not e.empty()) {
      int j = 0;
      float maxpriority = 0;
      for (auto &i : e) {
        if (ind->gene[i] >= maxpriority) {
          j = i;
          maxpriority = ind->gene[i];
        }
      }
      e.erase(find(e.begin(), e.end(), j));
      // Compute earliest finish (precedence only)
      vector<int> precedencesEarliestFinish;
      for (auto &p : inst.i->precedences[j])
        precedencesEarliestFinish.push_back(inst.i->f[p]);
      auto maxe = std::max_element(precedencesEarliestFinish.begin(),
                                   precedencesEarliestFinish.end());
      inst.i->earliestJobCompletion[j] =
          (maxe == precedencesEarliestFinish.end())
              ? 0
              : *maxe + inst.i->jobDuration[j];
      vector<int> possibletimes;
      for (int t = inst.i->earliestJobCompletion[j] - inst.i->jobDuration[j];
           t <= inst.i->desiredDueDate; t++) {
        if (std::find(gama.begin(), gama.end(), t) != gama.end()) {
          for (int td = t; td <= t + inst.i->jobDuration[j]; td++) {
            bool possible = true;
            for (int r = 0; r < inst.mRenewableResourceAvailability.size();
                 r++) {
              if (inst.i->resourceRequirement[j][r] > RD[td][r]) {
                possible = false;
                break;
              }
            }
            if (possible)
              possibletimes.push_back(td);
          }
        }
      }
      inst.i->f[j] =
          *std::min_element(possibletimes.begin(), possibletimes.end()) +
          inst.i->jobDuration[j];
      scheduled.push_back(j);
      gama.push_back(inst.i->f[j]);
      g++;
      active.clear();
      for (int i = 0; i < inst.i->mNumberOfJobs; i++) {
        if (inst.i->f[i] - inst.i->jobDuration[i] <= t and t < inst.i->f[i]) {
          active.push_back(i);
        }
      }
      vector<int> diference;
      for (int job = 0; job < inst.i->mNumberOfJobs; job++) {
        if (std::find(scheduled.begin(), scheduled.end(), job) ==
            scheduled.end()) {
          bool feasible = true;
          for (int p = 0; p < inst.i->precedences[job].size(); p++) {
            if (inst.i->f[inst.i->precedences[job][p]] <= t + ind->gene[inst.i->mNumberOfJobs + g])
              feasible = true;
            else {
              feasible = false;
              break;
            }
          }
          if (feasible)
            diference.push_back(job);
        }
      }
      e.clear();
      e.insert(diference.begin(), diference.end());
      for (int i = inst.i->f[j] - inst.i->jobDuration[j]; i < inst.i->f[i];
           i++) {
        for (int r = 0; r < inst.mRenewableResourceAvailability.size(); r++) {
          for (int a = 0; a < active.size(); a++)
            RD[i][r] = inst.mRenewableResourceAvailability[r] -
                       inst.i->resourceRequirement[a][r];
        }
      }
      vector<int> rg;
      for (int tg = 0; tg < gama.size(); tg++) {
        if (tg > t)
          rg.push_back(tg);
      }
      t = *std::min_element(rg.begin(), rg.end());
    }
  }
  inst.i->earliestJobCompletion.clear();
  inst.i->f.clear();
  inst.i->reinitializeF();
  int max = inst.i->f[inst.i->precedences[inst.i->mNumberOfJobs - 1][0]];
  for (auto &i : inst.i->precedences[inst.i->mNumberOfJobs - 1]) {
    if (inst.i->f[i] > max)
      max = inst.i->f[i];
  }
  return max;
}

/* selects two individuals, each one is selected by binary tournament */
void selectTwoIndividuals(Individual *pop, int *i1, int *i2) {
  {
    int s1 = rand() % popSize;
    int s2 = rand() % popSize;

    if (pop[s1].fitness > pop[s2].fitness)
      *i1 = s1;
    else
      *i1 = s2;
  }

  do {
    int s1 = rand() % popSize;
    int s2 = rand() % popSize;

    if (pop[s1].fitness > pop[s2].fitness)
      *i2 = s1;
    else
      *i2 = s2;

  } while (*i2 == *i1);
}

void applyMutation(Instance &inst) {
  int i, j;
  for (i = 0; (i < popSize); ++i)
    for (j = 0; (j < inst.i->mNumberOfJobs); ++j)
      if (DBL_RND <= mtRate)
        currGen[i].gene[j] = DBL_RND;
}
