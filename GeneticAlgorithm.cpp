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
#include <map>
#include <chrono>

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
static int popSize;   /* population size */
static int generations = 250; /* number of generations */

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

  auto t1 = std::chrono::high_resolution_clock::now();
//  cout << "starting GA for project scheduling problem with " << inst.project->mNumberOfJobs << " jobs.\n";
  popSize = inst.project->mNumberOfJobs * 5;
  allocate_memory_population(inst);

  for (int i = inst.project->mNumberOfJobs; i < inst.project->mNumberOfJobs * 2; i++)

    populate(inst);

  for (int gen = 1; (gen < +generations); ++gen) {
    evaluatePopulation(inst, currGen);

    /* generates new population by crossover */
    applyCrossover(inst);

    /* mutates */
    applyMutation(inst);

    //cout << "generation " << gen + 1 << " fitness: (worst, average, best): "<< worstFit << "  " << avFit << "  "<< bestFit << "  " << endl;

    /* swaping populations */
    vector<Individual> *temp = &currGen;
    currGen = nextGen;
    nextGen = *temp;
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  int parameter, instance;
  sscanf(argv[1],"j30%d_%d.sm.json", &parameter, &instance);
  cout << parameter << "\t" << instance << "\t" << bestFit << "\t " << chrono::duration<double, std::milli>(t2-t1).count()/1000 << endl;
}

void applyCrossover(Instance &inst) {
  int i;
  std::sort(currGen.begin(),currGen.end(),[](const Individual & i1, const Individual & i2)->bool{
      return i1.fitness < i2.fitness;
  });
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
    // Line 1: sets definition.
  set<int> e, gama,active, scheduled;
  active.insert(0);
  gama.insert(0);
  scheduled.insert(0);
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
      if (scheduled.find(job) ==
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
      std::map<double,int> priorities;
      for (auto &p : e)
          priorities.insert(std::pair<double,int>(ind->gene[p],p));
      j = priorities.begin()->second;
      e.erase(find(e.begin(), e.end(), j));

//    Compute earliest finish (precedence only)
      set<int> precedencesEarliestFinish;
      for (auto &p : inst.project->precedences[j])
        precedencesEarliestFinish.insert(inst.project->f[p]);
      auto maxe = *precedencesEarliestFinish.rbegin();
      inst.project->earliestJobCompletion[j] = maxe + inst.project->jobDuration[j];

//      queue<int> possibletimes;
//      for (int jobCourse = inst.project->earliestJobCompletion[j] - inst.project->jobDuration[j];
//           jobCourse <= inst.project->desiredDueDate; jobCourse++) {
//        if (gama.find(jobCourse) != gama.end()) {
//            bool possible = true;
//          for (int td = jobCourse; td <= jobCourse + inst.project->jobDuration[j]; td++) {
//            for (int r = 0; r < inst.mRenewableResourceAvailability.size();
//                 r++) {
//              if (inst.project->resourceRequirement[j][r] > RD[td][r]) {
//                possible = false;
//                break;
//              }
//            }
//            if (possible)
//              possibletimes.push(td);
//          }
//        }
//      }
      int pTime = inst.project->earliestJobCompletion[j] - inst.project->jobDuration[j];
      while(pTime < inst.project->desiredDueDate){
          bool possible = true;
            for (int r = 0; r < inst.mRenewableResourceAvailability.size();
                 r++) {
              if (inst.project->resourceRequirement[j][r] > RD[pTime][r]) {
                possible = false;
                break;
              }
            }
            if (possible)
              break;
            else
                pTime++;
      }
      inst.project->f[j] = pTime + inst.project->jobDuration[j];
//      inst.project->f[j] = possibletimes.front() + inst.project->jobDuration[j];
      scheduled.insert(j);
      gama.insert(inst.project->f[j]);
      g++;
      active.clear();
      for (int i = 0; i < inst.project->mNumberOfJobs; i++) {
        if (inst.project->f[i] - inst.project->jobDuration[i] <= t and t < inst.project->f[i]) {
          active.insert(i);
        }
      }
      for (int jobDuration = inst.project->f[j] - inst.project->jobDuration[j]; jobDuration < inst.project->f[j]; jobDuration++){
      for (int r = 0; r < inst.mRenewableResourceAvailability.size(); r++) {
            RD[jobDuration][r] = inst.mRenewableResourceAvailability[r];
          for (auto &a : active)
            RD[jobDuration][r] -= inst.project->resourceRequirement[j][r];
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
  int max = inst.project->f[inst.project->mNumberOfJobs - 1];

  inst.project->earliestJobCompletion.clear();
  inst.project->f.clear();
  inst.project->reinitializeF();
  inst.project->reinitializeEF();
  return max;
}

/* selects two individuals, one from TOP and other from whole population. */
void selectTwoIndividuals(vector<Individual> &pop, int *i1, int *i2) {
  {
    int s1 = rand() % ((int) (0.20 * popSize));
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
  for (i = popSize-1; (i > popSize *0.70); --i){
        for (int j = 0; j < inst.project->mNumberOfJobs; ++j)
          currGen[i].gene[j] = DBL_RND;
        for (int j = inst.project->mNumberOfJobs; j < inst.project->mNumberOfJobs * 2; j++)
          currGen[i].gene[j] = DBL_RND * 1.5 * inst.project->desiredDueDate;
      }
}
