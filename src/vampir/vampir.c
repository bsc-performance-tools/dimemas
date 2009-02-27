char vampir_c_rcsid[] = "$Id: vampir.c,v 1.2 2005/02/15 09:49:39 paraver Exp $";

#include "define.h"

#include "types.h"
#include "extern.h"
#include "list.h"
#include "mallocame.h"
#include "subr.h"
#include "vampir.h"

#include "binlib.h"

int vampir_binary = 1;		/* Write binary tracefiles by default */
int vampir_details = 0;		/* Don't include simulator details */

t_boolean 
vampir_initial_timer = FALSE, vampir_final_timer = FALSE;
/** Disable start/stop time by default **/
t_boolean vampir_start, vampir_stop;
/** Start/stop time for Vampir tracefile defined? **/
dimemas_timer vampir_start_timer, vampir_stop_timer;
/** Start/stop time for Vampir tracefile **/

char *vampir_filename = NULL;	/* No tracefile name defined */

static void    *fp= (void *)0;
static char    *bigpointer;
static char     buffer[80];

static struct t_queue ACTIVITY_queue;
struct t_queue *Activity_queue = &ACTIVITY_queue;

static struct t_activity*
get_activity (char *name, int num)
{
    register struct t_activity *ac;
    
    for (ac=(struct t_activity *)head_queue(Activity_queue);
         ac!=(struct t_activity *)0;
         ac=(struct t_activity *)next_queue(Activity_queue))
    {
        if (ac->number==num)
        {
            if (name==(char *)0)
            {
                return (ac);
            }
            else
            {
                if (strcmp(name,ac->name)==0)
                    return(ac);
            }
        }
    }
    return (ac);
}

static void
new_activity (void *fp, char *name, int num, char *symname)
{
    struct t_activity *ac;    
    
    if (fp == NULL)
      return;
    
    ac = get_activity(name, num);
    if (ac==(struct t_activity *)0)
    {
        ac = (struct t_activity *)mallocame(sizeof(struct t_activity));
        ac->number = num;
        ac->name = mallocame(strlen(name)+1);
        strcpy (ac->name,name);
        ac->symname = mallocame(strlen(symname)+1);
        strcpy (ac->symname, symname);
        inLIFO_queue(Activity_queue, (char *)ac);
    }
    
    BinFmt_WriteSymbol (fp, name, num, symname);
}

void
Vampir_Send(int sender, int receiver, int communicator, int type, int length)
{
  float t1;
  
  if ((fp == NULL) ||
      (vampir_start && (GT_TIMER(vampir_start_timer, current_time))) ||
      (vampir_stop && (GT_TIMER(current_time, vampir_stop_timer))))
    return;
    
  TIMER_TO_FLOAT (current_time, t1);
  BinFmt_WriteSENDMSG (fp, t1, (sender - 1), (receiver - 1),
		       communicator, type, length);
  return;
}        
        
void
Vampir_Receive(int receiver, int sender, int communicator, int type,
	       int length)
{
  float t1;

  if ((fp == NULL) ||
      (vampir_start && (GT_TIMER(vampir_start_timer, current_time))) ||
      (vampir_stop && (GT_TIMER(current_time, vampir_stop_timer))))
    return;

  TIMER_TO_FLOAT(current_time, t1);
  BinFmt_WriteRECVMSG (fp, t1, (receiver - 1), (sender - 1),
		       communicator, type, length);
  return;
}        
        

void 
Activity_Enter (int from_user, struct t_thread *thread, int activityid)
{
  float t1;
  struct t_activity *ac, *last_ac;    
    
  if (fp == NULL)
    return;
  
  if (from_user != 0)		/* Log user-level activity */
    ac = get_activity((char *)0, activityid);
  else if (vampir_details)	/* Log simulator activity */
    ac = get_activity("DIMEMAS", activityid);
  else				/* Don't log simulator activity */
    return;
    
  if (ac == NULL) {
    TIMER_TO_FLOAT(current_time, t1);
    if (from_user)
      sprintf (bigpointer, "%f Unkown activity: %d\n", t1, activityid);
    else
      sprintf (bigpointer, "%f Unkown activity: DIMEMAS %d\n", t1, activityid);
    panic (bigpointer);
  }
    
  last_ac = (struct t_activity *) tail_queue (&(thread->Activity));
  inFIFO_queue (&(thread->Activity), (char *)ac);

  if ((vampir_start && (GT_TIMER(vampir_start_timer, current_time))) ||
      (vampir_stop && (GT_TIMER(current_time, vampir_stop_timer))))
    return;

  TIMER_TO_FLOAT(current_time, t1);
  BinFmt_WriteEXCHANGE(fp, t1, (thread->task->taskid - 1), 
		       (last_ac != NULL ? last_ac->name : ""),
		       (last_ac != NULL ? last_ac->number : BINFMT_NOACTNUM),
		       ac->name, ac->number, BINFMT_NOJOB);
}

