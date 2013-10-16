
#include "ParaverTraceTask.h"
#include "ParaverTraceThread.h"

namespace domain {

ParaverTraceTask::ParaverTraceTask() {
}

ParaverTraceTask::ParaverTraceTask(unsigned int & key):key(key) {
}

ParaverTraceTask::~ParaverTraceTask() {
  for(unsigned int i = 0; i < threads.size(); i++){
     delete threads[i];
  }
}

void ParaverTraceTask::set_threads(vector<ParaverTraceThread *> & value) {
  threads = value;
}

void ParaverTraceTask::addThread(unsigned int & thread, ParaverTraceNode * node) {
  threads.push_back(new ParaverTraceThread(thread));
  executedOn = node;
}

void ParaverTraceTask::set_key(unsigned int value) {
  key = value;
}

void ParaverTraceTask::set_executedOn(ParaverTraceNode * value) {
  executedOn = value;
}


} // namespace domain
