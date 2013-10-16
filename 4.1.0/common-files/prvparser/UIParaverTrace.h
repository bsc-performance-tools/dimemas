#ifndef DOMAIN_UIPARAVERTRACE_H
#define DOMAIN_UIPARAVERTRACE_H


#include <string>
using namespace std;

#include <sstream>
using namespace std;
namespace domain { class ParaverTrace; } 
class LibException;
namespace domain { class ParaverTraceApplication; } 
namespace domain { class ParaverTraceTask; } 

namespace domain {

/**
 * \brief Class interface read the values defined in the Paraver header.
 */
class UIParaverTrace {
  public:
    /**
     * \brief This is the constructor. The system parses and store internally the data defined in the header of the Paraver trace file.
     * \param prvFile refers to the path of the Paraver  trace file (.prv).
     */
    UIParaverTrace(const string prvFile);

    /**
     * \brief This is the destructor.
     */
    virtual ~UIParaverTrace();

    /**
     * \brief This method returns the path file to the Paraver trace file (.prv).
     * \return The path to the Paraver trace file (.prv).
     */
    string getTraceFile();

    /**
     * \brief This method returns the number of applications from the Paraver trace header.
     * \return The number of applications.
     */
    unsigned int getNumberOfApplications() const;

    /**
     * \brief This method returns the number of Tasks for a certain Application.
     * \param appl This is the Application number to query.
     * \return The number of Tasks for the application 'appl'.
     */
    unsigned int getNumberOfTasks(const unsigned int appl) const;

    /**
     * \brief This method returns the number of threads for a certain Application and Task.
     * \param appl refers to the number of Application.
     * \param task refers to the Task to query.
     * \return The number of threads for a certain application and task.
     */
    unsigned int getNumberOfThreads(const unsigned int appl, const unsigned int task) const;

    /**
     * \brief This method returns the number of nodes defined in the header of Paraver trace file (.prv).
     * \return The number of nodes defined in the trace.
     */
    unsigned int getNumberOfNodes() const;

    /**
     * \brief This methods, for a certain task from a certain application, returns the node where has been executed the task.
     * \param appl The application to query.
     * \param task The task to query.
     * \return The node where the task has been executed.
     */
    unsigned int getNodeOfTask(const unsigned int appl, const unsigned int task) const;

    /**
     * \brief This methods returns the number of CPUs belonging to a Node.
     * \param node The node to query.
     * \return The number of CPUs belonging to the node 'node'.
     */
    unsigned int getNumberOfCPUFromNode(const unsigned int node) const;

    /**
     * \brief This methods returns the CPU global ID from one CPU belonging to a node.
     * \param node The node to query.
     * \param cpu number from this node.
     * \return The number of CPU ID belonging to the node 'node'.
     */
    unsigned int getCPUFromNode(unsigned int node, unsigned int cpu) const;


  protected:
    ParaverTrace * trace;

};

} // namespace domain
#endif
