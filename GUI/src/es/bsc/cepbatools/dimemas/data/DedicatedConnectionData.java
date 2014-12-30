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

package es.bsc.cepbatools.dimemas.data;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import es.bsc.cepbatools.dimemas.data.Data.*;
import es.bsc.cepbatools.dimemas.tools.Tools;
import java.io.*;

/*
* Clase que albergará los datos correspondientes a DEDICATED CONNECTION.
*/
public class DedicatedConnectionData
{
  // Valores por defecto.
  static public final String DEFAULT_SOURCE = "";
  static public final String DEFAULT_TARGET = "";
  static public final String DEFAULT_BANDWIDTH = "0.0";
  static public final String DEFAULT_TAGS = "0";
  static public final String DEFAULT_FIRST_SIZE = "0";
  static public final String DEFAULT_FIRST_COND = "";
  static public final String DEFAULT_OPERATION = "";
  static public final String DEFAULT_SECOND_SIZE = "0";
  static public final String DEFAULT_SECOND_COND = "";
  static public final String DEFAULT_COMMUNICATORS = "1";
  static public final String DEFAULT_STARTUP = "0.0";
  static public final String DEFAULT_FLIGHT = "0.0";

  private int nConnections = 0;   // Número de conexiones dedicadas.
  public Connection[] connection; // Conexiones dedicadas.

  // Método que permite acceder al número de D. CONNECTIONS fuera de la clase.
  public int getNumberOfConnections()
  {
    return nConnections;
  }

  // Método que permite fijar el número de D. CONNECTIONS fuera de la clase.
  public void setNumberOfConnections(int value)
  {
    nConnections = value;
  }

  /*
  * El método defaultValues permite comprobar si la totalidad de D. CONNECTIONS,
  * tienen asociado los valores por defecto en sus datos.
  *
  * @ret boolean: TRUE si todos tienen el valor por defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    for(int i = 0; i < nConnections; i++)
    {
      if(!connection[i].defaultValues())
      {
        return false;
      }
    }

    return true;
  }

  // Método que genera las estructuras de las nuevas conexiones dedicadas.
  public void createConnections()
  {
    connection = new Connection[nConnections];

    for(int i = nConnections-1; i >= 0; i--)
    {
      connection[i] = new Connection(String.valueOf(i));
    }
  }

  // Método que borra la información de las conexiones dedicadas existentes.
  public void eraseConnectionInfo()
  {
    for(int i = nConnections-1; i >= 0; i--)
    {
      connection[i].initialValues();
    }
  }

  // Método que borra las conexiones dedicadas existentes.
  public void destroyConnections()
  {
    for(int i = nConnections-1; i >= 0; i--)
    {
      connection[i] = null;
    }

    connection = null;
    nConnections = 0;
    System.gc();
  }

  /*
  * El método changeAtConnections comprueba que no haya cambiado el número de
  * conexiones (dato modificable al configurar la info. de la red WAN). En caso
  * de variación, crearía las nuevas estrucuturas preservando los datos de las
  * conexiones existentes hasta ese instante (si hubiera alguna).
  */
  public void changeAtConnections()
  {
    Connection[] aux = new Connection[nConnections];

    if(connection == null)      // No había conexiones dedicadas hasta ahora.
    {
      createConnections();
    }
    else                        // Había conexiones préviamente creadas.
    {
      if(nConnections >= connection.length) // #Co. nuevas >=  #Co. existentes.
      {
        int index = nConnections-1;

        while(index >= connection.length)
        {
          aux[index] = new Connection(Integer.toString(index));
          index--;
        }

        for(int i = index; i >= 0; i--)
        {
          aux[i] = connection[i];
          connection[i] = null;
        }
      }
      else                                  // #Co. nuevas <  #Co. existentes.
      {
        int index = connection.length-1;

        while(index >= nConnections)
        {
          connection[index] = null;
          index--;
        }

        for(int i = nConnections-1; i >= 0; i--)
        {
          aux[i] = connection[i];
          connection[i] = null;
        }
      }

      connection = null;
      connection = aux;
      aux = null;
      System.gc();
    }
  }

  /*
  * La clase Connection provee la información de una DEDICATED CONNECTION
  * determinada.
  */
  public class Connection
  {
    public final String DEFAULT_ID;

    private int nTags;            // Número de "Tags".
    private int nComm;            // Número de "Comunnicators".

    private String id;            // Dedicated connection identificator.
    private String source;        // Source machine number.
    private String target;        // Target machine number.
    private String bandwidth;     // Dedicated connection bandwidth.
    private String tags;          // List of tags using the connection.
    private String firstSize;     // First condition (size of messages).
    private String firstCond;     // "<=","=",">=" relative to first condition.
    private String operation;     // Operation & (AND), | (OR).
    private String secondSize;    // Second condition (size of messages).
    private String secondCond;    // "<=","=",">=" relative to second condition.
    private String communicators; // List of operations allowed.
    private String startup;       // Connection startup.
    private String flightTime;    // Flight time.

