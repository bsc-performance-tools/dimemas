
#include "UIParaverTrace.h"
#include "ParaverTrace.h"
#include "LibException.h"
#include "ParaverTraceApplication.h"
#include "ParaverTraceTask.h"

namespace domain {

/**
 * \brief This is the constructor. The system parses and store internally the data defined in the header of the Paraver trace file.
 * \param prvFile refers to the path of the Paraver  trace file (.prv).
 */
UIParaverTrace::UIParaverTrace(const string prvFile) {
  trace = new ParaverTrace(prvFile, true);
}

/**
 * \brief This is the destructor.
 */
UIParaverTrace::~UIParaverTrace() {
  delete trace;
}

/**
 * \brief This method returns the path file to the Paraver trace file (.prv).
 * \return The path to the Paraver trace file (.prv).
 */
string UIParaverTrace::getTraceFile() {
  return trace->get_prvFile();
}

/**
 * \brief This method returns the number of applications from the Paraver trace header.
 * \return The number of applications.
 */
unsigned int UIParaverTrace::getNumberOfApplications() const {
  return trace->get_applications().size();
}

/**
 * \brief This method returns the number of Tasks for a certain Application.
 * \param appl This is the Application number to query.
 * \return The number of Tasks for the application 'appl'.
 */
unsigned int UIParaverTrace::getNumberOfTasks(const unsigned int appl) const {
  vector<ParaverTraceApplication *> v_apps = trace->get_applications();
  if(v_apps.size() < appl){
    stringstream msg;
    msg<<"Wrong number of application: "<<appl<<" max:"<<v_apps.size();
    throw LibException(__FILE__, __LINE__, msg.str());
  }
  return v_apps[appl]->get_tasks().size();
}

/**
 * \brief This method returns the number of threads for a certain Application and Task.
 * \param appl refers to the number of Application.
 * \param task refers to the Task to query.
 * \return The number of threads for a certain application and task.
 */
unsigned int UIParaverTrace::getNumberOfThreads(const unsigned int appl, const unsigned int task) const {
  vector<ParaverTraceApplication *> v_apps = trace->get_applications();
  if(v_apps.size() < appl){
    stringstream msg;
    msg<<"Wrong number of application: "<<appl<<" max:"<<v_apps.size();
    throw LibException(__FILE__, __LINE__, msg.str());
  }
  vector<ParaverTraceTask*> v_tasks = v_apps[appl]->get_tasks();
  if(v_tasks.size() < task){
    stringstream msg;
    msg<<"Wrong number of task: "<<appl<<"->"<<task<<" max:"<<v_tasks.size();
    throw LibException(__FILE__, __LINE__, msg.str());
  }
  return v_tasks[task]->get_threads().size();
}

/**
 * \brief This method returns the number of nodes defined in the header of Paraver trace file (.prv).
 * \return The number of nodes defined in the trace.
 */
unsigned int UIParaverTrace::getNumberOfNodes() const {
  return trace->get_nodes().size();
}

/**
 * \brief This methods, for a certain task from a certain application, returns the node where has been executed the task.
 * \param appl The application to query.
 * \param task The task to query.
 * \return The node where the task has been executed.
 */
unsigned int UIParaverTrace::getNodeOfTask(const unsigned int appl, const unsigned int task) const {
  vector<ParaverTraceApplication *> v_apps = trace->get_applications();
  if(v_apps.size() < appl){
    stringstream msg;
    msg<<"Wrong number of application: "<<appl<<" max:"<<v_apps.size();
    throw LibException(__FILE__, __LINE__, msg.str());
  }
  vector<ParaverTraceTask*> v_tasks = v_apps[appl]->get_tasks();
  if(v_tasks.size() < task){
    stringstream msg;
    msg<<"Wrong number of task: "<<appl<<"->"<<task<<" max:"<<v_tasks.size();
    throw LibException(__FILE__, __LINE__, msg.str());
  }
  return v_tasks[task]->get_executedOn()->get_nodeId();
}

/**
 * \brief This methods returns the number of CPUs belonging to a Node.
 * \param node The node to query.
 * \return The number of CPUs belonging to the node 'node'.
 */
unsigned int UIParaverTrace::getNumberOfCPUFromNode(const unsigned int node) const {
  vector<ParaverTraceNode*> v_nodes = trace->get_nodes();
  if(v_nodes.size() <= node){
    stringstream msg;
    msg<<"Wrong number of node: "<<node<<" max:"<<v_nodes.size();
    throw LibException(__FILE__, __LINE__, msg.str());
  }
  return v_nodes[node]->get_cpus().size();
}

/**
 * \brief This methods returns the CPU global ID from one CPU belonging to a node.
 * \param node The node to query.
 * \param cpu number from this node.
 * \return The number of CPU ID belonging to the node 'node'.
 */
unsigned int UIParaverTrace::getCPUFromNode(unsigned int node, unsigned int cpu) const {
  vector<ParaverTraceNode*> v_nodes = trace->get_nodes();
  if(v_nodes.size() <= node){
    stringstream msg;
    msg<<"Wrong number of node: "<<node<<" max:"<<v_nodes.size();
    throw LibException(__FILE__, __LINE__, msg.str());
  }
  return v_nodes[node]->get_cpus().size();
}


} // namespace domain
