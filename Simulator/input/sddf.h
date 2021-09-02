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

#ifndef __sddf_h
#define __sddf_h

/**
 * External routines defined in file sddf.c
 **/
extern void SDDF_start( int cpuid, int taskid, dimemas_timer time );
extern void SDDF_stop( int cpuid, int taskid, dimemas_timer time );
extern void SDDF_recv_start( int cpuid_r, int taskid_r, dimemas_timer logical_time );
extern void SDDF_recv_block( int cpuid_r, int taskid_r, dimemas_timer logical_time );
extern void SDDF_recv_stop( int cpuid_r, int taskid_r, dimemas_timer tim, int tag, int size, int cpuid_s, int taskid_s );
extern void SDDF_send_start( int cpuid_s, int taskid_s, dimemas_timer physical );
extern void SDDF_send_stop( int cpuid_s, int taskid_s, dimemas_timer physical, int tag, int size, int cpuid_r, int taskid_r );
extern void SDDF_in_message( int by );
extern void SDDF_out_message( int by );
extern void SDDF_do( void );

#endif
