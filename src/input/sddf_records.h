#ifndef RCS_sddf_records_h
#define RCS_sddf_records_h
static char sddf_records_h_rcsid[]="$Id: sddf_records.h,v 1.4 2005/05/24 15:41:40 paraver Exp $";
static char *__a_sddf_records_h=sddf_records_h_rcsid;
#endif
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1994, 1995, 1996 - CENTRO EUROPEO DE PARALELISMO DE BARCELONA
*				  - UNIVERSITAT POLITECNICA DE CATALUNYA
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON. NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY CEPBA-UPC
*  OR ITS THIRD PARTY SUPPLIERS  
*  
*  	CEPBA-UPC AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE . DIMEMAS AND ITS GUI IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND CEPBA-UPC EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
* 
*  
*******************************************************************************
******************************************************************************/

/*
 * sddf_records.h
 * 
 * Sergi Girona (sergi@ac.upc.es)
 * 
 * (c) DAC-UPC 1993-94
 * 
 */

/* Configuration file */


/* Record 0 - DAMIEN format */
#define SDDFA_0C_NAME   "wide area network information"
#define SDDFA_0C_1A_D   "wan_name"
#define SDDFA_0C_2A_D   "number_of_machines"
#define SDDFA_0C_3A_D   "number_dedicated_connections"
#define SDDFA_0C_4A_D   "function_of_traffic"
#define SDDFA_0C_5A_D   "max_traffic_value"
#define SDDFA_0C_6A_D   "external_net_bandwidth"
#define SDDFA_0C_7A_D   "communication_group_model"


/* Record 1 */
#define SDDFA_1C_NAME "environment information"
  /* Old format */
#define SDDFA_1C_1A   "instrumented_architecture"
#define SDDFA_1C_2A   "number_of_nodes"
#define SDDFA_1C_3A   "network_bandwidth"
#define SDDFA_1C_4A   "number_of_buses_on_network"
#define SDDFA_1C_5A   "communication_group_model"
  /* DAMIEN format */
#define SDDFA_1C_1A_D   "machine_name"
#define SDDFA_1C_2A_D   "machine_id"
#define SDDFA_1C_3A_D   "instrumented_architecture"
#define SDDFA_1C_4A_D   "number_of_nodes"
#define SDDFA_1C_5A_D   "network_bandwidth"
#define SDDFA_1C_6A_D   "number_of_buses_on_network"
#define SDDFA_1C_7A_D   "communication_group_model"


/* Record 2 */
#define SDDFA_2C_NAME	"node information"
  /* Old format */
#define SDDFA_2C_1A	"node_id"
#define SDDFA_2C_2A	"simulated_architecture"
#define SDDFA_2C_3A	"number_of_processors"
#define SDDFA_2C_4A	"number_of_input_links"
#define SDDFA_2C_5A	"number_of_output_links"
#define SDDFA_2C_6A	"startup_on_local_communication"
#define SDDFA_2C_7A	"startup_on_remote_communication"
#define SDDFA_2C_8A	"speed_ratio_instrumented_vs_simulated"
#define SDDFA_2C_9A	"memory_bandwidth"
#define SDDFA_2C_10A	"local_port_startup"
#define SDDFA_2C_11A	"remote_port_startup"
#define SDDFA_2C_12A	"local_memory_startup"
#define SDDFA_2C_13A	"remote_memory_startup"
  /* DAMIEN format */
