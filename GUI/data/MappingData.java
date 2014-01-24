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
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/*
* Clase que albergará los datos correspondientes al MAPPING de tareas y nodos.
*/
public class MappingData
{
  public class MapResult
  {
    public int[]  generatedMap;
    public String errorMessage;
    
    public void MapResult()
    {
      generatedMap = new int[0];
      errorMessage = null;
    }
  }
  
  public class MapTest
  {
    public int mapInfo;
    public int nTasksPerNode;
    
    public void MapTest()
    {
      mapInfo       = Data.NO_MAP;
      nTasksPerNode = 1;
    }
  }

  // Valores por defecto.
  public final int    DEFAULT_TASKS     = Data.NO_TASKS_DETECTED;
  public final String DEFAULT_TRACEFILE = "";
  public final int[]  DEFAULT_MAP       = new int[0];
  
  
  NodeData      nodesInformation;
  
  // private int    tasks;     // Number of tasks of the application.
  private String tracefile;    // Tracefile name of application.
  // private String map;       // List of nodes in application.
  // private int    mapInfo;   // Map type, for informational pourposes
  
  private int   nTasks;
  private int[] map; // Actual storage of the mapping 
  int           mapInfo;
  int           nTasksPerNode;
  
  boolean       modified = false;

  // Constructor de la clase MappingData.
  public MappingData(NodeData nodesInformation)
  {
    this.nodesInformation = nodesInformation;
    initialValues();
  }
  
  /**
  * Loads an explicit task mapping defined in @line.
  *
  * @param line      record with the mapping information
  * @param lineCount line number of the current record in the configuration file
  * 
  * @return          true if mapping correctly load, false otherwise
  */
  public boolean loadTaskMapping(String line, int lineCount) throws Exception
  {
    MapTest typeOfMap;
    // Pattern mapPattern = Pattern.compile("#DIMEMAS:\"(.*)\":(.*):(\\d+)\\((.*)$");
    
    // Pattern mapPattern = Pattern.compile("^\"mapping information\" \\{\"(.*)\", (\\d+), \\[(\\d+)\\] \\{[(\\d\\+),|(\\d+)]+\\}\\};;$");
    Pattern mapPattern = Pattern.compile("^\"mapping information\" \\{\"(.*)\", (\\d+), \\[(\\d+)\\]\\s*\\{(.*)\\}\\};;$");
    
    Matcher mapMatcher = mapPattern.matcher(line);
    
    if (mapMatcher.matches())
    {
      /* 1: Trace Name
       * 2: Number of tasks
       * 3: # Tasks (array size)
       * 4: Array representing the map */
      
      tracefile       = mapMatcher.group(1);
      nTasks          = Integer.parseInt(mapMatcher.group(2));
      int mapLength   = Integer.parseInt(mapMatcher.group(3));
      String[] mapStr = mapMatcher.group(4).split(",");
      
      if (nTasks != mapLength || nTasks != mapStr.length)
      {
        Tools.showErrorDialog("Incorrect length of the map array in current file");
        return false;
      }
      
      map = new int[nTasks];
      
      for (int i = 0; i < nTasks; i++)
      {
        try
        {
          map[i] = Integer.parseInt(mapStr[i]);
        }
        catch (NumberFormatException e)
        {
          Tools.showErrorDialog("Incorrect node value in the map array present in current file");
          return true;
        }
      }
      
    }
    else
    {
      Tools.showErrorDialog("Unable to read the mapping information");
      return false;
    }
  
   
    // Parse the actual map string
    typeOfMap = detectMap(map);
    
    mapInfo       = typeOfMap.mapInfo;
    nTasksPerNode = typeOfMap.nTasksPerNode;
   
    return true;
  }

