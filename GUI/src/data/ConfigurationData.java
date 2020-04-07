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
* Clase que albergará los nombres de los ficheros de configuración extra.
*/
public class ConfigurationData
{
  // Valores por defecto.
  public final String DEFAULT_SCHEDULER = "";
  public final String DEFAULT_FILE_SYS = "";
  public final String DEFAULT_COMMUNICATION = "";
  public final String DEFAULT_SENSITIVITY = "";

  private String scheduler;
  private String fileSys;
  private String communication;
  private String sensitivity;

  /*
  * El método loadData permite obtener, a partir de una cadena @line, los
  * nombres de los ficheros de configuración extra.
  *
  * @param: String line --> cadena de caracteres con los datos de la config.
  *                         extra.
  */
  public boolean loadData(String line, int lineCount)
  {
    int first = Data.CONFIG.length();
    int second = line.indexOf(",",first);

    setScheduler(Tools.blanks(line.substring(first,second)));
    first = second + 2;
    second = line.indexOf(",",first);
    setFileSys(Tools.blanks(line.substring(first,second)));
    first = second + 2;
    second = line.indexOf(",",first);
    setCommunication(Tools.blanks(line.substring(first,second)));
    first = second + 2;
    second = line.indexOf("}",first);
    setSensitivity(Tools.blanks(line.substring(first,second)));

    return true;
  }

  /*
  * El método saveData permite generar, en un fichero de configuración @target,
  * la información (configuration files) referente a los ficheros de
  * confiruación extra.
  *
  * @param: RandomAccessFile target --> fichero de configuración destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    target.writeBytes(Data.CONFIG);
    target.writeBytes(getScheduler(true) + ", ");
    target.writeBytes(getFileSys(true) + ", ");
    target.writeBytes(getCommunication(true) + ", ");
    target.writeBytes(getSensitivity(true) + "};;\n");
  }

  // Constructor de la clase ConfigurationData.
  public ConfigurationData()
  {
    initialValues();
  }

  // Método que inicializa los datos a valores por defecto.
  public void initialValues()
  {
    scheduler     = DEFAULT_SCHEDULER;
    fileSys       = DEFAULT_FILE_SYS;
    communication = DEFAULT_COMMUNICATION;
    sensitivity   = DEFAULT_SENSITIVITY;
  }

  /*
  * El método defaultValues permite comprobar si la información coincide con los
  * valores por defecto.
  *
  * @ret boolean: TRUE si son valores por defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    if(scheduler.equalsIgnoreCase(DEFAULT_SCHEDULER) &&
       fileSys.equalsIgnoreCase(DEFAULT_FILE_SYS) &&
       communication.equalsIgnoreCase(DEFAULT_COMMUNICATION) &&
       sensitivity.equalsIgnoreCase(DEFAULT_SENSITIVITY))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // Métodos GET: Permiten el acceso externo a los datos de configuración extra.
  public String getScheduler(boolean toDisk)
  {
    if(toDisk) // Si los datos van a disco --> nombre entre comillas.
    {
      return "\"" + scheduler + "\"";
    }

    return scheduler;
  }

  public String getFileSys(boolean toDisk)
  {
    if(toDisk) // Si los datos van a disco --> nombre entre comillas.
    {
      return "\"" + fileSys + "\"";
    }

    return fileSys;
  }

  public String getCommunication(boolean toDisk)
  {
    if(toDisk) // Si los datos van a disco --> nombre entre comillas.
    {
      return "\"" + communication + "\"";
    }

    return communication;
  }

  public String getSensitivity(boolean toDisk)
  {
    if(toDisk) // Si los datos van a disco --> nombre entre comillas.
    {
      return "\"" + sensitivity + "\"";
    }

    return sensitivity;
  }

  // Métodos SET: Permiten modificar, externamente, los nombres de los ficheros
  // de configuración extra.
  public void setScheduler(String value)
  {
    if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
    {
      scheduler = value.substring(1,value.length()-1);
    }
    else
    {
      scheduler = value;
    }
  }

  public void setFileSys(String value)
  {
    if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
    {
      fileSys = value.substring(1,value.length()-1);
    }
    else
    {
      fileSys = value;
    }
  }

  public void setCommunication(String value)
  {
    if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
    {
      communication = value.substring(1,value.length()-1);
    }
    else
    {
      communication = value;
    }
  }

  public void setSensitivity(String value)
  {
    if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
    {
      sensitivity = value.substring(1,value.length()-1);
    }
    else
    {
      sensitivity = value;
    }
  }
}
