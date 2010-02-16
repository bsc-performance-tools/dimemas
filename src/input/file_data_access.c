/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                  Dimemas                                  *
 *       Simulation tool for the parametric analysis of the behaviour of     *
 *       message-passing applications on a configurable parallel platform    *
 *                                                                           * 
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL::                  $:  File
  $Rev::                  $:  Revision of last commit
  $Author::               $:  Author of last commit
  $Date::                 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "file_data_access.h"
#include <stdio.h>
#include "types.h"
#include "list.h"
#include <sys/stat.h>

/* JGG: to use 'USE_RENDEZ_VOUS' macro */
#include "define.h"
#include "extern.h"

// Definition and configuartion parameters of data access api 

// Configuration parameter of maximmum buffer size for header, definition and acction register reading
#ifndef DATA_ACCESS_MAX_BUFFER_SIZE
#define DATA_ACCESS_MAX_BUFFER_SIZE 10000000
#endif

// Configure standard buffer size for header, definition and accition register reading
#ifndef DATA_ACCESS_STANDARD_BUFFER_SIZE
#define DATA_ACCESS_STANDARD_BUFFER_SIZE 40000
#endif

// Configure standard filed length 
#ifndef DATA_ACCESS_STANDARD_FIELD_SIZE
#define DATA_ACCESS_STANDARD_FIELD_SIZE 100000
#endif

// Defines prefix of format in trace file
#ifndef DATA_ACCESS_DIMEMAS_HELLO_SIGN
#define DATA_ACCESS_DIMEMAS_HELLO_SIGN "#DIMEMAS"
#endif

// Configures maximim number of applications supported by API
#ifndef DATA_ACCESS_MAX_NUM_APP
#define DATA_ACCESS_MAX_NUM_APP 100
#endif

// Configures maximum error message length
#ifndef DATA_ACCESS_MAX_ERROR_LENGTH
#define DATA_ACCESS_MAX_ERROR_LENGTH 100
#endif

// Configures maximum number of streams that can be extra reservaded for each application
// This option is deprecated due to maximum number of streams will always be fixed to
// total number of threads. Changing this option doesn´t affect at all.
#ifndef DATA_ACCES_STREAMS_APP
#define DATA_ACCES_STREAMS_APP FOPEN_MAX
#endif

// Thereshold value that deterimnates sequential or advanced metrhod for offsets search
#define DATA_ACCESS_THRESHOLD 1000000

// File name for backdoor log of opertions order of reading
#define DATA_BAKCDOOR_FILE_NAME "trace_op_reading.log"

// Option for backdoor registring of operations record reading
#ifndef DATA_ACCESS_OP_BACKDOOR
#define DATA_ACCESS_OP_BACKDOOR 0
#endif


// MODULE VARIABLES
// Specific module variables that are determinated by trace file format
char FIELD_DELIMITERS[] = { '\n', ')', '(', ':', ',', '\0' };
int NUM_FIELD_DELIMITERS = 6;
char DATA_ACCESS_EOR = '\n';	// End of register character
char DATA_ACCESS_OBJDEF = 'd';	// Object line definition char
char DATA_ACCESS_OFFDEF = 's';	// Offsets line definition char
long int DATA_ACCESS_API_stream_free = 0;	// Simbolize that stream is free to use
long int DATA_ACCESS_no_stream_assig = 0;	// Simbolize that no stream is assigned to thread
long int DAP_thread_closed = -1;	// Thread colsed flag value
int DATA_ACCESS_SUPPORTED_ACTIONS[] = { 1, 2, 3, 10, 20 };
int DATA_ACCESS_SUPPORTED_ACT_NUM = 5;
int DAP_FILE_NAME_SIZE = 250;

// STREAMS FOR FILTERS
FILE *op_log_stream = NULL;

/*
 *	INTERNAL STRUCTURES
 */

// Application structure,
// eneables to store basic information about certain application, as well as data
// for eficient data extraction from trace file
struct app_struct
{
  int ptask_id;
  int task_num;
  int *thread_num;
  int offset_type;
  long int offset_value;
  int comm_num;
  long int **tasks_offsets;
  char *file_name;
  long int **tasks_stream;
  FILE **streams;
  int num_streams;
  int num_free_streams;
  long int *free_streams;
  int header_length;
  int offset_length;
  int def_size;
  FILE *def_stream;
  struct t_queue *comms;
  int def_flag;
};

/* Main struct, applications container eneables multi-application storage 
 * to API */
struct data_access_layer_struc
{
  struct app_struct **apps;
  int num_apps;
  char *error_string;
  int init_flag;
} main_struct;

/*
 *	T_QUEUE COMPLEMENTAL FUNCTIONS
 */

// Adds all elements from queue from to queue to
void
DAP_add_to_queue (struct t_queue *from, struct t_queue *to)
{
  char *item = NULL;

  item = head_queue (from);
  while (item != NULL)
  {
    insert_queue (to, item, 0);
    item = next_queue (from);
  }
}

/*
 *	T_COMMUNICATOR STRUCT FUNCTIONS
 */

// Sets communicator id
void
DAP_set_comm_id (struct t_communicator *comm, int id)
{
  comm->communicator_id = id;
}

// Returns comm id
int
DAP_get_comm_id (struct t_communicator *comm)
{
  return comm->communicator_id;
}

// Returns tasks list defined for communicator
struct t_queue *
DAP_get_comm_granks (struct t_communicator *comm)
{
  return &comm->global_ranks;
}

// Prints the communicator to standard output
void
DAP_print_communicator (struct t_communicator *comm)
{
  char *item = NULL;
  int *task = NULL;
  struct t_queue *rank = NULL;

  rank = DAP_get_comm_granks (comm);
  printf ("Comunicator defined with id: %d\n", DAP_get_comm_id (comm));
  // Print prcedure of global ranks
  item = head_queue (rank);
  while (item != NULL)
  {
    task = (int *) item;
    printf ("Task involved in communicator %d is %d\n",
	    DAP_get_comm_id (comm), *task);
    item = next_queue (rank);
  }
}

/*
 *	PTASK_STRUCTURE FUNCTIONS
 */

// Initalizes ptask_structure
void
DAP_init_ptask (struct ptask_structure *ptask,
		int ptask_id, int task_num, int *th_num)
{
  ptask->ptask_id = ptask_id;
  ptask->task_count = task_num;
  ptask->threads_per_task = th_num;
}

// Returns number of tasks
int
DAP_get_ptask_task_num (struct ptask_structure *ptask)
{
  return ptask->task_count;
}

// Returns therad number of task
int
DAP_get_thread_ptask_num (struct ptask_structure *ptask, int task)
{
  return ptask->threads_per_task[task];
}

/*
 *	T_ACTION FUNCTIONS
 *	
 */




// Sets action atributes for even action
void
DAP_set_even_action (struct t_action *act, int actid, long int type,
		     long int value)
{
  act->action = actid;
  act->desc.even.type = type;
  act->desc.even.value = value;
}

// Creates action register
struct t_action *
DAP_create_action ()
{
  struct t_action *act = NULL;
  struct t_action *res = NULL;
  act = (struct t_action *) malloc (sizeof (struct t_action));
  if (act != NULL)
  {
    act->next = NULL;		// This is very important
    res = act;
  }
  return res;
}

// Creates and returns next action variable
struct t_action *
DAP_create_next_action (struct t_action *act)
{
  struct t_action *res = NULL;

  act->next = DAP_create_action ();
  if (act->next != NULL)
    res = act->next;
  return res;
}


// Sets action atributes for message recive action
void
DAP_set_action_msg_rcv (struct t_action *act, int actid, int task,
			int msgsize, int tag, int commid, int rcvt)
{
  /* act->action = actid; */
  act->desc.recv.ori         = task+1; /* JGG: Id correction */
  act->desc.recv.mess_tag    = tag;
  act->desc.recv.communic_id = commid;

  /* JGG set correct action id */
  switch (rcvt)
  {
  case RECVTYPE_RECV:
    act->action = RECV;
    break;
  case RECVTYPE_IRECV:
    act->action = IRECV;
    break;
  case RECVTYPE_WAIT:
    act->action = WAIT;
    break;
  }
}

// Sets action atributes for cpu burst action type
void
DAP_set_action (struct t_action *act,
                int              id,
                int              task_id,
                int              thread_id,
                double           burst)
{
  act->action = id;
  act->desc.compute.cpu_time = burst;
}

// Sets action atributes for message passing action type
void
DAP_set_action_msg(struct t_action *act,
                   int              id,
                   int              dest,
                   int              msgs,
                   int              tag,
                   int              commid,
                   int              sync)
{
  act->action                = id;
  act->desc.send.mess_size   = msgs;
  act->desc.send.dest        = dest+1; /* JGG: Id correction */
  act->desc.send.mess_tag    = tag;
  act->desc.send.communic_id = commid;

  /* JGG: Apply 'USE_RENDEZ_VOUS' macro to compute this value */

  act->desc.send.rendez_vous = USE_RENDEZ_VOUS((sync & ((int)1)), msgs);
                     
  if (sync & ((int)2))
  {
    act->desc.send.immediate = TRUE;
  }
  else
  {
    act->desc.send.immediate = FALSE;
  }
}

// Saving information of global opertion to t_action structure
void
DAP_set_glob_action (struct t_action *act, int actid, int globopid,
		     int commid, int rtask, int rthread, long int bsent,
		     long int brecv)
{
  act->action                     = actid;
  act->desc.global_op.glop_id     = globopid;
  act->desc.global_op.comm_id     = commid;
  act->desc.global_op.root_rank   = rtask;
  act->desc.global_op.root_thid   = rthread;
  act->desc.global_op.bytes_send  = bsent;
  act->desc.global_op.bytes_recvd = brecv;

}

/*
 *	ERROR MESSAGE FUNCTION
 */

// Saves error message
void
DAP_report_error (char *message)
{
  char *rcp = NULL;

  if (main_struct.error_string == NULL)
  {
    main_struct.error_string =
      (char *) malloc (DATA_ACCESS_MAX_ERROR_LENGTH * sizeof (char));
  }

  rcp = strcpy (main_struct.error_string, message);

  if (rcp == NULL)
  {
    // Error of saving error report
    perror ("Error produced on error repoting in data access api");
  }
  else
  {
    // Temporal error informing for testing
    // perror(message);
  }
}

// Returns internal error message
char *
get_internal_err_msg ()
{
  return main_struct.error_string;
}

/*
 *	UTILITY FUNCTIONS
 *	These functions are of general purpose, and may
 *	be used anywhere in program.
 */

// RETURNS THE SUM OF ALL ELEMENTS OF ARRAY
int
DAP_sum_array (int *array, int size, int start, int end)
{
  int i = 0;
  int suma = 0;

  for (i = start; i < end; i++)
  {
    suma += array[i];
  }
  return suma;
}

// Convert numeric no negative char to int
int
convert_to_int (char c)
{
  int res = -1;

  if (c == '0')
    res = 0;
  if (c == '1')
    res = 1;
  if (c == '2')
    res = 2;
  if (c == '3')
    res = 3;
  if (c == '4')
    res = 4;
  if (c == '5')
    res = 5;
  if (c == '6')
    res = 6;
  if (c == '7')
    res = 7;
  if (c == '8')
    res = 8;
  if (c == '9')
    res = 9;
  return res;
}

// Convert numeric non negative string to double where:
// str: is a numeric string
// length: is a length in chars of numeric string
double
read_double (char *str, int length)
{
  int i = 0;
  double res = 0;
  char simbol;
  int fin = 0;
  int cipher = 0;
  int bvalue = 0;
  int bdec = 1;
  double lvalue = 0;
  double ldec = 0.1;

  while (i < length && fin == 0)
  {
    simbol = str[i];
    if (simbol == '.' || simbol == '\0')
      fin = 1;
    else
    {
      cipher = convert_to_int (simbol);
      if (cipher == -1)
	return 0;
      bvalue = cipher;
      res = res * 10 + bvalue;


      i++;
    }
  }
  if (simbol == '.')
  {
    fin = 0;
    i++;
    while (i < length && fin == 0)
    {
      simbol = str[i];
      if (simbol == '\0')
	fin = 1;
      else
      {
	cipher = convert_to_int (simbol);
	if (cipher == -1)
	  return 0;
	lvalue = ldec * cipher;
	res = res + lvalue;

	ldec = ldec / 10;
	i++;
      }
    }
  }
  return res;
}

// Sets the elements of array in sequence of starting to starting+array_size, where
/*
	array : is an array of elements
	starting : starting value of sequence
	array_size : size of array

*/
void
DAP_set_sequence (long int *array, long int starting, int array_size)
{
  int i = 0;
  int value = starting;

  for (i = 0; i < array_size; i++)
  {
    array[i] = value;
    value++;
  }
}

