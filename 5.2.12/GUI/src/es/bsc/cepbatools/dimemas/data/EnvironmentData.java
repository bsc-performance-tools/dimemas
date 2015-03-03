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
import es.bsc.cepbatools.dimemas.tools.*;
import java.io.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/*
* Clase que albergará los datos correspondientes a MACHINES.
*/
public class EnvironmentData
{
  public final String COMM_CT  = "1";
  public final String COMM_LIN = "2";
  public final String COMM_LOG = "3";

  // Valores por defecto.
  public final String DEFAULT_NODE_ARCHITECTURE = "";
  public final String DEFAULT_NAME = "";
  public final String DEFAULT_ARCHITECTURE = "";
  public final String DEFAULT_NODES = "1";
  public final String DEFAULT_BANDWIDTH = "0.0";
  public final String DEFAULT_BUSES = "0";
  public final String DEFAULT_COMMUNICATION = COMM_CT;

  private double[][] ft;      // Flight Time de las máquinas.
  private int nMachines = 0;  // Número de máquinas.
  public  Machine[] machine;   // Máquinas.

  // Método que permite acceder al número de máquinas fuera de la clase.
  public int getNumberOfMachines()
  {
    return nMachines;
  }

  // Método que permite fijar el número de máquinas desde fuera de la clase.
  public void setNumberOfMachines(int value)
  {
    nMachines = value;

    // Revisión de los Flight Time asociados a las máquinas.
    if(nMachines == 1)
    {
      ft = new double[1][1];
    }
    else if(ft == null)
    {
      ft = new double[nMachines][nMachines-1];
    }
    else
    {
      double[][] aux = ft;
      ft = null;
      ft = new double[nMachines][nMachines-1];

      for(int i = 0; i < aux.length; i++)
      {
        for(int j = 0; j < aux[i].length; j++)
        {
          if(i < nMachines && j < nMachines-1)
          {
            ft[i][j] = aux[i][j];
          }
        }
      }
    }
  }

  /*
  * El método ftSetValue permite fijar el valor de Flight Time @value de la
  * máquina origen @machine respecto la máquina destino @elementIndex.
  *
  * @param: · int machine -> máquina origen.
  *         · int elementIndex -> máquina destino.
  *         · double value -> valor de Flight Time asociado.
  */
  public void ftSetValue(int machine, int elementIndex, double value)
  {
    ft[machine][elementIndex] = value;
  }

  /*
  * El método ftGetValue permite obtener el valor de Flight Time de la máquina
  * origen @machine respecto la máquina destino @elementIndex.
  *
  * @param: · int machine -> máquina origen.
  *         · int elementIndex -> máquina destino.
  *
  * @ret String: Flight Time existente entre las dos máquinas.
  */
  public String ftGetValue(int machine, int elementIndex)
  {
    return Double.toString(ft[machine][elementIndex]);
  }

  /*
  * El método ftLoad permite almacenar los datos contenidos en @line referentes
  * a los valores Flight Time de una máquina.
  *
  * @param: String line -> Línea con los valores Flight Time de una máquina.
  */
  public void ftLoad(String line)
  {
    int i = 0;
    int first = 0;
    int second = line.indexOf(" ", first);
    int elementIndex = -1;
    String macId = Tools.blanks(line.substring(first,second));

    // Búsqueda de la máquina correspondiente.
    for(int j = 0; j < nMachines; j++)
    {
      if(macId.equalsIgnoreCase(machine[j].getId()))
      {
        elementIndex = j;
        break;
      }
    }

    // Obtener los valores Flight Time.
    while((elementIndex != -1) && (second != line.length()) && (i < nMachines))
    {
      first = second + 1;
      second = line.indexOf(" ", first);

      if(second == -1)
      {
        second = line.length();
      }

      if(i < elementIndex)
      {
        ft[elementIndex][i] = Double.parseDouble(Tools.blanks(line.substring(first,second)));
      }
      else if(i > elementIndex)
      {
        ft[elementIndex][i-1] = Double.parseDouble(Tools.blanks(line.substring(first,second)));
      } // "else if(i == elementIndex)" --> diagonal (0.0) --> ignorar.

      i++;
    }
  }

