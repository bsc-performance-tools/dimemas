/*
 * C Interface for Dimemas
 */

#ifdef __cplusplus
extern "C" {
#endif

#define PRINT_VENUS_INFO     0
#define PRINT_VENUS_SENDS    1
#define VENUS_STATS          1

/* extern int channel_socket; */
extern int venusmsgs_in_flight;
extern int venus_enabled;

int vc_initialize(char const *venusconn);
int vc_finish();

int vc_send(char *s);

int vc_recv(char *s, int maxlen);

int print_event (struct t_event *event);
struct t_event* EVENT_venus_timer(dimemas_timer when, int daemon, int module, struct t_thread *thread, int info);
int venus_outFIFO_event (struct t_queue *q, struct t_item *e);

int vc_command_send(double dtime, int src, int dest, int size, void *event, void *out_resources_event);
int vc_command_rdvz_send (double dtime, int src, int dest, int tag, int size);
int vc_command_rdvz_ready (double dtime, int src, int dest, int tag, int size, void *event, void *event_resources_out);
	
#ifdef __cplusplus
}
#endif
