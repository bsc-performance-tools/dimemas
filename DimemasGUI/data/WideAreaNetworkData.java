package data;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import data.Data;
import tools.*;
import java.io.*;

/*
* Clase que albergará los datos correspondientes a la red WAN.
*/
public class WideAreaNetworkData
{
  public final String COMM_GROUP_LOG = "3";
  public final String COMM_GROUP_LIN = "2";
  public final String COMM_GROUP_CT = "1";

  public final String TRAFFIC_FUNCTION_EXP = "1";
  public final String TRAFFIC_FUNCTION_LOG = "2";
  public final String TRAFFIC_FUNCTION_LIN = "3";
  public final String TRAFFIC_FUNCTION_CT = "4";

  // Valores por defecto.
  public final String DEFAULT_NAME = "";
  public final String DEFAULT_MACHINES = "1";
  public final String DEFAULT_DEDICATED = "0";
  public final String DEFAULT_TRAFFIC = TRAFFIC_FUNCTION_CT;
  public final String DEFAULT_MAX = "0.0";
  public final String DEFAULT_BANDWIDTH = "0.0";
  public final String DEFAULT_COMMUNICATION = COMM_GROUP_CT;

  private String name;          // Name of WAN.
  private String machines;      // Number of machines.
  private String dedicated;     // Number of dedicated connections.
  private String traffic;       // Influence of traffic.
  private String max;           // Max traffic value.
  private String bandwidth;     // External bandwidth.
  private String communication; // Communication group model.

  // Datos referentes a las COLLECTIVE OPERATIONS.
  private int[][] mpi = new int[4][Data.DEFAULT_MPI_ITEMS];

  /*
  * El método loadData almacena la información facilitada por @line, en la
  * estructura de datos de la red WAN.
  *
  * @param: String line -> línea con todos los datos de una red WAN.
  *
  * @exc: Valor numérico no válido.
  */
  public void loadData(String line) throws Exception
  {
    int first = Data.WAN.length();
    int second = line.indexOf(",",first);

    setName(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setMachines(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setDedicated(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setTraffic(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setMax(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf(",",first);
    setBandwidth(Tools.blanks(line.substring(first,second)));
    first = second + 1;
    second = line.indexOf("}",first);
    setCommunication(Tools.blanks(line.substring(first,second)));
  }

  /*
  * El método saveData genera, en un fichero de configuración @target, la
  * información correspondiente a una red WAN.
  *
  * @param: RandomAccessFile target -> fichero de configuración destino.
  *
  * @exc: Error de I/O (escritura a fichero).
  */
  public void saveData(RandomAccessFile target) throws IOException
  {
    target.writeBytes(Data.WAN);
    target.writeBytes(getName(true) + ", ");
    target.writeBytes(getMachines() + ", ");
    target.writeBytes(getDedicated() + ", ");
    target.writeBytes(getTraffic() + ", ");
    target.writeBytes(getMax() + ", ");
    target.writeBytes(getBandwidth() + ", ");
    target.writeBytes(getCommunication() + "};;\n");
  }

  // Constructor de la clase WideAreaNetworkData.
  public WideAreaNetworkData()
  {
    initialValues();
  }

  // Método que inicializa los datos con los valores por defecto.
  public void initialValues()
  {
    name = DEFAULT_NAME;
    machines = DEFAULT_MACHINES;
    dedicated = DEFAULT_DEDICATED;
    traffic = DEFAULT_TRAFFIC;
    max = DEFAULT_MAX;
    bandwidth = DEFAULT_BANDWIDTH;
    communication = DEFAULT_COMMUNICATION;

    for(int i = 0; i < 4; i++)
    {
      for(int j = 0; j < Data.DEFAULT_MPI_ITEMS; j++)
      {
        mpi[i][j] = 0;
      }
    }
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
  * contenidos en @line.
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
        if(communication.equalsIgnoreCase(COMM_GROUP_CT))
        {
          mpi[0][i] = 4;
        }
        else if(communication.equalsIgnoreCase(COMM_GROUP_LIN))
        {
          mpi[0][i] = 3;
        }
        else if(communication.equalsIgnoreCase(COMM_GROUP_LOG))
        {
          mpi[0][i] = 2;
        }

        mpi[1][i] = Tools.mpiValue(1,"MAX");
        mpi[2][i] = Tools.mpiValue(1,"0");
        mpi[3][i] = Tools.mpiValue(1,"MAX");
      } // END if(no hay datos).

      // Generación de la trama.
      raf.writeBytes("External globalop: " + i + " " +
      Tools.mpiString(0,mpi[0][i]) + " " +
      Tools.mpiString(1,mpi[1][i]) + " " +
      Tools.mpiString(2,mpi[2][i]) + " " +
      Tools.mpiString(3,mpi[3][i]) + " " + "\n");
    }
  }

  /*
  * El método defaultValues comprueba si los datos poseen valores por defecto.
  *
  * @ret boolean: TRUE si todos los datos de la red WAN tienen los valores por
  *               defecto, FALSE en otro caso.
  */
  public boolean defaultValues()
  {
    if(name.equalsIgnoreCase(DEFAULT_NAME) &&
       machines.equalsIgnoreCase(DEFAULT_MACHINES) &&
       dedicated.equalsIgnoreCase(DEFAULT_DEDICATED) &&
       traffic.equalsIgnoreCase(DEFAULT_TRAFFIC) &&
       max.equalsIgnoreCase(DEFAULT_MAX) &&
       bandwidth.equalsIgnoreCase(DEFAULT_BANDWIDTH) &&
       communication.equalsIgnoreCase(DEFAULT_COMMUNICATION))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // Métodos GET: permiten el acceso externo a los datos de la red WAN.
  public String getName(boolean toDisk)
  {
    if(toDisk) // Si el dato va a disco --> nombre entre comillas.
    {
      return "\"" + name + "\"";
    }

    return name;
  }

  public String getMachines()
  {
    return machines;
  }

  public String getDedicated()
  {
    return dedicated;
  }

  public String getTraffic()
  {
    return traffic;
  }

  public String getMax()
  {
    return max;
  }

  public String getBandwidth()
  {
    return bandwidth;
  }

  public String getCommunication()
  {
    return communication;
  }

  // Métodos SET: permiten la modificación, de forma externa, de los datos de
  // la red WAN.
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

  public void setMachines(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      machines = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("NUMBER OF MACHINES");
        throw e;
      }
  }

  public void setDedicated(String value) throws Exception
  {
    try
    {
      Integer.parseInt(value);
      dedicated = value;
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("DEDICATED CONNECTIONS");
        throw e;
      }
  }

  public void setTraffic(String value)
  {
    traffic = value;
  }

  public void setMax(String value) throws Exception
  {
    try
    {
      Double.parseDouble(value);
      max = Tools.filterForDouble(value);
    } catch(NumberFormatException e)
      {
        Tools.showErrorMessage("MAX TRAFFIC VALUE");
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
        Tools.showErrorMessage("EXTERNAL NET BANDWIDTH");
        throw e;
      }
  }

  public void setCommunication(String value)
  {
    communication = value;
  }
}
