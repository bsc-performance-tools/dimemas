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

/*
 * C Interface for Dimemas
 */

#ifdef __cplusplus
extern "C"
{
#endif

#define PRINT_VENUS_INFO  0
#define PRINT_VENUS_SENDS 0
#define VENUS_STATS       0

  /* extern int channel_socket; */
  extern int venusmsgs_in_flight;

  extern void VC_enable( const char *venus_conn_url );
  extern t_boolean VC_is_enabled( void );

  extern void VC_Init( void );
  extern void VC_End( void );

  extern int VC_send( char *s );
  extern int VC_recv( char *s, int maxlen );

  int print_event( struct t_event *event );
  struct t_event *EVENT_venus_timer( dimemas_timer when, int daemon, int module, struct t_thread *thread, int info );
#ifdef USE_EQUEUE
  int venus_outFIFO_event( Equeue *q, struct t_event *e );
#else
int venus_outFIFO_event( struct t_queue *q, struct t_event *e );
#endif

  int VC_command_send( double dtime, int src, int dest, int tag, int size, void *event, void *out_resources_event, int src_app, int dest_app );
  int VC_command_rdvz_send( double dtime, int src, int dest, int tag, int size, int src_app, int dest_app );
  int VC_command_rdvz_ready( double dtime, int src, int dest, int tag, int size, void *event, void *event_resources_out, int src_app, int dest_app );


#ifdef __cplusplus
}
#endif
