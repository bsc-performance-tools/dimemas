#ifndef DOMAIN_PARAVERHEADERGRAMMAR_H
#define DOMAIN_PARAVERHEADERGRAMMAR_H


#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/assign_actor.hpp>
#include <boost/spirit/actor/increment_actor.hpp>
#include <boost/bind.hpp>

#include <boost/spirit/phoenix/primitives.hpp>
#include <boost/spirit/phoenix/operators.hpp>
using namespace boost;
using namespace boost::spirit;
using namespace phoenix;


namespace domain {

template<typename Actions>
class ParaverHeaderGrammar : public ::grammar<ParaverHeaderGrammar<Actions> > {
  protected:
    Actions & actions;

    mutable int int1;

    mutable int int2;

    mutable int int3;

    mutable int int4;


  public:
    ParaverHeaderGrammar(Actions & actions);

    template<typename ScannerT>
    class definition {
      protected:
        uint_parser<unsigned long long, 10, 1, -1> bigint_p;

        rule<ScannerT> prvheader;

        rule<ScannerT> date;

        rule<ScannerT> time;

        rule<ScannerT> lasttime;

        rule<ScannerT> units;

        rule<ScannerT> hard_architecture;

        rule<ScannerT> soft_architecture;

        rule<ScannerT> communicators;

        rule<ScannerT> nnodes;

        rule<ScannerT> cpus_by_node;

        rule<ScannerT> nappl;

        rule<ScannerT> ntasks;

        rule<ScannerT> config_tasks;

        rule<ScannerT> threads_by_node;


      public:
        definition(const ParaverHeaderGrammar<Actions> & self);

        rule<ScannerT> const & start() const;

    };
    
};
template<typename Actions>
ParaverHeaderGrammar<Actions>::ParaverHeaderGrammar(Actions & actions):actions(actions) {
}

template<typename Actions>
template<typename ScannerT>
ParaverHeaderGrammar<Actions>::definition<ScannerT>::definition(const ParaverHeaderGrammar<Actions> & self) {
  Actions & actions = self.actions;
  
  prvheader
     = str_p("#Paraver") >> '(' >> date >> str_p("at") >> time >> ')' >> ':'
     >> lasttime >> units >> ':' >> hard_architecture >> ':' >> soft_architecture >> *(',' >> communicators) >> end_p
     ;
  
  date
     = int_p >> '/' >> int_p >> '/' >> int_p
     ;
  
  time
     = int_p >> ':' >> int_p
     ;
  
  lasttime
     = bigint_p[bind(&Actions::set_lastTime, ref(actions), _1)]
     ;
  
  hard_architecture
     = nnodes >> *( '(' >> cpus_by_node >> *(',' >> cpus_by_node) >> ')' )
     ;
  
  soft_architecture
     = nappl[var(self.int1) = 0] >> +(
           ':' >> ntasks[var(self.int2) = 0]
           >> config_tasks
        )[increment_a(self.int1)]
     ;
  
  config_tasks
     = '(' >>  threads_by_node[increment_a(self.int2)] >> *(',' >> threads_by_node[increment_a(self.int2)]) >> ')'
     ;
  
  threads_by_node
     = (int_p[assign_a(self.int3)] >> ':'
     >> int_p[assign_a(self.int4)])[bind(&Actions::addTasksToAppl, ref(actions), ref(self.int1), ref(self.int2), ref(self.int3), ref(self.int4))]
     ;
  
  units = !('_' >> (str_p("ns") | str_p("us") | str_p("ms") | str_p("s") | str_p("mm") | str_p("h")) );
  nnodes = int_p[bind(&Actions::setNumberOfNodes, ref(actions), _1)];
  cpus_by_node = int_p;
  nappl = int_p[bind(&Actions::setNumberOfApplications, ref(actions), _1)];
  ntasks = int_p[bind(&Actions::setNumberOfTasksForAppl, ref(actions), ref(self.int1), _1)];
  communicators = int_p;
  
  BOOST_SPIRIT_DEBUG_NODE(prvheader);
  BOOST_SPIRIT_DEBUG_NODE(date);
  BOOST_SPIRIT_DEBUG_NODE(time);
  BOOST_SPIRIT_DEBUG_NODE(lasttime);
  BOOST_SPIRIT_DEBUG_NODE(units);
  BOOST_SPIRIT_DEBUG_NODE(hard_architecture);
  BOOST_SPIRIT_DEBUG_NODE(soft_architecture);
  BOOST_SPIRIT_DEBUG_NODE(communicators);
  BOOST_SPIRIT_DEBUG_NODE(nnodes);
  BOOST_SPIRIT_DEBUG_NODE(cpus_by_node);
  BOOST_SPIRIT_DEBUG_NODE(nappl);
  BOOST_SPIRIT_DEBUG_NODE(ntasks);
  BOOST_SPIRIT_DEBUG_NODE(config_tasks);
  BOOST_SPIRIT_DEBUG_NODE(threads_by_node);
}

template<typename Actions>
template<typename ScannerT>
rule<ScannerT> const & ParaverHeaderGrammar<Actions>::definition<ScannerT>::start() const {
  return prvheader;
}


} // namespace domain
#endif
