#ifndef _ALLTESTS_H
#define _ALLTESTS_H


#include <cxxtest/TestSuite.h>
#include "ParaverWindowConfig.h"
#include "ParaverTraceConfig.h"
#include "ParaverDisplayWindow.h"
#include "ParaverTrace.h"
#include "Paramedir.h"
#include "Paraver.h"
#include "Paraver2DOutput.h"
#include "MParaver2DOutput.h"
#include "MParaver2DOutputGrammar.h"

class TestParaverWindowConfig : public CxxTest::TestSuite {
  public:
    domain::ParaverWindowConfig * pTestObj;


  protected:
    domain::ParaverTraceConfig * pcf;


  public:
    inline void setUp();

    inline void tearDown();

    inline void testParseFile();

    inline void testParaverWindowConfig();

    inline void testGet_dependencies();

    inline void testSet_dependencies();

};

inline void TestParaverWindowConfig::setUp() {
  pcf = new domain::ParaverTraceConfig("prv2viz_cfgs/bt.A.4.pcf");
  
}

inline void TestParaverWindowConfig::tearDown() {
  delete pcf;
}

inline void TestParaverWindowConfig::testParseFile() {
  pTestObj = new domain::ParaverWindowConfig("prv2viz_cfgs/2dmpiline_mpicall.cfg", pcf->get_i_eventTypes());
  TS_ASSERT(pTestObj->parse());
  delete pTestObj;
  pTestObj = new domain::ParaverWindowConfig("prv2viz_cfgs/2dmpiline_in_mpicallTime.cfg", pcf->get_i_eventTypes());
  TS_ASSERT(pTestObj->parse());
  delete pTestObj;
  pTestObj = new domain::ParaverWindowConfig("prv2viz_cfgs/2dmpiline_in_mpicallNBursts.cfg", pcf->get_i_eventTypes());
  TS_ASSERT(pTestObj->parse());
  delete pTestObj;
  pTestObj = new domain::ParaverWindowConfig("prv2viz_cfgs/2d_user_functions_st_cycles.cfg", pcf->get_i_eventTypes());
  TS_ASSERT(pTestObj->parse());
  delete pTestObj;
  pTestObj = new domain::ParaverWindowConfig("prv2viz_cfgs/lines2_mpicmd.cfg");
  TS_ASSERT(pTestObj->parse());
  TS_ASSERT_EQUALS(pTestObj->get_displayWindows().size(), 2);
  TS_ASSERT_EQUALS(pTestObj->get_displayWindows()[0]->get_filteredEvents().size(), 1);
  TS_ASSERT_EQUALS(pTestObj->get_displayWindows()[1]->get_filteredEvents().size(), 3);
  delete pTestObj;
}

inline void TestParaverWindowConfig::testParaverWindowConfig() {
}

inline void TestParaverWindowConfig::testGet_dependencies() {
}

inline void TestParaverWindowConfig::testSet_dependencies() {
}

class TestParaverTraceConfig : public CxxTest::TestSuite {
  protected:
    domain::ParaverTraceConfig * pTestObj;


  public:
    inline void setUp();

    inline void tearDown();

    inline void testParaverTraceConfig();

    inline void testGet_pcfFile();

    inline void testParse();

    inline void testGet_level();

    inline void testGetEventType();

    inline void testGetNumValues();

    inline void testGetValue();

    inline void testAddState();

    inline void testAddStateColor();

    inline void testAddEventType();

    inline void testAddGradientColor();

    inline void testAddGradientNames();

    inline void testCommitEventTypes();

    inline void testAddValues();

    inline void testGet_eventTypes();

    inline void testGet_i_eventTypes();

};

inline void TestParaverTraceConfig::setUp() {
}

inline void TestParaverTraceConfig::tearDown() {
}

inline void TestParaverTraceConfig::testParaverTraceConfig() {
}

inline void TestParaverTraceConfig::testGet_pcfFile() {
}