// Inserts element to the array
/*
	where:

	array : is array of elements
	array_size : is a size of array
	element : is an element to instert to array
	elements_in_array : number of existing elements in array

	returns: total number of elements after insertion which is eleements_in_array+1
	or returns -1 if error occured like strack is full.

*/
int
DAP_add_to_array (long int *array, int array_size, long int element,
		  int elements_in_array)
{
  int res = 0;

  if (elements_in_array == array_size)
  {
    // Case of array overflof
    res = -1;
  }
  else
  {
    // Store element to the array
    array[elements_in_array] = element;
    elements_in_array++;
    res = elements_in_array;
  }
  return res;
}

// Extracts element from array, that is last element
/*
	where:

	array : is array of elements
	element : variable theat stores the extracted value
	elements_in_array : number of existing elements in array

	returns: total number of elements in array after extraction which is elements_in_array-1
	or returns -1 in case of error like array is empty.
*/
int
DAP_extract_from_array (long int *array, long int *element,
			int elements_in_array)
{
  int res = 0;

  if (elements_in_array == 0)
  {
    // array is empty
    res = -1;
  }
  else
  {
    // element_in_array -1 is the index of last element
    *element = array[elements_in_array - 1];
    elements_in_array--;
    res = elements_in_array;
  }
  return res;
}

// Deletes all arrays of array conteiner
/*
	where:

	array : is the contiener of array
	array_size: is a size of conteiner or a number of arrays it has.

*/
int
DAP_delete_arrays (long int **array, int array_size)
{
  int i = 0;

  if (array != NULL)
  {
    for (i = 0; i < array_size; i++)
    {
      if (array[i] != NULL)
	free (array[i]);
    }
  }
  return i;
}

// Creates and reserves memory for arrays of contiener.
// Returns 0 if ok else returns array index that failed to be created
/*
	where:

	conteiner: is a conteiner of arrays
	conteiner_size: is a size o conteiner or number of arrays it has.
	array_size: is a array of sizes of each array the conteiner has.
*/
int
DAP_create_arrays (long int **conteiner, int conteiner_size, int *array_size)
{
  int i = 0, j = 0;

  for (i = 0; i < conteiner_size; i++)
  {
    conteiner[i] = (long int *) malloc (array_size[i] * sizeof (long int));
    for (j = 0; j < array_size[i]; j++)
      conteiner[i][j] = 0;
    if (conteiner[i] == NULL)
    {
      return i;
      DAP_report_error
	("create_array in data_access_layer_api, failed to crate arrays.");
    }
  }
  return i;
}

// Create conteiner of arrays, but it invokes the DAP_create_array to reserve memeory en create
// arrays
/*
	where:

	array_size: are sizes of each array of conteiner
	conteiner_size: is a size o conteiner or number of arrays it has.

	returns: the conteiner

*/
long int **
DAP_create_array_of_arrays (int *array_size, int conteiner_size)
{
  long int **conteiner = NULL;
  int n_arrays = 0;
  conteiner = (long int **) malloc (conteiner_size * sizeof (long int *));
  if (conteiner != NULL)
  {
    n_arrays = DAP_create_arrays (conteiner, conteiner_size, array_size);
    if (n_arrays != conteiner_size)
      DAP_delete_arrays (conteiner, n_arrays);
  }
  return conteiner;
}

// Checks if char type item is contained in array
int
DAP_seek_char_item (char *array, int size, char item)
{
  int res = -1;
  int i = 0;
  int end = 0;

  for (i = 0; i < size && end == 0; i++)
  {
    if (array[i] == item)
    {
      res = 0;
      end = 1;
    }
  }
  return res;
}

// Checks if int type item is contained in array
int
DAP_seek_int_item (int *array, int size, int item)
{
  int res = -1;
  int i = 0;
  int end = 0;

  for (i = 0; i < size && end == 0; i++)
  {
    if (array[i] == item)
    {
      res = 0;
      end = 1;
    }
  }
  return res;

}

// Checks if it is end of file
int
check_endoffile (FILE * stream)
{
  int c;

  c = fgetc (stream);
  if (c == EOF)
    return 1;
  return 0;
}

// Returns the array from conteiner specifing array id number
/*
	where:

	conteiner: is a conteiner of arrays
	conteiner_size: is a size of conteiner
	array_index: is a array id number
*/
long int *
get_array_conteiner (long int **conteiner, int conteiner_size,
		     int array_index)
{
  long int *item = NULL;

  if (array_index < conteiner_size && conteiner_size >= 0)
  {
    item = conteiner[array_index];
  }
  return item;
}

// Returns item from the array
/*
	where:

	array: is a array
	array_size: is a size of a array
	item_index: is a item id number
*/
// PETA
long int
get_array_item (long int *array, int array_size, int item_index)
{
  long int item = -1;

  if (item_index < array_size && array_size >= 0)
  {
    item = array[item_index];
  }
  return item;
}

// Sets item of the array
/*
	where:

	array: is a array
	array_size: is a size of a array
	item_index: is a item id number
	item_value: is a value of item to be assigned
*/
int
set_array_item (long int *array, int array_size, int item_index,
		long int item_value)
{
  int res = 0;

  if (item_index < array_size && array_size >= 0)
  {

    array[item_index] = item_value;
  }
  else
  {
    res = -1;
    DAP_report_error
      ("set_array_item in data_access_layer_api, no such record.");
  }
  return res;
}

// Set item of the conteiner
/*
	where:

	conteiner: is a conteiner of arrays
	conteiner_size: is a number of arrays that conteiner has
	array_id: is a array id number
	array_size: is a size of a array
	item_id: is a item id number
	item_value: is a value of item to be assigned
*/
int
set_conteiner_item (long int **conteiner, int conteiner_size, int array_id,
		    int array_size, int item_id, long int item_value)
{
  int result = -1;
  long int *array = NULL;

  array = get_array_conteiner (conteiner, conteiner_size, array_id);
  if (array != NULL)
  {

    result = set_array_item (array, array_size, item_id, item_value);
  }
  return result;
}

// Gets the item of a conteiner
/*
	where:

	conteiner: is a conteiner of array.
	conteiner_size: is a number of arrays that conteiner has.
	array_id: is a array id number.
	array_size: is a size of a array.
	item_id: is a item id number.

	returns: the item value.
*/
long int
get_item_conteiner (long int **conteiner, int conteiner_size, int array_id,
		    int array_size, int item_id)
{
  long int res = -1;		// Offset result value, if less then 0 indicates error
  long int *array = NULL;

  array = get_array_conteiner (conteiner, conteiner_size, array_id);
  if (array != NULL)
  {
    res = get_array_item (array, array_size, item_id);
  }
  return res;
}


/*
 *	APP_STRUCT functions
 *	Main structural idea in this API is not to use structures directly,
 *	thus functions should be used instead. Here are all functions for
 *	manipulating app_struct variables in correct way.
 */

// Get yes or no if there is a thread in cuestion
int
DAP_has_thread (struct app_struct *app, int task, int thread)
{
  int num_tasks = 0;
  int num_th = 0;

  num_tasks = app->task_num;
  if (num_tasks <= task || task < 0)
    return 0;
  num_th = app->thread_num[task];
  if (num_th <= thread || thread < 0)
    return 0;
  return 1;
}

// Prints number of internal streams currently in use
void
DAP_print_streams (struct app_struct *app)
{
  printf ("Internal streams in use:%d\n",
	  app->num_streams - app->num_free_streams);
}

// Returns comm number
int
DAP_get_comm_num (struct app_struct *app)
{
  return app->comm_num;
}

// Returns communicators of application
struct t_queue *
DAP_get_comms (struct app_struct *app)
{
  return app->comms;
}

// Prints communicators
void
DAP_print_comms (struct app_struct *app)
{
  char *item = NULL;
  struct t_communicator *comm = NULL;
  struct t_queue *comms = DAP_get_comms (app);

  item = head_queue (comms);
  while (item != NULL)
  {
    comm = (struct t_queue *) item;
    DAP_print_communicator (comm);
    item = next_queue (comms);
  }
}

// Prints comm number
void
DAP_print_comm_num (struct app_struct *app)
{
  printf ("Total communicators number is %d\n", DAP_get_comm_num (app));
}

// Returns application id
int
DAP_get_ptask_id (struct app_struct *app)
{
  return app->ptask_id;
}

// Prints id of application to standard output
void
DAP_print_app_id (struct app_struct *app)
{
  printf ("Application id: %d\n", DAP_get_ptask_id (app));
}

// Adds communcator definition to application
void
DAP_add_comm (struct app_struct *app, struct t_communicator *com)
{
  /* JGG: INITIALIZE QUEUES */

  create_queue (&com->threads);
  create_queue (&com->machines_threads);
  create_queue (&com->m_threads_with_links);

  insert_queue (app->comms, (char *) com, 0);
}

// Returns certan stream identified by id
FILE *
DAP_get_stream (struct app_struct *app, long int id)
{
  if (id <= app->num_streams && id >= 0)
  {
    return app->streams[id - 1];
  }

  return NULL;

}

// Get default stream, stream used for action reading
FILE *
DAP_get_default_stream (struct app_struct * app)
{
  return app->def_stream;
}

// Get offset value from application
int
DAP_get_off_v_app (struct app_struct *app)
{
  return app->offset_value;
}

// Prints application offset value to standard output
void
DAP_print_off_v (struct app_struct *app)
{
  printf ("Offset value: %d\n", DAP_get_off_v_app (app));
}

// Get offset type from application
int
DAP_get_off_t_app (struct app_struct *app)
{
  return app->offset_type;
}

// Get number of tasks
int
DAP_get_task_num (struct app_struct *app)
{
  return app->task_num;
}

// Prints number of tasks of application to standard output 
void
DAP_print_task_num (struct app_struct *app)
{
  printf ("Total tasks number of application: %d\n", DAP_get_task_num (app));
}

// Returns list of threads number for each task
int *
DAP_get_threads_num (struct app_struct *app)
{
  int *res = NULL;
  int i = 0;
  res = (int *) malloc (app->task_num * sizeof (int));
  if (res != NULL)
  {
    for (i = 0; i < app->task_num; i++)
    {
      res[i] = app->thread_num[i];
    }
  }
  return res;
}

// Sets definition read flag
void
DAP_set_rdef_flag (struct app_struct *app, int flag)
{
  app->def_flag = flag;
}

// Returns definition flag
int
DAP_get_def_flag (struct app_struct *app)
{
  return app->def_flag;
}

// Returns threads number for task in question
int
DAP_get_threads_num_task (struct app_struct *app, int task_id)
{
  return app->thread_num[task_id];
}

// Prints threads number for all tasks to standard output
void
DAP_print_th_num (struct app_struct *app)
{
  int i = 0;

  for (i = 1; i < DAP_get_task_num (app) + 1; i++)
  {
    printf ("Total number of threads in task %d is %d\n",
            i,
            DAP_get_threads_num_task (app, i - 1));
  }
}

// Sets header length in bytes, this function is invoiced ones header register is read
void
DAP_set_h_length (struct app_struct *app, int length)
{
  app->header_length = length;
}

// Gets header length in bytes
int
DAP_get_h_size (struct app_struct *app)
{
  return app->header_length;
}

// Sets definition block size
void
DAP_set_def_size (struct app_struct *app, int size)
{
  app->def_size = size;
}

// Gets definition block size
int
DAP_get_def_size (struct app_struct *app)
{
  return app->def_size;
}

// Prints definition block size to standard output
void
DAP_print_def_size (struct app_struct *app)
{
  printf ("Definition block size is %d\n", DAP_get_def_size (app));
}

// Prints header length to standard output
void
DAP_print_h_size (struct app_struct *app)
{
  printf ("Header length in chars is %d\n", DAP_get_h_size (app));
}

// Sets offset register length in bytes, this function is invoiced ones offset register is read
void
DAP_set_o_length (struct app_struct *app, int length)
{
  app->offset_length = length;
}

// Gets gets offset length in bytes
long int
DAP_get_o_size (struct app_struct *app)
{
  return app->offset_length;
}

// Print offset size in chars
void
DAP_print_o_size (struct app_struct *app)
{
  printf ("Offset size in chars is %d\n", DAP_get_o_size (app));
}

// Gets offset type
int
DAP_get_o_type (struct app_struct *app)
{
  return app->offset_type;
}

// Prints offset type to standard output
void
DAP_print_o_type (struct app_struct *app)
{
  printf ("Offset type is %d\n", DAP_get_o_type (app));
}

