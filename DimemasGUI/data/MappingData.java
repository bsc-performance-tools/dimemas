/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                               Dimemas GUI                                 *
 *                  GUI for the Dimemas simulation tool                      *
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

  $URL::              $:  File
  $Rev::              $:  Revision of last commit
  $Author::           $:  Author of last commit
  $Date::             $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

package data;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import data.Data.*;
import tools.*;
import java.io.*;

/*
* Clase que albergará los datos correspondientes al MAPPING de tareas y nodos.
*/
public class MappingData
{
  // Valores por defecto.
  public final int    DEFAULT_TASKS     = 0;
  public final String DEFAULT_TRACEFILE = "";
  public final String DEFAULT_MAP       = "";

  private int    tasks;     // Number of tasks of the application.
  private String tracefile; // Tracefile name of application.
  private String map;       // List of nodes in application.
  private int    mapInfo;   // Map type, for informational pourposes


  /*
  * El método loadData almacena en la estructura de datos de MAPPING, la
  * información facilitada en @line.
  *
  * @param: String line -> línea con los datos del MAPPING.
  *
  * @exc: Valor numérico no válido.
  */
  public void loadData(String line) throws Exception
  {
    int first = Data.MAPPING.length();
    int second = line.indexOf(",",first);
    setTracefile(Tools.blanks(line.substring(first,second)));

    first = second + 1;
    second = line.indexOf(",",first);
    setTasks(Tools.blanks(line.substring(first,second)));

    first = line.indexOf("{",second) + 1;
    second = line.indexOf("}",first);
    setMap(Tools.blanks(line.substring(first,second)));
  }

  /*
  * El método saveData genera, en un fichero de configuración @target, la
  * información correspondiente al MAPPING.
  *
  * @param: RandomAccessFile target -> fichero de configuración destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    target.writeBytes(Data.MAPPING);
    target.writeBytes(getTracefile(true) + ", ");
    target.writeBytes(getTasks() + ", ");
    target.writeBytes(getMap(true) + "};;\n");
  }

  // Constructor de la clase MappingData.
  public MappingData()
  {
    initialValues();
  }

  // Método que inicializa los datos con los valores por defecto.
  public void initialValues()
  {
    tracefile = DEFAULT_TRACEFILE;
    tasks     = DEFAULT_TASKS;
    map       = DEFAULT_MAP;
    mapInfo   = Data.NO_MAP;
  }

  /*
  * El método defaultValues comprueba si los datos poseen valores por defecto.
  *
  * @ret boolean: TRUE si todos los datos del MAPPING tienen los valores por
  *               defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    /* OLD COMPARISON
    if(tracefile.equalsIgnoreCase(DEFAULT_TRACEFILE) &&
       tasks == DEFAULT_TASKS && map.equalsIgnoreCase(DEFAULT_MAP))
    {
    */
    if (map.equalsIgnoreCase(DEFAULT_MAP))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // Métodos GET: permiten el acceso externo a los datos del MAPPING.
  public String getTracefile(boolean toDisk)
  {
    if(toDisk) // Si el dato va a disco --> nombre entre comillas.
    {
      return "\"" + tracefile + "\"";
    }

    return tracefile;
  }

  public int getTasks()
  {
    return tasks;
  }

  public String getMap(boolean toDisk)
  {
    if(toDisk) // Si va a disco, crear formato [n Tasks] {M1,M2,...,Mn}.
    {
      return "[" + tasks + "]" + " {" + map + "}";
    }

    return map;
  }

  public int getMapInfo()
  {
    return mapInfo;
  }

  // Métodos SET: permiten la modificación, de forma externa, de los datos del
  // MAPPING.
  public void setTracefile(String value)
  {
    if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
    {
      tracefile = value.substring(1,value.length()-1);
    }
    else
    {
      tracefile = value;
    }
  }

  public void setTasks(String value) throws Exception
  {
    try
    {
      tasks = Integer.parseInt(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NUMBER OF TASKS");
        throw e;
      }
  }

  public void setMap(String value) throws Exception
  {
    if(Tools.verifyData(value,"MAPPING") != -1)
    {
      map = value;
    }
    else
    {
      throw new NumberFormatException();
    }
  }

  public void setMapInfo(int newMapInfo)
  {
    mapInfo = newMapInfo;
  }

}
