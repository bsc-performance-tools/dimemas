#ifndef LIBPARAVER_PARAVERSTATE_H
#define LIBPARAVER_PARAVERSTATE_H


#include <string>
using namespace std;

namespace libparaver {

class ParaverState {
  protected:
    int key;

    string value;


  public:
    ParaverState(int key, string value);

    virtual ~ParaverState();

    inline int get_key() const;

    inline string get_value() const;
};

inline int ParaverState::get_key() const
{
  return key;
}

inline string ParaverState::get_value() const
{
  return value;
}

} // namespace libparaver
#endif