// Initalizes the application, should use this function to create applications
int
DAP_initialize_app (struct app_struct *app,
                    int ptask_id,
                    int task_n,
                    int *thread_n,
                    int comm_n,
                    int offset_t,
                    long int offset_v,
                    char *file_name,
                    int stream_n,
                    int h_length)
{
  // Local variables
  int i, j;			// Local iterators
  int result = 0;		// Result of function, if less then 0 indicates error
  char read_mode = 'r';

  stream_n = DAP_sum_array (thread_n, task_n, 0, task_n);	// Total number of threads

  // End of local variables
  app->ptask_id = ptask_id;	// Set ptask_id
  app->task_num = task_n;	// Set task_num
  app->thread_num = thread_n;	// Set thread_num
  app->comm_num = comm_n;	// Set comm_num
  app->offset_type = offset_t;	// Set offset_type
  app->offset_value = offset_v;	// Set offset_value
  app->file_name = (char *) malloc (DAP_FILE_NAME_SIZE * sizeof (char));
  strcpy (app->file_name, file_name);	// Set file name
  app->header_length = h_length;	// Set length of header in chars
  app->offset_length = 0;
  app->tasks_offsets = DAP_create_array_of_arrays (thread_n, task_n);
  app->num_free_streams = stream_n;
  //app->num_streams=stream_n; CHANGED BECAUSE IT LIMITS UP TO 16 STREAMS
  app->num_streams = stream_n;
  app->def_stream = fopen (file_name, &read_mode);
  app->def_flag = 0;
  app->comms = (struct t_queue *) malloc (sizeof (struct t_queue));
  if (app->tasks_offsets == NULL || app->def_stream == NULL
      || app->comms == NULL)
  {

    DAP_report_error
      ("DAP_initialize_app in data_access_layer_api, offsets lists failed to be created or default stream failed.");
    return -1;
  }
  app->tasks_stream = DAP_create_array_of_arrays (thread_n, task_n);
  if (app->tasks_offsets == NULL)
  {
    DAP_delete_arrays (app->tasks_offsets, task_n);
    fclose (app->def_stream);
    free (app->comms);
    DAP_report_error
      ("DAP_initialize_app in data_access_layer_api, stream lists failed to be created.");
    return -1;
  }
  app->streams = (FILE **) malloc (stream_n * sizeof (FILE *));
  if (app->streams == NULL)
  {
    DAP_delete_arrays (app->tasks_offsets, task_n);
    DAP_delete_arrays (app->tasks_stream, task_n);
    fclose (app->def_stream);
    free (app->comms);
    DAP_report_error
      ("DAP_initialize_app in data_access_layer_api, streams failed to be created.");
    return -1;
  }
  for (i = 0; i < stream_n; i++)
    app->streams[i] = NULL;
  app->free_streams = (long int *) malloc (stream_n * sizeof (long int));

  if (app->free_streams == NULL)
  {
    DAP_delete_arrays (app->tasks_offsets, task_n);
    DAP_delete_arrays (app->tasks_stream, task_n);
    fclose (app->def_stream);
    free (app->streams);
    free (app->comms);
    DAP_report_error
      ("DAP_initialize_app in data_access_layer_api, streams failed to be created.");
    return -1;
  }
  // Set list of free streams 
  DAP_set_sequence (app->free_streams, 1, stream_n);
  // Create comms
  create_queue (app->comms);
  return result;
}

// Frees resource of application
void
DAP_end_app (struct app_struct *app)
{
  int i = 0;
  t_boolean res = 0;

  // Frees offset list
  DAP_delete_arrays (app->tasks_offsets, app->task_num);
  // Free state stream lists
  DAP_delete_arrays (app->tasks_stream, app->task_num);

  fclose (app->def_stream);
  // Free all streams
  for (i = 0; i < app->num_streams; i++)
  {
    if (app->streams[i] != NULL)
    {
      fclose (app->streams[i]);
      app->streams[i] = NULL;
    }
  }

  // Free array of streams
  free (app->streams);
  free (app->file_name);
  // Deletes queue of comunicators
  res = empty_queue (app->comms);
  // Free queue DBG
  free (app->comms);
  // Frees list of avaluable streams
  free (app->free_streams);
}

// Sets thread stream index
int
DAP_set_thread_stream (struct app_struct *app, int task_id, int thread_id,
		       long int stream_id)
{
  int result = -1;		// Result variable, if less then 0 indicates error

  result =
    set_conteiner_item (app->tasks_stream, app->task_num, task_id,
			app->thread_num[task_id], thread_id, stream_id);
  return result;

}

// Sets thread offset value
int
DAP_set_offset_thread (struct app_struct *app, int task_id, int thread_id,
		       long int offset)
{
  // Local variables
  int result = -1;		// Result variable, if less then 0 indicates error

  result =
    set_conteiner_item (app->tasks_offsets, app->task_num, task_id,
			app->thread_num[task_id], thread_id, offset);
  return result;
}

// Gets thread stream index, this is used for eficient file reading
long int
DAP_get_stream_thread (struct app_struct *app, int task_id, int thread_id)
{
  long int stream = -1;

  stream =
    get_item_conteiner (app->tasks_stream, app->task_num, task_id,
			app->thread_num[task_id], thread_id);
  return stream;
}

// Gets thread offset value
long int
DAP_get_offset_thread (struct app_struct *app, int task_id, int thread_id)
{
  long int offset = -1;

  offset =
    get_item_conteiner (app->tasks_offsets, app->task_num, task_id,
			app->thread_num[task_id], thread_id);
  return offset;
}

// Prints all threads offset values
void
DAP_print_offsets (struct app_struct *app)
{
  int i = 0;
  int j = 0;

  for (i = 1; i < DAP_get_task_num (app) + 1; i++)
  {
    for (j = 1; j < DAP_get_threads_num_task (app, i - 1) + 1; j++)
    {
      printf ("Task %d and thread  %d has offset %d\n", i, j,
	      DAP_get_offset_thread (app, i - 1, j - 1));
    }
  }
}


// Returns file name of application
char *
get_file_name (struct app_struct *app)
{
  char *res = NULL;

  if (app == NULL)
  {
    res = NULL;
  }
  else
  {
    res = app->file_name;
  }
  return res;
}

// Close action thread for reading
int
DAP_close_thread (struct app_struct *app, int task_id, int thread_id)
{
  int res = 0;
  int assig_stream = 0;

  // Get stream logical index
  assig_stream = DAP_get_stream_thread (app, task_id, thread_id);
  // Frees stream resurce
  if (assig_stream != DATA_ACCESS_no_stream_assig)
  {
    res =
      DAP_add_to_array (app->free_streams, app->num_streams, assig_stream,
			app->num_free_streams);
    fclose (app->streams[assig_stream - 1]);
    app->streams[assig_stream - 1] = NULL;
    if (res != -1)
    {
      app->num_free_streams = res;
      // Sets closed the thread in question, the thread can not be read from this moment
      res =
	DAP_set_thread_stream (app, task_id, thread_id, DAP_thread_closed);
    }
  }
  else
  {
    res = DAP_set_thread_stream (app, task_id, thread_id, DAP_thread_closed);
  }

  return res;
}


// Returns stream prepared for acction reading
FILE *
DAP_get_action_stream (struct app_struct * app, int task_id, int thread_id)
{
  int res = 0;
  FILE *stream = NULL;
  long int assig_stream = DATA_ACCESS_no_stream_assig;
  long int offset = 0;
  char open_mode = 'r';
  int num_streams = 0;

  // Get stream logical id for thread if there is stream assigned for thread
  assig_stream = DAP_get_stream_thread (app, task_id, thread_id);

  if (assig_stream == DATA_ACCESS_no_stream_assig)
  {
    // There is no stream assiged for this thread
    // Get free stream from the list of free streams
    num_streams =
      DAP_extract_from_array (app->free_streams, &assig_stream,
			      app->num_free_streams);

    if (num_streams == -1)
    {
      // No free stream, usage of default stream forced
      stream = DAP_get_default_stream (app);
      // Get thread offset
      offset = DAP_get_offset_thread (app, task_id, thread_id);
      // Set default stream for reading
      res = fseek (stream, offset, SEEK_SET);
      if (res == -1)
	return NULL;


    }
    else
    {

      // Opens stream for reading
      app->streams[assig_stream - 1] = fopen (app->file_name, &open_mode);
      stream = app->streams[assig_stream - 1];
      if (stream != NULL)
      {
	// Update number of free streams which is one less
	app->num_free_streams = num_streams;
	// Set index of stream reserved for thread
	res = DAP_set_thread_stream (app, task_id, thread_id, assig_stream);
	if (res == -1)
	  return NULL;
	// Sets stream for reading
	offset = DAP_get_offset_thread (app, task_id, thread_id);
	res = fseek (stream, offset, SEEK_SET);
	if (res == -1)
	  return NULL;
      }
      else
      {
	// No free system resorses, usage of default stream forced
	stream = DAP_get_default_stream (app);
	// Recover extracted stream
	app->num_free_streams =
	  DAP_add_to_array (app->free_streams, app->num_streams,
			    assig_stream, app->num_free_streams);
	// Get thread offset
	offset = DAP_get_offset_thread (app, task_id, thread_id);
	// Set default stream for reading
	res = fseek (stream, offset, SEEK_SET);
	if (res == -1)
	  return NULL;
      }

    }
  }
  else
  {
    if (assig_stream == DAP_thread_closed)
    {
      // Thread is closed for reading
      stream = NULL;
    }
    else
    {
      // Thread has stream, so just return it
      stream = DAP_get_stream (app, assig_stream);

    }
  }
  return stream;
}

// Fills ptask_struct with application information
void
DAP_set_ptask (struct app_struct *app, struct ptask_structure *ptask)
{
  DAP_init_ptask (ptask, DAP_get_ptask_id (app), DAP_get_task_num (app),
		  DAP_get_threads_num (app));
}

/*
 *	FILTERS AND BACKDOOR ROUTINES
 *
 */

// Sets resources for log
void
DAP_start_backdoor_filter ()
{
  char open_mode = 'w';

  if (op_log_stream != NULL)
  {
    op_log_stream = NULL;
  }
  op_log_stream = fopen (DATA_BAKCDOOR_FILE_NAME, &open_mode);
  if (op_log_stream == NULL)
  {
    // Error, stream could not be created
    DAP_report_error
      ("backdoor log filter could not be created in data_access_layer_api.");
  }
}

// Writes log to file
void
DAP_write_op_log (int ptask_id, int task_id, int thread_id)
{
  if (op_log_stream != NULL)
  {
    fprintf (op_log_stream,
	     "Application %d reads task %d  and thread %d action record.\n",
	     ptask_id, task_id, thread_id);
  }
}

void
DAP_close_op_backdoor_filter ()
{
  if (op_log_stream != NULL)
  {
    fclose (op_log_stream);
  }
}

/*
 *	MAIN_STRUCT FUNCTIONS
 *	In order to manage varios applications at the time, these functions
 *	manage applications container of api
 */

// Create main structure or applications container
int
create_data_access_layer ()
{
  int res = 0;
  int i = 0;

  main_struct.apps == NULL;
  main_struct.error_string == NULL;
  main_struct.apps =
    (struct app_struct **) malloc (DATA_ACCESS_MAX_NUM_APP *
				   sizeof (struct app_struct *));
  if (main_struct.apps == NULL)
  {
    DAP_report_error
      ("create_data_access_layer in data_access_layer_api, not enough memeory.");
    return -1;
  }
  // Inialize apps to avoid problems
  for (i = 0; i < DATA_ACCESS_MAX_NUM_APP; i++)
    main_struct.apps[i] = NULL;
  // Set to zero number of applications recorded
  main_struct.num_apps = 0;
  // Set first message for error record
  main_struct.error_string =
    (char *) malloc (DATA_ACCESS_MAX_ERROR_LENGTH * sizeof (char));
  if (main_struct.error_string == NULL)
  {
    DAP_report_error
      ("create_data_access_layer in data_access_layer_api, not enough memeory.");
    DATA_ACCESS_end ();
    return -1;
  }
  // Set init flag
  main_struct.init_flag = 1;
  // DO BACKDOR FILTER
  if (DATA_ACCESS_OP_BACKDOOR == 1)
  {
    DAP_start_backdoor_filter ();
  }
  return res;
}

//
//      See API routines for more main_struct functions at the
//      end of file.
//

/*
 *	REGISTERS AND FIELDS READING AND VERYFING PROCEDURES
 */

// Reading of register from file, general purpose register reading
/*

where,

stream: file stream
buffer: buffer for register storing
buffer_size: size that buffer size will not pass over
offset: offset inside the buffer that indicates starting position for writing in buffer
END_REG: end of register delimiting char

returns chars read of register or total register size

*/

