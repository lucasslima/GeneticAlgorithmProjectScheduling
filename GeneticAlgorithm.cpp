/*
 * ga.c
 * example of a Genetic Algorithm for the knapsack problem
 * Copyright (C) 2016 H.G. Santos <haroldo.santos@gmail.com>
 */

#include <stdlib.h>
#include <float.h>
#include <algorithm>
#include "instance.h"
#include <string>
#include <limits>
#include <vector>
#include <set>
#include <queue>
#include <iostream>

using namespace std;
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// computes a random number [0,1)
#define DBL_RND (((double)rand()) / (((double)RAND_MAX) + 1.0))

class Individual {
public:
  /* incidence vector indicating if each element will or not be chosen */
  vector<float> gene;
  int fitness;
};

/* parameters of the algorithm */
int popSize = 100;   /* population size */
float mtRate = 0.01;   /* mutation rate */
int generations = 250; /* number of generations */

/* statistics about the population, best, worst and average */
double bestFit;
double worstFit;
double avFit;

/* individuals are stored as binary vectors
 * population of solutions, space for current and next generation  */

vector<Individual> currGen;
vector<Individual> nextGen;

void allocate_memory_population(Instance &inst);

/* fills initial population */
void populate(Instance &inst);

/* binary tournament */
void selectTwoIndividuals(vector<Individual> &pop, int *i1, int *i2);

/* generates new population by uniform crossover */
void applyCrossover(Instance &inst);

void applyMutation(Instance &inst);

void evaluatePopulation(Instance &inst, vector<Individual> &population);

int main(int argc, const char **argv) {
  if (argc < 2) {
    fprintf(stderr, "enter instance name.\n");
    exit(EXIT_FAILURE);
  }

  /* instance with problem data */

  Instance inst(argv[1]);

  cout << "starting GA for project scheduling problem with " << inst.project->mNumberOfJobs << " jobs.\n";

  allocate_memory_population(inst);

  for (int i = inst.project->mNumberOfJobs; i < inst.project->mNumberOfJobs * 2; i++)

    populate(inst);

  for (int gen = 1; (gen < +generations); ++gen) {
    evaluatePopulation(inst, currGen);

    /* generates new population by crossover */
    applyCrossover(inst);

    /* mutates */
    applyMutation(inst);

    cout << "generation " << gen + 1 << " fitness: (worst, average, best): "<< worstFit << endl << avFit << endl << bestFit << endl;

    /* swaping populations */
    vector<Individual> *temp = &currGen;
    currGen = nextGen;
    nextGen = *temp;
  }
}

void applyCrossover(Instance &inst) {
  int i;
  for (i = 0; (i < popSize); ++i) {
    int i1, i2;
    selectTwoIndividuals(currGen, &i1, &i2);

    int j;
    for (j = 0; (j < inst.project->mNumberOfJobs * 2); ++j)
      nextGen[i].gene[j] = (DBL_RND > 0.7) ? currGen[i1].gene[j] : currGen[i2].gene[j];
  }
}

void allocate_memory_population(Instance &inst) {
  currGen.resize(popSize);
  int i;
  for (i = 0; (i < popSize); ++i) {
    currGen[i].gene.resize(inst.project->mNumberOfJobs * 2);
  }

  nextGen.resize(popSize);
  for (i = 0; (i < popSize); ++i) {
    nextGen[i].gene.resize(inst.project->mNumberOfJobs * 2);
  }
}

void populate(Instance &inst) {
  for (int i = 0; (i < popSize); ++i) {
    for (int j = 0; j < inst.project->mNumberOfJobs; ++j)
      currGen[i].gene[j] = DBL_RND;
    for (int j = inst.project->mNumberOfJobs; j < inst.project->mNumberOfJobs * 2; j++)
      currGen[i].gene[j] = DBL_RND * 1.5 * inst.project->desiredDueDate;
  }
}

int evaluateFitness(const Instance &inst, const Individual *ind);

void evaluatePopulation(Instance &inst, vector<Individual> &population) {
  bestFit = DBL_MAX;
  worstFit = -DBL_MAX;
  avFit = 0.0;
  int i;
  for (i = 0; (i < popSize); ++i) {
    population[i].fitness = evaluateFitness(inst, &population[i]);
    bestFit = MIN(bestFit, population[i].fitness);
    worstFit = MAX(worstFit, population[i].fitness);
    avFit += population[i].fitness;
  }
  avFit /= ((double)popSize);
}

