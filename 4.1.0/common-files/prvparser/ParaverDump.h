#ifndef PARAVER_DUMP_H_INCLUDED
#define PARAVER_DUMP_H_INCLUDED

namespace domain {

class ParaverDump
{
	public:
	static void DumpState (struct state_t &s, fstream &o)
	{
		o << "1:" << s.ObjectID.cpu << ":" << s.ObjectID.ptask << ":" << s.ObjectID.task << ":" << s.ObjectID.thread << ":" << s.Begin_Timestamp << ":" << s.End_Timestamp << ":" << s.State << endl;
	}

	static void DumpEvent (struct singleevent_t &e, fstream &o)
	{
		o << "2:" << e.ObjectID.cpu << ":" << e.ObjectID.ptask << ":" << e.ObjectID.task << ":" << e.ObjectID.thread << ":" << e.Timestamp << ":" << e.event.Type << ":" << e.event.Value << endl;
	}

	static void DumpMultiEvent (struct multievent_t &e, fstream &o)
	{
		if (e.events.size() > 0)
		{
			o << "2:" << e.ObjectID.cpu << ":" << e.ObjectID.ptask << ":" << e.ObjectID.task << ":" << e.ObjectID.thread << ":" << e.Timestamp;
			for (vector<struct event_t>::iterator i = e.events.begin(); i != e.events.end(); i++)
				o << ":" << (*i).Type << ":" << (*i).Value;
			o << endl;
		}
	}

	static void DumpCommunication (struct comm_t &c, fstream &o)
	{
		o << "3:" << c.Send_ObjectID.cpu << ":" << c.Send_ObjectID.ptask << ":" << c.Send_ObjectID.task << ":" << c.Send_ObjectID.thread << ":" << c.Logical_Send << ":" << c.Physical_Send;
		o << ":"  << c.Recv_ObjectID.cpu << ":" << c.Recv_ObjectID.ptask << ":" << c.Recv_ObjectID.task << ":" << c.Recv_ObjectID.thread << ":" << c.Logical_Recv << ":" << c.Physical_Recv;
		o << ":"  << c.Size << ":" << c.Tag << endl;
	}
};

} /* namespace */

#endif /* PARAVER_DUMP_H_INCLUDED */