  /*
  * El método ftSave permite generar, en un fichero de configuración @raf, la
  * información referente a los valores Flight Time de todas las máquinas.
  *
  * @param: RandomAccessFile raf --> fichero de configuración destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void ftSave(RandomAccessFile raf) throws IOException
  {
    for(int i = 0; i < nMachines; i++)
    {
      raf.writeBytes("Flight time: " + machine[i].getId() + " ");

      for(int j = 0; j < nMachines-1; j++)
      {
        if(i == j) // Diagonal --> valor de F.T. == 0.0
        {
          raf.writeBytes(0.0 + " ");
        }

        raf.writeBytes(ft[i][j] + " ");
      }

      if(i == nMachines-1)
      {
        raf.writeBytes(0.0 + " ");
      }

      raf.writeBytes("\n");
    }
  }

  // Método que realiza una cópia de los datos para poder deshacer los posibles
  // cambios si fuera necesario.
  public void mpiBackup()
  {
    for(int i = nMachines-1; i >= 0; i--)
    {
      machine[i].mpiBackupData();
    }
  }

  // Método que permite recuperar los datos y desechar los posibles cambios.
  public void mpiRestore()
  {
    for(int i = nMachines-1; i >= 0; i--)
    {
      machine[i].mpiRestoreData();
    }
  }

  /*
  * El método defaultValues permite comprobar si la totalidad de máquinas tienen
  * todos sus datos inicializados con los valores por defecto.
  *
  * @ret boolean: TRUE si todos tienen el valor por defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    for(int i = 0; i < nMachines; i++)
    {
      if(!machine[i].defaultValues())
      {
        return false;
      }
    }

    return true;
  }

  /*
  * El método createMachines genera las estructuras que contendrán los datos de
  * las máquinas.
  *
  * @param: · String architecture -> INSTRUMENTED ARQUITECTURE.
  *         · String buses -> número de buses correspondientes a la INST.ARQ.
  *
  * @exc: Valor numérico no válido (buses).
  */
  public void createMachines(String architecture, String buses) throws NumberFormatException
  {
    machine = new Machine[nMachines];

    for(int i = nMachines-1; i >= 0; i--)
    {
      machine[i] = new Machine(String.valueOf(i));
      machine[i].setArchitecture(architecture);
      machine[i].setBuses(buses);
    }
  }

  // Método que borra la información de las máquinas existentes.
  public void eraseMachineInfo()
  {
    for(int i = nMachines-1; i >= 0; i--)
    {
      machine[i].initialValues();
    }
  }

  // Método que borra las máquinas existentes.
  public void destroyMachines()
  {
    for(int i = nMachines-1; i >= 0; i--)
    {
      machine[i] = null;
    }

    machine = null;
    nMachines = 0;
    System.gc();
  }

  /*
  * El método changeAtMachines comprueba que no haya cambiado el número de
  * máquinas, lo que afectaría a las estructuras. En caso de variación, crearía
  * las estructuras adecuadas al nuevo número de máquinas, preservando los datos
  * de las máquinas existentes hasta ese instante (si hubiera alguna).
  *
  * @exc: Valor numérico no válido (buses).
  */
  public void changeAtMachines() throws Exception
  {
    Machine[] aux = new Machine[nMachines];

    if(machine == null)     // No había máquinas definidas hasta ahora.
    {
      createMachines(DEFAULT_ARCHITECTURE,DEFAULT_BUSES);
    }
    else                    // Había máquinas previamente creadas.
    {
      if(nMachines >= machine.length) // #Mac nuevas >=  #Mac existentes.
      {
        int index = nMachines-1;

        while(index >= machine.length)
        {
          aux[index] = new Machine(Integer.toString(index));
          index--;
        }

        for(int i = index; i >= 0; i--)
        {
          aux[i] = machine[i];
          machine[i] = null;
        }
      }
      else                          // #Mac nuevas <  #Mac existentes.
      {
        int index = machine.length-1;

        while(index >= nMachines)
        {
          machine[index] = null;
          index--;
        }

        for(int i = nMachines-1; i >= 0; i--)
        {
          aux[i] = machine[i];
          machine[i] = null;
        }
      }

      machine = null;
      machine = aux;
      aux = null;
      System.gc();
    }
  }

