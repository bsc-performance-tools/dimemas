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

package gui;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Oscar Bardillo Luján
 * @version 1.0
 */

import data.*;
import tools.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración correspondiente al MAPPING entre
* nodos y tareas.
*/
public class MappingWindow extends GUIWindow
{
  public static final long serialVersionUID = 14L;
  
  private int nodes;
  private int tasks;

  private String stz     = data.map.getMap(false);
  private int    mapInfo = data.map.getMapInfo();

  private JButton[]        node;
  private JRadioButton[][] map;
  private ButtonGroup[]    group;
  private JScrollPane      scrollPanel;
  private JPanel           buttonPanel;
  
  private JPanel infoLabelPanel;
  private JLabel mappingInfoLabel;

  // Logical map between tasks and nodes
  private boolean[][] logical_map;

  private JButton b_close      = createButton("Close");
  private JButton b_linear     = createButton("Linear");
  private JButton b_chunk      = createButton("Chunk");
  private JButton b_interleave = createButton("Interleave");

  private boolean showMap = true;

  // Método que crea la matriz con la distribución del MAPPING.
  private void createPanel()
  {
    logical_map = new boolean[nodes][tasks];

    for (int i = 0; i < nodes; i++)
      for (int j = 0; j < tasks; j++)
        logical_map[i][j] = false;

    if (tasks > 64)
    {
      /* Only shows maps window with little number of tasks */
      showMap = false;
      createReducedPanel();
    }
    else
    {
      oldCreatePanel();
    }

    return;
  }

  // Este método era el antiguo 'createPanel' pero lo he cambiado en dos partes
  // pero al no escalar, solo se invoca cuando hay 'pocas' tasks

  private void oldCreatePanel()
  {
    int n        = 0;
    node         = new JButton[nodes];
    group        = new ButtonGroup[tasks];
    map          = new JRadioButton[nodes][tasks];
    JPanel panel = new JPanel(new GridLayout(nodes+1,tasks+1));

    for(int i = group.length-1; i >= 0; i--)
    {
      group[i] = new ButtonGroup();
    }

    for(int i = 0; i < nodes+1; i++)
    {
      for(int j = 0; j < tasks+1; j++)
      {
        if((i == 0 && j == 0))
        {
          panel.add(new JLabel());
        }
        else if(i == 0)
        {
          panel.add(new JLabel("T-"+(j-1)));
        }
        else if(j == 0)
        {
          node[i-1] = new JButton(" N-"+(i-1)+" ");
          node[i-1].setBorder(null);
          node[i-1].addActionListener(this);
          panel.add(node[i-1]);
        }
        else
        {
          map[i-1][j-1] = new JRadioButton((String)null);
          group[j-1].add(map[i-1][j-1]);
          panel.add(map[i-1][j-1]);
        }
      }
    }

    InitializeMaps();
    
    scrollPanel = new  JScrollPane(panel,
                            ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                            ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);

    buttonPanel = new JPanel(new GridLayout(3,1));
    buttonPanel.add(b_linear);
    buttonPanel.add(b_chunk);
    buttonPanel.add(b_interleave);
  }

  private void createReducedPanel()
  {
    infoLabelPanel   = new JPanel();
    mappingInfoLabel = new JLabel();
    
    switch(mapInfo)
    {
      case Data.NO_MAP:
        mappingInfoLabel.setText("No map set");
        mappingInfoLabel.setForeground(Color.red);
        break;
      case Data.UNKNOW_MAP:
        mappingInfoLabel.setText("Unknow map distribution");
        break;
      case Data.LINEAR_MAP:
        mappingInfoLabel.setText("LINEAR map");
        break;
      case Data.CHUNK_MAP:
        mappingInfoLabel.setText("CHUNK map");
        break;
      case Data.INTERLEAVE_MAP:
        mappingInfoLabel.setText("INTERLEAVE map");
        break;
      default:
        mappingInfoLabel.setText("Uncoherent map distribution");
        mappingInfoLabel.setForeground(Color.red);
        break;
    }
    infoLabelPanel.add(mappingInfoLabel);
    
    InitializeMaps();
    
    buttonPanel = new JPanel(new GridLayout(1,3));
    buttonPanel.add(b_linear);
    buttonPanel.add(b_chunk);
    buttonPanel.add(b_interleave);

    return;
  }

