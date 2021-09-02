#include <assert.h>
#include <communic.h>
#include <deadlock_analysis.h>
#include <file_data_access.h>
#include <graph.h>

struct dependency_queue *_graph_map;
int _nodes;

FILE *log_file;
int file_index;
char *log_file_name;

/*
 * Initialize the graph
 */
void GRAPH_init( int nodes )
{
  _nodes     = nodes;
  _graph_map = (struct dependency_queue *)malloc( sizeof( struct dependency_queue ) * _nodes );

  int i;
  for ( i = 0; i < _nodes; ++i )
  {
    _graph_map[ i ].num_deps  = 0;
    _graph_map[ i ].first_dep = NULL;
    _graph_map[ i ].last_dep  = NULL;
  }

#if DEBUG
  file_index    = 0;
  log_file_name = malloc( sizeof( char ) * 13 );
  sprintf( log_file_name, "log.%04d.txt", file_index );
  log_file = fopen( log_file_name, "w" );
#endif // DEBUG
}

/*
 * This method creates a new dependency arc between node 'from_taskid'
 * to node 'to_taskid'. This new arc could be a strong or a soft dependency
 * depending on 'type'. Also 'thread' is required in order to get
 * information about the dependency.
 */
struct dependency *GRAPH_add_dependency( int from_taskid, int to_taskid, int type, struct t_thread *thread )
{
  struct dependency *new_dep = (struct dependency *)malloc( sizeof( struct dependency ) );

  new_dep->from       = from_taskid;
  new_dep->to         = to_taskid;
  double current_time = 0.0;
  new_dep->time       = current_time;

  new_dep->communicator = NO_COMM;
  new_dep->global_op    = NO_GLOP;
  new_dep->file_offset  = DAP_get_offset( thread->task->Ptask->Ptaskid, thread->task->taskid, thread->threadid );
#if DEBUG
  new_dep->p2p_tag         = 0;
  new_dep->p2p_communic_id = 0;
#endif
  switch ( type )
  {
    case GLOP_DEP:
      new_dep->global_op = thread->action->desc.global_op.glop_id;

      struct t_communicator *communicator;
      communicator          = locate_communicator( &thread->task->Ptask->Communicator, thread->action->desc.global_op.comm_id );
      new_dep->communicator = communicator;
    case SOFT_DEP:
    case STRG_DEP:
      new_dep->action = thread->action->action;
      new_dep->type   = type;

      switch ( thread->action->action )
      {
        case SEND:
        {
          new_dep->p2p_tag         = thread->action->desc.send.mess_tag;
          new_dep->p2p_communic_id = thread->action->desc.send.communic_id;
          break;
        }
        case RECV:
        case IRECV:
        case WAIT:
        {
          new_dep->p2p_tag         = thread->action->desc.recv.mess_tag;
          new_dep->p2p_communic_id = thread->action->desc.recv.communic_id;
          break;
        }
      }
#if DEBUG

      fprintf( log_file,
               "%20f:+:%d:%d:%d:%s:%d:%d\n",
               current_time,
               from_taskid,
               to_taskid,
               type,
               get_name_of_action( thread->action->action ),
               new_dep->p2p_tag,
               new_dep->p2p_communic_id );
#endif // DEBUG
      break;
    default:
      assert( FALSE );
  }


  // Adding the new node to the graph
  //
  if ( !_graph_map[ from_taskid ].first_dep ) /* I'm the first one */
  {
    new_dep->previous                   = NULL;
    new_dep->next                       = NULL;
    _graph_map[ from_taskid ].first_dep = new_dep;
    _graph_map[ from_taskid ].last_dep  = new_dep;
    _graph_map[ from_taskid ].num_deps++;
  }
  else
  {
    struct dependency *d;
    for ( d = _graph_map[ from_taskid ].first_dep; d != NULL; d = d->next )
    {
      if ( d->to >= to_taskid )
      {
        new_dep->previous = d->previous;
        new_dep->next     = d;

        if ( d->previous != NULL )
          d->previous->next = new_dep;
        else /* I'm the first one */
          _graph_map[ from_taskid ].first_dep = new_dep;

        d->previous = new_dep;

        _graph_map[ from_taskid ].num_deps++;

        return new_dep;
      }
    }

    new_dep->next     = NULL; /* I'm the last one */
    new_dep->previous = _graph_map[ from_taskid ].last_dep;

    _graph_map[ from_taskid ].last_dep->next = new_dep;
    _graph_map[ from_taskid ].last_dep       = new_dep;

    _graph_map[ from_taskid ].num_deps++;
  }
  return new_dep;
}