inline void TestParaverTraceConfig::testParse() {
for(int i = 0; i < 1000; i++){
cout<<"It: "<<i<<" ";
  pTestObj = new domain::ParaverTraceConfig("cx.pcf");
//   delete pTestObj;
  pTestObj = new domain::ParaverTraceConfig("mar2.pcf");
//   delete pTestObj;
  pTestObj = new domain::ParaverTraceConfig("sampletrace.pcf");
//   delete pTestObj;
}
cout<<endl;
}

inline void TestParaverTraceConfig::testGet_level() {
  
  	//TS_WARN("Test testGet_level() not implemented");
}

inline void TestParaverTraceConfig::testGetEventType() {
  
  	//TS_WARN("Test testGetEventType() not implemented");
}

inline void TestParaverTraceConfig::testGetNumValues() {
  
  	//TS_WARN("Test testGetNumValues() not implemented");
}

inline void TestParaverTraceConfig::testGetValue() {
  
  	//TS_WARN("Test testGetValue() not implemented");
}

inline void TestParaverTraceConfig::testAddState() {
  
  	//TS_WARN("Test testAddState() not implemented");
}

inline void TestParaverTraceConfig::testAddStateColor() {
  
  	//TS_WARN("Test testAddStateColor() not implemented");
}

inline void TestParaverTraceConfig::testAddEventType() {
  
  	//TS_WARN("Test testAddEventType() not implemented");
}

inline void TestParaverTraceConfig::testAddGradientColor() {
  
  	//TS_WARN("Test testAddGradientColor() not implemented");
}

inline void TestParaverTraceConfig::testAddGradientNames() {
  
  	//TS_WARN("Test testAddGradientNames() not implemented");
}

inline void TestParaverTraceConfig::testCommitEventTypes() {
  
  	//TS_WARN("Test testCommitEventTypes() not implemented");
}

inline void TestParaverTraceConfig::testAddValues() {
  
  	//TS_WARN("Test testAddValues() not implemented");
}

inline void TestParaverTraceConfig::testGet_eventTypes() {
  
  	//TS_WARN("Test testGet_eventTypes() not implemented");
}

inline void TestParaverTraceConfig::testGet_i_eventTypes() {
  
  	//TS_WARN("Test testGet_i_eventTypes() not implemented");
}

class TestParamedir : public CxxTest::TestSuite {
  protected:
    domain::ParaverTraceConfig * pcfFile;

    domain::ParaverTrace * prvFile;

    domain::ParaverWindowConfig * cfg;


  public:
    domain::Paramedir * pTestObj;

    inline void setUp();

    inline void tearDown();

    inline void testParaverLauncher();

    inline void testSpawn();

    inline void testParamedir();

    inline void testAddCfg();

    inline void testAddCfgs();

    inline void testGet_cfgs();

};

inline void TestParamedir::setUp() {
//   pTestObj = new domain::Paramedir(*prvFile);
}

inline void TestParamedir::tearDown() {
  delete pTestObj;
}

inline void TestParamedir::testParaverLauncher() {
}

inline void TestParamedir::testSpawn() {
}

inline void TestParamedir::testParamedir() {
  
  	//TS_WARN("Test testParamedir() not implemented");
}

inline void TestParamedir::testAddCfg() {
  
  	//TS_WARN("Test testAddCfg() not implemented");
}

inline void TestParamedir::testAddCfgs() {
  
  	//TS_WARN("Test testAddCfgs() not implemented");
}

inline void TestParamedir::testGet_cfgs() {
  
  	//TS_WARN("Test testGet_cfgs() not implemented");
}

class TestParaverTrace : public CxxTest::TestSuite {
  public:
    domain::ParaverTrace * pTestObj;

    inline void setUp();

    inline void tearDown();

    inline void testParaverTrace();

    inline void testGet_lastTime();

    inline void testSet_lastTime();

    inline void testSetNumberOfApplications();

    inline void testAddTasksToAppl();

    inline void testSetNumberOfNodes();

    inline void testSetNumberOfTasksForAppl();

    inline void testGet_applications();