  /*
  * La clase Machine provee la información de una máquina determinada.
  */
  public class Machine
  {
    // private int index; // MachineDB[index] --> Simulated Architecture properties.
    private String DEFAULT_ID;
    private String nodeArchitecture; // Simulated architecture.
    private String name;             // Machine name.
    private String id;               // Machine identificator.
    private String architecture;     // Instrumented architecture.
    private String nodes;            // Number of nodes belonging to the mac.
    private String bandwidth;        // Data transfer rate between nodes.
    private String buses;            // Number of buses of the machine.
    private String communication;    // Communication model.

    // Datos referentes a las COLLECTIVE OPERATIONS.
    private int[][] mpi = new int[4][Data.DEFAULT_MPI_ITEMS];
    private int[][] temp_mpi = new int[4][Data.DEFAULT_MPI_ITEMS];

    // Constructor de la clase Machine.
    public Machine(String s)
    {
      DEFAULT_ID = s;
      initialValues();
    }

    // Método que inicializa los datos con los valores por defecto.
    private void initialValues()
    {
      // index = -1;
      nodeArchitecture = DEFAULT_NODE_ARCHITECTURE;
      name             = DEFAULT_NAME;
      id               = DEFAULT_ID;
      architecture     = DEFAULT_ARCHITECTURE;
      nodes            = DEFAULT_NODES;
      bandwidth        = DEFAULT_BANDWIDTH;
      buses            = DEFAULT_BUSES;
      communication    = DEFAULT_COMMUNICATION;

      for(int i = 0; i < 4; i++)
      {
        for(int j = 0; j < Data.DEFAULT_MPI_ITEMS; j++)
        {
          mpi[i][j] = 0;
          temp_mpi[i][j] = 0;
        }
      }
    }

    // Método que permite hacer un cópia de los datos para poder deshacer los
    // posibles cambios en caso necesario.
    private void mpiBackupData()
    {
      for(int i = 0; i < 4; i++)
      {
        for(int j = 0; j < Data.DEFAULT_MPI_ITEMS; j++)
        {
          temp_mpi[i][j] = mpi[i][j];
        }
      }
    }

    // Método que permite recuperar los datos existentes antes de la
    // modificación de los mismos.
    private void mpiRestoreData()
    {
      for(int i = 0; i < 4; i++)
      {
        for(int j = 0; j < Data.DEFAULT_MPI_ITEMS; j++)
        {
          mpi[i][j] = temp_mpi[i][j];
        }
      }
    }

    /*
    * El método mpiConfigured comprueba que los datos referentes a las
    * COLLECTIVE OPERATIONS ya hayan sido configurados. La estructura temporal
    * @temp_mpi solo tendrá valor distinto de 0 si alguna vez se guardaron los
    * datos de COLLECTIVE OPERATIONS.
    *
    * @ret boolean: TRUE si los datos ya se han modificado y salvado, FALSE en
    *               otro caso.
    */
    public boolean mpiConfigured()
    {
      return (temp_mpi[0][0] != 0);
    }

    /*
    * El método mpiGetValue permite obtener el valor de una operación colectiva
    * correspondiente a la columna @column dada.
    *
    * @param: · int column -> columna deseada.
    *                         #1 - Columna MODEL, apartado FAN IN
    *                         #2 - Columna SIZE, apartado FAN IN
    *                         #3 - Columna MODEL, apartado FAN OUT
    *                         #4 - Columna SIZE, apartado FAN OUT
    *         · int elementIndex -> posición de la operación colectiva a
    *                               consultar.
    *
    * @ret int: valor correspondiente a la columna @column y a la operación
    *           colectiva situada en lugar indicado por @elementIndex.
    */
    public int mpiGetValue(int column, int elementIndex)
    {
      return mpi[column-1][elementIndex];
    }

    /*
    * El método mpiSetValue permite fijar el valor de una operación colectiva
    * correspondiente a la columna @column dada.
    *
    * @param: · int column -> columna deseada.
    *                         #1 - Columna MODEL, apartado FAN IN
    *                         #2 - Columna SIZE, apartado FAN IN
    *                         #3 - Columna MODEL, apartado FAN OUT
    *                         #4 - Columna SIZE, apartado FAN OUT
    *         · int elementIndex -> posición de la operación colectiva a
    *                               consultar.
    *         · int value -> valor numérico que tendrá asociado la operación
    *                      colectiva en el apartado indicado por @column.
    */
    public void mpiSetValue(int column, int elementIndex, int value)
    {
      mpi[column-1][elementIndex] = value;
    }

