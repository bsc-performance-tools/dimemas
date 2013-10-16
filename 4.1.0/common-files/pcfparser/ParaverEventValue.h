#ifndef LIBPARAVER_PARAVEREVENTVALUE_H
#define LIBPARAVER_PARAVEREVENTVALUE_H


#include <string>
using namespace std;
#include <iostream>
using namespace std;

namespace libparaver {

class ParaverEventValue {
  protected:
    unsigned int key;

    string value;


  public:
    ParaverEventValue(int key, string value);

    virtual ~ParaverEventValue();

    inline const unsigned int get_key() const;

    void set_key(unsigned int value);

    inline const string get_value() const;

    void set_value(string new_value);

    friend ostream & operator<<(ostream & os, const ParaverEventValue & ptraceConfig);

};
inline const unsigned int ParaverEventValue::get_key() const {
  return key;
}

inline const string ParaverEventValue::get_value() const {
  return value;
}


} // namespace libparaver
#endif