    inline void testSet_applications();

    inline void testGet_prvFile();

    inline void testSet_prvFile();

};

inline void TestParaverTrace::setUp() {
  string prvFile("sampletrace.prv");
  pTestObj = new domain::ParaverTrace(prvFile);
}

inline void TestParaverTrace::tearDown() {
  delete pTestObj;
}

inline void TestParaverTrace::testParaverTrace() {
}

inline void TestParaverTrace::testGet_lastTime() {
  
  	//TS_WARN("Test testGet_lastTime() not implemented");
}

inline void TestParaverTrace::testSet_lastTime() {
  
  	//TS_WARN("Test testSet_lastTime() not implemented");
}

inline void TestParaverTrace::testSetNumberOfApplications() {
  
  	//TS_WARN("Test testSetNumberOfApplications() not implemented");
}

inline void TestParaverTrace::testAddTasksToAppl() {
  
  	//TS_WARN("Test testAddTasksToAppl() not implemented");
}

inline void TestParaverTrace::testSetNumberOfNodes() {
  
  	//TS_WARN("Test testSetNumberOfNodes() not implemented");
}

inline void TestParaverTrace::testSetNumberOfTasksForAppl() {
  
  	//TS_WARN("Test testSetNumberOfTasksForAppl() not implemented");
}

inline void TestParaverTrace::testGet_applications() {
  
  	//TS_WARN("Test testGet_applications() not implemented");
}

inline void TestParaverTrace::testSet_applications() {
  
  	//TS_WARN("Test testSet_applications() not implemented");
}

inline void TestParaverTrace::testGet_prvFile() {
  
  	//TS_WARN("Test testGet_prvFile() not implemented");
}

inline void TestParaverTrace::testSet_prvFile() {
  
  	//TS_WARN("Test testSet_prvFile() not implemented");
}

class TestParaver : public CxxTest::TestSuite {
  public:
    domain::Paraver * pTestObj;

    inline void setUp();

    inline void tearDown();

    inline void testParaverInstance();

    inline void testStartParaver();

    inline void testSendSignalWithData();

    inline void testParaver();

    inline void testGetRunningPid();

    inline void testCreateParaloadFile();

};

inline void TestParaver::setUp() {
//   pTestObj = new domain::Paraver("/home/xavierp/FIB/PFCSuperior/src/src/test/cx.prv");
}

inline void TestParaver::tearDown() {
//   delete pTestObj;
}

inline void TestParaver::testParaverInstance() {
}

inline void TestParaver::testStartParaver() {
//   try{
//     pTestObj->startParaver();
//     TS_ASSERT(pTestObj->getRunningPid() != -1);
//   }catch(exception & e){
//     cout<<"\nException sent with: "<<e.what()<<endl;
//   }
  // 	//TS_WARN("Test testParaverInstance() not implemented");
}

inline void TestParaver::testSendSignalWithData() {
//   try{
//     pTestObj->sendSignalWithData("/home/xavierp/FIB/PFCSuperior/src/src/test/cx_user_functionNBursts.cfg", 0.0, 23.0);
//   }catch(exception & e){
//     cout<<"\nException sent with: "<<e.what()<<endl;
//   }
}

inline void TestParaver::testParaver() {
  
  	//TS_WARN("Test testParaver() not implemented");
}

inline void TestParaver::testGetRunningPid() {
  
  	//TS_WARN("Test testGetRunningPid() not implemented");
}

inline void TestParaver::testCreateParaloadFile() {
  
  	//TS_WARN("Test testCreateParaloadFile() not implemented");
}

class TestParaver2DOutput : public CxxTest::TestSuite {
  public:
    domain::Paraver2DOutput * pTestObj;

    inline void setUp();

    inline void tearDown();

    inline void testGetThreadValue();

    inline void testGetThreadColumns();

    inline void testGetTaskValue();

    inline void testGetTaskColumns();

    inline void testGetApplValue();

    inline void testGetApplColumns();

    inline void testGetWorkloadValue();

    inline void testGetCpuValue();

