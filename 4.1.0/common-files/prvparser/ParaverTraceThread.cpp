
#include "ParaverTraceThread.h"

namespace domain {

unsigned int ParaverTraceThread::globalKey=0;

ParaverTraceThread::ParaverTraceThread() {
  particularKey = globalKey++;
}

ParaverTraceThread::ParaverTraceThread(unsigned int & key):key(key) {
  particularKey = globalKey++;
}

ParaverTraceThread::~ParaverTraceThread() {
}

void ParaverTraceThread::set_key(unsigned int value) {
  key = value;
}


} // namespace domain