int
read_r (FILE * stream, char *buffer, int buffer_size, int offset,
	char END_REG)
{
  // Local variables
  int chars_read = offset;	// Chars read from file, if less than 0 indicates error
  int i;			// Iterator
  int char_buff = 0;		// Char buffer
  char c;			// Char read

  // End locals
  i = offset;			// Initialize the iterator
  char_buff = fgetc (stream);	// Read char
  c = (char) char_buff;		// Convert to char
  // Read the register char by char
  while (i < buffer_size && char_buff != EOF && c != END_REG)
  {
    buffer[i] = c;		// Save to buffer
    // Continue interating
    char_buff = fgetc (stream);
    c = (char) char_buff;
    i++;
    // Check if buffer is overflowed and resize the buffer to max size
  }
  // Check cases, check to se if we read end of register and there is enought space 
  // to place end of string char
  if (c == END_REG && i + 1 < buffer_size)
  {
    // Regiser read correctly
    buffer[i] = '\0';		// Set end of string char
    // i equals number of chars read
    chars_read = i + 1;		// One more char added which is '\0'
  }
  else
  {
    if (char_buff == EOF && offset == i)
    {
      chars_read = -1;		// Buffer overflow or End of file reached
    }
    else
    {
      if (i + 1 == buffer_size)
	chars_read = -1;
      else
      {
	if (char_buff == EOF && i + 1 < buffer_size)
	{
	  buffer[i] = '\0';
	  chars_read = i + 1;
	}
      }
    }

  }
  return chars_read;
}

// Reading of register whith state variable returning state of reading procedure
/*

where,

stream: file stream
buffer: buffer for register storing
buffer_size: size that buffer size will not pass over
END_REG: char that delimits end of register
offset: offset inside the buffer that indicates starting position for writing in buffer
state: the key element of this function, it is returning variable and,
	if it has value -2 then buffer is overflowed
	if it has value -1 then end of file is reached
	if it has value 0 then reading is finished correctly

returns chars read of register or total register size

*/
int
read_r_state (FILE * stream, char *buffer, int buffer_size, char END_REG,
	      int offset, int *state)
{
  // Local variables
  int chars_read = offset;	// Chars read from file, if less than 0 indicates error
  int i;			// Iterator
  int char_buff = 0;		// Char buffer
  char c;			// Char read

  // End locals
  // Set initial state
  *state = 0;
  i = offset;			// Initialize the iterator
  char_buff = fgetc (stream);	// Read char
  c = (char) char_buff;		// Convert to char
  // Read the register char by char
  // Contition i<buffer_size-1 ensures that there is one space left for additional
  // or last char to put
  while (i < buffer_size - 1 && char_buff != EOF && c != END_REG)
  {
    buffer[i] = c;		// Save to buffer

    // Continue interating
    char_buff = fgetc (stream);
    c = (char) char_buff;
    i++;
    // Check if buffer is overflowed and resize the buffer to max size
  }

  // Check cases, check to se if we read end of register and there is enought space 
  // to place end of string char
  if (c == END_REG)
  {
    // Regiser read correctly
    buffer[i] = '\0';		// Set end of string char
    // i equals number of chars read
    chars_read = i + 1;		// One more char added which is '\0'
  }
  else
  {
    if (i == buffer_size - 1)
    {
      // Buffer overflow
      *state = -2;
      chars_read = i + 1;
      buffer[i] = c;		// Place the last char readden from stream
    }
    else
    {
      if (i < buffer_size - 1 && char_buff == EOF)
      {
	*state = -1;		// End of file reached
	chars_read = i + 1;
	buffer[i] = '\0';
      }
    }
  }

  return chars_read;
}

// Coping of buffers
int
copy_buffer (char *from, int from_size, char *to, int to_size)
{
  int res = 0;
  int i = 0;

  while (i < from_size && i < to_size)
  {
    to[i] = from[i];
    i++;
  }
  return res;
}

// Get new buffer size, squere increment is asumed
int
get_new_buff_size (int ini_size)
{
  int res = ini_size * ini_size;

  return res;
}

// Returns copy of buffer with double space
char *
increce_buffer (char *buffer, int buffer_size, int max_buffer_size,
		int *new_buffer_size)
{
  char *new_buffer = NULL;
  int res = 0;

  *new_buffer_size = get_new_buff_size (buffer_size);
  if (*new_buffer_size > max_buffer_size)
  {
    // No increment is avaluable
  }
  else
  {
    new_buffer = (char *) malloc (*new_buffer_size * sizeof (char));
    res = copy_buffer (buffer, buffer_size, new_buffer, *new_buffer_size);
  }
  return new_buffer;
}

// Reading of register from file autoincrementing the buffer if necessety, general purpose function
/*

where,

stream: file stream
max_buffer_size: size that buffer size will not pass over
chars_r: total number of chars read or register size
END_REG: char that delimits end of register
offset: offset inside the buffer that indicates starting position for writing in buffer

returns buffer

*/
char *
read_r_autoinc (FILE * stream, int max_buffer_size, char END_REG, int offset,
		int *chars_r)
{
  int buffer_size = DATA_ACCESS_STANDARD_BUFFER_SIZE;
  char *buffer = (char *) malloc (buffer_size * sizeof (char));
  int state = -2;		// Condition to continue incrementing the buffer
  int char_r = 0;
  char *buffptr = NULL;
  int new_buffer_size = 0;

  char_r =
    read_r_state (stream, buffer, buffer_size, END_REG, offset, &state);
  while (state == -2)
  {
    // Increment buffer here
    buffptr =
      increce_buffer (buffer, buffer_size, max_buffer_size, &new_buffer_size);
    if (buffptr == NULL)
    {
      free (buffer);
      buffer = NULL;
    }
    else
    {
      free (buffer);
      buffer = buffptr;
      offset = char_r;

      buffer_size = new_buffer_size;
      char_r =
	read_r_state (stream, buffer, buffer_size, END_REG, offset, &state);
    }
  }
  *chars_r = char_r;
  return buffer;
}

// Reading of register autoincremeing buffer for trace file special
/*

where,

stream: file stream
max_buffer_size: size that buffer size will not pass over
chars_r: total number of chars read or register size

returns buffer

*/
char *
DAP_read_register_autoincr (FILE * stream, int max_buffer_size, int *chars_r)
{
  char *res = NULL;
  int offset = 0;

  res =
    read_r_autoinc (stream, max_buffer_size, DATA_ACCESS_EOR, offset,
		    chars_r);
  return res;
}

// Reading of field from buffer DEPRECATED
int
DAP_read_f (char *buffer, int buffer_size, char *field, int field_size,
	    int offset, char *F_DEL, int n_fd)
{
  int c_read = -1;
  int i = offset;
  int j = 0;

  while (i < buffer_size && j < field_size
	 && DAP_seek_char_item (F_DEL, n_fd, buffer[i]) == -1)
  {
    field[j] = buffer[i];
    i++;
    j++;
  }
  if (DAP_seek_char_item (F_DEL, n_fd, buffer[i]) == 0 && j + 1 < field_size)
  {
    field[j] = '\0';		// Add end of string char
    c_read = j + 1;		// Set total number of chars to return 
  }
  else
  {
    c_read = -1;		// Buffer overflow
  }
  return c_read;
}

// Reading of field from biffer with special delimiters, general purpose function
/*

where:

buffer: is a register buffer that contains fields
buffer_size: size of register buffer needed to br known in order to not to overflow the buffer
field: storing variable for field
field_size: size of field memory in order to not overflow the field memory
offset: indicates strating position for reding inside register buffer
delimiters: list of delimiting chars
delim_size: size of list of delimiting chars

*/
int
read_df (char *buffer, int buffer_size, char *field,
	 int field_size, int offset, char *delimiters, int delim_size)
{
  int c_read = 0;
  int i = offset;
  int j = 0;

  while (i < buffer_size && j < field_size
	 && DAP_seek_char_item (delimiters, delim_size, buffer[i]) == -1)
  {
    field[j] = buffer[i];
    i++;
    j++;
  }
  if (DAP_seek_char_item (delimiters, delim_size, buffer[i]) == 0
      && j + 1 < field_size)
  {
    field[j] = '\0';		// Add end of string char
    c_read = j + 1;		// Set total number of chars to return 
  }
  else
  {
    c_read = -1;		// Buffer overflow
  }
  return c_read;
}

// Reading of register specific for new trace
int
DAP_read_register (FILE * stream, char *buffer, int buffer_size)
{
  // Local variables
  int chars_read = 0;		// Chars read from file, if less than 0 indicates error
  int offset = 0;
  char *buffptr = buffer;

  chars_read = read_r (stream, buffptr, buffer_size, offset, DATA_ACCESS_EOR);
  if (chars_read == -1)
  {
    DAP_report_error ("DAP_read_register in data_access_layer_api, failed.");
  }
  else
  {
    if (chars_read == -2)
    {
      // Buffer overflow case
      DAP_report_error
	("DAP_read_register in data access api, buffer overfflow");
    }
  }
  return chars_read;
}

// Reading of field from buffer specific for new trace
/*

where:

buffer: is a register buffer that contains fields
buffer_size: size of register buffer needed to br known in order to not to overflow the buffer
field: storing variable for field
field_size: size of field memory in order to not overflow the field memory
offset: indicates strating position for reding inside register buffer

*/
int
DAP_read_field (char *buffer, int buffer_size, char *field, int field_size,
		int offset)
{
  int c_read = 0;

  c_read =
    read_df (buffer, buffer_size, field, field_size, offset,
	     FIELD_DELIMITERS, NUM_FIELD_DELIMITERS);
  if (c_read == -1)
  {
    DAP_report_error ("DAP_read_field in data_access_layer_api, failed.");
  }
  return c_read;
}

// Reading of field with special delimiting chars, used in new trace format
/*

where:

buffer: is a register buffer that contains fields
buffer_size: size of register buffer needed to br known in order to not to overflow the buffer
field: storing variable for field
field_size: size of field memory in order to not overflow the field memory
offset: indicates strating position for reding inside register buffer
delimiters: listo of delimiting chars
delim_size: number of item in delimiting chars list

*/
int
DAP_read_delimited_field (char *buffer, int buffer_size, char *field,
			  int field_size, int offset, char *delimiters,
			  int delim_size)
{
  int c_read = 0;

  c_read =
    read_df (buffer, buffer_size, field, field_size, offset, delimiters,
	     delim_size);
  if (c_read == -1)
  {
    DAP_report_error
      ("DAP_read_delimited_field in data_access_layer_api, failed.");
  }
  return c_read;
}

// Verifies if there is certan number of int fields in buffer, returns the offset of last delimiting char
int
DAP_verify_num_int_fields (char *buffer, int buffer_size, int offset,
			   int fields_num)
{
  int res = 0;			// Offset of last delimiting char
  int field_size = DATA_ACCESS_STANDARD_FIELD_SIZE;
  int i = 0;
  int char_r = 0;
  int value = 0;
  char *field = (char *) malloc (field_size * sizeof (char));

  for (i = 0; i < fields_num; i++)
  {
    char_r =
      DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			  &value);
    if (char_r == -1)
    {
      res = -1;
    }
    offset = char_r;
  }
  res = offset;
  free (field);
  return res;
}

// Verifies if there is ( ) sintax in buffer
int
DAP_verify_bracket_sintx (char *buffer, int buffer_size, int offset)
{
  int res = 0;			// Return is offset of last ')' char
  char delim[] = { ')' };	// Delimiting reading char
  int delim_size = 1;		// Size of uper delimiter array
  int field_size = DATA_ACCESS_STANDARD_FIELD_SIZE;
  int deli_offset = 0;		// Offset of delimiting char
  int i = 0, char_r = 0;

  // Check if offset element is '(' char
  if (buffer[offset] != '(')
  {
    DAP_report_error
      ("DAP_verify_bracket_sintx in data_access_layer_api, sintax error");
    return -1;
  }
  // Check if there is '( )' sintaxis 
  // Set offset to next element
  offset += 1;
  char *field = (char *) malloc (field_size * sizeof (char));	// Create memory for reading

  if (field != NULL)
  {
    deli_offset =
      DAP_read_delimited_field (buffer, buffer_size, field, field_size,
				offset, delim, delim_size);
    if (deli_offset == -1)
    {
      res = -1;
    }
    else
    {
      res = deli_offset;
    }
  }
  else
  {
    // Not enought memory
    DAP_report_error
      ("DAP_verify_bracket_sintx in data_access_layer_api, not enough memory");
    res = -1;
  }
  free (field);
  return res;
}

// Reading of double field in buffer
/*

where:

buffer: is a register buffer that contains fields
buffer_size: size of register buffer needed to br known in order to not to overflow the buffer
field: storing variable for field
field_size: size of field memory in order to not overflow the field memory
offset: indicates strating position for reding inside register buffer
field_value: variable that stores the double field

*/
int
DAP_double_field_read (char *buffer, int buffer_size, char *field,
		       int field_size, int offset, double *field_value)
{
  int res = -1;
  int c_read = 0;
  double field_v = 0;

  c_read = DAP_read_field (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    res = -1;
  }
  else
  {
    field_v = read_double (field, c_read);
    //field_v=atof(field);// Pages 310, 108, 61 (chars verifications) in GNU manual
    *field_value = field_v;
    res = c_read + offset;
  }
  return res;
}

