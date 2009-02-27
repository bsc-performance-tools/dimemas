#include "paraver_pcf.h"
#include "define.h"
#include "types.h"

/* ESTADOS ********************************************************************/
/* Estos estados se definen en este punto, puesto que son internos a Dimemas
 * y no se 'exportan' a las trazas Paraver, por lo que no se incluyen en el
 * fichero 'paraver_pcf.h' 
 * Los estados se definen numericamente como incrementos sobre PRV_STATE_COUNT
 * para evitar, de esta manera, el solapamiento con los estados definidos para
 * las trazas Paraver
 */
 
#define WAIT_LINKS_ST  PRV_STATE_COUNT
#define BUSY_WAIT_ST   PRV_STATE_COUNT+1
#define CONTEXT_SWX_ST PRV_STATE_COUNT+2
 
#define TOTAL_THREAD_STATES PRV_STATE_COUNT+3


extern dimemas_timer current_time;

char          *errorStr;
dimemas_timer  last_state_time;

/* FUNCIONES PUBLICAS *********************************************************/
extern t_boolean
init_thread_state (struct t_thread* thread, int state);

extern t_boolean
end_thread_state (struct t_thread* thread, int state);

extern char*
get_last_state_error(void);

extern dimemas_timer
get_last_state_time(void);
