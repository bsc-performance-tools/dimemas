#ifndef DOMAIN_PARAVERTRACETASK_H
#define DOMAIN_PARAVERTRACETASK_H


#include <vector>
using namespace std;
#include "ParaverTraceNode.h"

namespace domain { class ParaverTraceThread; } 

namespace domain {

class ParaverTraceTask {
  protected:
    unsigned int key;

    vector<ParaverTraceThread *> threads;

    ParaverTraceNode * executedOn;


  public:
    ParaverTraceTask();

    ParaverTraceTask(unsigned int & key);

    virtual ~ParaverTraceTask();

    inline const vector<ParaverTraceThread *> & get_threads() const;

    void set_threads(vector<ParaverTraceThread *> & value);

    void addThread(unsigned int & thread, ParaverTraceNode * node);

    inline const unsigned int get_key() const;

    void set_key(unsigned int value);

    inline const ParaverTraceNode * get_executedOn() const;

    void set_executedOn(ParaverTraceNode * value);

};
inline const vector<ParaverTraceThread *> & ParaverTraceTask::get_threads() const {
  return threads;
}

inline const unsigned int ParaverTraceTask::get_key() const {
  return key;
}

inline const ParaverTraceNode * ParaverTraceTask::get_executedOn() const {
  return executedOn;
}


} // namespace domain
#endif