// Reading of long int field in buffer
/*

where:

buffer: is a register buffer that contains fields
buffer_size: size of register buffer needed to br known in order to not to overflow the buffer
field: storing variable for field
field_size: size of field memory in order to not overflow the field memory
offset: indicates strating position for reding inside register buffer
offset_rec: variable that stores the long int field, has the name offset_rec which is old name but correct name should be simply field_value.

*/
int long
DAP_long_int_field_read (char *buffer, int buffer_size, char *field,
			 int field_size, int offset, long int *offset_rec)
{
  int res = -1;
  int c_read = 0;
  long int off_r = 0;

  c_read = DAP_read_field (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    res = -1;
  }
  else
  {
    off_r = atol (field);	// Pages 310, 108, 61 (chars verifications) in GNU manual
    *offset_rec = off_r;
    res = c_read + offset;
  }
  return res;
}

// Reading of int field in buffer
/*

where:

buffer: is a register buffer that contains fields
buffer_size: size of register buffer needed to br known in order to not to overflow the buffer
field: storing variable for field
field_size: size of field memory in order to not overflow the field memory
offset: indicates strating position for reding inside register buffer
rec: variable that stores the int field

*/
int
DAP_int_field_read (char *buffer, int buffer_size, char *field,
		    int field_size, int offset, int *rec)
{
  int res = -1;
  int c_read = 0;
  int r = 0;


  c_read = DAP_read_field (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    res = -1;
  }
  else
  {
    r = atoi (field);		// Pages 310, 108, 61 (chars verifications) in GNU manual
    *rec = r;
    res = c_read + offset;
  }
  return res;
}


/*
 *	HEADER READING PROCEDURES
 */

// Verifing if register is of header type
int
DAP_h_r_filef (char *buffer, int buffer_size, char *field, int field_size,
	       int offset)
{
  int res = 0;
  int c_read = 0;

  c_read = DAP_read_field (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    res = -1;

  }
  else
  {
    if (strcmp (DATA_ACCESS_DIMEMAS_HELLO_SIGN, field) != 0)
    {
      res = -1;
      DAP_report_error
	("DAP_h_r_filef in data access API, hello sign is not #DIMEMAS");
    }
    else
    {
      res = offset + c_read;
    }
  }
  return res;
}

// Reading of trace name
int
DAP_h_r_tr_name (char *buffer, int buffer_size, char *field, int field_size,
		 int offset)
{
  int res = 0;
  int c_read = 0;

  c_read = DAP_read_field (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    res = -1;
  }
  else
  {
    // By default trace name is not stored
    res = c_read + offset;
  }
  return res;
}

// Reading of offset fields from header in buffer
int
DAP_h_r_offset (char *buffer, int buffer_size, char *field, int field_size,
		int offset, int *offset_type, long int *offset_value)
{
  int res = -1;
  int c_read = 0;
  int off_t = 0;
  long int off_v = 0;

  // Read type of offset
  c_read =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&off_t);
  if (c_read != -1)
  {
    res = c_read;
    *offset_type = off_t;
    if (off_t == OFFSET_PRESENT)
    {
      // Read offset register position
      c_read =
	DAP_long_int_field_read (buffer, buffer_size, field, field_size,
				 c_read, &off_v);
      res = c_read;
      *offset_value = off_v;
    }
  }
  return res;
}

// Reading of number of tasks in header in buffer
int
DAP_h_r_num_tasks (char *buffer,
		   int buffer_size, char *field, int field_size, int offset,
		   int *num_tasks)
{
  int res = 0;
  int c_read = 0;
  int n_tasks = 0;

  c_read = DAP_read_field (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    res = -1;
  }
  else
  {
    n_tasks = atoi (field);

    *num_tasks = n_tasks;
    res = c_read + offset;
  }
  return res;
}

// Verifing of number of threads in header 
int
DAP_h_verify_num_th (char *buffer, int buffer_size, int offset, int task_num)
{
  int res = 0;
  int brack_ch = 0;
  int num_ch = 0;

  if (task_num != 0)
  {
    brack_ch = DAP_verify_bracket_sintx (buffer, buffer_size, offset - 1);
    if (brack_ch == -1)
    {
      return -1;
    }
    num_ch =
      DAP_verify_num_int_fields (buffer, buffer_size, offset, task_num);
    if (num_ch == -1)
    {
      return -1;
    }
    if (num_ch == brack_ch)
    {
      return -1;
    }
  }
  return res;
}

// Read number of threads for each task
int
DAP_h_r_num_threads (char *buffer, int buffer_size, char *field,
		     int field_size, int offset, int task_num, int *th_num)
{
  int res = 0;
  int i = 0;
  int value = 0;
  int char_r = 0;

  if (DAP_h_verify_num_th (buffer, buffer_size, offset, task_num) == -1)
    return -1;
  if (task_num != 0)
  {
    for (i = 0; i < task_num; i++)
    {
      char_r =
	DAP_int_field_read (buffer, buffer_size, field, field_size,
			    offset, &value);
      if (char_r < 0)
      {
	return -1;
      }
      th_num[i] = value;
      offset = char_r;
    }
    res = offset;
  }
  else
  {
    res = offset - 1;
  }
  return res;
}

// Read communicators number from header
int
DAP_h_r_comm_num (char *buffer, int buffer_size, char *field, int field_size,
		  int offset, int *comm_num)
{
  int res = 0;
  int c_read = 0;

  // Verify ',' char in place
  if (buffer[offset] != ',')
  {
    DAP_report_error
      ("DAP_h_r_comm_num in data_access_layer_api, sintax error.");
    return -1;
  }
  offset = offset + 1;		// Avoiding ',' char
  c_read =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			comm_num);
  if (c_read < 0)
  {
    return -1;
  }
  res = c_read;
  return res;
}

// Reading and checking of every field in header
int
DAP_h_check_and_save (struct app_struct *app, char *buffer, int buffer_size,
		      char *field, int field_size, int ptask_id,
		      char *file_name, int char_r)
{
  int res = 0;
  int c_read = 0;
  int offset = 0;
  int offset_type = 0;
  long int offset_value = 0;
  int task_num = 0;
  int *th_num;
  int comm_num = 0;
  int stream_n = DATA_ACCES_STREAMS_APP;

  c_read = DAP_h_r_filef (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    return -1;
  }
  offset = c_read;
  c_read = DAP_h_r_tr_name (buffer, buffer_size, field, field_size, offset);
  if (c_read < 0)
  {
    return -1;
  }
  offset = c_read;
  c_read =
    DAP_h_r_offset (buffer, buffer_size, field, field_size, offset,
		    &offset_type, &offset_value);
  if (c_read < 0)
  {
    return -1;
  }
  offset = c_read;
  c_read =
    DAP_h_r_num_tasks (buffer, buffer_size, field, field_size, offset,
		       &task_num);
  if (c_read < 0)
  {
    return -1;
  }
  offset = c_read;
  th_num = (int *) malloc (task_num * sizeof (int));
  if (th_num == NULL)
  {
    return -1;
  }
  c_read =
    DAP_h_r_num_threads (buffer, buffer_size, field, field_size, offset,
			 task_num, th_num);
  if (c_read < 0)
  {
    return -1;
  }
  offset = c_read;
  c_read =
    DAP_h_r_comm_num (buffer, buffer_size, field, field_size, offset,
		      &comm_num);
  if (c_read < 0)
  {
    return -1;
  }
  if (c_read != char_r)
  {
    DAP_report_error
      ("DAP_h_check_and_save in data access layer api, sintax error.");
    return -1;
  }
  res =
    DAP_initialize_app (app, ptask_id, task_num, th_num, comm_num,
			offset_type, offset_value, file_name, stream_n,
			char_r);

  return res;
}

// Main header reading procedure
int
DAP_read_header (struct app_struct *app, int ptask_id, char *file_name)
{
  int res = 0;
  FILE *stream = NULL;
  char open_mode = 'r';

  stream = fopen (file_name, &open_mode);
  int buffer_size = DATA_ACCESS_STANDARD_BUFFER_SIZE;
  int field_size = DATA_ACCESS_STANDARD_FIELD_SIZE;

  //char* buffer=(char*)malloc(buffer_size*sizeof(char));
  //char* field=(char*)malloc(field_size*sizeof(char));
  char buffer[DATA_ACCESS_STANDARD_BUFFER_SIZE];
  char field[DATA_ACCESS_STANDARD_FIELD_SIZE];
  int c_read = 0;

  if (buffer != NULL && field != NULL && stream != NULL)
  {
    c_read = DAP_read_register (stream, buffer, buffer_size);
    if (c_read < 0)
    {

      res = -1;
    }
    else
    {
      res = DAP_h_check_and_save (app, buffer, buffer_size,
				  field, field_size, ptask_id, file_name,
				  c_read);

    }
    fclose (stream);
  }
  else
  {
    res = -1;
    DAP_report_error ("DAP_read_header in data_access_layer_api, failed.");
  }
  //free(buffer);
  //free(field);
  return res;
}

/*
 *	FUNCTIONS FOR LECTURE OF OFFSET REGISTER
 */

// Verifies if register is of offset type
int
DAP_ch_offset_register (char *buffer, int buffer_size)
{
  int res = 0;

  // Check offset rec id
  if (buffer[0] != DATA_ACCESS_OFFDEF || buffer[1] != ':')
  {
    DAP_report_error
      ("DAP_ch_offset_register in data_access_layer_api, sintax error.");
    return -1;
  }
  return res;
}

// Reading of thread offset from a register
int
DAP_read_offset_thread_rec (struct app_struct *app, FILE * stream,
			    char *buffer, int buffer_size, char *field,
			    int field_size, int task_id, int offset,
			    int read_r)
{
  int res = 0;
  int num_threads = 0;
  int i = 0;
  int char_r = 0;
  int set_res = 0;
  long int value = 0;

  num_threads = DAP_get_threads_num_task (app, task_id);

  for (i = 0; i < num_threads; i++)
  {
    char_r =
      DAP_long_int_field_read (buffer, buffer_size, field, field_size,
			       offset, &value);
    if (char_r <= 0 || char_r > read_r)
    {

      DAP_report_error
	("DAP_read_offset_thread_recs in data_access_layer_api, sintax error");
      return -1;
    }
    // Set value of offsets
    set_res = DAP_set_offset_thread (app, task_id, i, value);
    if (set_res == -1)
    {
      return -1;
    }
    offset = char_r;
  }
  res = offset;
  return res;
}

// Reading of threads offset task by task
int
DAP_read_offset_recs (struct app_struct *app, FILE * stream, char *buffer,
		      int buffer_size, char *field, int field_size)
{
  int res = 0;
  int read_r = 0;
  int i = 0, ch_o_r = 0;
  int min_c_read = 2;
  int num_task = 0;
  int char_r = 0;
  int offset = 0;
  int value = 0;
  int set_res = 0;
  char *secbuffer = NULL;

  secbuffer =
    DAP_read_register_autoincr (stream, DATA_ACCESS_MAX_BUFFER_SIZE, &read_r);

  if (secbuffer == NULL)
  {
    // Not enought memory
    return -1;
  }
  if (read_r < min_c_read)
  {
    DAP_report_error
      ("DAP_read_offset_recs in data_access_layer_api, sintax error");
    return -1;
  }
  ch_o_r = DAP_ch_offset_register (secbuffer, read_r);
  if (ch_o_r == -1)
    return -1;
  num_task = DAP_get_task_num (app);

  offset = 2;			// Set inital offset reading
  for (i = 0; i < num_task; i++)
  {
    char_r = DAP_read_offset_thread_rec (app, stream, secbuffer, read_r,
					 field, field_size, i, offset,
					 read_r);

    if (char_r <= 0 || read_r < char_r)
    {
      DAP_report_error
	("DAP_read_offset_recs in data_access_layer_api, sintax error");
      return -1;
    }
    offset = char_r;
  }
  if (char_r != read_r)
  {
    DAP_report_error
      ("DAP_read_offset_recs in data_access_layer_api, sintax error");
    return -1;
  }
  res = char_r;
  return res;
}

// Reading of offset register from the end of a trace file
int
DAP_read_from_end_offset (struct app_struct *app, FILE * stream, char *buffer,
			  int buffer_size, char *field, int field_size)
{
  int res = 0;
  int pos_res = 0;
  int read_res = 0;

  // Set to read from the end
  int offset = DAP_get_off_v_app (app);

  pos_res = fseek (stream, offset, SEEK_SET);
  if (pos_res < 0)
  {
    DAP_report_error
      ("DAP_read_from_end_offset in data_access_layer_api, error on file stream positioning");
    return -1;
  }
  read_res =
    DAP_read_offset_recs (app, stream, buffer, buffer_size, field,
			  field_size);
  if (read_res < 0)
  {
    return -1;
  }
  res = read_res;
  return res;
}

