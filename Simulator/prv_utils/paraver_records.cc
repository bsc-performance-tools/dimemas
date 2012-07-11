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

  $URL:: https://svn.bsc.es/repos/DIMEMAS/trunk/s#$:  File
  $Rev:: 35                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-01-11 19:45:04 +0100 (Wed, 11 Jan #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifdef __cplusplus
extern "C" {
#endif

#include "paraver.h"

#ifdef __cplusplus
}
#endif

#include <iostream>
using std::ostream;

#include <cstdio>

#include "paraver_records.h"

void SimpleParaverRecord::toTracefile(FILE* prv_file)
{
  int real_type = _TYPE;

  if (_TYPE == PRV_STATE_RUN)
    real_type = PRV_STATE;

  fprintf(prv_file,
          "%d:%d:%d:%d:%d:%llu:%s\n",
          real_type,
          _CPU,
          _Ptask+1,
          _Task+1,
          _Thread+1,
          _Timestamp,
          _ascii_record.c_str());
}

void SimpleParaverRecord::serialize(FILE* prv_file)
{
  fprintf(prv_file,
          "%d:%d:%d:%d:%d:%llu:%s\n",
          _TYPE,
          _CPU,
          _Ptask,
          _Task,
          _Thread,
          _Timestamp,
          _ascii_record.c_str());
}

bool SimpleParaverRecord::deserialize(const char* ascii_record)
{
  if (ascii_record == NULL)
    return false;

  // _ascii_record.clear();
  char* ascii_record_def = (char*) malloc(strlen(ascii_record));

  int matches = sscanf(ascii_record,
                       "%d:%d:%d:%d:%d:%llu:%s\n",
                       &_TYPE,
                       &_CPU,
                       &_Ptask,
                       &_Task,
                       &_Thread,
                       &_Timestamp,
                       ascii_record_def);

  if (matches != 7)
  {
    printf("Number of matches = %d\n", matches);
    return false;
  }

  _ascii_record = string(ascii_record_def);

  /* This free caused an error when deleting the record! */
  // free(ascii_record_def);

  return true;
}

bool operator< (const SimpleParaverRecord& R1, const SimpleParaverRecord& R2)
{
  // std::cout << "R1 Timestamp = " << R1.Timestamp() << " and R2 Timestamp = " << R2.Timestamp() << std::endl;

  if (R1.Timestamp() < R2.Timestamp())
    return true;

  if (R1.Timestamp() > R2.Timestamp())
    return false;

  if (R1.Ptask() < R2.Ptask())
    return true;

  if (R1.Ptask() > R2.Ptask())
    return false;

  if (R1.Task() < R2.Task())
    return true;

  if (R1.Task() > R2.Task())
    return false;

  if (R1.Thread() < R2.Thread())
    return true;

  if (R1.Thread() > R2.Thread())
    return false;

  if (R1.CPU() < R2.CPU())
    return true;

  if (R1.CPU() > R2.CPU())
    return false;

  /*
   * When simultaneous, first events, then states and the comms
   */

  /* Running state after events and before comms */
  if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_EVENT)
    return false;

  if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_STATE)
    return true;

  if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_COMM)
    return true;

  if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_STATE_RUN) // SHOULD NEVER HAPPEN
    return true;

  /* Events after running states,  */
  if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_STATE_RUN)
    return true;

  if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_STATE)
    return true;

  if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_COMM)
    return true;

  if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_EVENT)
    return true;


  if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_STATE_RUN)
    return false;

  if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_EVENT)
    return false;

  if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_COMM)
    return true;

  if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_STATE)
    return false;

  /* All communications after everything else */
  return false;
}

std::ostream& operator << (std::ostream& o, const SimpleParaverRecord& obj)
{
  o << obj.TYPE() << ":"  << obj.CPU()     << ":" << obj.Ptask()     << ":";
  o << obj.Task() << ":"  << obj.Thread()  << ":" << obj.Timestamp() << ":";
  o << obj.ascii_record();

  return o;
}

class SimpleTimeOrder
{
  public:
    bool operator()(SimpleParaverRecord R1, SimpleParaverRecord R2)
    {
      return (R1.Timestamp() < R2.Timestamp());
    }
};

class ComplexRecordOrder
{
  public:
    bool operator()(SimpleParaverRecord R1, SimpleParaverRecord R2)
    {
      // std::cout << "R1 Timestamp = " << R1.Timestamp() << " and R2 Timestamp = " << R2.Timestamp() << std::endl;

      if (R1.Timestamp() < R2.Timestamp())
        return true;

      if (R1.Timestamp() > R2.Timestamp())
        return false;

      if (R1.Ptask() < R2.Ptask())
        return true;

      if (R1.Ptask() > R2.Ptask())
        return false;

      if (R1.Task() < R2.Task())
        return true;

      if (R1.Task() > R2.Task())
        return false;

      if (R1.Thread() < R2.Thread())
        return true;

      if (R1.Thread() > R2.Thread())
        return false;

      if (R1.CPU() < R2.CPU())
        return true;

      if (R1.CPU() > R2.CPU())
        return false;

      /*
       * When simultaneous, first events, then states and the comms
       */

      /* Running state after events and before comms */
      if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_EVENT)
        return false;

      if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_STATE)
        return true;

      if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_COMM)
        return true;

      if (R1.TYPE() == PRV_STATE_RUN && R2.TYPE() == PRV_STATE_RUN) // SHOULD NEVER HAPPEN
        return true;

      /* Events after running states,  */
      if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_STATE_RUN)
        return true;

      if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_STATE)
        return true;

      if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_COMM)
        return true;

      if (R1.TYPE() == PRV_EVENT && R2.TYPE() == PRV_EVENT)
        return true;


      if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_STATE_RUN)
        return false;

      if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_EVENT)
        return false;

      if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_COMM)
        return true;

      if (R1.TYPE() == PRV_STATE && R2.TYPE() == PRV_STATE)
        return false;

      /* All communications after everything else */
      return false;
    }
};
