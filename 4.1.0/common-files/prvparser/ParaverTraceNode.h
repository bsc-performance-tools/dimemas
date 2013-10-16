#ifndef DOMAIN_PARAVERTRACENODE_H
#define DOMAIN_PARAVERTRACENODE_H


#include <vector>
using namespace std;

namespace domain { class ParaverTraceCPU; } 

namespace domain {

class ParaverTraceNode {
  protected:
    vector<ParaverTraceCPU *> cpus;


  public:
    ParaverTraceNode();

    virtual ~ParaverTraceNode();


  protected:
    unsigned int nodeId;


  public:
    inline const unsigned int get_nodeId() const;

    void set_nodeId(unsigned int value);

    inline const vector<ParaverTraceCPU *> & get_cpus() const;

    void set_cpus(vector<ParaverTraceCPU *> & value);

};
inline const unsigned int ParaverTraceNode::get_nodeId() const {
  return nodeId;
}

inline const vector<ParaverTraceCPU *> & ParaverTraceNode::get_cpus() const {
  return cpus;
}


} // namespace domain
#endif