// Exame register for offset seeking
int
DAP_exame_register_seek_off (char *buffer, int buffer_size, char *field,
			     int field_size, struct app_struct *app,
			     FILE * stream, int rchar)
{
  int res = 0;
  int fchar = 0;
  int id = 0;
  int task = 0;
  int thread = 0;
  int offset = 0;
  long int off_v = 0;
  long int f_pos = 0;

  // Read id of register
  fchar =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset, &id);
  if (fchar == -1)
    return -1;
  if (DAP_seek_int_item
      (DATA_ACCESS_SUPPORTED_ACTIONS, DATA_ACCESS_SUPPORTED_ACT_NUM, id) == 0)
  {
    // Read task, thread
    offset = fchar;
    fchar =
      DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			  &task);
    if (fchar == -1)
      return -1;
    offset = fchar;
    fchar =
      DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			  &thread);
    if (fchar == -1)
      return -1;
    offset = fchar;

    off_v = DAP_get_offset_thread (app, task, thread);
    if (off_v == 0)
    {
      // Set offset
      f_pos = ftell (stream);
      f_pos = f_pos - rchar;
      res = DAP_set_offset_thread (app, task, thread, f_pos);
    }
  }

  return res;
}

// Get task id and thread id from register 
int
DAP_get_comparation_info (char *buffer, int buffer_size, char *field,
			  int field_size, int *task, int *thread)
{
  int res = 0;
  int offset = 0;
  int regid = 0;

  // Read register id
  res =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&regid);
  if (res < 0)
    return -1;
  offset = res;
  if (DAP_seek_int_item
      (DATA_ACCESS_SUPPORTED_ACTIONS, DATA_ACCESS_SUPPORTED_ACT_NUM,
       regid) == 0)
  {
    // Read fields
    res =
      DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			  task);
    if (res < 0)
      return -1;
    offset = res;
    res =
      DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			  thread);
    if (res < 0)
      return -1;
    offset = res;
  }
  else
  {
    // Error
    res = -1;
    DAP_report_error ("DAP_get_comparation_inf invalid operation register.");
  }
  offset = res;
  return res;
}

// Compares two action registers
int
DAP_exam_seek (FILE * stream, struct app_struct *app, char *buffer,
	       int buffer_size, char *field, int field_size, long int *offset,
	       long int cut_point, int task, int thread, long int low_limit,
	       long int high_limit)
{
  int res = 0;
  int LOWER = -1;
  int BIGGER = 1;
  int EQUAL = 0;
  int ERROR_CASE = -2;
  int NO_FOUND = -3;
  int first_task = 0;
  int first_thread = 0;
  int sec_task = 0;
  int sec_thread = 0;
  long int offset_first = 0;
  long int offset_sec = 0;

  res = fseek (stream, cut_point, SEEK_SET);
  if (res < 0)
    return ERROR_CASE;
  // Read or move from cut point to first register
  res = DAP_read_register (stream, buffer, buffer_size);
  if (res < 0)
    return ERROR_CASE;
  // Read first register
  offset_first = ftell (stream);
  res = DAP_read_register (stream, buffer, buffer_size);
  if (res < 0)
  {
    // From cut point to first register read OK.
    // Read of first examing register failed, there is no register or we are out from area.
    // This is case where there is only one register left to exam.
    // Reposition for reading.
    res = fseek (stream, low_limit, SEEK_SET);
    // Read first register
    res = DAP_read_register (stream, buffer, buffer_size);
    if (res < 0)
      return ERROR_CASE;
  }
  res =
    DAP_get_comparation_info (buffer, buffer_size, field, field_size,
			      &first_task, &first_thread);
  if (res < 0)
    return ERROR_CASE;
  if (task == first_task && thread == first_thread)
  {
    if (DAP_get_offset_thread (app, task, thread) > offset_first)
    {
      DAP_set_offset_thread (app, task, thread, offset_first);
      *offset = cut_point;
    }
    return LOWER;
  }
  if (task < first_task)
  {
    *offset = cut_point;
    return LOWER;
  }
  if (task > first_task)
  {
    *offset = cut_point;
    return BIGGER;
  }
  return res;
}

// Dicotomic seek 
int
DAP_dicotomic_seek (FILE * stream, struct app_struct *app, long int file_size,
		    char *buffer, int buffer_size, char *field,
		    int field_size, long int *offset, int task, int thread)
{
  long int low_limit = DAP_get_h_size (app) + DAP_get_def_size (app) - 2;
  long int high_limit = file_size;
  int res = 0;
  long int cut_point = (high_limit + low_limit) / 2;
  int found = 0;
  long int exam_point = 0;
  int aval = 0;
  int LOWER = -1;
  int BIGGER = 1;
  int EQUAL = 0;
  int ERROR_CASE = -2;
  int NO_FOUND = -3;

  DAP_set_offset_thread (app, task, thread, file_size);
  while (low_limit < cut_point && cut_point < high_limit && found == 0)
  {
    // Exame or avaluate


    aval =
      DAP_exam_seek (stream, app, buffer, buffer_size, field, field_size,
		     &exam_point, cut_point, task, thread, low_limit,
		     high_limit);

    if (aval == LOWER)
    {
      // Lower size
      high_limit = cut_point;
    }
    if (aval == BIGGER)
    {
      // Bigger size
      low_limit = cut_point;

    }
    if (aval == ERROR_CASE)
    {
      return -1;
    }
    cut_point = (high_limit + low_limit) / 2;
  }
  return res;

}

// Advanced seek of offset values
int
DAP_advaced_offset_seek (FILE * stream, struct app_struct *app,
			 long int file_size, char *buffer, int buffer_size,
			 char *field, int field_size)
{
  int res = 0;
  int task_num = 0;
  int thread_num = 0;
  int i = 0;
  int j = 0;
  int end = 0;
  int dic_res = 0;
  long int offset = 0;

  res = DAP_read_definitions (app);
  if (res < 0)
    return -1;
  DAP_set_rdef_flag (app, 1);
  task_num = DAP_get_task_num (app);
  // Modified due to task, thread index numeration changed
  for (i = 0; i < task_num  && end == 0; i++)
  {
    thread_num = DAP_get_threads_num_task (app, i);
    for (j = 0; j < thread_num  && end == 0; j++)
    {
      dic_res =
	DAP_dicotomic_seek (stream, app, file_size, buffer, buffer_size,
			    field, field_size, &offset, i, j);
      if (dic_res < 0)
      {
	end = 1;
	res = -1;
      }
    }
  }
  return res;
}

// Sequencial seek
//
int
DAP_sec_seek_offsets (char *buffer, int buffer_size, char *field,
		      int field_size, struct app_struct *app, FILE * stream)
{

  int res = 0;
  int rchar = 0;
  int resex = 0;

  rchar = DAP_read_register (stream, buffer, buffer_size);
  while (rchar != -1 && rchar != -2)
  {
    resex =
      DAP_exame_register_seek_off (buffer, buffer_size, field, field_size,
				   app, stream, rchar);
    if (resex == -1)
      return -1;
    rchar = DAP_read_register (stream, buffer, buffer_size);
  }
  return res;

}


// Seek routne for threads offset value
int
DAP_seek_offsets (char *buffer, int buffer_size, char *field, int field_size,
		  struct app_struct *app, FILE * stream)
{
  int res = 0;
  long int file_size = 0;
  struct stat file_atribs;

  res = stat (get_file_name (app), &file_atribs);
  if (res < 0)
    return -1;
  file_size = (long int) file_atribs.st_size;
  // TEMP FORCE ADVANCED SEARCH
  if (file_size < DATA_ACCESS_THRESHOLD)
  {
    res =
      DAP_sec_seek_offsets (buffer, buffer_size, field, field_size, app,
			    stream);
  }
  else
  { 
    // Advanced search method
    res =
      DAP_advaced_offset_seek (stream, app, file_size, buffer, buffer_size,
			       field, field_size);
  }
  return res;
}

// Main offset register reading procedure
int
read_offsets (struct app_struct *app)
{
  int res = 0;
  int offset_t = 0;
  FILE *stream = NULL;
  char open_mode = 'r';
  int res_seek = 0;
  char *file_name = get_file_name (app);

  stream = fopen (file_name, &open_mode);
  int buffer_size = DATA_ACCESS_STANDARD_BUFFER_SIZE;
  int field_size = DATA_ACCESS_STANDARD_FIELD_SIZE;
  char *buffer = (char *) malloc (buffer_size * sizeof (char));
  char *field = (char *) malloc (field_size * sizeof (char));
  int read_r = 0;
  int end_off_r = 0;

  // Check the offset type
  offset_t = DAP_get_off_t_app (app);
  if (offset_t == OFFSET_NOT_PRESENT)
  {

    // No offset record
    // Seek for offsets
    res_seek =
      DAP_seek_offsets (buffer, buffer_size, field, field_size, app, stream);
    if (res_seek == -1)
    {
      res = -1;
    }
  }

  /*
     if(offset_t==1) {
     // Second line is offset record
     read_r = fseek(stream, DAP_get_h_size(app), SEEK_SET);
     // Jump header register in upper fseek funtion
     read_r = DAP_read_offset_recs(app, stream, buffer, buffer_size, field, field_size);
     if(read_r==-1) {
     res=-1;
     }
     res=read_r;
     DAP_set_o_length(app, res);
     }
   */
  if (offset_t == OFFSET_PRESENT)
  {
    // Offset is last line in trace file, GNU manual 137
    end_off_r =
      DAP_read_from_end_offset (app, stream, buffer, buffer_size, field,
				field_size);
    if (end_off_r == -1)
    {
      res = -1;
    }
    res = end_off_r;
    DAP_set_o_length (app, res);
  }
  free (buffer);
  free (field);
  return res;
}

/*
 *	READ OBJECT DEFINITIONS ROUTINE
 */

// Read communicator tasks
int
read_tasks_comm (struct app_struct *app,
                 char *buffer,
                 int buffer_size,
                 char *field,
                 int field_size,
                 int offset,
                 int task_num,
                 struct t_communicator *comm)
{
  int res = 0;
  int i = 0;
  int char_r = 0;
  int *task_id = NULL;
  struct t_queue *tasks = DAP_get_comm_granks (comm);

  // Initalize the queue
  create_queue (tasks);
  while (i < task_num)
  {
    task_id = (int *) malloc (sizeof (int));
    char_r = DAP_int_field_read (buffer,
                                 buffer_size,
                                 field,
                                 field_size,
                                 offset,
                                 task_id);
    
    //printf("Commuicator task include task:%d\n", task_id);
    if (char_r == -1)
    {
      DAP_report_error ("read_tasks_comm in data_access_api, sinatx error");
      return -1;
    }
    
    /* JGG: correct the object numeration */
    *task_id = *task_id+1;
    
    offset = char_r;
    insert_queue (tasks, (char *) task_id, 0);
    i++;
  }
  res = char_r;
  return res;
}

// Read object definition of type communicator
int
read_comm_regs (struct app_struct *app,
                char *buffer,
                int buffer_size,
                char *field,
                int field_size,
                int offset)
{
  // PROPOSITION: Comtrol that comms are not repeated with same id
  int res = 0;
  int char_r = 0;
  int commid = 0;
  int taskc = 0;
  struct t_communicator *comm;
  comm = (struct t_communicator *) malloc (sizeof (struct t_communicator));
  if (comm == NULL)
    return -1;
  char_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&commid);
  if (char_r < 0)
  {
    free (comm);
    return -1;
  }
  //comm->communicator_id=commid;
  DAP_set_comm_id (comm, commid);
  //printf("Communicator id is:%d\n", commid);
  offset = char_r;
  char_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&taskc);
  if (char_r < 0)
    return -1;
  //printf("Number of task involved in commuicator id: %d is %d\n", commid, taskc);
  offset = char_r;
  char_r =
    read_tasks_comm (app, buffer, buffer_size, field, field_size, offset,
		     taskc, comm);
  if (char_r < 0)
    return -1;
  res = char_r;
  // Store to app
  DAP_add_comm (app, comm);
  return res;
}

// Switching routine for selecting wich type of object is definition for
int
switch_obj_type (struct app_struct *app, char *buffer, int buffer_size,
		 char *field, int field_size, int offset, int type)
{
  int res = 0;

  switch (type)
  {
  case DEF_COMMUNICATOR:
    /* Comunicator type TODO contorl number of comm definitions read */
    res =
      read_comm_regs (app, buffer, buffer_size, field, field_size, offset);
    break;
  case DEF_FILE:
    break;
  case DEF_ONESIDED_WIN:
    break;
  default:
    // Invalid type of object def
    DAP_report_error
      ("switch_obj_type in data access layer api, invalid object type definition");
    res = -1;
  }

  return res;
}