#define SDDFA_2C_1A_D	"machine_id"
#define SDDFA_2C_2A_D	"node_id"
#define SDDFA_2C_3A_D	"simulated_architecture"
#define SDDFA_2C_4A_D	"number_of_processors"
#define SDDFA_2C_5A_D	"number_of_input_links"
#define SDDFA_2C_6A_D	"number_of_output_links"
#define SDDFA_2C_7A_D	"startup_on_local_communication"
#define SDDFA_2C_8A_D	"startup_on_remote_communication"
#define SDDFA_2C_9A_D	"speed_ratio_instrumented_vs_simulated"
#define SDDFA_2C_10A_D	"memory_bandwidth"
#define SDDFA_2C_11A_D	"external_net_startup"
#define SDDFA_2C_12A_D	"local_port_startup"
#define SDDFA_2C_13A_D	"remote_port_startup"
#define SDDFA_2C_14A_D	"local_memory_startup"
#define SDDFA_2C_15A_D	"remote_memory_startup"
  
  
/* Record 3 */
#define SDDFA_3C_NAME	"mapping information"
#define SDDFA_3C_1A	"tracefile"
#define SDDFA_3C_2A	"number_of_tasks"
#define SDDFA_3C_3A	"mapping_tasks_to_nodes"
#define SDDFA_3C_4A	"priority"


/* Record 4 */
#define SDDFA_4C_NAME	"configuration files"
#define SDDFA_4C_1A	"scheduler"
#define SDDFA_4C_2A	"file_system"
#define SDDFA_4C_3A	"communication"
#define SDDFA_4C_4A	"sensitivity"


/* Record 5 */
#define SDDFA_5C_NAME   "modules information"
#define SDDFA_5C_1A     "identificator"
#define SDDFA_5C_2A     "execution_ratio"


/* Record 6 */
#define SDDFA_6C_NAME   "file system parameters"
#define SDDFA_6C_1A     "disk latency"
#define SDDFA_6C_2A     "disk bandwidth"
#define SDDFA_6C_3A     "block size"
#define SDDFA_6C_4A     "concurrent requests"
#define SDDFA_6C_5A     "hit ratio"


/* Record 7 - DAMIEN format */
#define SDDFA_7C_NAME   "dedicated connection information"
#define SDDFA_7C_1A_D   "connection_id"
#define SDDFA_7C_2A_D   "source_machine"
#define SDDFA_7C_3A_D   "destination_machine"
#define SDDFA_7C_4A_D   "connection_bandwidth"
#define SDDFA_7C_5A_D   "tags_list"
#define SDDFA_7C_6A_D   "first_message_size"
#define SDDFA_7C_7A_D   "first_size_condition"
#define SDDFA_7C_8A_D   "operation"
#define SDDFA_7C_9A_D   "second_message_size"
#define SDDFA_7C_10A_D  "second_size_condition"
#define SDDFA_7C_11A_D  "list_communicators"
#define SDDFA_7C_12A_D	"connection_startup"
#define SDDFA_7C_13A_D	"flight_time"




/* Trace file */

/* Record 1 */
#define SDDFA_1_NAME	"CPU burst"
#define SDDFA_1_1A	"taskid"
#define SDDFA_1_2A	"thid"
#define SDDFA_1_3A	"time"

/* Record 2 */
#define SDDFA_2_NAME	"NX send"
#define SDDFA_2_1A	"taskid"
#define SDDFA_2_2A	"thid"
#define SDDFA_2_3A	"dest taskid"
#define SDDFA_2_4A	"msg length"
#define SDDFA_2_5A	"tag"
#define SDDFA_2_6A	"commid"
#define SDDFA_2_7A     "synchronism"
#define SDDFA_2_7A_OLD "use_rendezvous" /* JGG (18/03/2005): Nombre 'viejo' */

/* Record 3 */
#define SDDFA_3_NAME	"NX recv"
#define SDDFA_3_1A	"taskid"
#define SDDFA_3_2A	"thid"
#define SDDFA_3_3A	"source taskid"
#define SDDFA_3_4A	"msg length"
#define SDDFA_3_5A	"tag"
#define SDDFA_3_6A	"commid"
#define SDDFA_3_7A	"type"

/* Record 40 */
#define SDDFA_40_NAME	"block begin"
#define SDDFA_40_1A	"taskid"
#define SDDFA_40_2A	"thid"
#define SDDFA_40_3A	"blockid"

/* Record 41 */
#define SDDFA_41_NAME	"block end"
#define SDDFA_41_1A	"taskid"
#define SDDFA_41_2A	"thid"
#define SDDFA_41_3A	"blockid"

