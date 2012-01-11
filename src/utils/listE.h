#ifndef LIST_E_HEADER
#define LIST_E_HEADER

/*** OPTIMIZED EVENT QUEUE ****/
#ifdef __cplusplus
extern "C" {
#endif

/*typedef int Equeue;*/
/*
 * Create empty queue
 */
void
create_Equeue(Equeue *q);

/*
 * Insert event preserving time sort
 */
void
insert_Eevent(Equeue *q, void *e);

/*
 * Get number of elements in queue
 */
t_count
count_Equeue(Equeue *q);

/*
 * Query first element in event queue
 */
void *
head_Eevent(Equeue *q);

/*
 * Query next element from event queue
 */
void *
next_Eevent(Equeue *q);

/*
 * Query first element in the queue
 */
void *
head_Equeue(Equeue *q);

/*
 * Query next element from the queue
 */
void *
next_Equeue(Equeue *q);

/*
 * Ask first event element from queue
 */
void *
top_Eevent(Equeue *q);

void
extract_from_Equeue(Equeue *q, void * content);

/*
 * Get first event element from queue
 */
void*
outFIFO_Eevent(Equeue *q);

/*
 * Insert element in priority queue (1 preceeds 2)
 */
void
insert_Equeue(Equeue *q, char *content, t_priority prio);

/*
 * Insert element in priority queue (1 preceeds 2) but if same priority
 * found, insert first
 */
void
insert_first_Equeue(Equeue *q, char *content, t_priority prio);

/*
 * Query element with priority from queue
 */
char *
query_prio_Equeue(Equeue *q, t_priority prio);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
