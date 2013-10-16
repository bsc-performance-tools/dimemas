#ifndef LIBPARAVER_PARAVERTRACECONFIG_H
#define LIBPARAVER_PARAVERTRACECONFIG_H


//#define BOOST_SPIRIT_DEBUG
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_increment_actor.hpp>
#include <boost/bind.hpp>
#include <boost/spirit/include/phoenix1_primitives.hpp>
#include <boost/spirit/include/phoenix1_operators.hpp>
#include <boost/spirit/include/classic_insert_at_actor.hpp>
#include <boost/spirit/include/classic_file_iterator.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_functor_parser.hpp>
using BOOST_SPIRIT_CLASSIC_NS::grammar;
using BOOST_SPIRIT_CLASSIC_NS::rule;
using BOOST_SPIRIT_CLASSIC_NS::space_p;
using BOOST_SPIRIT_CLASSIC_NS::end_p;
using BOOST_SPIRIT_CLASSIC_NS::anychar_p;
using BOOST_SPIRIT_CLASSIC_NS::eol_p;
using BOOST_SPIRIT_CLASSIC_NS::lexeme_d;
using BOOST_SPIRIT_CLASSIC_NS::alnum_p;
using BOOST_SPIRIT_CLASSIC_NS::alpha_p;
using BOOST_SPIRIT_CLASSIC_NS::int_p;
using BOOST_SPIRIT_CLASSIC_NS::real_p;
using BOOST_SPIRIT_CLASSIC_NS::str_p;
using BOOST_SPIRIT_CLASSIC_NS::ch_p;
using BOOST_SPIRIT_CLASSIC_NS::uint_parser;
using BOOST_SPIRIT_CLASSIC_NS::functor_parser;
using BOOST_SPIRIT_CLASSIC_NS::assign_a;
using BOOST_SPIRIT_CLASSIC_NS::increment_a;
using BOOST_SPIRIT_CLASSIC_NS::push_back_a;
using BOOST_SPIRIT_CLASSIC_NS::parse_info;
using BOOST_SPIRIT_CLASSIC_NS::file_iterator;
using BOOST_SPIRIT_CLASSIC_NS::scanner;
using boost::ref;
using boost::bind;
using phoenix::var;

#include <string>
using namespace std;
#include <vector>
using namespace std;
#include <iostream>
using namespace std;

namespace libparaver { class ParaverEventValue; } 
namespace libparaver { class ParaverState; } 
namespace libparaver { class ParaverStatesColor; } 
namespace libparaver { class ParaverEventType; } 
namespace libparaver { class ParaverGradientColor; } 
namespace libparaver { class ParaverGradientNames; } 
namespace libparaver { class LibException; } 
namespace libparaver { template<typename Actions> class ParaverTraceConfigGrammar; } 

namespace libparaver {

class ParaverTraceConfig {
  public:
    typedef char char_t;

    typedef file_iterator<char_t> iterator_t;

    typedef scanner<iterator_t> scanner_t;

    typedef rule<scanner_t> rule_t;


  protected:
    string pcfFile;


  public:
    string level;

    string units;

    int look_back;

    int speed;

    string flag_icons;

    int num_of_state_colors;

    int ymax_scale;

    // vector<string> level_func;  ?
    string default_task_semantic_func;
    string default_thread_semantic_func;

  protected:
    ParaverEventValue * eventValue;

    vector<int> i_eventTypes;

    vector<ParaverState *> states;

    vector<ParaverStatesColor *> statesColor;

    vector<ParaverEventType *> eventTypes;

    vector<ParaverEventType *> currentEvents;

    vector<ParaverGradientColor *> gradientColors;

    vector<ParaverGradientNames *> gradientNames;


  public:
    ParaverTraceConfig(string pcfFile);

    virtual ~ParaverTraceConfig();

    inline const string get_pcfFile() const;

    inline const string get_level() const;

    inline const string get_default_thread_semantic_func() const;
    inline const string get_default_task_semantic_func() const;

    void parse();

    ParaverEventType * getEventType(const string eventType);

    ParaverEventType * getEventType(int eventType);

    int getNumValues();

    int getValue();

    void addState(const int key, const string value);

    void addStateColor(const int key, const int red, const int green, int blue);

    void addEventType(const int color, const int key, const string description);

    void addGradientColor(const int key, const int red, const int green, const int blue);

    void addGradientNames(const int key, const string value);

    void commitEventTypes();

    void addValues(const int key, const string value);

    inline const vector<ParaverEventType *> & get_eventTypes() const;

    inline const vector<int> & get_i_eventTypes() const;

    inline const vector<ParaverStatesColor *> & get_statesColor() const;

    inline const vector<ParaverGradientColor *> & get_gradientColors() const;

    inline const vector<ParaverState *> & get_states() const;

    friend ostream & operator<<(ostream & os, const ParaverTraceConfig & ptraceConfig);

};
inline const string ParaverTraceConfig::get_pcfFile() const {
  return pcfFile;
}

inline const string ParaverTraceConfig::get_level() const {
  return level;
}

inline const string ParaverTraceConfig::get_default_thread_semantic_func() const {
  return default_thread_semantic_func;
}

inline const string ParaverTraceConfig::get_default_task_semantic_func() const {
  return default_task_semantic_func;
}

inline const vector<ParaverEventType *> & ParaverTraceConfig::get_eventTypes() const {
  return eventTypes;
}

inline const vector<int> & ParaverTraceConfig::get_i_eventTypes() const {
  return i_eventTypes;
}

inline const vector<ParaverStatesColor *> & ParaverTraceConfig::get_statesColor() const {
  return statesColor;
}

inline const vector<ParaverGradientColor *> & ParaverTraceConfig::get_gradientColors() const {
  return gradientColors;
}

inline const vector<ParaverState *> & ParaverTraceConfig::get_states() const {
  return states;
}

} // namespace libparaver
#endif