    inline void testGetNodeValue();

    inline void testGetSystemValue();

    inline void testGetThreadValues();

    inline void testGetThreadFullValues();

};

inline void TestParaver2DOutput::setUp() {
}

inline void TestParaver2DOutput::tearDown() {
}

inline void TestParaver2DOutput::testGetThreadValue() {
  string outputFile("2DOutput_THREAD.txt");
  TS_ASSERT_EQUALS(20, domain::Paraver2DOutput::getThreadValue(outputFile, 1, 1, 1, 1));
  TS_ASSERT_EQUALS(3, domain::Paraver2DOutput::getThreadValue(outputFile, 1, 2, 1, 20));
  TS_ASSERT_EQUALS(5, domain::Paraver2DOutput::getThreadValue(outputFile, 1, 3, 1, 27));
  TS_ASSERT_EQUALS(4, domain::Paraver2DOutput::getThreadValue(outputFile, 1, 4, 1, 41));
}

inline void TestParaver2DOutput::testGetThreadColumns() {
  vector<unsigned int> columns = domain::Paraver2DOutput::getThreadColumns("2DOutput_THREAD.txt", 1, 1, 1);
  TS_ASSERT_EQUALS((unsigned int) 49, columns.size());
}

inline void TestParaver2DOutput::testGetTaskValue() {
  string outputFile("2DOutput_TASK.txt");
  TS_ASSERT_EQUALS(344215, domain::Paraver2DOutput::getTaskValue(outputFile, 1, 1, 2));
  TS_ASSERT_EQUALS(59946792, domain::Paraver2DOutput::getTaskValue(outputFile, 1, 2, 1));
  TS_ASSERT_EQUALS(7630500, domain::Paraver2DOutput::getTaskValue(outputFile, 1, 3, 8));
  TS_ASSERT_EQUALS(30221214, domain::Paraver2DOutput::getTaskValue(outputFile, 1, 4, 15));
}

inline void TestParaver2DOutput::testGetTaskColumns() {
  vector<unsigned int> columns = domain::Paraver2DOutput::getTaskColumns("2DOutput_TASK.txt", 1, 1);
  TS_ASSERT_EQUALS((unsigned int) 8, columns.size());
}

inline void TestParaver2DOutput::testGetApplValue() {
  string outputFile("2DOutput_APPL.txt");
  TS_ASSERT_EQUALS(41855443, domain::Paraver2DOutput::getApplValue(outputFile, 1, 4));
  TS_ASSERT_EQUALS(339572, domain::Paraver2DOutput::getApplValue(outputFile, 1, 19));
}

inline void TestParaver2DOutput::testGetApplColumns() {
  vector<unsigned int> columns = domain::Paraver2DOutput::getApplColumns("2DOutput_APPL.txt", 1);
  TS_ASSERT_EQUALS((unsigned int) 40, columns.size());
}

inline void TestParaver2DOutput::testGetWorkloadValue() {
  string outputFile("2DOutput_WORKLOAD.txt");
  TS_ASSERT_EQUALS(41855443, domain::Paraver2DOutput::getWorkloadValue(outputFile, 4));
  TS_ASSERT_EQUALS(789929, domain::Paraver2DOutput::getWorkloadValue(outputFile, 23));
}

inline void TestParaver2DOutput::testGetCpuValue() {
  string outputFile("2DOutput_CPU.txt");
  TS_ASSERT_EQUALS(9111363, domain::Paraver2DOutput::getCpuValue(outputFile, 1, 1, 8));
  TS_ASSERT_EQUALS(7338790, domain::Paraver2DOutput::getCpuValue(outputFile, 2, 1, 8));
}

inline void TestParaver2DOutput::testGetNodeValue() {
  string outputFile("2DOutput_NODE.txt");
  TS_ASSERT_EQUALS(46215, domain::Paraver2DOutput::getNodeValue(outputFile, 1, 5));
  TS_ASSERT_EQUALS(13357440, domain::Paraver2DOutput::getNodeValue(outputFile, 4, 10));
}

