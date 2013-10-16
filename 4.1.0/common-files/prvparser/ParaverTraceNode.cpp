
#include "ParaverTraceNode.h"
#include "ParaverTraceCPU.h"

namespace domain {

ParaverTraceNode::ParaverTraceNode() {
}

ParaverTraceNode::~ParaverTraceNode() {
  for(unsigned int i = 0; i < cpus.size(); i++){
     delete cpus[i];
  }
}

void ParaverTraceNode::set_nodeId(unsigned int value) {
  nodeId = value;
}

void ParaverTraceNode::set_cpus(vector<ParaverTraceCPU *> & value) {
  cpus = value;
}


} // namespace domain
