#ifndef DOMAIN_PARAVERTRACEAPPLICATION_H
#define DOMAIN_PARAVERTRACEAPPLICATION_H


#include <string>
using namespace std;
#include <vector>
using namespace std;

#include <iostream>
using namespace std;
namespace domain { class ParaverTraceTask; } 
namespace domain { class ParaverTraceNode; } 

namespace domain {

class ParaverTraceApplication {
  protected:
    unsigned int key;

    string description;

    vector<ParaverTraceTask *> tasks;


  public:
    ParaverTraceApplication(unsigned int & key);

    ParaverTraceApplication(int & key, string & description);

    virtual ~ParaverTraceApplication();

    void addTask(unsigned int & task, unsigned int & thread, ParaverTraceNode * node);

    void setNumberOfTasks(unsigned int & ntasks);

    inline const vector<ParaverTraceTask *> & get_tasks() const;

    void set_tasks(vector<ParaverTraceTask *> & value);

    inline const unsigned int get_key() const;

    void set_key(unsigned int value);

};
inline const vector<ParaverTraceTask *> & ParaverTraceApplication::get_tasks() const {
  return tasks;
}

inline const unsigned int ParaverTraceApplication::get_key() const {
  return key;
}


} // namespace domain
#endif
