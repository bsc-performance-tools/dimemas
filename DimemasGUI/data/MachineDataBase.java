package data;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import tools.*;
import java.io.*;

/*
* Clase que albergará los datos correspondientes a las arquitecturas
* predefinidas de las máquinas.
*/
public class MachineDataBase
{
  static private final String MACHINE_DB = "machine : ";

  private int nMachines = 0;  // Número de máquinas/arquitecturas predefinidas.
  public Machine[] machine;   // Máquinas/arquitecturas.

  // Constructor de la clase MachineDataBase.
  public MachineDataBase()
  {
    loadDB();
  }

  // Método que permite acceder al número de arquitecturas fuera de la clase.
  public int getNumberOfMachinesInDB()
  {
    return nMachines;
  }

  /*
  * El método addElement permite añadir una nueva máquina a la base de datos con
  * las especificaciones dadas en @info[].
  *
  * @param: String[] info -> listado con las propiedades de la nueva máquina.
  *
  * @exc: Valor numérico no válido.
  */
  public void addElement(String[] info) throws Exception
  {
    nMachines++;
    Machine[] aux = new Machine[nMachines];

    try
    {
      // Se añade el nuevo elemento (en último lugar).
      aux[nMachines-1] = new Machine();
      aux[nMachines-1].setName(info[0]);
      aux[nMachines-1].setLabel(info[1]);
      aux[nMachines-1].setProcessors(info[2]);
      aux[nMachines-1].setInputLinks(info[3]);
      aux[nMachines-1].setOutputLinks(info[4]);
      aux[nMachines-1].setLocalStartup(info[5]);
      aux[nMachines-1].setRemoteStartup(info[6]);
      aux[nMachines-1].setDataTransferRate(info[7]);
    } catch(Exception e)
      {
        nMachines--;
        aux = null;
        System.gc();
        throw e;
      }

    // Si todo ha ido bien, copiar el resto de la base de datos.
    for(int i = 0; i < nMachines - 1; i++)
    {
      aux[i] = machine[i];
      machine[i] = null;
    }

    machine = null;
    machine = aux;
    aux = null;
    System.gc();
    saveDB();
  }

  /*
  * El método deleteElement permite eliminar una máquina de la base de datos.
  *
  * @param: int index -> posición de la máquina a borrar.
  */
  public void deleteElement(int index)
  {
    nMachines--;
    int i = nMachines;
    int j = i-1;
    Machine[] aux = new Machine[nMachines];

    while(i >= 0)
    {
      if(i != index)
      {
        aux[j] = machine[i];
        j--;
      }

      machine[i] = null;
      i--;
    }

    machine = null;
    machine = aux;
    aux = null;
    System.gc();
    saveDB();   // Salvar la nueva base de datos a fichero.
  }

  // Método que carga en memoria la base de datos de máquinas predefinidas.
  public void loadDB()
  {
    int first = 0;
    int second = 0;
    String line;
    RandomAccessFile source;

    try
    {
      source = new RandomAccessFile(new File(Data.MACHINE_DB_FILE),"r");

      // Adquirir el número de arquitecturas definidas.
      while(source.getFilePointer() != source.length())
      {
        if(source.readLine().startsWith(MACHINE_DB))
        {
          nMachines++;
        }
      }

      source.seek(0);
      machine = new Machine[nMachines];

      // Carga de los datos desde el fichero a memoria.
      for(int i = 0; i < nMachines; i++)
      {
        machine[i] = new Machine();
        line = source.readLine();
        first = MACHINE_DB.length();
        second = line.indexOf(" ", first);
        machine[i].setLabel(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        machine[i].setName(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        machine[i].setProcessors(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        machine[i].setInputLinks(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        machine[i].setOutputLinks(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        machine[i].setLocalStartup(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.indexOf(" ", first);
        machine[i].setRemoteStartup(Tools.blanks(line.substring(first,second)));
        first = second + 1;
        second = line.length();
        machine[i].setDataTransferRate(Tools.blanks(line.substring(first,second)));
      }

      source.close();
    } catch(Throwable exc)
      {
        Tools.showInformationMessage(exc.toString());
      }
  }

  // Método que salva en fichero la base de datos existente en memoria.
  public void saveDB()
  {
    RandomAccessFile target;
    File file = new File(Data.MACHINE_DB_FILE);

    try
    {
      target = new RandomAccessFile(file,"rw");
      target.setLength(0);

      for(int i = 0; i < nMachines; i++)
      {
        target.writeBytes(MACHINE_DB + machine[i].getLabel() + " " +
                          machine[i].getName() + " " +
                          machine[i].getProcessors() + " " +
                          machine[i].getInputLinks() + " " +
                          machine[i].getOutputLinks() + " " +
                          machine[i].getLocalStartup() + " " +
                          machine[i].getRemoteStartup() + " " +
                          machine[i].getDataTransferRate() + "\n");
      }

      target.close();
    } catch(Throwable exc)
      {
        Tools.showInformationMessage(exc.toString());
      }
  }

  /*
  * La clase Machine provee la información de una máquina predefinida.
  */
  public class Machine
  {
    private String name;
    private String label;
    private String processors;
    private String input;
    private String output;
    private String local;
    private String remote;
    private String dataTR;

    // Métodos GET: permiten el acceso externo a los datos de una máquina.
    public String getName()
    {
      return name;
    }

    public String getLabel()
    {
      return label;
    }

    public String getProcessors()
    {
      return processors;
    }

    public String getInputLinks()
    {
      return input;
    }

    public String getOutputLinks()
    {
      return output;
    }

    public String getLocalStartup()
    {
      return local;
    }

    public String getRemoteStartup()
    {
      return remote;
    }

    public String getDataTransferRate()
    {
      return dataTR;
    }

    // Métodos SET: permiten la modificación, de forma externa, de los datos de
    // una máquina predefinida.
    public void setName(String value)
    {
      if(!value.equalsIgnoreCase(" "))
      {
        name = Tools.blanks(value).replace(' ','_'); // Reemplazar espacios.
      }
      else
      {
        name = "_";
      }
    }

    public void setLabel(String value)
    {
      if(!value.equalsIgnoreCase(" "))
      {
        label = Tools.blanks(value).replace(' ','_'); // Reemplazar espacios.
      }
      else
      {
        label = "_";
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

    public void setInputLinks(String value) throws Exception
    {
      try
      {
        Integer.parseInt(value);
        input = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("INPUT LINKS");
          throw e;
        }
    }

    public void setOutputLinks(String value) throws Exception
    {
      try
      {
        Integer.parseInt(value);
        output = value;
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("OUTPUT LINKS");
          throw e;
        }
    }

    public void setLocalStartup(String value) throws Exception
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

    public void setRemoteStartup(String value) throws Exception
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

    public void setDataTransferRate(String value) throws Exception
    {
      try
      {
        Double.parseDouble(value);
        dataTR = Tools.filterForDouble(value);
      } catch(NumberFormatException e)
        {
          Tools.showErrorMessage("DATA TRANSFER RATE");
          throw e;
        }
    }
  }
}