void 
Activity_Exit(int from_user,  struct t_thread *thread, int activityid)
{
  float t1;
  register struct t_activity *ac, *last_ac;    
 
  if (fp == NULL)
    return; 
  
  if (from_user != 0)		/* Log user-level activity */
    ac = get_activity((char *)0, activityid);
  else if (vampir_details)	/* Log simulator activity */
    ac = get_activity("DIMEMAS", activityid);
  else				/* Don't log simulator activity */
    return;
    
  if (ac == NULL)
    panic ("Unkown activity\n");
    
  last_ac = (struct t_activity *) outLIFO_queue(&(thread->Activity));
  if (last_ac != ac) {
    sprintf (bigpointer, "Exiting activity %s %d that has not been enter\n",
	     from_user ? "" : "DIMEMAS", activityid);
    panic(bigpointer);
  }
    
  ac = (struct t_activity *)tail_queue (&(thread->Activity));

  if ((vampir_start && (GT_TIMER(vampir_start_timer, current_time))) ||
      (vampir_stop && (GT_TIMER(current_time, vampir_stop_timer))))
    return;
    
  TIMER_TO_FLOAT(current_time, t1);
  BinFmt_WriteEXCHANGE (fp, t1, (thread->task->taskid - 1), 
			last_ac->name, last_ac->number,
			(ac == NULL ? "" : ac->name),
			(ac == NULL ? BINFMT_NOACTNUM : ac->number),
			BINFMT_NOJOB);
}

void
Vampir_GlobalOpToken(int token, char *name)
{
  if (fp == NULL)
    return; 
  
  BinFmt_WriteGLOBALOPTOKEN(fp, token, name);
}

void
Vampir_GlobalOp(dimemas_timer t1, dimemas_timer t2, int optoken, 
    struct t_thread *thread, int commid, struct t_thread *root,
    int bsend, int brecv)
{
  float t11, t12;
  if (fp == NULL)
    return;
  
  TIMER_TO_FLOAT (t1, t11);
  SUB_TIMER (t2, t1, t2);
  TIMER_TO_FLOAT(t2, t12);
  if (t12<0)
    t12 = 0;
  BinFmt_WriteGlobalOp (fp, t11,
      optoken, (thread->task->taskid - 1), commid, (root->task->taskid - 1), 
      bsend, brecv, t12);
}

void
Vampir_communicator (int commid, int commsize, int tripcount, int *trips)
{
    if (fp == NULL)
    return;
  
    BinFmt_WriteCOMDEF(fp, commid, commsize, tripcount, trips);
}

void
Vampir_Start (char *filename)
{
   int             i;
   struct t_Ptask *Ptask;
   time_t          h;
   char           *hostname;
   double          cp;
   int             numprocs;
   
   create_queue (Activity_queue);

   Ptask = (struct t_Ptask *) head_queue (&Ptask_queue);   
   numprocs = count_queue (&(Ptask->tasks));
#ifdef DEBUG
   fprintf(stderr, "numprocs = %d\n", numprocs);
#endif

   bigpointer = (char *) mallocame (BUFSIZE); /* Abans era 4096 */
   fp = BinFmt_CreateFile (filename, (vampir_binary != 0));
   
   if (fp==(void *)0)
   {
       sprintf (bigpointer, "Unable to use %s as output for VAMPIR traces\n",
               filename);
       panic (bigpointer);
   }

/* Creator record */
   hostname = (char *) mallocame (32);
   gethostname (hostname, 32);
   time (&h);
   strftime (buffer, 80, "%d.%m.%y - %H:%M", localtime (&h));
   sprintf (bigpointer, "Dimemas %d.%d on %s (%s)",
	    VERSION, SUBVERSION, hostname, buffer);
   freeame (hostname, 32);
   BinFmt_WriteCreator (fp, bigpointer);

/* Clock period record */
   cp = 1e-6;
   BinFmt_WriteCLKPERIOD (fp, cp);

/* Number of CPU's record */
   BinFmt_WriteNCPUs (fp, 1L, &numprocs);

   new_activity (fp, "DIMEMAS", 1, "Latency");
   new_activity (fp, "DIMEMAS", 2, "Blocking");
   new_activity (fp, "DIMEMAS", 3, "Group Blocking");
}

void
Vampir_new_application_symbol (char *name, int num, char *symname)
{
  if (fp == NULL)
    return;
    
  new_activity (fp, name, num, symname);
}

void
Vampir_Stop()
{
  if (fp == NULL)
    return;
    
  BinFmt_CloseWriteFile(fp);    
}