/* Record 42 */
#define SDDFA_42_NAME   "block definition"
#define SDDFA_42_1A     "block_id"
#define SDDFA_42_2A     "block_name"
#define SDDFA_42_3A     "activity_name"
#define SDDFA_42_4A     "src_file"
#define SDDFA_42_5A     "src_line"

/* Record 3 */
#define SDDFA_43_NAME   "file definition"
#define SDDFA_43_1A     "file_id"
#define SDDFA_43_2A     "location"

/* Record 48 */
#define SDDFA_48_NAME	"user event type definition"
#define SDDFA_48_1A	"type"
#define SDDFA_48_2A	"name"
#define SDDFA_48_3A	"color"

/* Record 49 */
#define SDDFA_49_NAME	"user event value definition"
#define SDDFA_49_1A	"type"
#define SDDFA_49_2A	"value"
#define SDDFA_49_3A	"name"

/* Record 50 */
#define SDDFA_50_NAME	"user event"
#define SDDFA_50_1A	"taskid"
#define SDDFA_50_2A	"thid"
#define SDDFA_50_3A	"type"
#define SDDFA_50_4A	"value"

/* Record 101 */
#define SDDFA_101_NAME	"FS open"
#define SDDFA_101_1A		"taskid"
#define SDDFA_101_2A          "thid"
#define SDDFA_101_3A          "fd"
#define SDDFA_101_4A          "initial_size"
#define SDDFA_101_5A          "open_mode"
#define SDDFA_101_6A          "filename"

/* Record 102 */
#define SDDFA_102_NAME	"FS read"
#define SDDFA_102_1A		"taskid"
#define SDDFA_102_2A          "thid"
#define SDDFA_102_3A          "fd"
#define SDDFA_102_4A          "requested_size"
#define SDDFA_102_5A          "delivered_size"

/* Record 103 */
#define SDDFA_103_NAME	"FS write"
#define SDDFA_103_1A		"taskid"
#define SDDFA_103_2A          "thid"
#define SDDFA_103_3A          "fd"
#define SDDFA_103_4A          "requested_size"
#define SDDFA_103_5A          "delivered_size"

/* Record 104 */
#define SDDFA_104_NAME	"FS seek"
#define SDDFA_104_1A		"taskid"
#define SDDFA_104_2A          "thid"
#define SDDFA_104_3A          "fd"
#define SDDFA_104_4A          "offset"
#define SDDFA_104_5A          "whence"
#define SDDFA_104_6A          "result"

/* Record 105 */
#define SDDFA_105_NAME	"FS close"
#define SDDFA_105_1A		"taskid"
#define SDDFA_105_2A          "thid"
#define SDDFA_105_3A          "fd"

/* Record 106 */
#define SDDFA_106_NAME	"FS dup"
#define SDDFA_106_1A		"taskid"
#define SDDFA_106_2A          "thid"
#define SDDFA_106_3A          "old_fd"
#define SDDFA_106_4A          "new_fd"

/* Record 107 */
#define SDDFA_107_NAME	"FS unlink"
#define SDDFA_107_1A		"taskid"
#define SDDFA_107_2A          "thid"
#define SDDFA_107_3A          "filename"

/* Record 100 */
#define SDDFA_100_NAME "communicator definition"
#define SDDFA_100_1A   "comm_id"
#define SDDFA_100_2A   "comm_size"
#define SDDFA_100_3A   "global_ranks"  

/* Record 200 */
#define SDDFA_200_NAME "global OP definition"
#define SDDFA_200_1A   "glop_id"
#define SDDFA_200_2A   "glop_name"

/* Record 201 */
#define SDDFA_201_NAME "global OP"
#define SDDFA_201_1A   "rank"
#define SDDFA_201_2A   "thid"
#define SDDFA_201_3A   "glop_id"
#define SDDFA_201_4A   "comm_id"
#define SDDFA_201_5A   "root_rank"
#define SDDFA_201_6A   "root_thid"
#define SDDFA_201_7A   "bytes_sent"
#define SDDFA_201_8A   "bytes_recvd"

