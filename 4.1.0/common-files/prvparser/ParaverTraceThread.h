#ifndef DOMAIN_PARAVERTRACETHREAD_H
#define DOMAIN_PARAVERTRACETHREAD_H


namespace domain {

class ParaverTraceThread {
  protected:
    static unsigned int globalKey;

    unsigned int key;

    unsigned int particularKey;


  public:
    ParaverTraceThread();

    ParaverTraceThread(unsigned int & key);

    virtual ~ParaverTraceThread();

    inline const unsigned int get_key() const;

    void set_key(unsigned int value);

};
inline const unsigned int ParaverTraceThread::get_key() const {
  return key;
}


} // namespace domain
#endif
