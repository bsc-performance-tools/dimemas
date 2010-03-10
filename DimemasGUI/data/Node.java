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
 * TÃ­tulo:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Juan Gonzalez Garcia
 * @version 1.0
 */

import data.Data.*;
import tools.*;
import java.io.*;

/*
* La clase Node provee la informaciÃ³n de un nodo.
*/
public class Node
{
  // Valores por defecto.
  static public final String DEFAULT_ARCHITECTURE = "";
  static public final String DEFAULT_PROCESSORS   = "1";
  static public final String DEFAULT_INPUT        = "1";
  static public final String DEFAULT_OUTPUT       = "1";
  static public final String DEFAULT_LOCAL        = "0.0";
  static public final String DEFAULT_REMOTE       = "0.0";
  static public final String DEFAULT_SPEED        = "1.0";
  static public final String DEFAULT_BANDWIDTH    = "0.0";
  static public final String DEFAULT_LATENCY      = "0.0";

  private String DEFAULT_NODE_ID;
  private String DEFAULT_MACHINE_ID;

  private String machine_id;   // Machine to which belongs the node.
  private String node_id;      // Node identificator.
  private String architecture; // Architecture name.
  private String processors;   // Number of processors within node.
  private String input;        // Number of input links.
  private String output;       // Number of output links.
  private String local;        // Startup on local communication.
  private String remote;       // Startup on remote communication.
  private String speed;        // Relative processor speed.
  private String bandwidth;    // Memory bandwidth.
  private String latency;      // External net startup.

  // Constructor de la clase Node.
  public Node(String identificator, String machineId)
  {
    DEFAULT_NODE_ID    = identificator;
    DEFAULT_MACHINE_ID = machineId;
    initialValues();
  }

  // MÃ©todo que inicializa los datos con los valores por defecto.
  public void initialValues()
  {
    machine_id   = DEFAULT_MACHINE_ID;
    node_id      = DEFAULT_NODE_ID;
    architecture = DEFAULT_ARCHITECTURE;
    processors   = DEFAULT_PROCESSORS;
    input        = DEFAULT_INPUT;
    output       = DEFAULT_OUTPUT;
    local        = DEFAULT_LOCAL;
    remote       = DEFAULT_REMOTE;
    speed        = DEFAULT_SPEED;
    bandwidth    = DEFAULT_BANDWIDTH;
    latency      = DEFAULT_LATENCY;
  }

