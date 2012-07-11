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

import java.io.*;
import tools.*;

/*
* Clase que albergará las opciones correspondientes a la llamada al simulador.
*/
public class SimulatorCallData
{
  // private final String DIMEMAS_PATH = System.getenv("DIMEMAS_HOME") + "/bin/Dimemas3";
  // private final String DIMEMAS_PATH = __PREFIX__+"/bin/Dimemas3";
  private final String DIMEMAS_PATH = System.getProperty("DIMEMAS_HOME") + "/bin/Dimemas3";

  private final int OUTPUT_NONE = 0;
  private final int OUTPUT_PARAVER = 1;
  private final int OUTPUT_VAMPIR = 2;

  // Campos correspondientes a las opciones del simulador (en ficheros ".opt").
  private final String FIELD_LOAD = "[LOAD TRACE] = ";
  private final String FIELD_BREAK = "[BREAK TIME] = ";
  private final String FIELD_SYNC = "[IGNORE SYNC] = ";
  private final String FIELD_SIZE = "[MESSAGE SIZE] = ";
  private final String FIELD_OUTPUT = "[OUTPUT TRACE] = ";
  private final String FIELD_NAME = "[OUTPUT NAME] = ";
  private final String FIELD_TYPE = "[OUTPUT TYPE] = ";
  private final String FIELD_START = "[OUTPUT START] = ";
  private final String FIELD_STOP = "[OUTPUT STOP] = ";

  // Valores por defecto.
  public final boolean DEFAULT_LOAD_TRACE = false;
  public final String DEFAULT_SIM_TIME = "0.0";
  public final int DEFAULT_OUTPUT = OUTPUT_NONE;
  public final String DEFAULT_FILENAME = "";
  public final boolean DEFAULT_TYPE_ASCII = true;
  public final String DEFAULT_TRACE_START = "0.0";
  public final String DEFAULT_TRACE_STOP = "0.0";
  public final boolean DEFAULT_IGNORE_SEND = false;
  public final String DEFAULT_SIZE = "0";

  private boolean load;
  private String simTime;
  private int out;
  private String filename;
  private boolean typeAscii;
  private String traceStart;
  private String traceStop;
  private boolean ignoreSend;
  private String size;

  // Método que inicializa los datos con los valores por defecto.
  private void initialValues()
  {
    load = DEFAULT_LOAD_TRACE;
    simTime = DEFAULT_SIM_TIME;
    out = DEFAULT_OUTPUT;
    filename = DEFAULT_FILENAME;
    typeAscii = DEFAULT_TYPE_ASCII;
    traceStart = DEFAULT_TRACE_START;
    traceStop = DEFAULT_TRACE_STOP;
    ignoreSend = DEFAULT_IGNORE_SEND;
    size = DEFAULT_SIZE;
  }

