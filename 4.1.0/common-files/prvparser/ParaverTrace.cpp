
#include "ParaverTrace.h"
#include "ParaverTraceApplication.h"
#include "ParaverTraceNode.h"
#include "LibException.h"
#include "ParaverHeaderGrammar.h"
#include "ParaverBodyGrammar.h"

namespace domain {

ParaverTrace::ParaverTrace(string prvFile, bool multievents):prvFile(prvFile),multievents(multievents) {
  parseHeader();
}

ParaverTrace::~ParaverTrace() {
  for(unsigned int i = 0; i < applications.size(); i++){
     delete applications[i];
  }
  for(unsigned int i = 0; i < nodes.size(); i++){
     delete nodes[i];
  }
}

void ParaverTrace::processEvents (struct multievent_t &e)
{
	if (!multievents)
	{
		singleevent_t s;

		s.Timestamp = e.Timestamp;
		s.ObjectID.cpu = e.ObjectID.cpu;
		s.ObjectID.task = e.ObjectID.task;
		s.ObjectID.ptask = e.ObjectID.ptask;
		s.ObjectID.thread = e.ObjectID.thread;
		for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
		{
			s.event.Type = (*i).Type;
			s.event.Value = (*i).Value;
			processEvent (s);
		}
	}
	else
		processMultiEvent (e);
}

void ParaverTrace::parseHeader() {
  bool found = false;
  string line;
  int numLines = 0;
  fstream input(prvFile.c_str(), ios_base::in);
  
  if(!input.good()){
    stringstream ss;
    ss<<"Error opening "<<prvFile;
    throw LibException(__FILE__, __LINE__, ss.str());
  }
  
  ParaverHeaderGrammar<ParaverTrace> headerGrammar(*this);
  while(getline(input, line) && !found && numLines < MAX_NUM_LINES){
     parse_info<> info = parse(line.c_str(),
        headerGrammar,
        space_p
     );
     if(info.full){
        found = true;
     }else{
        cout<<"Error reading at: "<<info.stop<<endl;
     }
     numLines++;
  }
  if(!found) cout<<"Paraver header not found!!!!\n"<<endl;
}

void ParaverTrace::set_lastTime(unsigned long long value) {
  lastTime = value;
}

void ParaverTrace::setNumberOfApplications(unsigned int nappl) {
  for(unsigned int i = 0; i < nappl; i++){
     applications.push_back(new ParaverTraceApplication(i));
  }
}

void ParaverTrace::addTasksToAppl(unsigned int appl, unsigned int task, unsigned int thread, unsigned int node) {
  if(applications.size() <= appl){
     cout<<"Error: not valid appl: "<<appl<<endl;
  }
  applications[appl]->addTask(task, thread, nodes[node]);
}

void ParaverTrace::setNumberOfNodes(unsigned int _nodes) {
  for(unsigned int i = 0; i < _nodes; i++){
     nodes.push_back(new ParaverTraceNode());
  }
}

void ParaverTrace::setNumberOfTasksForAppl(unsigned int appl, unsigned int tasks) {
  applications[appl]->setNumberOfTasks(tasks);
}

void ParaverTrace::set_applications(vector<ParaverTraceApplication *> & value) {
  applications = value;
}

void ParaverTrace::set_prvFile(string value) {
  prvFile = value;
}

void ParaverTrace::set_nodes(vector<ParaverTraceNode *> & value) {
  nodes = value;
}

void ParaverTrace::parseBody (void)
{
	fstream input(prvFile.c_str());
	string line;

	if (!input.good())
	{
		cout << "Error opening " << prvFile << endl;
		return;
	}

	ParaverBodyGrammar<ParaverTrace> bodyGrammar(*this);
	while(getline(input, line))
	{
		parse_info<> info = parse(line.c_str(), bodyGrammar, space_p);
		if(!info.full)
			cout<<"Error reading at: "<<info.stop<<endl;
	}
}

void ParaverTrace::clearEvent (struct multievent_t &me)
{
  me.events.clear();
}

void ParaverTrace::newEvent (struct multievent_t &me, struct event_t &e)
{
  me.events.push_back (e);
}

void ParaverTrace::processState (struct state_t &s)
{
}

void ParaverTrace::processCommunication (struct comm_t &c)
{
}

void ParaverTrace::processEvent (struct singleevent_t &e)
{
}

void ParaverTrace::processMultiEvent (struct multievent_t &e)
{
}

void ParaverTrace::processComment (string &c)
{
}

void ParaverTrace::processCommunicator (string &c)
{
}

void ParaverTrace::set_multievents (bool multievents)
{
	this->multievents = multievents;
}

} // namespace domain