  private void InitializeMaps()
  {
    if(stz.equalsIgnoreCase(""))
    {
      linear();
    }
    else
    {
      int number;
      int nTask  = 0;
      int first  = 0;
      int second = stz.indexOf(",",first);
  
      while(second != -1)
      {
        number = Integer.parseInt(Tools.blanks(stz.substring(first,second)));
  
        if((number < nodes) && (nTask < tasks))
        {
          // Updating all maps
          logical_map[number][nTask] = true;
  
          if (showMap)
            map[number][nTask].setSelected(true);

          nTask++;
          first = second + 1;
          second = stz.indexOf(",",first);
        }
        else
        {
          break;
        }
      }
  
      if(second == -1)
      {
        number = Integer.parseInt(Tools.blanks(stz.substring(first,stz.length())));
  
        if((number < nodes) && (nTask < tasks))
        {
          logical_map[number][nTask] = true;
          if (showMap)
            map[number][nTask].setSelected(true);
        }
      }
      stz = "";
    }
  }


  // Constructor de la clase MappingWindow.
  public MappingWindow(Data d)
  {
    super(d);
    
    Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();

    // Propiedades de la ventana.
    setTitle("Mapping information");
    setResizable(true);
    windowPanel.setLayout(new BorderLayout());

    // Anyadiendo los componentes a la ventana.

    
    nodes = data.processor.getNumberOfNodes();
    tasks = data.map.getTasks();
    createPanel();

    if (showMap)
    {
      // Show Mapping Area
      windowPanel.add(BorderLayout.CENTER,scrollPanel);
      windowPanel.add(BorderLayout.EAST,buttonPanel);
    }
    else
    {
      windowPanel.add(BorderLayout.NORTH, infoLabelPanel);
      windowPanel.add(BorderLayout.CENTER,buttonPanel);
    }
    windowPanel.add(BorderLayout.SOUTH,b_close);

    // Más propiedades de ventana.
    // System.out.println("Resolucion: "+screenSize.getWidth()+"x"+screenSize.getHeight());

    if (showMap)
    {
      setBounds(
       new Double(screenSize.getWidth()-screenSize.getWidth()*0.9).intValue(),
       new Double(screenSize.getHeight()-screenSize.getHeight()*0.9).intValue(),
       new Double(screenSize.getWidth()*0.8).intValue(),
       new Double(screenSize.getHeight()*0.8).intValue()
      );
    }
    else
    {
      pack();
    }

    setVisible(true);
  }

  // Método que realiza la distribución automática del MAPPING.
  private void linear()
  {
    int it = tasks;
    
    clearMaps();

    for( int pointer = 0; pointer < tasks; pointer++)
    {
      if (pointer >= nodes)
      {
        logical_map[nodes-1][pointer] = true;
        if (showMap)
          map[nodes-1][pointer].setSelected(true);
      }
      else
      {
        logical_map[pointer][pointer] = true;

        if (showMap)
          map[pointer][pointer].setSelected(true);
      }
    }
    
    mapInfo = Data.LINEAR_MAP;
    
    if (!showMap)
    {
      mappingInfoLabel.setText("LINEAR map");
      mappingInfoLabel.setForeground(Color.blue);
      mappingInfoLabel.repaint();
    }
  }