  private boolean trueOrFalse(String value)
  {
    if(value.equalsIgnoreCase("true"))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /*
  * Método que carga todas las opciones de una llamada al simulador.
  *
  * @param: String filename -> nombre del fichero con la infromación referente a
  *                            las opciones para llamar al simulador.
  */
  public void loadFromDisk(File filename)
  {
    String line;
    String value;
    RandomAccessFile source;

    initialValues();

    try
    {
      source = new RandomAccessFile(filename,"r");

      while(source.getFilePointer() != source.length())
      {
        line = source.readLine();

        if(line.startsWith(FIELD_LOAD))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setLoadInMemory(trueOrFalse(value));
        }
        else if(line.startsWith(FIELD_BREAK))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setSimTime(value);
        }
        else if(line.startsWith(FIELD_SYNC))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setIgnoreSend(trueOrFalse(value));
        }
        else if(line.startsWith(FIELD_SIZE))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setMinSize(value);
        }
        else if(line.startsWith(FIELD_OUTPUT))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setOutputFile(Integer.parseInt(value));
        }
        else if(line.startsWith(FIELD_NAME))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setFilename(value);
        }
        else if(line.startsWith(FIELD_TYPE))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setIsTypeAscii(trueOrFalse(value));
        }
        else if(line.startsWith(FIELD_START))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setStartTime(value);
        }
        else if(line.startsWith(FIELD_STOP))
        {
          value = Tools.blanks(line.substring(line.indexOf("=")+1));
          setStopTime(value);
        }
      }

      source.close();
    } catch(Exception exc)
      {
        Tools.showInformationMessage(exc.toString());
      }

  }

  /*
  * Método que graba, en un fichero con nombre @filename, todos las opciones
  * utilizadas en una llamada al simulador.
  *
  * @param: String filename -> nombre del fichero de opciones a generar.
  */
  public void saveToDisk(String filename)
  {
    RandomAccessFile target;

    try
    {
      target = new RandomAccessFile(new File(filename),"rw");
      target.writeBytes(FIELD_LOAD + getLoadInMemory() + "\n");
      target.writeBytes(FIELD_BREAK + getSimTime() + "\n");
      target.writeBytes(FIELD_SYNC + getIgnoreSend() + "\n");
      target.writeBytes(FIELD_SIZE + getMinSize() + "\n");
      target.writeBytes(FIELD_OUTPUT + getOutputFile() + "\n");
      target.writeBytes(FIELD_NAME + getFilename() + "\n");
      target.writeBytes(FIELD_TYPE + getIsTypeAscii() + "\n");
      target.writeBytes(FIELD_START + getStartTime() + "\n");
      target.writeBytes(FIELD_STOP + getStopTime() + "\n");
      target.close();
    } catch(Exception exc)
      {
        Tools.showInformationMessage(exc.toString());
      }
  }

  // Constructor de la clase SimulatorCallData.
  public SimulatorCallData()
  {
    initialValues();
  }

  /*
  * El método generateCommand crea el comando con todos los parámetros requeridos
  * para llamar al simulador Dimemas.
  *
  * @param: String configFile -> fichero de configuración que será utilizado en
  *                              la simulación.
  *
  * @ret String: Comando para llamar a Dimemas.
  */
  public String generateCommand(String configFile)
  {

    String command = DIMEMAS_PATH + " ";

    if(!simTime.equalsIgnoreCase(DEFAULT_SIM_TIME))
    {
      command += "-T " + simTime + " ";
    }

    if(!load)
    {
      command += "-l ";
    }

    if(out == OUTPUT_PARAVER)
    {
      if(typeAscii)
      {
        command  += "-pa " + filename + " ";
      }
      else
      {
        command  += "-pb " + filename + " ";
      }

      if(!traceStart.equalsIgnoreCase(DEFAULT_TRACE_START))
      {
        command += "-y " + traceStart + " ";
      }

      if(!traceStop.equalsIgnoreCase(DEFAULT_TRACE_STOP))
      {
        command += "-z " + traceStop + " ";
      }
    }
    else if(out == OUTPUT_VAMPIR)
    {
      if(typeAscii)
      {
        command  += "-Va " + filename + " ";
      }
      else
      {
        command  += "-Vb " + filename + " ";
      }

      if(!traceStart.equalsIgnoreCase(DEFAULT_TRACE_START))
      {
        command += "-Vy " + traceStart + " ";
      }

      if(!traceStop.equalsIgnoreCase(DEFAULT_TRACE_STOP))
      {
        command += "-Vz " + traceStop + " ";
      }
    }

    if(ignoreSend)
    {
      command += "-F ";
    }

    if(!size.equalsIgnoreCase("0"))
    {
      command += "-S " + size + " ";
    }

    command += configFile;

    return command;
  }

  // Métodos GET: permiten el acceso externo a las opciones del simulador.
  public boolean getLoadInMemory()
  {
    return load;
  }

  public String getSimTime()
  {
    return simTime;
  }

  public int getOutputFile()
  {
    return out;
  }

  public String getFilename()
  {
    return filename;
  }

  public boolean getIsTypeAscii()
  {
    return typeAscii;
  }

  public String getStartTime()
  {
    return traceStart;
  }

  public String getStopTime()
  {
    return traceStop;
  }

  public boolean getIgnoreSend()
  {
    return ignoreSend;
  }

  public String getMinSize()
  {
    return size;
  }

  // Métodos SET: permiten la modificación, de forma externa, de las opciones
  // del simulador.
  public void setLoadInMemory(boolean value)
  {
    load = value;
  }

  public void setSimTime(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      simTime = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("SIMULATION BREAK TIME");
        throw e;
      }
  }

  public void setOutputFile(int value)
  {
    out = value;
  }

  public void setFilename(String value)
  {
    filename = value;
  }

  public void setIsTypeAscii(boolean value)
  {
    typeAscii = value;
  }

  public void setStartTime(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      traceStart = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("SIMULATION START TIME");
        throw e;
      }
  }

  public void setStopTime(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      traceStop = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("SIMULATION STOP TIME");
        throw e;
      }
  }

  public void setIgnoreSend(boolean value)
  {
    ignoreSend = value;
  }

  public void setMinSize(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      size = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("MINIMUM SIZE");
        throw e;
      }
  }
}
