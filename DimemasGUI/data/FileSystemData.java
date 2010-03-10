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
* Clase que albergará los datos correspondientes a FILE SYSTEM.
*/
public class FileSystemData
{
  // Valores por defecto.
  public final String DEFAULT_LATENCY = "0.0";
  public final String DEFAULT_BANDWIDTH = "0.0";
  public final String DEFAULT_SIZE = "8.0";
  public final String DEFAULT_REQ = "0";
  public final String DEFAULT_HIT = "1.0";

  private String latency;   // Disk latency.
  private String bandwidth; // Disk bandwidth.
  private String size;      // Block size.
  private String req;       // Concurrent requests.
  private String hit;       // Hit ratio.

  /*
  * El método loadData almacena los datos facilitados en @line en la estructura
  * de datos del FILE SYSTEM.
  *
  * @param: String line -> línea con los datos correspondientes al FILE SYSTEM.
  *
  * @exc: Valor numérico no válido.
  */
  public void loadData(String line) throws Exception
  {
    int first = Data.FILE_SYS.length();
    int second = line.indexOf(",",first);

    setLatency(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setBandwidth(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setSize(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setReq(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf("}",first);
    setHit(Tools.blanks(line.substring(first,second)));
  }

  /*
  * El método saveData genera, en un fichero de configuración @target, la
  * información correspondiente al FILE SYSTEM.
  *
  * @param: RandomAccessFile target -> fichero de configuración destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    target.writeBytes(Data.FILE_SYS);
    target.writeBytes(getLatency() + ", ");
    target.writeBytes(getBandwidth() + ", ");
    target.writeBytes(getSize() + ", ");
    target.writeBytes(getReq() + ", ");
    target.writeBytes(getHit() + "};;\n");
  }

  // Constructor de la clase FileSystemData.
  public FileSystemData()
  {
    initialValues();
  }

  // Método que inicializa los datos con los valores por defecto.
  public void initialValues()
  {
    latency = DEFAULT_LATENCY;
    bandwidth = DEFAULT_BANDWIDTH;
    size = DEFAULT_SIZE;
    req = DEFAULT_REQ;
    hit = DEFAULT_HIT;
  }

  /*
  * El método defaultValues comprueba si los datos poseen valores por defecto.
  *
  * @ret boolean: TRUE si todos los datos del sitema de ficheros tienen los
  *               valores por defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    if(latency.equalsIgnoreCase(DEFAULT_LATENCY) &&
       bandwidth.equalsIgnoreCase(DEFAULT_BANDWIDTH) &&
       size.equalsIgnoreCase(DEFAULT_SIZE) &&
       req.equalsIgnoreCase(DEFAULT_REQ) &&
       hit.equalsIgnoreCase(DEFAULT_HIT))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // Métodos GET: permiten el acceso externo a los datos del FILE SYSTEM.
  public String getLatency()
  {
    return latency;
  }

  public String getBandwidth()
  {
    return bandwidth;
  }

  public String getSize()
  {
    return size;
  }

  public String getReq()
  {
    return req;
  }

  public String getHit()
  {
    return hit;
  }

  // Métodos SET: permiten la modificación, de forma externa, de los datos del
  // FILE SYSTEM.
  public void setLatency(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      latency = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("DISK LATENCY");
        throw e;
      }
  }

  public void setBandwidth(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      bandwidth = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("DISK BANDWIDTH");
        throw e;
      }
  }

  public void setSize(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      size = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("BLOCK SIZE");
        throw e;
      }
  }

  public void setReq(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      req = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("CONCURRENT REQUESTS");
        throw e;
      }
  }

  public void setHit(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      hit = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("HIT RATIO");
        throw e;
      }
  }
}