    /*
    * El método mpiLoad permite obtener los datos de una operación colectiva
    * contenidos en la línea dada @line.
    *
    * @param: String line -> cadena de caracteres con los datos de una operación
    *                        colectiva.
    */
    public void mpiLoad(String line)
    {
      int first = 0;
      int second = line.indexOf(" ", first);
      int elementIndex = Integer.parseInt(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(" ", first);
      mpi[0][elementIndex] = Tools.mpiValue(0,Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(" ", first);
      mpi[1][elementIndex] = Tools.mpiValue(1,Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(" ", first);
      mpi[2][elementIndex] = Tools.mpiValue(2,Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.length();
      mpi[3][elementIndex] = Tools.mpiValue(3,Tools.blanks(line.substring(first,second)));
    }

    /*
    * El método mpiSave genera la información de una operación colectiva en un
    * fichero destino @raf.
    *
    * @param: RandomAccessFile raf -> fichero destino.
    *
    * @exc: Error de I/O (escritura a fichero).
    */
    public void mpiSave(RandomAccessFile raf) throws IOException
    {
      for(int i = 0; i < Data.DEFAULT_MPI_ITEMS; i++)
      {
        if(mpi[0][i] == 0) // Si no hay datos --> valores por defecto.
        {
          if(communication.equalsIgnoreCase(COMM_CT))
          {
            mpi[0][i] = 4;
          }
          else if(communication.equalsIgnoreCase(COMM_LIN))
          {
            mpi[0][i] = 3;
          }
          else if(communication.equalsIgnoreCase(COMM_LOG))
          {
            mpi[0][i] = 2;
          }

          mpi[1][i] = Tools.mpiValue(1,"MAX");
          mpi[2][i] = Tools.mpiValue(1,"0");
          mpi[3][i] = Tools.mpiValue(1,"MAX");
        } // END if(no hay datos).

        // Generación de la trama.
        raf.writeBytes("Machine globalop: " +
        id + " " +
        i + " " +
        Tools.mpiString(0,mpi[0][i]) + " " +
        Tools.mpiString(1,mpi[1][i]) + " " +
        Tools.mpiString(2,mpi[2][i]) + " " +
        Tools.mpiString(3,mpi[3][i]) + " " + "\n");
      }
    }

    /*
    * El método defaultValues comprueba si los datos poseen valores por defecto.
    *
    * @ret boolean: TRUE si todos los datos de una máquina tienen los valores
    *               por defecto, FALSE en otro caso.
    */
    public boolean defaultValues()
    {
      if(nodeArchitecture.equalsIgnoreCase(DEFAULT_NODE_ARCHITECTURE) &&
         name.equalsIgnoreCase(DEFAULT_NAME) &&
         id.equalsIgnoreCase(DEFAULT_ID) &&
         architecture.equalsIgnoreCase(DEFAULT_ARCHITECTURE) &&
         nodes.equalsIgnoreCase(DEFAULT_NODES) &&
         bandwidth.equalsIgnoreCase(DEFAULT_BANDWIDTH) &&
         buses.equalsIgnoreCase(DEFAULT_BUSES) &&
         communication.equalsIgnoreCase(DEFAULT_COMMUNICATION))
      {
        return true;
      }
      else
      {
        return false;
      }
    }

    /*
    * El método loadData almacena la información contenida en @line en la
    * estructura de datos de una máquina.
    *
    * @param: · String line -> línea con todos los datos de una máquina.
    *         · boolean oldFile -> TRUE si los datos provienen de un fichero de
    *                              configuración antiguo (menos datos), FALSE
    *                              en otro caso.
    *
    * @exc: Valor numérico no válido.
    */
    public boolean loadData(String line, boolean oldFile, int lineCount) throws Exception
    {
      Pattern pattern = Pattern.compile("\"environment information\" \\{(.*)\\};;$");
      Matcher matcher = pattern.matcher(line);

      if (!matcher.matches())
      {
        Tools.showErrorDialog("Wrong node information record structure");
        return false;
      }
      
      String fields          = matcher.group(1);
      String machineFields[] = fields.split(",");
      
      if (machineFields.length == 7)
      {
        setName(Tools.blanks(machineFields[0]));
        setId(Tools.blanks(machineFields[1]));
        setArchitecture(Tools.blanks(machineFields[2]));
        setNodes(Tools.blanks(machineFields[3]));
        setBandwidth(Tools.blanks(machineFields[4]));
        setBuses(Tools.blanks(machineFields[5]));
        setCommunication(Tools.blanks(machineFields[6]));
      }
      else
      {
        Tools.showErrorDialog("Wrong node information record fields");
        return false;
      }
      
      /*
      if(!oldFile)
      {
        setName(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(",",first);
        setId(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(",",first);
      }

      setArchitecture(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setNodes(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setBandwidth(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf(",",first);
      setBuses(Tools.blanks(line.substring(first,second)));
      first = second + 1;
      second = line.indexOf("}",first);
      setCommunication(Tools.blanks(line.substring(first,second)));
      */

      return true;
    }

    /*
    * El método saveData genera, en un fichero de configuración @target, la
    * información correspondiente a una máquina.
    *
    * @param: RandomAccessFile target -> fichero de configuración destino.
    *
    * @exc: Error de I/O (escritura a fichero).
    */
    public void saveData(RandomAccessFile target) throws IOException
    {
      target.writeBytes(Data.ENVIRONMENT);
      target.writeBytes(getName(true) + ", ");
      target.writeBytes(getId() + ", ");
      target.writeBytes(getArchitecture(true) + ", ");
      target.writeBytes(getNodes() + ", ");
      target.writeBytes(getBandwidth() + ", ");
      target.writeBytes(getBuses() + ", ");
      target.writeBytes(getCommunication() + "};;\n");
    }

    /*
    // Métodos GET: permiten el acceso externo a los datos de una máquina.
    public int getIndex()
    {
      return index;
    }

    public void setIndex(int value)
    {
      index = value;
    }
    */

    public String getNodeArchitecture()
    {
      return nodeArchitecture;
    }

    public void setNodeArchitecture(String value)
    {
      nodeArchitecture = value;
    }

    public String getName(boolean toDisk)
    {
      if(toDisk) // Si el dato va a disco --> nombre entre comillas.
      {
        return "\"" + name + "\"";
      }

      return name;
    }

    public String getId()
    {
      return id;
    }

    public String getArchitecture(boolean toDisk)
    {
      if(toDisk) // Si el dato va a disco --> nombre entre comillas.
      {
        return "\"" + architecture + "\"";
      }

      return architecture;
    }

    public String getNodes()
    {
      return nodes;
    }

    public String getBandwidth()
    {
      return bandwidth;
    }

    public String getBuses()
    {
      return buses;
    }

    public String getCommunication()
    {
      return communication;
    }

    // Métodos SET: permiten la modificación, de forma externa, de los datos de
    // una máquina.
    public void setName(String value)
    {
      if(value.startsWith("\"") && value.endsWith("\"")) // Quitar comillas.
      {
        name = value.substring(1,value.length()-1);
      }
      else
      {
        name = value;
      }
    }

    public void setId(String value) throws NumberFormatException
    {
      try
      {
        Integer.parseInt(value);
        id = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("MACHINE ID");
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

    public void setNodes(String value) throws NumberFormatException
    {
      try
      {
        Integer.parseInt(value);
        nodes = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("NUMBER OF NODES");
          throw e;
        }
    }

    public void setBandwidth(String value) throws NumberFormatException
    {
      try
      {
        Double.parseDouble(value);
        bandwidth = Tools.filterForDouble(value);
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("NETWORK BANDWIDTH");
          throw e;
        }
    }

    public void setBuses(String value) throws NumberFormatException
    {
      try
      {
        Integer.parseInt(value);
        buses = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("NUMBER OF BUSES");
          throw e;
        }
    }

    public void setCommunication(String value) throws NumberFormatException
    {
      try
      {
        Integer.parseInt(value);
        communication = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("COMMUNICATION MODEL");
          throw e;
        }
    }
  }
}