// Reading definition register
int
DAP_read_def_reg (struct app_struct *app, char *buffer, int buffer_size,
		  char *field, int field_size, FILE * stream)
{
  // DAP_read_def_reg fails on non register left to read, register is not of object definition type and
  // register could not be read.
  int res = 1;
  int char_r = 1;
  int offset = 0;
  int char_rr = 0;
  long int size = 0;

  // Check to see if we are at the end of a file
  char_rr = DAP_read_register (stream, buffer, buffer_size);

  int start_off = 0;
  int obj_type = 0;

  if (char_rr < 0)
  {
    return 0;
  }
  // Identify the register
  if (buffer[0] != DATA_ACCESS_OBJDEF)
  {
    return 0;
  }
  // Identify object definition type
  // Set starting offset for field reading
  start_off = 2;
  char_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, start_off,
			&obj_type);
  if (char_r < 0)
    return -1;
  //printf("Object def type is:%d\n", obj_type);
  offset = char_r;
  // Switch procedure of every object type threting
  char_r =
    switch_obj_type (app, buffer, buffer_size, field, field_size, offset,
		     obj_type);
  offset = char_r;
  if (char_r < 0)
    return -1;
  if (char_rr != char_r)
    return -1;
  // Update definition block size
  size = ftell (stream);
  DAP_set_def_size (app, size - DAP_get_h_size (app));
  return res;
}

// Loop precedure of reding definitions, iterations finish when no more definition registers are found
int
DAP_do_def_reading (struct app_struct *app, char *buffer, int buffer_size,
		    char *field, int field_size, FILE * stream)
{
  int res = 1;			// 1 is OK, 0 end of registers, -1 error

  while (res == 1)
  {
    res =
      DAP_read_def_reg (app, buffer, buffer_size, field, field_size, stream);
  }
  if (res == -1)
    return -1;
  return 0;
}

// Setting and positioning the stream to start definition registers for reading
int
DAP_set_def_reading (struct app_struct *app, char *buffer, int buffer_size,
		     char *field, int field_size, FILE * stream)
{
  int res = 0;
  int char_r = 0;
  int off_t = 0;

  // Jump the first register "header register"
  char_r = fseek (stream, DAP_get_h_size (app), SEEK_SET);
  if (char_r < 0)
    return -1;

  off_t = DAP_get_off_t_app (app);

  /* DEFINITIONS ALWAYS START AT SECOND LINE! 
     if(off_t == OFFSET_NOT_PRESENT) { */
  // Definitions are staring from second register 
  // "offset register is at the end of file or there is non offset register"
  char_r =
    DAP_do_def_reading (app, buffer, buffer_size, field, field_size, stream);
  if (char_r == -1)
    return -1;
  /*
     } else {
     char_r = fseek(stream, DAP_get_h_size(app)+DAP_get_o_size(app), SEEK_SET);
     // Jump to third register
     char_r = DAP_do_def_reading(app, buffer, buffer_size, field, field_size, stream);
     if(char_r==-1) return -1;
     }  
   */
  return res;

}

// Main procedure for object definition reading
int
DAP_read_definitions (struct app_struct *app)
{
  int res = 0;
  FILE *stream = NULL;
  char open_mode = 'r';

  stream = fopen (app->file_name, &open_mode);
  int buffer_size = DATA_ACCESS_STANDARD_BUFFER_SIZE;
  int field_size = DATA_ACCESS_STANDARD_FIELD_SIZE;

  if (DAP_get_def_flag (app) == 1)
    return res;
  //char* buffer=(char*)malloc(buffer_size*sizeof(char));
  //char* field=(char*)malloc(field_size*sizeof(char));
  char buffer[DATA_ACCESS_STANDARD_BUFFER_SIZE];
  char field[DATA_ACCESS_STANDARD_FIELD_SIZE];
  int c_read = 0;

  if (buffer != NULL && field != NULL)
  {
    c_read =
      DAP_set_def_reading (app, buffer, buffer_size, field, field_size,
			   stream);
    if (c_read == -1)
      res = -1;
  }
  else
  {
    DAP_report_error
      ("DAP_read_definitions in data access api, not enought memory.");
    res = -1;
  }
  //free(buffer);
  //free(field);
  return res;
}

/*
 *	ACCTION READING ROUTINES
 */

// Message reciving action register read
int
DAP_message_rcv_read (char *buffer, int buffer_size, char *field,
		      int field_size, int task_id, int th_id,
		      struct t_action *act, int offset, int actid,
		      int rchar_r)
{
  int res = 0;

  // Read
  // 1. rcv_task
  // 2. msg_zie
  // 3. tag
  // 4. commid
  // 5 rcv type
  int task = 0;
  int msgsize = 0;
  int tag = 0;
  int commid = 0;
  int rcvt = 0;
  int fchar_r = 0;

  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&task);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&msgsize);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset, &tag);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&commid);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&rcvt);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  if (fchar_r == rchar_r)
  {
    res = fchar_r;
    DAP_set_action_msg_rcv (act, actid, task, msgsize, tag, commid, rcvt);
  }
  else
  {
    res = -1;
    DAP_report_error ("Action reading in data access api, sintax error.");

  }
  return res;
}


// Message passing action register reading
int
DAP_message_pass_read (char *buffer, int buffer_size, char *field,
		       int field_size, int task_id, int th_id,
		       struct t_action *act, int offset, int actid,
		       int rchar_r)
{
  int res = 0;

  // Read 
  // 1. dest_task_id
  // 2. msg_size
  // 3. tag
  // 4. comm_id
  // 5. syncronism
  int dest = 0;
  int msgs = 0;
  int tag = 0;
  int commid = 0;
  int sync = 0;
  int fchar_r = 0;

  // Dest reading
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&dest);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  // Msg size reading
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&msgs);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  // Tag reading
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset, &tag);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  // Commid read
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&commid);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  // Sync read
  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&sync);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;
  if (fchar_r == rchar_r)
  {
    // Save results
    DAP_set_action_msg (act, actid, dest, msgs, tag, commid, sync);
    res = fchar_r;
  }
  else
  {
    res = -1;
    DAP_report_error ("Action reading in data access api, sintax error.");
  }
  return res;
}

// Reading action of CPU burst register 
int
DAP_cpu_burst_read (char *buffer, int buffer_size, char *field,
		    int field_size, int task_id, int th_id,
		    struct t_action *act, int offset, int actid, int rchar_r)
{
  int fchar_r = 0;
  int res = 0;
  double burst = 0;

  fchar_r =
    DAP_double_field_read (buffer, buffer_size, field, field_size, offset,
			   &burst);

  if (fchar_r == -1)
  {
    return -1;
  }

  if (fchar_r == rchar_r)
  {
    res = fchar_r;
    DAP_set_action (act, actid, task_id, th_id, burst);
  }
  else
  {
    res = -1;
    DAP_report_error
      ("Action read register in data access api, sintax error.");
  }
  return res;
}

// Read event action register fields
// where,
// 
// buffer: register in memory
// buffer_size: size of buffer
// field: field buffer
// field_size: size of field buffer
// task_id: task id number
// th_id: thread id number
// act: variable that stores event
// offset: start point for field reading inside the register buffer
// actid: action id number
// rchar_r: size of register in chars
//
// returns: total size of register in chars
int
DAP_even_read (char *buffer, int buffer_size, char *field, int field_size,
	       int task_id, int th_id, struct t_action *act, int offset,
	       int actid, int rchar_r)
{
  int res = 0;
  long int type = 0;
  long int value = 0;
  int fchar_r = 0;
  int go = 1;
  struct t_action *next = act;

  while (go == 1)
  {

    fchar_r =
      DAP_long_int_field_read (buffer, buffer_size, field, field_size,
			       offset, &type);
    if (fchar_r == -1)
    {
      DAP_report_error
	("Action read register in data access api, sintax error.");
      return -1;
    }
    offset = fchar_r;

    fchar_r =
      DAP_long_int_field_read (buffer, buffer_size, field, field_size,
			       offset, &value);
    if (fchar_r == -1)
    {
      DAP_report_error
	("Action read register in data access api, sintax error.");
      return -1;
    }
    offset = fchar_r;

    if (fchar_r == rchar_r)
    {
      res = fchar_r;
      DAP_set_even_action (next, actid, type, value);
      go = 0;
    }
    else
    {
      if (fchar_r < rchar_r)
      {
	DAP_set_even_action (next, actid, type, value);
	// Get next action variable
	next = DAP_create_next_action (next);
	if (next != NULL)
	{
	  go = 1;
	}
	else
	{
	  DAP_report_error
	    ("Action read register in data access api, memory error.");
	  go = 0;
	  res = -1;
	}
      }
      else
      {
	DAP_report_error
	  ("Action read register in data access api, sintax error.");
	go = 0;
	res = -1;
      }
    }
  }

  return res;
}

// Global operation register reading
/// where,
// 
// buffer: register in memory
// buffer_size: size of buffer
// field: field buffer
// field_size: size of field buffer
// task_id: task id number
// th_id: thread id number
// act: variable that stores event
// offset: start point for field reading inside the register buffer
// actid: action id number
// rchar_r: size of register in chars
//
// returns: total size of register in chars/
int
DAP_globop_read (char *buffer, int buffer_size, char *field, int field_size,
		 int task_id, int th_id, struct t_action *act, int offset,
		 int actid, int rchar_r)
{
  int res = 0;
  int fchar_r = 0;
  int gop_id = 0;
  int comm_id = 0;
  int r_task_id = 0;
  int r_thread_id = 0;
  long int bsent = 0;
  long int brecv = 0;

  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&gop_id);

  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;

  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&comm_id);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;

  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&r_task_id);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;

  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, offset,
			&r_thread_id);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;

  fchar_r =
    DAP_long_int_field_read (buffer, buffer_size, field, field_size, offset,
			     &bsent);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;

  fchar_r =
    DAP_long_int_field_read (buffer, buffer_size, field, field_size, offset,
			     &brecv);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;

  if (fchar_r == rchar_r)
  {
    res = fchar_r;
    DAP_set_glob_action (act, actid, gop_id, comm_id, r_task_id,
			 r_thread_id, bsent, brecv);
  }
  else
  {
    res = -1;
    DAP_report_error
      ("Action read register in data access api, sintax error.");
  }



  return res;
}

// Reading of action fields 
//// where,
// 
// buffer: register in memory
// buffer_size: size of buffer
// field: field buffer
// field_size: size of field buffer
// task_id: task id number
// th_id: thread id number
// act: variable that stores event
// offset: start point for field reading inside the register buffer
// actid: action id number
// rchar_r: size of register in chars
//
// returns: 0 if operation ended succesfuly
int
DAP_read_actions_fields (char *buffer,
			 int buffer_size,
			 char *field,
			 int field_size,
			 int task_id,
			 int th_id, struct t_action *act, int offset,
			 int actid, int rchar_r)
{
  int res = 0;
  int fchar_r = 0;
  int task = 0;
  int thread = 0;
  double burst = 0;
  long int f_pos = 0;

  // ALL actions have task, thread fields
  // Read task
  fchar_r = DAP_int_field_read (buffer,
				buffer_size, field, field_size, offset,
				&task);
  if (fchar_r == -1)
  {
    return -1;
  }
  offset = fchar_r;

  // Read thread
  fchar_r = DAP_int_field_read (buffer,
				buffer_size, field, field_size, offset,
				&thread);
  if (fchar_r == -1)
  {
    return -1;
  }

  offset = fchar_r;
  // task_id and th_id goes fomr 0..N-1 sould be sumed 1 to compare with fields read in register
  // that go from 1..N
  // task_id++;
  // th_id++;
  if (task == task_id && thread == th_id)
  {

    if (actid == RECORD_CPU_BURST)
    {
      // CPU BURST action
      // Read burst duration
      fchar_r =
	DAP_cpu_burst_read (buffer, buffer_size, field, field_size,
			    task_id, th_id, act, offset, WORK, rchar_r);
      if (fchar_r == -1)
      {
	return -1;
      }
      res = 0;
    }
    if (actid == RECORD_MSG_SEND)
    {
      // MESSAGE SENT action
      // Read 
      // 1. dest_task_id
      // 2. msg_size
      // 3. tag
      // 4. comm_id
      // 5. syncronism
      fchar_r =
	DAP_message_pass_read (buffer, buffer_size, field, field_size,
			       task_id, th_id, act, offset, SEND, rchar_r);
      if (fchar_r == -1)
	return -1;
      res = 0;
    }
    if (actid == RECORD_MSG_RECV)
    {
      // MESSAGE RECIVE action
      fchar_r =
	DAP_message_rcv_read (buffer, buffer_size, field, field_size,
			      task_id, th_id, act, offset, RECV, rchar_r);
      if (fchar_r == -1)
	return -1;
      res = 0;
    }
    if (actid == RECORD_EVENT)
    {
      // EVENT action 
      fchar_r =
	DAP_even_read (buffer, buffer_size, field, field_size, task_id,
		       th_id, act, offset, EVEN, rchar_r);
      if (fchar_r == -1)
	return -1;
      res = 0;

    }
    if (actid == RECORD_GLOBAL_OP)
    {
      // GLOBAL OPERATION
      fchar_r =
	DAP_globop_read (buffer, buffer_size, field, field_size, task_id,
			 th_id, act, offset, GLOBAL_OP, rchar_r);
      if (fchar_r == -1)
	return -1;
      res = 0;
    }
  }
  else
  {
    // It is not action register that we are asking for
    // This may be caused be ausence of action registes that realate with therad
    res = -2;
  }

  return res;
}

