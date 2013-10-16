
#include "ParaverTraceApplication.h"
#include "ParaverTraceTask.h"
#include "ParaverTraceNode.h"

namespace domain {

ParaverTraceApplication::ParaverTraceApplication(unsigned int & key):key(key) {
}

ParaverTraceApplication::ParaverTraceApplication(int & key, string & description):key(key),description(description) {
}

ParaverTraceApplication::~ParaverTraceApplication() {
  for(unsigned int i = 0; i < tasks.size(); i++){
     delete tasks[i];
  }
}

void ParaverTraceApplication::addTask(unsigned int & task, unsigned int & thread, ParaverTraceNode * node) {
  if(tasks.size() <= task){
     cout<<"Error: not valid task "<<task<<endl;
  }
  tasks[task]->addThread(thread, node);
}

void ParaverTraceApplication::setNumberOfTasks(unsigned int & ntasks) {
  for(unsigned int i = 0; i < ntasks; i++){
     tasks.push_back(new ParaverTraceTask(i));
  }
}

void ParaverTraceApplication::set_tasks(vector<ParaverTraceTask *> & value) {
  tasks = value;
}

void ParaverTraceApplication::set_key(unsigned int value) {
  key = value;
}


} // namespace domain
