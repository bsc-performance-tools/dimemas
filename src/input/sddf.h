#ifndef __sddf_h
#define __sddf_h

/**
 * External routines defined in file sddf.c
 **/
extern void SDDF_start(int cpuid, int taskid, dimemas_timer time);
extern void SDDF_stop(int cpuid, int taskid, dimemas_timer time);
extern void SDDF_recv_start (int cpuid_r, int taskid_r, 
			     dimemas_timer logical_time);
extern void SDDF_recv_block (int cpuid_r, int taskid_r, 
			     dimemas_timer logical_time);
extern void SDDF_recv_stop(int cpuid_r, int taskid_r, dimemas_timer tim,
			   int tag, int size, int cpuid_s, int taskid_s);
extern void SDDF_send_start(int cpuid_s, int taskid_s, dimemas_timer physical);
extern void SDDF_send_stop(int cpuid_s, int taskid_s, dimemas_timer physical,
			   int tag, int size, int cpuid_r, int taskid_r);
extern void SDDF_in_message (int by);
extern void SDDF_out_message(int by);
extern void SDDF_do(void);

#endif