inline void TestParaver2DOutput::testGetSystemValue() {
  string outputFile("2DOutput_SYSTEM.txt");
  TS_ASSERT_EQUALS(99715, domain::Paraver2DOutput::getSystemValue(outputFile, 6));
}

inline void TestParaver2DOutput::testGetThreadValues() {
  
  	//TS_WARN("Test testGetThreadValues() not implemented");
}

inline void TestParaver2DOutput::testGetThreadFullValues() {
  
  	//TS_WARN("Test testGetThreadFullValues() not implemented");
}

class TestMParaver2DOutput : public CxxTest::TestSuite {
  public:
    domain::MParaver2DOutput * pTestObj;

    inline void setUp();

    inline void tearDown();

    inline void testGetInstance();

    inline void testMParaver2DOutput();

    inline void testParseFile();

    inline void testGetThreadValue();

    inline void testGetThreadColumns();

    inline void testGetThreadValues();

    inline void testGetThreadFullValues();

    inline void testGetTaskValue();

    inline void testGetTaskColumns();

    inline void testGetApplValue();

    inline void testGetApplColumns();

    inline void testGetWorkloadValue();

    inline void testGetCpuValue();

    inline void testGetNodeValue();

    inline void testGetSystemValue();

    inline void testAddValue();

    inline void testGetTotalValue();

    inline void testGetAverageValue();

    inline void testGetMaximumValue();

    inline void testGetMinimumValue();

    inline void testGetStdevValue();

    inline void testGetCVValue();

    inline void testAddSummary();

    inline void testGetVersion();

};

inline void TestMParaver2DOutput::setUp() {
  pTestObj = new domain::MParaver2DOutput();
//   pTestObj = domain::MParaver2DOutput::getInstance();
}

inline void TestMParaver2DOutput::tearDown() {
  delete pTestObj;
}

inline void TestMParaver2DOutput::testGetInstance() {
  
//   	TS_WARN("Test testGetInstance() not implemented");
}

inline void TestMParaver2DOutput::testMParaver2DOutput() {
  
//   	TS_WARN("Test testMParaver2DOutput() not implemented");
}

inline void TestMParaver2DOutput::testParseFile() {
  pTestObj->parseFile("2DOutput_THREAD.txt");
  pTestObj->parseFile("2DOutput_THREAD3.txt");
}

inline void TestMParaver2DOutput::testGetThreadValue() {
  TS_ASSERT(20 == pTestObj->getThreadValue("2DOutput_THREAD.txt", 1, 1, 1, 1));
//   TS_ASSERT(666 == pTestObj->getThreadValue("2DOutput_THREAD2.txt", 1, 1, 1, 1));

  TS_ASSERT(19 == pTestObj->getThreadValue("2DOutput_THREAD.txt", 1, 1, 1, 2));
  TS_ASSERT(3 == pTestObj->getThreadValue("2DOutput_THREAD.txt", 1, 1, 1, 15));
  TS_ASSERT(32 == pTestObj->getThreadValue("2DOutput_THREAD.txt", 1, 1, 1, 49));

  TS_ASSERT(7 == pTestObj->getThreadValue("2DOutput_THREAD.txt", 1, 2, 1, 5));
  TS_ASSERT(5 == pTestObj->getThreadValue("2DOutput_THREAD.txt", 1, 2, 1, 30));

  TS_ASSERT(5 == pTestObj->getThreadValue("2DOutput_THREAD.txt", 1, 3, 1, 27));
}

inline void TestMParaver2DOutput::testGetThreadColumns() {
  vector<unsigned int> columns = pTestObj->getThreadColumns("2DOutput_THREAD.txt", 1, 1, 1);
  TS_ASSERT_EQUALS((unsigned int) 49, columns.size());
}

inline void TestMParaver2DOutput::testGetThreadValues() {
  vector<double> values = pTestObj->getThreadValues("2DOutput_THREAD.txt", 1, 1, 1);
  TS_ASSERT_EQUALS((double) 11, values.size());
}