    // Constructor de la clase Connection.
    public Connection(String identifier)
    {
      DEFAULT_ID = identifier;
      initialValues();
    }

    // Método que inicializa los datos con los valores por defecto.
    public void initialValues()
    {
      nTags = 0;
      nComm = 0;
      id = DEFAULT_ID;
      source = DEFAULT_SOURCE;
      target = DEFAULT_TARGET;
      bandwidth = DEFAULT_BANDWIDTH;
      tags = DEFAULT_TAGS;
      firstSize = DEFAULT_FIRST_SIZE;
      firstCond = DEFAULT_FIRST_COND;
      operation = DEFAULT_OPERATION;
      secondSize = DEFAULT_SECOND_SIZE;
      secondCond = DEFAULT_SECOND_COND;
      communicators = DEFAULT_COMMUNICATORS;
      startup = DEFAULT_STARTUP;
      flightTime = DEFAULT_FLIGHT;
    }

    /*
    * El método defaultValues comprueba si los datos poseen valores por defecto.
    *
    * @ret boolean: TRUE si todos los datos de una conexión dedicada tienen los
    *               valores por defecto, FALSE en otro caso.
    */
    public boolean defaultValues()
    {
      if(id.equalsIgnoreCase(DEFAULT_ID) &&
         source.equalsIgnoreCase(DEFAULT_SOURCE) &&
         target.equalsIgnoreCase(DEFAULT_TARGET) &&
         bandwidth.equalsIgnoreCase(DEFAULT_BANDWIDTH) &&
         tags.equalsIgnoreCase(DEFAULT_TAGS) &&
         firstSize.equalsIgnoreCase(DEFAULT_FIRST_SIZE) &&
         firstCond.equalsIgnoreCase(DEFAULT_FIRST_COND) &&
         operation.equalsIgnoreCase(DEFAULT_OPERATION) &&
         secondSize.equalsIgnoreCase(DEFAULT_SECOND_SIZE) &&
         secondCond.equalsIgnoreCase(DEFAULT_SECOND_COND) &&
         communicators.equalsIgnoreCase(DEFAULT_COMMUNICATORS) &&
         startup.equalsIgnoreCase(DEFAULT_STARTUP) &&
         flightTime.equalsIgnoreCase(DEFAULT_FLIGHT))
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    /*
    * El método loadData almacena la información que contiene @line en la
    * estructura de datos de una conexión dedicada.
    *
    * @param: · String line -> línea con todos los datos de una D. CONNECTION.
    *         · boolean oldFile -> TRUE si los datos provienen de un fichero de
    *                              configuración antiguo (menos datos), FALSE
    *                              en otro caso.
    *
    * @exc: Valor numérico no válido.
    */
    public boolean loadData(String line, boolean oldFile, int lineCount) throws Exception
    {
      int first = Data.CONNECTION.length();
      int second = line.indexOf(",",first);

      setId(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setSource(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setTarget(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setBandwidth(Tools.blanks(line.substring(first,second)));
      first = line.indexOf("{",second) + 1;
      second = line.indexOf("}",first);
      setTags(Tools.blanks(line.substring(first,second)));
      first = line.indexOf(",",second) + 1;
      second = line.indexOf(",",first);
      setFirstSize(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setFirstCond(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setOperation(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setSecondSize(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setSecondCond(Tools.blanks(line.substring(first,second)));
      first = line.indexOf("{",second) + 1;
      second = line.indexOf("}",first);
      setCommunicators(Tools.blanks(line.substring(first,second)));
      first = line.indexOf(",",second) + 1;

      if(oldFile)
      {
        second = line.indexOf("}",first);
        setStartup(Tools.blanks(line.substring(first,second)));
      }
      else
      {
        second = line.indexOf(",",first);
        setStartup(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf("}",first);
        setFlightTime(Tools.blanks(line.substring(first,second)));
      }

      return true;
    }

    /*
    * El método saveData genera, en un fichero de configuración @target, la
    * información correspondiente a una conexión dedicada.
    *
    * @param: RandomAccessFile target -> fichero de configuración destino.
    *
    * @exc: Error de I/O (escritura a fichero).
    */
    public void saveData(RandomAccessFile target) throws IOException
    {
      target.writeBytes(Data.CONNECTION);
      target.writeBytes(getId() + ", ");
      target.writeBytes(getSource() + ", ");
      target.writeBytes(getTarget() + ", ");
      target.writeBytes(getBandwidth() + ", ");
      target.writeBytes(getTags(true) + ", ");
      target.writeBytes(getFirstSize() + ", ");
      target.writeBytes(getFirstCond(true) + ", ");
      target.writeBytes(getOperation(true) + ", ");
      target.writeBytes(getSecondSize() + ", ");
      target.writeBytes(getSecondCond(true) + ", ");
      target.writeBytes(getCommunicators(true) + ", ");
      target.writeBytes(getStartup() + ",");
      target.writeBytes(getFlightTime() + "};;\n");
    }

    // Métodos GET: permiten el acceso externo a los datos de una con. dedicada.
    public String getId()
    {
      return id;
    }

    public String getSource()
    {
      return source;
    }

    public String getTarget()
    {
      return target;
    }

    public String getBandwidth()
    {
      return bandwidth;
    }

    public String getTags(boolean toDisk)
    {
      if(toDisk) // Si va a disco, crear formato [n Tags] {T1,T2,...,Tn}.
      {
        return "[" + nTags + "] {" + tags + "}";
      }

      return tags;
    }

    public String getFirstSize()
    {
      return firstSize;
    }

    public String getFirstCond(boolean toDisk)
    {
      if(toDisk) // Si los datos van a disco --> símbolo entre comillas.
      {
        return "\"" + firstCond + "\"";
      }
      else
      {
        return firstCond;
      }
    }

    public String getOperation(boolean toDisk)
    {
      if(toDisk) // Si los datos van a disco --> símbolo entre comillas.
      {
        return "\"" + operation + "\"";
      }

      return operation;
    }

    public String getSecondSize()
    {
      return secondSize;
    }

    public String getSecondCond(boolean toDisk)
    {
      if(toDisk) // Si los datos van a disco --> símbolo entre comillas.
      {
        return "\"" + secondCond + "\"";
      }

      return secondCond;
    }

    public String getCommunicators(boolean toDisk)
    {
      if(toDisk) // Si va a disco, crear formato [n Comm] {C1,C2,...,Cn}.
      {
        return "[" + nComm + "] {" + communicators + "}";
      }

      return communicators;
    }

    public String getStartup()
    {
      return startup;
    }

    public String getFlightTime()
    {
      return flightTime;
    }

    // Métodos SET: permiten la modificación, de forma externa, de los datos de
    // una conexión dedicada.
    public void setId(String value) throws Exception
    {
      try
      {
        Integer.parseInt(value);
        id = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("DEDICATED CONNECTION ID");
          throw e;
        }
    }

    public void setSource(String value)throws Exception
    {
      try
      {
        Integer.parseInt(value);
        source = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("SOURCE MACHINE");
          throw e;
        }
    }

    public void setTarget(String value)throws Exception
    {
      try
      {
        Integer.parseInt(value);
        target = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("TARGET MACHINE");
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
          Tools.showErrorMessage("CONNECTION BANDWIDTH");
          throw e;
        }
    }

    public void setTags(String value) throws Exception
    {
      nTags = Tools.verifyData(value,"LIST OF TAGS"); // Todos los TAGS son núm.

      if(nTags != -1)
      {
        tags = value;
      }
      else
      {
        throw new Exception();
      }
    }

    public void setFirstSize(String value) throws Exception
    {
      try
      {
        Integer.parseInt(value);
        firstSize = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("FIRST MESSAGE CONDITION");
          throw e;
        }
    }

    public void setFirstCond(String value)
    {
      if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
      {
        firstCond = Tools.blanks(value.substring(1,value.length()-1));
      }
      else
      {
        firstCond = value;
      }
    }

    public void setOperation(String value)
    {
      if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
      {
        operation = Tools.blanks(value.substring(1,value.length()-1));
      }
      else
      {
        operation = value;
      }
    }

    public void setSecondSize(String value) throws Exception
    {
      try
      {
        Integer.parseInt(value);
        secondSize = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("SECOND MESSAGE CONDITION");
          throw e;
        }
    }

    public void setSecondCond(String value)
    {
      if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
      {
        secondCond = Tools.blanks(value.substring(1,value.length()-1));
      }
      else
      {
        secondCond = value;
      }
    }

    public void setCommunicators(String value) throws Exception
    {
      nComm = Tools.verifyData(value,"LIST OF COMMUNICATORS"); // Todos los COMMUNICATORS son núm.

      if(nComm != -1)
      {
        communicators = value;
      }
      else
      {
        throw new Exception();
      }
    }

    public void setStartup(String value) throws Exception
    {
      try
      {
        Double.parseDouble(value);
        startup = Tools.filterForDouble(value);
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("CONNECTION STARTUP");
          throw e;
        }
    }

    public void setFlightTime(String value) throws Exception
    {
      try
      {
        Double.parseDouble(value);
        flightTime = Tools.filterForDouble(value);
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("FLIGHT TIME");
          throw e;
        }
    }
  }
}
