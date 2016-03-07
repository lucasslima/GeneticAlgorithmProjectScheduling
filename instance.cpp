#include "instance.h"
#include <json/value.h>
#include <json/reader.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace Json;
Instance::Instance(string filename) {
    load(filename);
}
void Instance::load(std::string fileName) {
    Json::Value instance;
    Json::Reader reader;

    std::ifstream instancefile(fileName);
    bool readSucess = reader.parse(instancefile, instance, false);
    if (readSucess) {
        Json::Value numberOfJobs = instance["numberOfJobs"];
        Json::Value resourceNumber = instance["renewable_number"];
        Json::Value horizon = instance["horizon"];
       Json::Value availableResources = instance["available_renewables"];
       for (Json::ValueIterator rIterator = availableResources.begin(); rIterator != availableResources.end(); rIterator++)
           mRenewableResourceAvailability.push_back(atoi((*rIterator).asCString()));
        this->project = new Project(numberOfJobs.asInt(), horizon.asInt(), resourceNumber.asInt());
        for (Json::ValueIterator jIterator = instance["jobs"].begin();
             jIterator != instance["jobs"].end(); jIterator++) {
            int j = atoi((*jIterator)["jobnr"].asString().c_str()) - 1;
            project->latestJobCompletion[j] = (*jIterator)["LF"].asInt();
            project->earliestJobCompletion[j] = (*jIterator)["EF"].asInt();
            for (Json::ValueIterator pIterator = (*jIterator)["precedents"].begin();
                 pIterator != (*jIterator)["precedents"].end(); pIterator++)
                project->precedences[j].push_back(atoi(pIterator->asCString()) - 1);
            Json::Value request = (*jIterator)["requests"];
            for (Json::ValueIterator rIterator = request.begin(); rIterator != request.end(); rIterator++) {
                project->jobDuration[j] = atoi((*rIterator)["duration"].asCString());
                Json::Value resourceRequest = (*rIterator)["renews"];
                int rpos = 0;
                for (Json::ValueIterator r = resourceRequest.begin(); r != resourceRequest.end(); r++)
                    project->resourceRequirement[j][rpos++] = atoi(r->asCString());
            }
        }
    }
    else {
        cout << "Failed to read json file: " << reader.getFormattedErrorMessages();
        exit(1);
    }
}
