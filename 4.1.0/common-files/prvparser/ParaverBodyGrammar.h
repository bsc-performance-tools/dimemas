#ifndef DOMAIN_BODYGRAMMAR_H
#define DOMAIN_BODYGRAMMAR_H

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

#include "ParaverRecord.h"

template<typename Actions>
class ParaverBodyGrammar : public ::grammar<ParaverBodyGrammar<Actions> > {
	protected:
	Actions & actions;

	mutable struct object_id_t OID;
	mutable struct state_t State;
	mutable struct multievent_t MultiEvent;
	mutable struct comm_t Comm;
	mutable struct event_t Event;
	mutable string Comment;
	mutable string Communicator;

	public:
	ParaverBodyGrammar(Actions & actions);

	template<typename ScannerT>
	class definition {
		protected:
		rule<ScannerT> prvbody_rule;
		rule<ScannerT> skip_rule;
		rule<ScannerT> state_rule;
		rule<ScannerT> event_rule;
		rule<ScannerT> communication_rule;
		rule<ScannerT> communicator_rule;
		
		uint_parser<unsigned long long, 10, 1, -1> big_uint;

		public:
		definition(const ParaverBodyGrammar<Actions> & self);
		rule<ScannerT> const & start() const;
	};
    
};
template<typename Actions>
ParaverBodyGrammar<Actions>::ParaverBodyGrammar(Actions & actions):actions(actions) {
}

template<typename Actions>
template<typename ScannerT>
ParaverBodyGrammar<Actions>::definition<ScannerT>::definition(const ParaverBodyGrammar<Actions> & self) {
	Actions & actions = self.actions;

	prvbody_rule = (skip_rule | state_rule | event_rule | communication_rule | communicator_rule ) >> end_p;

	skip_rule = ("#" >> (*anychar_p)[assign_a(self.Comment)])
			[bind(&Actions::processComment, ref(actions), ref(self.Comment))];
	
	communicator_rule = ("c" >> (*anychar_p)[assign_a(self.Communicator)])
			[bind(&Actions::processCommunicator, ref(actions), ref(self.Communicator))];

	state_rule = 
		("1:" >>
		uint_p[assign_a(self.State.ObjectID.cpu)] >> ':' >>
		uint_p[assign_a(self.State.ObjectID.ptask)] >> ':' >>
		uint_p[assign_a(self.State.ObjectID.task)] >> ':' >>
		uint_p[assign_a(self.State.ObjectID.thread)] >> ':' >>
		big_uint[assign_a(self.State.Begin_Timestamp)] >> ':' >>
		big_uint[assign_a(self.State.End_Timestamp)] >> ':' >>
		uint_p[assign_a(self.State.State)])
		[bind(&Actions::processState, ref(actions), ref(self.State))];

	event_rule = 
		(str_p("2:")[bind(&Actions::clearEvent, ref(actions), ref(self.MultiEvent))] >>
		uint_p[assign_a(self.MultiEvent.ObjectID.cpu)] >> ':' >>
		uint_p[assign_a(self.MultiEvent.ObjectID.ptask)] >> ':' >>
		uint_p[assign_a(self.MultiEvent.ObjectID.task)] >> ':' >>
		uint_p[assign_a(self.MultiEvent.ObjectID.thread)] >> ':' >>
		big_uint[assign_a(self.MultiEvent.Timestamp)] >>
		+(':' >>big_uint[assign_a(self.Event.Type)] >> ':' >> big_uint[assign_a(self.Event.Value)])
			[bind(&Actions::newEvent, ref(actions), ref(self.MultiEvent), ref(self.Event))])
		[bind(&Actions::processEvents, ref(actions), ref(self.MultiEvent))];

	communication_rule = 
		("3:" >>
		uint_p[assign_a(self.Comm.Send_ObjectID.cpu)] >> ':' >>
		uint_p[assign_a(self.Comm.Send_ObjectID.ptask)] >> ':' >>
		uint_p[assign_a(self.Comm.Send_ObjectID.task)] >> ':' >>
		uint_p[assign_a(self.Comm.Send_ObjectID.thread)] >> ':' >>
		big_uint[assign_a(self.Comm.Logical_Send)] >> ':' >>
		big_uint[assign_a(self.Comm.Physical_Send)] >> ':' >>
		uint_p[assign_a(self.Comm.Recv_ObjectID.cpu)] >> ':' >>
		uint_p[assign_a(self.Comm.Recv_ObjectID.ptask)] >> ':' >>
		uint_p[assign_a(self.Comm.Recv_ObjectID.task)] >> ':' >>
		uint_p[assign_a(self.Comm.Recv_ObjectID.thread)] >> ':' >>
		big_uint[assign_a(self.Comm.Logical_Recv)] >> ':' >>
		big_uint[assign_a(self.Comm.Physical_Recv)] >> ':' >>
		uint_p[assign_a(self.Comm.Size)] >> ':' >>
		uint_p[assign_a(self.Comm.Tag)])
		[bind(&Actions::processCommunication, ref(actions), ref(self.Comm))];

	BOOST_SPIRIT_DEBUG_NODE(prvbody_rule);
	BOOST_SPIRIT_DEBUG_NODE(skip_rule);
	BOOST_SPIRIT_DEBUG_NODE(state_rule);
	BOOST_SPIRIT_DEBUG_NODE(event_rule);
	BOOST_SPIRIT_DEBUG_NODE(communication_rule);
	BOOST_SPIRIT_DEBUG_NODE(communicator_rule);
}

template<typename Actions>
template<typename ScannerT>
rule<ScannerT> const & ParaverBodyGrammar<Actions>::definition<ScannerT>::start() const {
	return prvbody_rule;
}


} // namespace domain
#endif