int evaluateFitness(const Instance &inst, const Individual *ind) {
  vector<int> active, scheduled; // Line 1: sets definition.
  set<int> e, gama;
  active.push_back(0);
  gama.insert(0);
  scheduled.push_back(0);
  int g = 0;
  int t = 0;
  // Line 2
  vector<vector<int>> RD;
  RD.resize(inst.project->desiredDueDate);
  for (auto &t : RD) {
    t.insert(t.end(), inst.mRenewableResourceAvailability.begin(),
             inst.mRenewableResourceAvailability.end());
  }
  // Line 3
  while (scheduled.size() < inst.project->mNumberOfJobs) {
    vector<int> diference;
    for (int job = 0; job < inst.project->mNumberOfJobs; job++) {
      if (std::find(scheduled.begin(), scheduled.end(), job) ==
          scheduled.end()) {
        bool feasible = true;
        for (auto &p :inst.project->precedences[job]) {
          if (inst.project->f[p] <= t + ind->gene[inst.project->mNumberOfJobs + g])
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
//    Compute earliest finish (precedence only)
      vector<int> precedencesEarliestFinish;
      for (auto &p : inst.project->precedences[j])
        precedencesEarliestFinish.push_back(inst.project->f[p]);
      auto maxe = std::max_element(precedencesEarliestFinish.begin(),
                                   precedencesEarliestFinish.end());
      inst.project->earliestJobCompletion[j] =
          (maxe == precedencesEarliestFinish.end())
              ? 0
              : *maxe + inst.project->jobDuration[j];
      queue<int> possibletimes;
      for (int t = inst.project->earliestJobCompletion[j] - inst.project->jobDuration[j];
           t <= inst.project->desiredDueDate; t++) {
        if (std::find(gama.begin(), gama.end(), t) != gama.end()) {
          for (int td = t; td <= t + inst.project->jobDuration[j]; td++) {
            bool possible = true;
            for (int r = 0; r < inst.mRenewableResourceAvailability.size();
                 r++) {
              if (inst.project->resourceRequirement[j][r] > RD[td][r]) {
                possible = false;
                break;
              }
            }
            if (possible)
              possibletimes.push(td);
          }
        }
      }
//      inst.project->f[j] =
//          *std::min_element(possibletimes.begin(), possibletimes.end()) +
//          inst.project->jobDuration[j];
      inst.project->f[j] = possibletimes.front() + inst.project->jobDuration[j];
      scheduled.push_back(j);
      gama.insert(inst.project->f[j]);
      g++;
      active.clear();
      for (int i = 0; i < inst.project->mNumberOfJobs; i++) {
        if (inst.project->f[i] - inst.project->jobDuration[i] <= t and t < inst.project->f[i]) {
          active.push_back(i);
        }
      }
      vector<int> diference;
      for (int job = 0; job < inst.project->mNumberOfJobs; job++) {
        if (std::find(scheduled.begin(), scheduled.end(), job) ==
            scheduled.end()) {
          bool feasible = true;
          for (auto &p : inst.project->precedences[job]) {
            if (inst.project->f[p] <= t + ind->gene[inst.project->mNumberOfJobs + g])
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
      for (int i = inst.project->f[j] - inst.project->jobDuration[j]; i < inst.project->f[j]; i++) {
        for (int r = 0; r < inst.mRenewableResourceAvailability.size(); r++) {
          for (auto &a : active)
            RD[i][r] = inst.mRenewableResourceAvailability[r] -
                       inst.project->resourceRequirement[a][r];
        }
      }
    }
    vector<int> rg;
    for (auto &tg: gama) {
      if (tg > t)
        rg.push_back(tg);
    }
    t = (std::min_element(rg.begin(), rg.end()) == rg.end()) ? t : *std::min_element(rg.begin(), rg.end());
  }
  int max = inst.project->f[inst.project->precedences[inst.project->mNumberOfJobs - 1][0]];
  for (auto &i : inst.project->precedences[inst.project->mNumberOfJobs - 1]) {
    if (inst.project->f[i] > max)
      max = inst.project->f[i];
  }
  inst.project->earliestJobCompletion.clear();
  inst.project->f.clear();
  inst.project->reinitializeF();
  return max;
}

/* selects two individuals, each one is selected by binary tournament */
void selectTwoIndividuals(vector<Individual> &pop, int *i1, int *i2) {
  {
    int s1 = rand() % ((int) (0.25 * popSize)) + popSize * 0.75;
    int s2 = rand() % popSize;

    if (pop[s1].fitness < pop[s2].fitness)
      *i1 = s1;
    else
      *i1 = s2;
  }

  do {
    int s1 = rand() % ((int) (0.25 * popSize)) + popSize * 0.75;
    int s2 = rand() % popSize;

    if (pop[s1].fitness < pop[s2].fitness)
      *i2 = s1;
    else
      *i2 = s2;

  } while (*i2 == *i1);
}

void applyMutation(Instance &inst) {
  int i, j;
  for (i = 0; (i < popSize); ++i)
    for (j = 0; (j < inst.project->mNumberOfJobs); ++j)
      if (DBL_RND <= mtRate)
        currGen[i].gene[j] = DBL_RND;
}