#define SDDFA_3
/* Record 300 */
#define SDDFA_300_NAME "IO OP definition"
#define SDDFA_300_1A   "ioop_id"
#define SDDFA_300_2A   "ioop_name"

/* Record 301 */
#define SDDFA_301_NAME "IO Collective Metadata"
#define SDDFA_301_1A   "taskid"
#define SDDFA_301_2A   "thid"
#define SDDFA_301_3A   "commid"
#define SDDFA_301_4A   "fh"
#define SDDFA_301_5A   "Oop"

/* Record 302 */
#define SDDFA_302_NAME "IO Blocking NonCollective"
#define SDDFA_302_1A   "taskid"
#define SDDFA_302_2A   "thid"
#define SDDFA_302_3A   "size"
#define SDDFA_302_4A   "Oop"

/* Record 303 */
#define SDDFA_303_NAME "IO Blocking Collective"
#define SDDFA_303_1A   "taskid"
#define SDDFA_303_2A   "thid"
#define SDDFA_303_3A   "fh"
#define SDDFA_303_4A   "size"
#define SDDFA_303_5A   "Oop"

/* Record 304 */
#define SDDFA_304_NAME "IO NonBlocking NonCollective begin"
#define SDDFA_304_1A   "taskid"
#define SDDFA_304_2A   "thid"
#define SDDFA_304_3A   "size"
#define SDDFA_304_4A   "request"
#define SDDFA_304_5A   "Oop"

/* Record 305 */
#define SDDFA_305_NAME "IO NonBlocking NonCollective end"
#define SDDFA_305_1A   "taskid"
#define SDDFA_305_2A   "thid"
#define SDDFA_305_3A   "request"
#define SDDFA_305_4A   "Oop"

/* Record 306 */
#define SDDFA_306_NAME "IO NonBlocking Collective begin"
#define SDDFA_306_1A   "taskid"
#define SDDFA_306_2A   "thid"
#define SDDFA_306_3A   "fh"
#define SDDFA_306_4A   "size"
#define SDDFA_306_5A   "Oop"

/* Record 307 */
#define SDDFA_307_NAME "IO NonBlocking Collective end"
#define SDDFA_307_1A   "taskid"
#define SDDFA_307_2A   "thid"
#define SDDFA_307_3A   "fh"
#define SDDFA_307_4A   "Oop"


/* Record 400 */
#define SDDFA_400_NAME "OS window definition"
#define SDDFA_400_1A   "win_id"
#define SDDFA_400_2A   "win_size"
#define SDDFA_400_3A   "global_ranks"  

/* Record 401 */
#define SDDFA_401_NAME "OS operation"
#define SDDFA_401_1A   "taskid"
#define SDDFA_401_2A   "thid"
#define SDDFA_401_3A   "Oop"
#define SDDFA_401_4A   "target_rank"
#define SDDFA_401_5A   "win_id"
#define SDDFA_401_6A   "size"

/* Record 402 */
#define SDDFA_402_NAME "OS fence"
#define SDDFA_402_1A   "taskid"
#define SDDFA_402_2A   "thid"
#define SDDFA_402_3A   "win_id"

/* Record 403 */
#define SDDFA_403_NAME "OS lock"
#define SDDFA_403_1A   "taskid"
#define SDDFA_403_2A   "thid"
#define SDDFA_403_3A   "win_id"
#define SDDFA_403_4A   "Oop"
#define SDDFA_403_5A   "mode"

/* Record 404 */
#define SDDFA_404_NAME "OS post"
#define SDDFA_404_1A   "taskid"
#define SDDFA_404_2A   "thid"
#define SDDFA_404_3A   "win_id"
#define SDDFA_404_4A   "Oop"
#define SDDFA_404_5A   "post_size"
#define SDDFA_404_6A   "post_ranks"