// Reading of action register
int
DAP_read_action_reg (struct app_struct *app,
		     int task_id, int thread_id, struct t_action *act,
		     FILE * stream)
{
  int res = 0;
  int char_r = 0;
  int fchar_r = 0;
  int jump_res = 0;
  int buffer_size = DATA_ACCESS_STANDARD_BUFFER_SIZE;
  int id = 0;
  int field_size = DATA_ACCESS_STANDARD_FIELD_SIZE;
  long int f_pos = 0;
  char *buffer = (char *) malloc (buffer_size * sizeof (char));
  char *field = (char *) malloc (field_size * sizeof (char));

  if (buffer == NULL)
  {
    DAP_report_error
      ("DAP_read_action_reg in data api layer, not enought memory.");
    return -1;
  }

  char_r = DAP_read_register (stream, buffer, buffer_size);

  if (char_r < 0)
  {
    res = DAP_close_thread (app, task_id, thread_id);
    free (buffer);
    free (field);
    return -2;
  }

  fchar_r =
    DAP_int_field_read (buffer, buffer_size, field, field_size, 0, &id);
  if (fchar_r == -1)
  {
    free (buffer);
    free (field);
    return -1;
  }

  // Check if id is vaild action register
  if (DAP_seek_int_item (DATA_ACCESS_SUPPORTED_ACTIONS,
			 DATA_ACCESS_SUPPORTED_ACT_NUM, id) == 0)
  {
    // Register id is valid acction register
    res = 0;
    // Read action register fields
    res = DAP_read_actions_fields (buffer,
				   buffer_size,
				   field, field_size, task_id, thread_id, act,
				   fchar_r, id, char_r);
    // Save offset of task

    if (res == 0)
    {
      // Register read sucessfuly
      f_pos = ftell (stream);
      res = DAP_set_offset_thread (app, task_id, thread_id, f_pos);
    }
    else
    {
      if (res == -2)
      {
	// No more registers left to read
	res = DAP_close_thread (app, task_id, thread_id);
	res = -2;
      }
      else
      {
	// Action register is not valid for thread or register is corrupted
	res = DAP_close_thread (app, task_id, thread_id);
	res = -1;
      }
    }
  }
  else
  {
    // Error, not a valid action id of action register or you reached offset record
    if (buffer[0] == DATA_ACCESS_OFFDEF)
    {
      res = DAP_close_thread (app, task_id, thread_id);
      res = -2;
    }
    else
    {
      res = DAP_close_thread (app, task_id, thread_id);
      res = -1;
    }
  }
  free (buffer);
  free (field);
  return res;
}

// Main action read procedure
int
DAP_read_action (struct app_struct *app,
		 int task_id, int thread_id, struct t_action *act)
{
  int res = 0;
  FILE *stream = NULL;

  stream = DAP_get_action_stream (app, task_id, thread_id);

  if (stream != NULL)
  {
    res = DAP_read_action_reg (app, task_id, thread_id, act, stream);
  }
  else
  {
    char ErrorMessage[DATA_ACCESS_STANDARD_BUFFER_SIZE];

    sprintf (ErrorMessage,
	     "Unable to find correct stream for T%02d (t%02d)",
	     task_id + 1, thread_id + 1);
    DAP_report_error (ErrorMessage);
    res = -2;
  }
  return res;
}

/*****************************************************************************
 *  API ROUTINES                                                             *
 ****************************************************************************/

char *
DATA_ACCESS_get_error ()
{
  return get_internal_err_msg ();
}

// Rutines specific for main structure
// eneabling addition as well as conuslts of application structures strored in
// main structure

// Get applictation from conteiner
struct app_struct *
DAP_get_app (int ptask_id)
{
  struct app_struct *app = NULL;
  int num_apps = 0;
  int i = 0;
  int id = 0;

  num_apps = main_struct.num_apps;

  for (i = 0; i < num_apps; i++)
  {
    app = main_struct.apps[i];
    id = DAP_get_ptask_id (app);

    if (id == ptask_id)
      return app;
  }
  // If you came here there is no app whit ptask_id
  app = NULL;
  return app;
}

// Initalizing routines of application, does file reading including
// header, offsets and objects defintions. 
int
DAP_initialize_app_proc (struct app_struct *app, int ptask_id,
			 char *trace_file)
{
  int res = 0;
  int res_h_read = 0;
  int res_o_read = 0;
  int res_d_read = 0;

  res_h_read = DAP_read_header (app, ptask_id, trace_file);
  if (res_h_read == -1)
    return -1;
  res_o_read = read_offsets (app);
  if (res_o_read == -1)
    return -1;
  res_d_read = DAP_read_definitions (app);
  if (res_d_read == -1)
    return -1;
  return res;
}

// Adding of appplication in applications conteiner
int
DAP_add_app (int ptask_id, char *trace_file_location)
{
  int res = 0;
  int num_a = 0;
  int res_i_proc = 0;

  num_a = main_struct.num_apps;

  if (num_a < DATA_ACCESS_MAX_NUM_APP)
  {
    main_struct.apps[num_a] =
      (struct app_struct *) malloc (sizeof (struct app_struct));

    if (main_struct.apps[num_a] == NULL)
    {
      DAP_report_error
	("DATA_ACCESS_get_error in data_access_layer_api, not enought memory");
      return -1;
    }

    res_i_proc =
      DAP_initialize_app_proc (main_struct.apps[num_a], ptask_id,
			       trace_file_location);

    if (res_i_proc == -1)
      return -1;

    // Strart reading procedures: header, offsets, definitiuons etc.
    main_struct.num_apps++;
  }
  else
  {
    // Error, max num of applictions reached.
    DAP_report_error
      ("DATA_ACCESS_get_error in data_access_layer_api, max number of applications reached.");
    return -1;
  }

  return res;
}

// Replacing of appplication in applications conteiner
int
DAP_replace_app (int ptask_id, char *trace_file_location, int index)
{
  int res = 0;
  int num_a = 0;
  int res_i_proc = 0;

  num_a = index;

  if (num_a < DATA_ACCESS_MAX_NUM_APP)
  {
    main_struct.apps[num_a] =
      (struct app_struct *) malloc (sizeof (struct app_struct));

    if (main_struct.apps[num_a] == NULL)
    {
      DAP_report_error
	("DATA_ACCESS_get_error in data_access_layer_api, not enought memory");
      return -1;
    }

    res_i_proc =
      DAP_initialize_app_proc (main_struct.apps[num_a], ptask_id,
			       trace_file_location);

    if (res_i_proc == -1)
      return -1;

  }
  else
  {
    // Error, index num of appliction too high.
    DAP_report_error
      ("DATA_ACCESS_get_error in data_access_layer_api, index number of applications too high.");
    return -1;
  }

  return res;
}

// API SPECIFIC ROUTINES
// INIT OF APPLICATION
t_boolean
DATA_ACCESS_init (int ptask_id, char *trace_file_location)
{

  // Init main structure if necessary
  if (main_struct.init_flag != 1)
    if (create_data_access_layer () < 0)
      return FALSE;

  if (DAP_add_app (ptask_id, trace_file_location) == -1)
    return FALSE;

  return TRUE;
}

// API SPECIFIC ROUTINES
// INIT OF APPLICATION
t_boolean
DATA_ACCESS_init_index (int ptask_id, char *trace_file_location, int index)
{

  // Init main structure if necessary
  if (main_struct.init_flag != 1)
    if (create_data_access_layer () < 0)
      return FALSE;

  if (DAP_replace_app (ptask_id, trace_file_location, index) == -1)
    return FALSE;

  return TRUE;
}

// RETURNS APLLICATION INFOFRMATION
t_boolean
DATA_ACCESS_get_ptask_structure (int ptask_id,
				 struct ptask_structure ** ptask_info)
{
  t_boolean res = 1;
  struct app_struct *app = NULL;
  struct ptask_structure *ptask =
    (struct ptask_structure *) malloc (sizeof (struct ptask_structure));
  if (ptask == NULL)
    return 0;
  app = DAP_get_app (ptask_id);
  if (app != NULL)
  {
    DAP_set_ptask (app, ptask);
    *ptask_info = ptask;
  }
  else
  {
    res = 0;
    *ptask_info = NULL;
    DAP_report_error ("Invalid ptask id.");
  }
  return res;
}

// RETURNS COMMUNICATORS
t_boolean
DATA_ACCESS_get_communicators (int ptask_id, struct t_queue ** communicators)
{
  t_boolean res = 1;
  struct app_struct *app = NULL;
  struct t_queue *comms = NULL;

  app = DAP_get_app (ptask_id);

  if (app != NULL)
  {
    comms = DAP_get_comms (app);
    // Copy queue routine
    *communicators = comms;
  }
  else
  {
    res = 0;
    *communicators = NULL;
    DAP_report_error ("Invalid Ptask Id");
  }
  return res;
}

// RETURNS ACCTION
t_boolean
DATA_ACCESS_get_next_action (int ptask_id,
			     int task_id, int thread_id,
			     struct t_action ** action)
{
  t_boolean res = TRUE;
  int res_act = 0;
  struct app_struct *app = NULL;
  struct t_action *act = DAP_create_action ();

  // Write to log if set as option
  if (DATA_ACCESS_OP_BACKDOOR == 1)
  {
    DAP_write_op_log (ptask_id, task_id, thread_id);
  }

  /* Internally task_id and thread_id are in range 1..N */
  task_id--;
  thread_id--;

  /*
  printf("%s -> Action requested for P%02d T%02d (t%02d)\n",
         __FUNCTION__,
         ptask_id,
         task_id,
         thread_id);
  */

  app = DAP_get_app (ptask_id);

  if (app != NULL && act != NULL)
  {
    if (DAP_has_thread (app, task_id, thread_id) == 0)
    {
      DAP_report_error ("Invalid Task/Thread Id.");
      return FALSE;
    }

    if (DAP_read_action (app, task_id, thread_id, act) < 0)
    { /* No more actions */
      res     = TRUE;
      *action = NULL;
      free (act);
    }
    else
    {
      *action = act;
      res     = TRUE;
    }
  }
  else
  {
    res = FALSE;
    DAP_report_error ("Invalid Ptask Id");
  }
  return res;
}

// ENDS API
void
DATA_ACCESS_end ()
{
  int i = 0;

  for (i = 0; i < main_struct.num_apps; i++)
  {
    if (main_struct.apps[i] != NULL)
      DAP_end_app (main_struct.apps[i]);
  }

  free (main_struct.apps);

  main_struct.init_flag = 0;
  main_struct.num_apps = 0;
}

// ENDS API for ptask_id
// returns the index in the "apps" structure
int 
DATA_ACCESS_ptask_id_end (int ptask_id)
{
  int i = 0;

  for (i = 0; i < main_struct.num_apps; i++)
  {
    if (main_struct.apps[i] != NULL) {
       if (main_struct.apps[i]->ptask_id == ptask_id) {
          /*printf("RELOAD: %d == %d ; i = %d\n", main_struct.apps[i]->ptask_id, ptask_id, i);*/
          DAP_end_app (main_struct.apps[i]);
          return i;
       }
    }
  }

  return -1;
}
/*
 *	PROBE AND INTERNAL TESTING ROUTINES
 */

// Prints all basic data relevant to appliction identified by ptask_id
// At this point DATA_ACCESS_Init should be invoiced previosly in order to have data to print.
int
DATA_ACCESS_test_routine (int ptask_id)
{
  int res = 0;
  struct app_struct *app = NULL;
  struct t_queue *comms = NULL;

  app = DAP_get_app (ptask_id);
  if (app != NULL)
  {
    DAP_print_app_id (app);
    DAP_print_off_v (app);
    DAP_print_task_num (app);
    DAP_print_th_num (app);
    DAP_print_h_size (app);
    DAP_print_o_size (app);
    // DAP_print_def_size(app);
    // DAP_print_o_type(app);
    DAP_print_comm_num (app);
    DAP_print_offsets (app);
    DAP_print_comms (app);
    DAP_print_streams (app);
  }
  return res;
}
