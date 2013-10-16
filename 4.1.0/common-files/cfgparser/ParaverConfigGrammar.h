#ifndef DOMAIN_PARAVERCONFIGGRAMMAR_H
#define DOMAIN_PARAVERCONFIGGRAMMAR_H


#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/assign_actor.hpp>
#include <boost/spirit/actor/increment_actor.hpp>
#include <boost/bind.hpp>

#include <boost/spirit/phoenix/primitives.hpp>
#include <boost/spirit/phoenix/operators.hpp>
using namespace boost;
using namespace boost::spirit;
using namespace phoenix;

#include <string>
using namespace std;

namespace domain {

template<typename Actions>
class ParaverConfigGrammar : public ::grammar<ParaverConfigGrammar<Actions> > {
  protected:
    Actions & actions;

    mutable string s_tmp1;


  public:
    ParaverConfigGrammar(Actions & actions);

    template<typename ScannerT>
    class definition {
      protected:
        rule<ScannerT> anychar_less_eol_p;

        rule<ScannerT> anychar_less_starter_p;

        rule<ScannerT> cfgHeader;

        rule<ScannerT> cfgVersion;

        rule<ScannerT> cfgNumWindows;

        rule<ScannerT> display_window;

        rule<ScannerT> display_header;

        rule<ScannerT> analyzer2d;

        rule<ScannerT> analyzer2d_header;

        rule<ScannerT> analyzer2d_vars;


      public:
        definition(const ParaverConfigGrammar<Actions> & self);

        rule<ScannerT> const & start() const;

    };
    
};
template<typename Actions>
ParaverConfigGrammar<Actions>::ParaverConfigGrammar(Actions & actions):actions(actions) {
}

template<typename Actions>
template<typename ScannerT>
ParaverConfigGrammar<Actions>::definition<ScannerT>::definition(const ParaverConfigGrammar<Actions> & self) {
  Actions & actions = self.actions;
  
  cfgHeader
     =
  //       cfgVersion >> cfgNumWindows >>
        +anychar_less_starter_p >>
        +(display_window[assign_a(self.s_tmp1)][bind(&Actions::addDisplayWindow, ref(actions), ref(self.s_tmp1))]
        | analyzer2d[assign_a(self.s_tmp1)][bind(&Actions::addAnalyzer, ref(actions), ref(self.s_tmp1))])
        >> end_p
     ;
  
  cfgVersion = *space_p >> str_p("ConfigFile.Version:") >> real_p >> *space_p;
  cfgNumWindows = *space_p >> str_p("ConfigFile.NumWindows:") >> int_p >> *space_p;
  
  display_window
     = display_header >> +(anychar_less_starter_p)
     ;
  
  display_header 
     = *space_p >> +ch_p('#') >> +space_p
        >> '<' >> str_p("NEW DISPLAYING WINDOW") >> +anychar_less_eol_p >> +space_p
        >> +ch_p('#') >> +space_p
     ;
  
  analyzer2d
     = analyzer2d_header >> +(anychar_less_starter_p)
     ;
  
  analyzer2d_header = ch_p('<') >> str_p("NEW ANALYZER2D") >> '>' >> +space_p;
  
  
  
  anychar_less_eol_p = anychar_p - eol_p;
  anychar_less_starter_p = anychar_p - str_p("< NEW") - str_p("##");
  
  BOOST_SPIRIT_DEBUG_NODE(cfgHeader);
  BOOST_SPIRIT_DEBUG_NODE(cfgVersion);
  BOOST_SPIRIT_DEBUG_NODE(cfgNumWindows);
  BOOST_SPIRIT_DEBUG_NODE(display_window);
  BOOST_SPIRIT_DEBUG_NODE(display_header);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d_header);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d_vars);
  BOOST_SPIRIT_DEBUG_NODE(anychar_less_eol_p);
}

template<typename Actions>
template<typename ScannerT>
rule<ScannerT> const & ParaverConfigGrammar<Actions>::definition<ScannerT>::start() const {
  return cfgHeader;
}


} // namespace domain
#endif
