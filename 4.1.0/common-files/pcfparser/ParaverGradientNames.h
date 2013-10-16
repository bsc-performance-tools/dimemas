#ifndef LIBPARAVER_PARAVERGRADIENTNAMES_H
#define LIBPARAVER_PARAVERGRADIENTNAMES_H


#include <string>
using namespace std;

namespace libparaver {

class ParaverGradientNames {
  protected:
    int key;

    string value;


  public:
    ParaverGradientNames(int key, string value);

    virtual ~ParaverGradientNames();

};

} // namespace libparaver
#endif
