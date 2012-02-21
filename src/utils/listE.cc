
extern "C" {
#include "define.h"
#include "types.h"
#include "extern.h"
#include "list.h"
#include "subr.h"
}

#ifdef VENUS_ENABLED
#include "venusclient.h"
#endif
#include "listE.h"

/*
#include "cmessageheap.h"
#include "cmessageheap.cc"
*/

#include <map>
#include <vector>
using namespace std;

/*typedef long evCount_t;
typedef pair <dimemas_timer, evCount_t> evOrder;
bool operator< (evOrder const &a, evOrder const &b) {
	if (a.first < b.first) return true;
	if (a.first == b.first && a.second < b.second) return true;
	return false;
}*/

/*** OPTIMIZED EVENT QUEUE ****/
class EqueueC {
private:
	multimap <double, void *> evQ;

	multimap <double, void *>::iterator curr;
	multimap <double, void *>::iterator curr_end; // invalidated if "head" changes

	multimap <double, void *>::iterator it;
        //evCount_t lastIn;
	t_count Ecount;
public:
	EqueueC() {
		Ecount = 0;
		//lastIn = 0;
	}
	~EqueueC() {
		evQ.clear();
		Ecount = 0;
		//lastIn = 0;
	}
	inline void insert(void *e) {
		/*t_item new_element;
   		new_element.order.list_time = e->event_time;
   		new_element.content = (char *) e;
   		new_element->next = Q_NIL;
   		new_element->prev = Q_NIL;*/
		evQ.insert(pair<double, void *>((double)((struct t_event*)e)->event_time, e));
		Ecount++;
	}
	inline void insert(void *e, double order) {
		evQ.insert(pair<double, void *>((double)order, e));
		Ecount++;
	}
	void insert_before(void *e, double order) {
		it = evQ.find(order);
		evQ.insert(it, pair<double, void *>((double)order, e));
		Ecount++;
	}
	inline t_count count() {
		return Ecount;
	}
	inline void *top() {
  		void *e = E_NIL;
		if (evQ.begin() != evQ.end()) {
			e = (*evQ.begin()).second;
		}
		return e;
	}
	inline void *find_first(double order) {
  		void *e = E_NIL;
		it = evQ.find(order);
		//if (it == evQ.end()) // try to return something
		//	it = evQ.begin();
		if (it != evQ.end())
			e = (*it).second;
		return e;
	}
	inline void *head() {
  		void *e = E_NIL;
		this->curr = evQ.begin();
		if (this->curr != (this->curr_end = evQ.end())) {
			e = (*this->curr).second;
		}
		return e;
	}
	inline void *next() {
  		void *e = E_NIL;
		++this->curr;
		if (this->curr != this->curr_end) {
		    e = (*this->curr).second;
		}
		return e;
	}
	inline void *pop() {
  		void *e = E_NIL;
		if (evQ.begin() != evQ.end()) {
			e = (*evQ.begin()).second;
			evQ.erase(evQ.begin());
			Ecount--;
		}
		return e;
	}
	inline void *extract(void *ex) {
  		void *e = E_NIL;
		for (it = evQ.begin(); it != evQ.end(); ++it) {
			e = (*it).second;
			if (ex == e) {
				evQ.erase(it);
				Ecount--;
				return e;
			}
		}
		return e;
	}
};

static vector<EqueueC> Equeues;
static int lastQueueCreated = 0;

/*
 * Create empty queue
 */
void
create_Equeue(Equeue *q)
{
	*q = lastQueueCreated;
	lastQueueCreated++;
        if (Equeues.size() <= (lastQueueCreated - 1)) {
		Equeues.resize(*q+10000);
	}
	//printf("Created queue %d\n", *q);
	return ;
}

/*
 * Insert event preserving time sort
 */
void
insert_Eevent(Equeue *q, void *e)
{
	Equeues[*q].insert(e);
}

/*
 * Get number of elements in queue
 */
t_count
count_Equeue(Equeue *q)
{
   return (Equeues[*q].count());
}

/*
 * Query first element in event queue
 */
void *
head_Eevent(Equeue *q)
{
   return Equeues[*q].head();
}

/*
 * Query next element from event queue
 */
void *
next_Eevent(Equeue *q)
{
   return Equeues[*q].next();
}

/*
 * Query first element in the queue
 */
void *
head_Equeue(Equeue *q)
{
   return Equeues[*q].head();
}

/*
 * Query next element from the queue
 */
void *
next_Equeue(Equeue *q)
{
   return Equeues[*q].next();
}

/*
 * Ask first event element from queue
 */
void *
top_Eevent(Equeue *q)
{
   return Equeues[*q].top();
}

/*
 * Extract element from queue
 */
void
extract_from_Equeue(Equeue *q, void * content)
{
   Equeues[*q].extract(content);
}

/*
 * Get first event element from queue
 */
void *outFIFO_Eevent(Equeue *q)
{
  register struct t_event *e = E_NIL;

#ifdef VENUS_ENABLED
  if (VC_is_enabled())
  {
#ifdef USE_EQUEUE
    venus_outFIFO_event(q,e);
#endif
  }
#endif

  return (Equeues[*q].pop());
}

/*
 * Insert element in priority queue (1 preceeds 2)
 */
void insert_Equeue(Equeue *q, char *content, t_priority prio)
{
   //printf("List %d Inserting prio %f\n", *q, prio);
   Equeues[*q].insert((void *)content, (double)prio);
}

/*
 * Insert element in priority queue (1 preceeds 2) but if same priority
 * found, insert first
 */
void
insert_first_Equeue(Equeue *q, char *content, t_priority prio)
{
   //printf("List %d Inserting first prio %f\n", *q, prio);
   Equeues[*q].insert_before((void *)content, (double)prio);
}

/*
 * Query element with priority from queue
 */
char *
query_prio_Equeue(Equeue *q, t_priority prio)
{
   //printf("List %d Querying prio %f, %p\n", *q, prio, Equeues[*q].find_first((double)prio));
   return (char *)Equeues[*q].find_first((double)prio);
}

