#ifndef _LIBEXCEPTION_H
#define _LIBEXCEPTION_H


#include <exception>
using namespace std;

#include <string>
using namespace std;

#include <sstream>
using namespace std;

class LibException : public std::exception {
  protected:
    string reason;

    string stack;


  public:
    LibException();

    LibException(string file, int line, string message);

    virtual ~LibException() throw();

    virtual const char * what() const throw();

    virtual const char * where() const throw();

};
#endif
