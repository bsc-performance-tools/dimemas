#ifndef PARAVER_RECORD_H_INCLUDED
#define PARAVER_RECORD_H_INCLUDED

#include <vector>

using namespace std;

namespace domain {

struct object_id_t {
	unsigned int cpu;
	unsigned int ptask;
	unsigned int task;
	unsigned int thread;
};

struct event_t {
	unsigned long long Type;
	unsigned long long Value;
};

struct multievent_t {
	struct object_id_t ObjectID;
	unsigned long long Timestamp;
	vector<struct event_t> events;
};

struct singleevent_t {
	struct object_id_t ObjectID;
	unsigned long long Timestamp;
	event_t event;
};

struct state_t {
	struct object_id_t ObjectID;
	unsigned long long Begin_Timestamp;
	unsigned long long End_Timestamp;
	unsigned int State;
};

struct comm_t {
	struct object_id_t Send_ObjectID;
	struct object_id_t Recv_ObjectID;
	unsigned long long Logical_Send;
	unsigned long long Physical_Send;
	unsigned long long Logical_Recv;
	unsigned long long Physical_Recv;
	unsigned int Tag;
	unsigned int Size;
};

} /* namespace */

#endif