  // Método que realiza la distribución automática del MAPPING.
  private void chunk()
  {
    int chunkSize = (int) Math.round(Math.floor(tasks/nodes));
    int currentTask;
    int currentNode;
    int tasksMapped;
    
    clearMaps();
    
    for (currentTask = 0, tasksMapped = 0, currentNode = 0;
         currentTask < tasks;
         currentTask++)
    {
      logical_map[currentNode][currentTask] = true;
      
      if (showMap)
        map[currentNode][currentTask].setSelected(true);
      
      tasksMapped++;
      
      if ( tasksMapped == chunkSize)
      {
        tasksMapped = 0;
        currentNode++;
      }
    }

    /*
    for(int pointer = 0; pointer < logical_map.length; pointer++)
    {
      logical_map[pointer] = true;

      if (showMap)
        map[pointer].setSelected(true);

      q--;

      if(q == 0)
      {
        q = (int)tasks/nodes;
        pointer += tasks;
      }
    }

    for(int i = tasks%nodes; i > 0; i--)
    {
      logical_map[tasks*nodes-i] = true;

      if (showMap)
        map[tasks*nodes-i].setSelected(true);
    }
    */
    
    mapInfo = Data.CHUNK_MAP;
    if (!showMap)
    {
      mappingInfoLabel.setText("CHUNK map");
      mappingInfoLabel.setForeground(Color.blue);
      mappingInfoLabel.repaint();
    }
  }

  // Método que realiza la distribución automática del MAPPING.
  private void interleave()
  {
    int it = 0;
    int currentTask;
    
    clearMaps();
    
    for ( currentTask = 0; currentTask < tasks; currentTask++)
    {
      logical_map[currentTask%nodes][currentTask] = true;
      
      if (showMap)
        map[currentTask%nodes][currentTask].setSelected(true);
    }

    /*
    while(it < nodes)
    {
      for(int pointer = (tasks*it)+it; pointer < tasks*(it+1); pointer += nodes)
      {
        logical_map[pointer] = true;

        if (showMap)
          map[pointer].setSelected(true);
      }

      it++;
    }
    */
    
    mapInfo = Data.INTERLEAVE_MAP;
    if (!showMap)
    {
      mappingInfoLabel.setText("INTERLEAVE map");
      mappingInfoLabel.setForeground(Color.blue);
      mappingInfoLabel.repaint();
    }
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_linear)
    {
      linear();
    }
    else if(e.getSource() == b_chunk)
    {
      if(nodes >= tasks)
      {
        linear();
        
        if (!showMap)
          mappingInfoLabel.setText(mappingInfoLabel.getText()+
                                   "(CHUNK not applicable)");
        else
          Tools.showInformationMessage("CHUNK not applicable");
      }
      else
      {
        chunk();
      }
    }
    else if(e.getSource() == b_interleave)
    {
      interleave();
    }
    else if(e.getSource() == b_close)
    {
      generateMapString();
      
      try
      {
        data.map.setMap(stz);
        data.map.setMapInfo(mapInfo);
        dispose();
      } catch(Exception exc) {}
    }
    else
    {
      for(int n = 0; n < data.processor.getNumberOfNodes(); n++)
      {
        if(e.getSource() == node[n])
        {
          new NodeWindow(data,n);
          break;
        }
      }
    }
  }
  
  
  private void clearMaps()
  {
    for (int i = 0; i < nodes; i++)
      for (int j = 0; j < tasks; j++)
      {
        logical_map[i][j] = false;
        if (showMap)
          map[i][j].setSelected(false);
      }
  }
  
  private void generateMapString()
  {
    stz = new String();
    
    for(int j = 0; j < tasks; j++)
    {
      boolean nodeFound = false;
      for(int pointer = 0;
          pointer < nodes && !nodeFound;
          pointer++)
      {
        /* JGG: Now, the mapping is based on logical_map, not on (visual)map*/
        
        if(logical_map[pointer][j])
        {
          if(stz.equalsIgnoreCase(""))
          {
            stz = String.valueOf(pointer);
          }
          else
          {
            stz = stz + "," + (pointer);
          }
          /* JGG: Only one node per task! */
          // nodeFound = true;
        }
      }
    }
  }
}
