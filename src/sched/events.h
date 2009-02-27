#ifndef __events_h
#define __events_h
/**
 * External routines defined in file events.c
 **/

extern struct t_event*
EVENT_timer(
  dimemas_timer    when,
  int              daemon, 
  int              module,
  struct t_thread *thread,
  int              info
);

extern void
EVENT_extract_timer(
  int              module,
  struct t_thread *thread, 
  dimemas_timer   *when
);

extern void
EVENT_init(void);

extern void
EVENT_end(void);

extern t_boolean
events_for_thread (struct t_thread *thread);

extern void
reload_events(void);

extern void
event_manager(struct t_event *event);

#endif
