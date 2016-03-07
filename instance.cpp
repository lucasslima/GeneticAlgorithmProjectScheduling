#include "instance.h"
#include <json/value.h>
#include <json/reader.h>
#include <iostream>
#include <fstream>


using namespace std;
using namespace Json;
Instance::Instance(string filename) {
    load(filename);
}
void Instance::load(std::string fileName) {
    Json::Value instance;
    Json::Reader reader;

    std::ifstream instancefile(fileName);
    bool readSucess = reader.parse(fileName, instance, false);
    if (readSucess) {
        Json::Value numberOfJobs = instance["numberOfJobs"];
        Json::Value resourceNumber = instance["resouceNumber"];
        Json::Value horizon = instance["horizon"];
        this->i = new Project(numberOfJobs.asInt(), horizon.asInt(), resourceNumber.asInt());
        for (Json::ValueIterator jIterator = instance["jobs"].begin();
             jIterator != instance["jobs"].end(); jIterator++) {
            int j = atoi((*jIterator)["jobnr"].asString().c_str()) - 1;
            i->latestJobCompletion[j] = (*jIterator)["LF"].asInt();
            i->earliestJobCompletion[j] = (*jIterator)["EF"].asInt();
            for (Json::ValueIterator pIterator = (*jIterator)["precedents"].begin();
                 pIterator != (*jIterator)["precedents"].end(); pIterator++)
                i->precedences[j].push_back(pIterator->asInt());
            Json::Value request = (*jIterator)["requests"];
            for (Json::ValueIterator rIterator = request.begin(); rIterator != request.end(); rIterator++) {
                i->jobDuration[j] = atoi(rIterator->asCString());
                Json::Value resourceRequest = (*rIterator)["renews"];
                int rpos = 0;
                for (Json::ValueIterator r = resourceRequest.begin(); r != resourceRequest.end(); r++)
                    i->resourceRequirement[j][rpos++] = atoi(r->asCString());
            }
        }
    }
    else
        cout << "Failed to read json file: " << reader.getFormattedErrorMessages();
    exit(1);
}