/*
 * This method remove the dependency d from the
 * graph
 */

void GRAPH_remove_dependency_p( struct dependency *d )
{
  int from_taskid = d->from;
#if DEBUG
  fprintf( log_file,
           "%20f:-:%d:%d:%d:%s:%d:%d\n",
           current_time,
           from_taskid,
           d->to,
           d->type,
           get_name_of_action( d->action ),
           d->p2p_tag,
           d->p2p_communic_id );
#endif // DEBUG

  if ( d->previous )
    d->previous->next = d->next;
  else
    _graph_map[ from_taskid ].first_dep = d->next; // If d is the last one, d->next must be null.

  if ( d->next )
    d->next->previous = d->previous;
  else
    _graph_map[ from_taskid ].last_dep = d->previous;

  free( d );
  _graph_map[ from_taskid ].num_deps--;
  assert( _graph_map[ from_taskid ].num_deps >= 0 );
}

/**
 * This method looks for a cycle in the graph where root is
 * involved
 */
t_boolean GRAPH_look_for_cycle( int root, int *chain, int *chain_size )
{
  t_boolean res = FALSE;
  struct dependency *d;

  *chain_size = 1;

  for ( d = _graph_map[ root ].first_dep; d != NULL; d = d->next )
  {
    chain[ 0 ] = root;

    if ( d->type == STRG_DEP || d->type == GLOP_DEP )
    {
      int my_chain_size = *chain_size;
      res               = __look_for_cycle( d->to, root, chain, &my_chain_size );
      if ( res )
      {
        *chain_size = my_chain_size;
        return TRUE;
      }
    }
  }
  return FALSE;
}

t_boolean __look_for_cycle( int from, int root, int *dep_chain_queue, int *deepness )
{
  dep_chain_queue[ ( *deepness )++ ] = from;
  if ( from == root )
    return TRUE;

  if ( _graph_map[ from ].num_deps == 0 )
    return FALSE;

  int res = 0;
  struct dependency *d;
  for ( d = _graph_map[ from ].first_dep; d != NULL; d = d->next )
  {
    if ( d->type == STRG_DEP || d->type == GLOP_DEP )
    {
      int my_deepness = *deepness;
      res             = __look_for_cycle( d->to, root, dep_chain_queue, &my_deepness );
      if ( res )
      {
        *deepness = my_deepness;
        return TRUE;
      }
    }
  }
  return FALSE;
}

/**
 * Get the first dependency that fulfill with the conditions.
 * mess_tag could be TAG_ALL
 * communic_id could be COMMUNIC_ALL
 */
struct dependency *GRAPH_get_dependency( int from_taskid, int to_taskid, int type, int action, int mess_tag, int communic_id )
{
  struct dependency *d;
  for ( d = _graph_map[ from_taskid ].first_dep; d != NULL; d = d->next )
  {
    if ( d->to == to_taskid && ( d->type & type ) != 0 && ( d->action & action ) != 0 )
    {
      if ( action == SEND || action == RECV || action == WAIT || action == IRECV || action == SOFT_ACTION_RESOLVED )
      // if (action | SEND | RECV | WAIT | IRECV | SOFT_ACTION_RESOLVED != 0) // No deberian ser ANDs???
      {
        if ( ( d->p2p_communic_id == communic_id && d->p2p_tag == mess_tag ) || ( mess_tag == TAG_ALL && communic_id == COMMUNIC_ID_ALL ) )
          return d;
        else
          continue;
      }
      else
        return d;
    }
    else if ( d->to > to_taskid )
    {
      return NULL;
    }
  }
  return NULL;
}

/*
 * Reset the graph, i.e. remove all the nodes and arcs
 */
void GRAPH_reset()
{
  int i;

  for ( i = 0; i < _nodes; ++i )
  {
    struct dependency *current;
    struct dependency *next;

    current = _graph_map[ i ].first_dep;
    while ( current != NULL )
    {
      next = current->next;
      free( current );
      current = next;
    }

    _graph_map[ i ].first_dep = NULL;
    _graph_map[ i ].last_dep  = NULL;
    _graph_map[ i ].num_deps  = 0;
  }

#if DEBUG
  fclose( log_file );
  free( log_file_name );

  log_file_name = malloc( sizeof( char ) * 13 );
  sprintf( log_file_name, "log.%04d.txt", ++file_index );
  log_file = fopen( log_file_name, "w" );
#endif // DEBUG
}