  /**
  * Loads a predefined task mapping described in @line.
  *
  * @param line      record with the mapping information
  * @param lineCount line number of the current record in the configuration file
  * 
  * @return          true if mapping correctly load, false otherwise
  */
  public boolean loadPredefinedMapping(String line, int lineCount) throws Exception
  {
    // Pattern mapPattern = Pattern.compile("#DIMEMAS:\"(.*)\":(.*):(\\d+)\\((.*)$");
    String recordFields;
    String predefinedMapStr;
    
    // Pattern mapPattern = Pattern.compile("^\"mapping information\" \\{\"(.*)\", (\\d+), \\[(\\d+)\\] \\{[(\\d\\+),|(\\d+)]+\\}\\};;$");
    // Pattern mapPattern = Pattern.compile("^\"predefined mapping information\" \\{\"(.*)\",.*\"(.*)\".*\\}\\};;$");
    Pattern mapPattern = Pattern.compile("^\"predefined mapping information\" \\{(.*)\\};;$");
    
    Matcher mapMatcher = mapPattern.matcher(line);
    
    if (mapMatcher.matches())
    {
      /* 1: Trace Name */
      recordFields = mapMatcher.group(1);
    }
    else
    {
      Tools.showErrorDialog("Unable to read the predefined mapping information (line "+lineCount+")");
      return false;
    }
    
    /* Check if the tracefile is expressed in the trace */
    
    String fields[] = recordFields.split(",");
    
    if (fields.length == 2) /* Trace and mapping */
    {   
      fields[0] = fields[0].trim();
      fields[1] = fields[1].trim();
      
      // Erase quotes
      tracefile        = fields[0].substring(1, fields[0].length()-1);    
      predefinedMapStr = fields[1].substring(1, fields[1].length()-1);
      
      if (!checkNumberOfTasks(tracefile))
      {
        nTasks = DEFAULT_TASKS;
      }
    }
    else /* Only mapping */
    {
      recordFields = recordFields.trim();
      predefinedMapStr = recordFields.substring(1, recordFields.length()-1);
      predefinedMapStr = predefinedMapStr.trim();
      
      tracefile = DEFAULT_TRACEFILE;
      nTasks    = DEFAULT_TASKS;
    }
   
    
    if (!checkPredefinedMapString(predefinedMapStr))
    {
      return false;
    }
    
    return true;
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
    if (mapInfo == Data.NO_MAP)
    {
      Tools.showErrorMessage("No task mapping information available to be saved");
    }
    else if (mapInfo == Data.UNKNOWN_MAP)
    {
      target.writeBytes(Data.MAPPING);
      target.writeBytes(getTracefile(true) + ", ");
      target.writeBytes(getTasks() + ", ");
      target.writeBytes(getMap(true) + "};;\n");
    }
    else
    {
      target.writeBytes(Data.PREDEF_MAP);
      if (!tracefile.isEmpty())
      {
        target.writeBytes(getTracefile(true));
        target.writeBytes(", ");
      }
      
      target.writeBytes(getPredefinedMapLabel()+"};;\n");
    }
    
  }

