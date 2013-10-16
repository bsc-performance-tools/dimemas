#ifndef LIBPARAVER_PARAVEREVENTTYPE_H
#define LIBPARAVER_PARAVEREVENTTYPE_H


#include <string>
using namespace std;
#include <vector>
using namespace std;
#include <iostream>
using namespace std;

namespace libparaver { class ParaverEventValue; } 

namespace libparaver {

class ParaverEventType {
  protected:
    bool eventValueIsMine;

    int key;

    string description;

    int color;

    vector<ParaverEventValue *> eventValues;


  public:
    ParaverEventType(int key, string description, int color);

    virtual ~ParaverEventType();

    void addValues(ParaverEventValue * value, bool isMine);

    inline const bool get_eventValueIsMine() const;

    void set_eventValueIsMine(bool value);

    inline const int get_key() const;

    void set_key(int value);

    inline const string get_description() const;

    void set_description(string value);

    inline const int get_color() const;

    void set_color(int value);

    inline const vector<ParaverEventValue *> & get_eventValues() const;

    void set_eventValues(vector<ParaverEventValue *> & value);

    string get_eventValueOfKey(unsigned int key);

    int get_eventKeyOfValue(const string value);

    friend ostream & operator<<(ostream & os, const ParaverEventType & ptraceConfig);

};
inline const bool ParaverEventType::get_eventValueIsMine() const {
  return eventValueIsMine;
}

inline const int ParaverEventType::get_key() const {
  return key;
}

inline const string ParaverEventType::get_description() const {
  return description;
}

inline const int ParaverEventType::get_color() const {
  return color;
}

inline const vector<ParaverEventValue *> & ParaverEventType::get_eventValues() const {
  return eventValues;
}


} // namespace libparaver
#endif
