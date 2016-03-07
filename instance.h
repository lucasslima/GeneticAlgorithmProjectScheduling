#ifndef INSTANCE_H
#define INSTANCE_H
#include <vector>
#include <string>

using std::vector;
class Project{
public:
    Project(int numberOfJobs,int duedate, int resourceNumber):mNumberOfJobs(numberOfJobs),desiredDueDate(duedate){
        resourceRequirement.resize(mNumberOfJobs);
        jobDuration.resize(numberOfJobs);
        earliestJobCompletion.resize(numberOfJobs);
        latestJobCompletion.resize(numberOfJobs);
        precedences.resize(numberOfJobs);
        f.resize(numberOfJobs);
        for (auto &j : resourceRequirement)
            j.resize(resourceNumber);
    }
    int mNumberOfJobs;
    vector<vector<int> > resourceRequirement;
    vector< vector<int> > precedences;
    int desiredDueDate;
    vector<int> jobDuration;            //Duration of each job i
    vector<int> earliestJobCompletion;  //Earliest finish of each job i
    vector<int> f;
    vector<int> latestJobCompletion;    //Latest possible finish for each job i
};

class Instance
{
public:
    Instance(std::string filename);
    Project *i; //Project
    vector<int> mRenewableResourceAvailability;
    void load(std::string filename);
};

/* cria um objeto instância e preenche com dados do arquivo */

#endif