  // Método que inicializa los datos con los valores por defecto.
  public void initialValues()
  {
    tracefile     = DEFAULT_TRACEFILE;
    nTasks        = DEFAULT_TASKS;
    map           = DEFAULT_MAP;
    mapInfo       = Data.NO_MAP;
    nTasksPerNode = 1;
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
    if (mapInfo == Data.NO_MAP)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  /*
  * If map was assigned, clears it.
  *
  * @ret boolean: TRUE if the map was cleared, FALSE if there was no map
  *               initialized
  */
  public boolean clearMap()
  {
    if (mapInfo == Data.NO_MAP)
    {
      return false;
    }
    else
    {
      nTasks  = DEFAULT_TASKS;
      map     = new int[0];
      mapInfo = Data.NO_MAP;
      return true;
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
    return nTasks;
  }

  public String getMap(boolean toDisk)
  {
    String Result = "";
    
    if(toDisk) // Si va a disco, crear formato [n Tasks] {M1,M2,...,Mn}.
    {
      if (nTasks == 0 || map.length == 0)
      {
        Result += "[0];";
      }
      else
      {
        Result += "["+nTasks+"]";
      }
    }

    if (map.length > 0)
    {
      Result +=  " {"+map[0];

      for (int i = 1; i < map.length; i++)
      {
        Result += ","+map[i];
      }

      Result += "}";
    }

    return Result;
  }

  public int[] getMap()
  {
    return map;
  }
  
  public int getMapInfo()
  {
    return mapInfo;
  }
  
  public int getNTasksPerNode()
  {
    return nTasksPerNode;
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
       
    if (checkNumberOfTasks(tracefile))
    {
      if (mapInfo != Data.NO_MAP)
      {
        /* Update the map! */
        generateMap();
      }
    }
  }

  public void setTasks(int value)
  {
    nTasks = value;
  }

  public void setMap(int[] value)
  {
    map = value;
   
    /*
    if(Tools.verifyData(value,"MAPPING") != -1)
    {
      logicalMap = value;
    }
    else
    {
      throw new NumberFormatException();
    }
    */
  }
  
  public String setMapInfo(int newMapInfo)
  {
    mapInfo = newMapInfo;
    
    return generateMap();
  }
  
  public void setNTasksPerNode(int nTasksPerNode)
  {
    this.nTasksPerNode = nTasksPerNode;
  }
  
  /*
  * If map was assigned, clears it.
  *
  * @ret boolean: TRUE if the map was cleared, FALSE if there was no map
  *               initialized
  */
  public String changeAtNodes()
  {
    return generateMap();
  }
 
  private String generateMap()
  {
    String    result;
    MapResult mappingOut;
    switch(mapInfo)
    {
      case Data.NO_MAP:
        result = null;
        break;
      case Data.FILL_NODE_MAP:
        // Trick to ensure that the map is correctly generated
        mappingOut = mapFillingNodes();
        map        = mappingOut.generatedMap;
        result     = mappingOut.errorMessage;
        break;
      case Data.N_TASKS_PER_NODE_MAP:
        // Trick to ensure that the map is correctly generated
        mappingOut = mapNTasksPerNode(nTasksPerNode);
        map        = mappingOut.generatedMap;
        result     = mappingOut.errorMessage;
        break;
      case Data.INTERLEAVE_MAP:
        // Trick to ensure that the map is correctly generated
        mappingOut = mapInterleaved();
        map        = mappingOut.generatedMap;
        result     = mappingOut.errorMessage;
        break;
      case Data.UNKNOWN_MAP:
        result = "Unable to generate an irregular map";
        break;
      default:
        result = "Wrong map value";
        break;
    }
    
    return result;
  }
 
  /**
   * Checks the number of tasks defined in a trace file (saving it in 'nTasks'
   * attribute. The code is duplicated from 'InitialMachineWindow.java'
   * 
   * @param tracefile Name of the trace file to be read
   * 
   * @return          True if the number of tasks was read successfully, false
   *                  otherwise
   */
  private boolean checkNumberOfTasks(String tracefile)
  {
    File             sourceFile;
    RandomAccessFile sourceFileRead;
    sourceFile = new File(tracefile);
    String headerLine;
      
    if (!sourceFile.exists() || !sourceFile.isFile())
    {
      return false;
    }

    try
    {
      sourceFileRead = new RandomAccessFile(sourceFile,"r");
    }
    catch(FileNotFoundException fnfe)
    {
      return false;
    }
    
    do
    {
      try
      {
        headerLine = sourceFileRead.readLine();
      }
      catch (IOException ioe)
      {
        return false;
      }

    } while(headerLine.equals(""));


    if (headerLine.startsWith("#DIMEMAS"))
    {
      /* line contains the header! */
      Pattern header_pattern = Pattern.compile("#DIMEMAS:\"(.*)\":(.*):(\\d+)\\((.*)$");
      Matcher header_matcher = header_pattern.matcher(headerLine);

      if (header_matcher.matches())
      {
      /* 1: Trace Name
       * 2: Offsets
       * 3: # Tasks
       * 4: Rest of object definitions*/
        String objects = header_matcher.group(3);
        nTasks = Integer.parseInt(objects);
      }
      else
      {
        return false;
      }
    }
    else if (headerLine.startsWith("SDDFA;;"))
    {
      return false;
    }
    else
    {
      return false;
    }

    try
    {
      sourceFileRead.close();
    }
    catch (IOException ioe)
    {
      return false;
    }
    
    return true;
  }
  
  
  /**
   * Checks if the parameter corresponds to a predefined map, and loads
   * @param predefinedMapStr
   * @return 
   */
  private boolean checkPredefinedMapString(String predefinedMapStr)
  {
    if (predefinedMapStr.indexOf(Data.FILL_NODE_MAP_STR) != -1)
    {
      mapInfo = Data.FILL_NODE_MAP;
    }
    else if (predefinedMapStr.indexOf(Data.N_TASKS_PER_NODE_MAP_STR) != -1)
    {
      String fields[] = predefinedMapStr.split(" ");
      
      if (fields.length != 2)
      {
        Tools.showErrorMessage("Error in 'n' tasks per node mapping definition ("+predefinedMapStr+")");
        return false;
      }
      
      try
      {
        nTasksPerNode = Integer.parseInt(fields[0]);
      } 
      catch (NumberFormatException nfe)
      {
        Tools.showErrorMessage("Wrong number of tasks in 'n' tasks per node definition ("+predefinedMapStr+")");
        return false;
      }
      mapInfo = Data.N_TASKS_PER_NODE_MAP;
    }
    else if (predefinedMapStr.indexOf(Data.INTERLEAVE_MAP_STR) != -1)
    {
      mapInfo = Data.INTERLEAVE_MAP;
    }
    else
    {
      Tools.showErrorMessage("Unknown map defined ("+predefinedMapStr+")");
    }
    
    return true;
  }
  
  /**
   * Checks which is the map stored
   * 
   * @param mapToTest Vector of integers representing a task array
   * 
   * @return          MapTest object indicating the detected map configuration
   */
 
  public MapTest detectMap(int[] mapToTest)
  {
    MapTest result       = new MapTest();
    int     nNodes       = nodesInformation.getNumberOfNodes();
    int[]   nCpusPerNode = nodesInformation.getCpusPerNode();;
    
    if (mapToTest.length == 0)
    {
      result.mapInfo       = Data.NO_MAP;
      result.nTasksPerNode = 0;
      return result;
    }
    else
    {
      boolean      filledNodes     = true, interleaved = true;
      int          notFilledNodes  = 0;
      int          previousNodeMap = 0;
      Set<Integer> DifferentTasksPerNode = new HashSet<Integer>();
      
      int[] mapedTasksCount = new int[nNodes];

      for (int i = 0; i < mapToTest.length; i++)
      {
        mapedTasksCount[mapToTest[i]]++;
      }

      for (int i = 0; i < mapedTasksCount.length; i++)
      {
        if (mapedTasksCount[i] > nCpusPerNode[i])
        {
          filledNodes = false;
          break;
        }
        
        if (mapedTasksCount[i] < nCpusPerNode[i] && mapedTasksCount[i] > 0)
        {
          notFilledNodes++;
        }
      }
      
      /* If all nodes are filled, it prevails over 'n Tasks per Node' */
      if (filledNodes && notFilledNodes <= 1)
      {
        result.mapInfo       = Data.FILL_NODE_MAP;
        result.nTasksPerNode = 0;
        return result;
      }
      
      /* Check if all nodes received the same number of tasks */
      for (int i = 0; i < mapedTasksCount.length; i++)
      {
        if (mapedTasksCount[i] != 0)
        {
          DifferentTasksPerNode.add(mapedTasksCount[i]);
        }
      }
      
      if (DifferentTasksPerNode.size() == 1)
      {
        Integer[] nTasksPerNodeRead = DifferentTasksPerNode.toArray(new Integer[0]);
        
        result.mapInfo       = Data.N_TASKS_PER_NODE_MAP;
        result.nTasksPerNode = nTasksPerNodeRead[0];
        return result;
      }
      
      /* Check if it is interleaved */
      for (int i = 0; i < mapToTest.length; i++)
      {
        if (i == 0)
        {
          previousNodeMap = mapToTest[0];
        }
        else
        {
          if (mapToTest[i] != previousNodeMap+1%nNodes)
          {
            interleaved = false;
            break;
          }
          else
          {
            previousNodeMap = mapToTest[i];
          }
        }
      }
      
      if (interleaved)
      {
        result.mapInfo       = Data.INTERLEAVE_MAP;
        result.nTasksPerNode = 0;
        
        return result;
      }
      else
      {
        result.mapInfo       = Data.UNKNOWN_MAP;
        result.nTasksPerNode = 0;
        
        return result;
      }
    }
  }

        /*
  * Fills the nodes at their maximum capacity (in terms of processors). If the
  * the number of tasks is larger than the total available CPUs, last node
  * in the system receives the trailing tasks
  *
  */
  public MapResult mapFillingNodes()
  {
    int     lastTaskAssigned = 0;
    boolean end              = false, saturatedNode = false;
    int     nNodes           = nodesInformation.getNumberOfNodes();
    int[]   nCpusPerNode     = nodesInformation.getCpusPerNode();
    int     totalCpus         = 0;

    MapResult result    = new MapResult();
    
    if (nTasks > 0)
    {
    
      result.generatedMap = new int[nTasks];

      for (int i = 0; i < nCpusPerNode.length && !end; i++)
      {
        for (int j = 0; j < nCpusPerNode[i] && !end; j++)
        {
          result.generatedMap[lastTaskAssigned] = i;
          lastTaskAssigned++;

          if (lastTaskAssigned == nTasks)
          {
            end = true;
          }
          totalCpus++;
        }
      }

      for (;lastTaskAssigned < nTasks; lastTaskAssigned++)
      {
        result.generatedMap[lastTaskAssigned] = nNodes-1;
        saturatedNode         = true;
      }

      if (saturatedNode)
      {
        result.errorMessage =  "Total available CPUs ("+totalCpus+") less than the number of tasks\n"+
                               "Last node in the machine will be saturated";
      }
    }

    return result;
  }
  
  /*
  * Assign a fixed number of tasks per node (even if they don't fit)
  *
  */
  public MapResult mapNTasksPerNode(int nTasksPerNode)
  {
    int lastAssignedNode   = 0;
    int assignedTasksCount = 0;
    int nNodes             = nodesInformation.getNumberOfNodes();

    MapResult result    = new MapResult();
 
    if (nTasks > 0)
    {
      result.generatedMap = new int[nTasks];

      if (nTasksPerNode * nNodes < nTasks)
      {
        result.errorMessage = nTasksPerNode+" tasks per node on "+nNodes+" nodes is less "+
                             "than the total application tasks ("+nTasks+")";
      }

      for (int i = 0; i < nTasks; i++)
      {
        if (assignedTasksCount < nTasksPerNode)
        {
          result.generatedMap[i] = lastAssignedNode;
          assignedTasksCount++;
        }
        else
        {
          result.generatedMap[i] = ++lastAssignedNode;
          assignedTasksCount = 1;
        }
      }
    }
  
    return result;
  }

  // Método que realiza la distribución automática del MAPPING.
  public MapResult mapInterleaved()
  {
    int nNodes = nodesInformation.getNumberOfNodes();

    MapResult result = new MapResult();
    
    if (nTasks > 0)
    {
      result.generatedMap = new int[nTasks];

      for (int i = 0; i < nTasks; i++)
      {
        result.generatedMap[i] = i%nNodes;
      }

      result.errorMessage = null;
    }
    
    return result;
  }
  
  private String getPredefinedMapLabel()
  {
    switch(mapInfo)
    {
      case Data.FILL_NODE_MAP:
        return "\""+Data.FILL_NODE_MAP_STR+"\"";
      case Data.N_TASKS_PER_NODE_MAP:
        return "\""+String.valueOf(nTasksPerNode)+" "+Data.N_TASKS_PER_NODE_MAP_STR+"\"";
      case Data.INTERLEAVE_MAP:
        return "\""+Data.INTERLEAVE_MAP_STR+"\"";
      default:
        return "";
    }
  }
}


