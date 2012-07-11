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

#ifndef _PARAVER_RECORDS_H_
#define _PARAVER_RECORDS_H_

#include <string>
using std::string;


typedef unsigned long long prv_time_t;

#define TIMER_TO_PRV_TIME_T(x,y) \
        y = static_cast<unsigned long long> (TIMER_TO_DOUBLE(x))

/*
 * Class to store the ASCII records before flushing them to disc
 */
static int TotalRecordsCreated = 0;

class SimpleParaverRecord
{
  private:
    int        _TYPE;

    int        _CPU;
    int        _Ptask;
    int        _Task;
    int        _Thread;

    prv_time_t _Timestamp;
    string     _ascii_record;

  public:

    SimpleParaverRecord(void)
    {
      TotalRecordsCreated++;
    }


    SimpleParaverRecord(int        TYPE,
                        int        CPU,
                        int        Ptask,
                        int        Task,
                        int        Thread,
                        prv_time_t timestamp,
                        string     ascii_record)
      :_TYPE(TYPE),
       _CPU(CPU),
       _Ptask(Ptask),
       _Task(Task),
       _Thread(Thread),
       _Timestamp(timestamp),
       _ascii_record(ascii_record)
    {
      TotalRecordsCreated++;
      // printf("PARAMETRIZED CONSTRUCTOR -> Live Records = %d\n", TotalRecordsCreated);
    }

    /*
    ~SimpleParaverRecord(void)
    {
      TotalRecordsCreated--;
      // printf("Live Records = %d\n", TotalRecordsCreated);
    }
    */

    /*
    SimpleParaverRecord(const SimpleParaverRecord& Other)
    {
      TotalRecordsCreated++;
      // printf("COPY CONSTRUCTOR -> Live Records = %d\n", TotalRecordsCreated);

      _TYPE         = Other.TYPE();
      _CPU          = Other.CPU();
      _Ptask        = Other.Ptask();
      _Task         = Other.Task();
      _Thread       = Other.Thread();
      _Timestamp    = Other.Timestamp();
      _ascii_record = string(Other.ascii_record());

      /*
      SimpleParaverRecord(Other.TYPE(),
                          Other.CPU(),
                          Other.Ptask(),
                          Other.Task(),
                          Other.Thread(),
                          Other.Timestamp(),
                          Other.ascii_record());

    }
    */

    void FillRecord(int        TYPE,
                    int        CPU,
                    int        Ptask,
                    int        Task,
                    int        Thread,
                    prv_time_t timestamp,
                    string     ascii_record)
    {
      _TYPE         = TYPE;
      _CPU          = CPU;
      _Ptask        = Ptask;
      _Task         = Task;
      _Thread       = Thread;
      _Timestamp    = timestamp;
      _ascii_record = string(ascii_record);
    }

    /*
     * 'toTrecfile' and 'serialize' methods are similar, but 'serialize'
     * distinguishes between RUNNING / NO RUNNING states and 'toTracefile' does
     * not so:
     *
     * 'serialize'   -> temporal files
     * 'toTracefile' -> final trace
     */

    void toTracefile(FILE* prv_file);

    void serialize(FILE* prv_file);

    bool deserialize(const char* ascii_record);

    int        TYPE(void)         const { return _TYPE; }
    int        CPU(void)          const { return _CPU; }
    int        Ptask(void)        const { return _Ptask; }
    int        Task(void)         const { return _Task; }
    int        Thread(void)       const { return _Thread; }
    prv_time_t Timestamp(void)    const { return _Timestamp; }
    string     ascii_record(void) const { return _ascii_record; }

    /*
    void operator=(const SimpleParaverRecord& Other)
    {
      _TYPE         = Other.TYPE();
      _CPU          = Other.CPU();
      _Ptask        = Other.Ptask();
      _Task         = Other.Task();
      _Thread       = Other.Thread();
      _Timestamp    = Other.Timestamp();
      _ascii_record = string (Other.ascii_record());
    }
    */
};

bool operator< (const SimpleParaverRecord& R1, const SimpleParaverRecord& R2);

std::ostream & operator << (std::ostream& o, const SimpleParaverRecord& obj);

#endif