/*
 * Get ALL the dependencies that fulfill with the parameters
 * - A pointer to an array with size 'dep_set_size' will be returned.
 * - mess_tag could be MESS_ALL
 * - communic_id could be COMMUNIC_ID_ALL
 * - to_taskid could be GRAPH_TO_ALL
 * - action could be GRAPH_ALL_ACTIONS
 */
struct dependency **GRAPH_get_dependencies( int from_taskid, int to_taskid, int type, int action, int mess_tag, int communic_id, int *dep_set_size )
{
  int res_index           = 0;
  unsigned int res_size   = 10;
  struct dependency **res = (struct dependency **)malloc( sizeof( struct dependency * ) * res_size );

  struct dependency *d;
  for ( d = _graph_map[ from_taskid ].first_dep; d != NULL; d = d->next )
  {
    if ( ( d->to == to_taskid || to_taskid == GRAPH_TO_ALL ) && ( d->type & type ) != 0 &&
         ( ( d->action & action ) != 0 || action == GRAPH_ALL_ACTIONS ) )
    {
      if ( action == SEND || action == RECV || action == WAIT || action == IRECV || action == SOFT_ACTION_RESOLVED )
      // if (action | SEND | RECV | WAIT | IRECV | SOFT_ACTION_RESOLVED != 0) // No deberian ser ANDs???
      {
        if ( ( d->p2p_communic_id == communic_id && d->p2p_tag == mess_tag ) || ( mess_tag == TAG_ALL && communic_id == COMMUNIC_ID_ALL ) )
          res[ res_index++ ] = d;
        else
          continue;
      }
      else
        res[ res_index++ ] = d;

      if ( res_index == res_size )
      {
        res_size *= 2;
        res = realloc( (void *)res, sizeof( struct dependency * ) * res_size );
        assert( res != NULL );
      }
    }
  }

  *dep_set_size = res_index;

  if ( res_index == 0 )
  {
    free( res );
    return NULL;
  }
  return res;
}

void GRAPH_change_action( struct dependency *d, int new_action )
{
  d->action = new_action;

#if DEBUG
  fprintf( log_file,
           "%20f:~:%d:%d:%d:%s:%d:%d\n",
           current_time,
           d->from,
           d->to,
           d->type,
           get_name_of_action( d->action ),
           d->p2p_tag,
           d->p2p_communic_id );
#endif // DEBUG
}

void GRAPH_print_state( int i )
{
  struct dependency *d;

#if DEBUG
  fprintf( log_file, "From %d -> \n\t", i );
#endif // DEBUG

  if ( _graph_map[ i ].num_deps > 0 )
  {
    int last_to = _graph_map[ i ].first_dep->to;
    for ( d = _graph_map[ i ].first_dep; d != NULL; d = d->next )
    {
      if ( last_to != d->to )
      {
#if DEBUG
        fprintf( log_file, "\n\t" );
//#else
// printf("|");
#endif
        last_to = d->to;
      }
#if DEBUG
      fprintf( log_file, "%f [->%d] %s(%d):%d:%d \n\t", d->time, d->to, get_name_of_action( d->action ), d->type, d->p2p_tag, d->p2p_communic_id );
//#else
// printf(" %d[%s(%d)] @ %f ", d->to, get_name_of_action(d->action), d->type, d->time/1e6);
#endif
    }
  }
#if DEBUG
  fprintf( log_file, "\n" );
//#else
// printf("\n");
#endif // DEBUG
}

void GRAPH_fini()
{
#if DEBUG
  fclose( log_file );
  free( log_file_name );
#endif // DEBUG
}

void GRAPH_debug( int from_taskid, int to_taskid, char *msg )
{
#if DEBUG
  fprintf( log_file, "MSG: [%d->%d] %s\n", from_taskid, to_taskid, msg );
//#else
// printf("MSG: [%d->%d] %s\n", from_taskid, to_taskid, msg);
#endif
}

void GRAPH_debug_msg( char *msg )
{
#if DEBUG
  fprintf( log_file, "%s", msg );
//#else
// printf("%s", msg);
#endif
}