inline void TestMParaver2DOutput::testGetThreadFullValues() {
  vector<double> values = pTestObj->getThreadFullValues("2DOutput_THREAD.txt", 1, 1, 1);
  TS_ASSERT_EQUALS((double) 49, values.size());
//   	TS_WARN("Test testGetThreadFullValues() not implemented");
}

inline void TestMParaver2DOutput::testGetTaskValue() {
  TS_ASSERT(69931914 == pTestObj->getTaskValue("2DOutput_TASK.txt", 1, 1, 1));
  TS_ASSERT(34038143 == pTestObj->getTaskValue("2DOutput_TASK.txt", 1, 1, 15));
  TS_ASSERT(27867285 == pTestObj->getTaskValue("2DOutput_TASK.txt", 1, 2, 15));
  TS_ASSERT(13357440 == pTestObj->getTaskValue("2DOutput_TASK.txt", 1, 4, 10));
}

inline void TestMParaver2DOutput::testGetTaskColumns() {
  vector<unsigned int> columns = pTestObj->getTaskColumns("2DOutput_TASK.txt", 1, 1);
  TS_ASSERT_EQUALS((unsigned int) 8, columns.size());
}

inline void TestMParaver2DOutput::testGetApplValue() {
}

inline void TestMParaver2DOutput::testGetApplColumns() {
  
//   	TS_WARN("Test testGetApplColumns() not implemented");
}

inline void TestMParaver2DOutput::testGetWorkloadValue() {
  
//   	TS_WARN("Test testGetWorkloadValue() not implemented");
}

inline void TestMParaver2DOutput::testGetCpuValue() {
  
//   	TS_WARN("Test testGetCpuValue() not implemented");
}

inline void TestMParaver2DOutput::testGetNodeValue() {
  
//   	TS_WARN("Test testGetNodeValue() not implemented");
}

inline void TestMParaver2DOutput::testGetSystemValue() {
  
//   	TS_WARN("Test testGetSystemValue() not implemented");
}

inline void TestMParaver2DOutput::testAddValue() {
  
//   	TS_WARN("Test testAddValue() not implemented");
}

inline void TestMParaver2DOutput::testGetTotalValue() {
  TS_ASSERT(22230788 == pTestObj->getTotalValue("2DOutput_TASK.txt", 2));
}

inline void TestMParaver2DOutput::testGetAverageValue() {
  TS_ASSERT(5557697 == pTestObj->getAverageValue("2DOutput_TASK.txt", 2));
}

inline void TestMParaver2DOutput::testGetMaximumValue() {
  TS_ASSERT(9111363 == pTestObj->getMaximumValue("2DOutput_TASK.txt", 8));
}

inline void TestMParaver2DOutput::testGetMinimumValue() {
  TS_ASSERT(46215 == pTestObj->getMinimumValue("2DOutput_TASK.txt", 5));
}

inline void TestMParaver2DOutput::testGetStdevValue() {
  TS_ASSERT(1215607.72 == pTestObj->getStdevValue("2DOutput_TASK.txt", 10));
}

inline void TestMParaver2DOutput::testGetCVValue() {
//   double rr = 0.49;
//   cout<<"\nVal: <"<<scientific<<pTestObj->getCVValue("2DOutput_TASK.txt", 13)<<"<"<<endl;
//   if(rr == pTestObj->getCVValue("2DOutput_TASK.txt", 13)) cout<<"\n SON IGUALESS!!!"<<endl;
//   else cout<<"COÑÑO!!!: "<<rr-pTestObj->getCVValue("2DOutput_TASK.txt", 13)<<endl;
//   TS_ASSERT(rr == pTestObj->getCVValue("2DOutput_TASK.txt", 13));
}

inline void TestMParaver2DOutput::testAddSummary() {
//   TS_WARN("Test testAddSummary() not implemented");
}

inline void TestMParaver2DOutput::testGetVersion() {
//   TS_WARN("Test testGetVersion() not implemented");
}

#endif
