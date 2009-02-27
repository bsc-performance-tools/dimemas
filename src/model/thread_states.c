#include "thread_states.h"

t_boolean
init_thread_state (struct t_thread* thread, int state)
{
  t_thread_state new_state;
  t_boolean      result;
  
  result = TRUE;
  
  if (thread->current_state != STATE_NIL)
  {
    result   = FALSE;
    errorStr = "Initalizing new thread state with previous state open\n";
  }
  else
  {
    new_state = (t_thread_state) mallocame(sizeof(struct thread_state));
    
    new_state->state      = state;
    new_state->init_time  = current_time;
    thread->current_state = new_state;
  }
  return result;
}

t_boolean
end_thread_state (struct t_thread* thread, int state)
{
  t_boolean result = TRUE;
  
  if (thread->current_state == STATE_NIL)
  {
    result   = FALSE;
    errorStr = "Trying to close an uninitialized state\n";
  }
  else if (thread->current_state->state != state)
  {
    result   = FALSE;
    errorStr = "Trying to close different state that initialized one\n";
  }
  else
  {
    /* JGG (02/05/2005) */
    SUB_TIMER (current_time, thread->current_state->init_time, last_state_time);
    
    freeame(thread->current_state, sizeof(struct thread_state));
    thread->current_state = STATE_NIL;
  }
}

char*
get_last_state_error(void)
{
  return errorStr;
}

extern dimemas_timer
get_last_state_time(void)
{
  return last_state_time;
}