  /*
  * El mÃ©todo defaultValues permite comprobar si la totalidad de nodos tienen
  * todos sus datos inicializados con los valores por defecto.
  *
  * @ret boolean: TRUE si todos tienen el valor por defecto, FALSE en otro
  *               caso.
  */
  public boolean defaultValues()
  {
    if(machine_id.equalsIgnoreCase(DEFAULT_MACHINE_ID) &&
       node_id.equalsIgnoreCase(DEFAULT_NODE_ID) &&
       architecture.equalsIgnoreCase(DEFAULT_ARCHITECTURE) &&
       processors.equalsIgnoreCase(DEFAULT_PROCESSORS) &&
       input.equalsIgnoreCase(DEFAULT_INPUT) &&
       output.equalsIgnoreCase(DEFAULT_OUTPUT) &&
       local.equalsIgnoreCase(DEFAULT_LOCAL) &&
       remote.equalsIgnoreCase(DEFAULT_REMOTE) &&
       speed.equalsIgnoreCase(DEFAULT_SPEED) &&
       bandwidth.equalsIgnoreCase(DEFAULT_BANDWIDTH) &&
       latency.equalsIgnoreCase(DEFAULT_LATENCY))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /*
  * El mÃ©todo loadData almacena los datos facilitados en @line en la
  * estructura de datos de un nodo.
  *
  * @param: Â· String line -> lÃ­nea con todos los datos de un nodo.
  *         Â· boolean oldFile -> TRUE si los datos provienen de un fichero de
  *                              configuraciÃ³n antiguo (menos datos), FALSE
  *                              en otro caso.
  *
  * @exc: Valor numÃ©rico no vÃ¡lido.
  */
  public void loadData(String line, boolean oldFile) throws Exception
  {
    int first = Data.NODE.length();
    int second = line.indexOf(",",first);

    if(!oldFile)
    {
      setMachine_id(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
    }

    setNode_id(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setArchitecture(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setProcessors(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setInput(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setOutput(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setLocal(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setRemote(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setSpeed(Tools.blanks(line.substring(first,second)));
    first = second + 1;

    if(!oldFile)
    {
      second = line.indexOf(",",first);
      setBandwidth(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf("}",first);
      setLatency(Tools.blanks(line.substring(first,second)));
    }
    else
    {
      second = line.indexOf("}",first);
      setBandwidth(Tools.blanks(line.substring(first,second)));
    }
  }

  /*
  * El mÃ©todo saveData genera, en un fichero de configuraciÃ³n @target, la
  * informaciÃ³n correspondiente a un nodo.
  *
  * @param: RandomAccessFile target -> fichero de configuraciÃ³n destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    target.writeBytes(Data.NODE);
    target.writeBytes(getMachine_id() + ", ");
    target.writeBytes(getNode_id() + ", ");
    target.writeBytes(getArchitecture(true) + ", ");
    target.writeBytes(getProcessors() + ", ");
    target.writeBytes(getInput() + ", ");
    target.writeBytes(getOutput() + ", ");
    target.writeBytes(getLocal() + ", ");
    target.writeBytes(getRemote() + ", ");
    target.writeBytes(getSpeed() + ", ");
    target.writeBytes(getBandwidth() + ", ");
    target.writeBytes(getLatency() + "};;\n");
  }

  // MÃ©todos GET: permiten el acceso externo a los datos de un nodo.
  public String getMachine_id()
  {
    return machine_id;
  }

  public String getNode_id()
  {
    return node_id;
  }

  public String getArchitecture(boolean toDisk)
  {
    if(toDisk) // Si el dato va a disco --> nombre entre comillas.
    {
      return "\"" + architecture + "\"";
    }

    return architecture;
  }

  public String getProcessors()
  {
    return processors;
  }

  public String getInput()
  {
    return input;
  }

  public String getOutput()
  {
    return output;
  }

  public String getLocal()
  {
    return local;
  }

  public String getRemote()
  {
    return remote;
  }

  public String getSpeed()
  {
    return speed;
  }

  public String getBandwidth()
  {
    return bandwidth;
  }

  public String getLatency()
  {
    return latency;
  }

  // MÃ©todos SET: permiten la modificaciÃ³n, de forma externa, de los datos de
  // un nodo.
  public void setMachine_id(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      machine_id = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("MACHINE ID");
        throw e;
      }
  }

  public void setNode_id(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      node_id = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NODE ID");
        throw e;
      }
  }

  public void setArchitecture(String value)
  {
    if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
    {
      architecture = value.substring(1,value.length()-1);
    }
    else
    {
      architecture = value;
    }
  }

  public void setProcessors(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      processors = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NUMBER OF PROCESSORS");
        throw e;
      }
  }

  public void setInput(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      input = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NUMBER OF INPUT LINKS");
        throw e;
      }
  }

  public void setOutput(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      output = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NUMBER OF OUTPUT LINKS");
        throw e;
      }
  }

  public void setLocal(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      local = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("LOCAL STARTUP");
        throw e;
      }
  }

  public void setRemote(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      remote = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("REMOTE STARTUP");
        throw e;
      }
  }

  public void setSpeed(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      speed = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("RELATIVE PROCESSOR SPEED");
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
        Tools.showErrorMessage("MEMORY BANDWIDTH");
        throw e;
      }
  }

  public void setLatency(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      latency = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("EXTERNAL NET LATENCY");
        throw e;
      }
  }
}
