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

package es.bsc.cepbatools.dimemas.gui;

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import es.bsc.cepbatools.dimemas.data.*;
import es.bsc.cepbatools.dimemas.tools.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración de los nodos.
*/
public class NodeWindow extends GUIWindow
{
  public static final long serialVersionUID = 12L;

  private JButton b_left = createButton("<<<");
  private JButton b_right = createButton(">>>");
  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");
  private JButton b_same = createButton("Do all the same");

  private JTextField tf_number        = createTextField(2);
  private JTextField tf_machine_id    = createTextField(18);
  private JTextField tf_node_id       = new JTextField(18);
  private JTextField tf_architecture  = createTextField(18);
  private JTextField tf_processors    = new JTextField(18);
  private JTextField tf_cpu_ratio     = new JTextField(18);

  private JTextField tf_intra_node_startup   = new JTextField(18);
  private JTextField tf_intra_node_bandwidth = new JTextField(18);
  private JTextField tf_intra_node_buses     = new JTextField(18);
  private JTextField tf_intra_node_in_links  = new JTextField(18);
  private JTextField tf_intra_node_out_links = new JTextField(18);

  private JTextField tf_inter_node_startup   = new JTextField(18);
  private JTextField tf_inter_node_in_links  = new JTextField(18);
  private JTextField tf_inter_node_out_links = new JTextField(18);

  private JTextField tf_wan_startup       = new JTextField(18);
  
  //Accelerator node info
  private JCheckBox  cb_acc				  = new JCheckBox("Heterogenous node");
  private JTextField tf_acc				  = new JTextField(18);
  private JTextField tf_acc_startup	 	  = new JTextField(18);
  private JTextField tf_acc_mem_startup	  = new JTextField(18);
  private JTextField tf_acc_bandwidth     = new JTextField(18);
  private JTextField tf_acc_buses         = new JTextField(18);
  private JTextField tf_acc_ratio		  = new JTextField(18);
  
  /*
  * El método createTextField genera un campo de texto Swing que actuará como
  * contador o indicador del #elemento que se está visualizando en ese momento.
  *
  * @param: int num -> tamaño del texfield.
  *
  * @ret JTextField: Campo de texto creado.
  */
  private JTextField createTextField(int num)
  {
    JTextField tf = new JTextField(num);

    tf.setBorder(null);
    tf.setEditable(false);

    return tf;
  }

  /*
  * El método fillInformation actualiza todos los componentes de la pantalla
  * para que muestren la información correspondiente al elemento indicado por el
  * parámetro @index.
  *
  * @param: int index -> índice del elemento del que se obtiene la información.
  */
  private void fillInformation(int index)
  {
    // System.out.println("Node "+index+" has "+data.nodes_information.node[index].getProcessors()+" processors");
    
    tf_number.setText(String.valueOf(index+1));
    tf_machine_id.setText(data.nodes_information.node[index].getMachine_id());
    tf_node_id.setText(data.nodes_information.node[index].getNode_id());

    if(data.nodes_information.node[index].getArchitecture(false).equalsIgnoreCase(""))
    {
      tf_architecture.setText("Custom architecture");
    }
    else
    {
      tf_architecture.setText(data.nodes_information.node[index].getArchitecture(false));
    }

    tf_processors.setText(data.nodes_information.node[index].getProcessors());
    tf_cpu_ratio.setText(data.nodes_information.node[index].getCPURatio());

    tf_intra_node_startup.setText(data.nodes_information.node[index].getIntraNodeStartup());
    tf_intra_node_bandwidth.setText(data.nodes_information.node[index].getIntraNodeBandwidth());
    tf_intra_node_buses.setText(data.nodes_information.node[index].getIntraNodeBuses());
    tf_intra_node_in_links.setText(data.nodes_information.node[index].getIntraNodeInLinks());
    tf_intra_node_out_links.setText(data.nodes_information.node[index].getIntraNodeOutLinks());

    tf_inter_node_startup.setText(data.nodes_information.node[index].getInterNodeStartup());
    tf_inter_node_in_links.setText(data.nodes_information.node[index].getInterNodeInLinks());
    tf_inter_node_out_links.setText(data.nodes_information.node[index].getInterNodeOutLinks());

    tf_wan_startup.setText(data.nodes_information.node[index].getWANStartup());
    
    cb_acc.setSelected(data.nodes_information.node[index].getAcc());
    /* If nodes has accelerator information */
    if (data.nodes_information.node[index].getAcc())
    {
	    tf_acc_startup.setText(data.nodes_information.node[index].getAccStartup());
	    tf_acc_mem_startup.setText(data.nodes_information.node[index].getAccMemStartup());
	    tf_acc_bandwidth.setText(data.nodes_information.node[index].getAccBandwidth());
	    tf_acc_buses.setText(data.nodes_information.node[index].getAccBuses());
	    tf_acc_ratio.setText(data.nodes_information.node[index].getAccRatio());
    }
  }

  /*
  * El método dataOK comprueba algunos aspectos que deben cumplir los datos que
  * se muestran en ese momento en pantalla (no puede haber campos vacíos, el
  * valor 0 no esta permitido en algunos casos, etc).
  */
  private boolean dataOK()
  {
    if(tf_machine_id.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Machine identificator missing");
      return false;
    }
    else if(tf_node_id.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Node identificator missing");
      return false;
    }
    else if(tf_processors.getText().equalsIgnoreCase("") || tf_processors.getText().equalsIgnoreCase("0"))
    {
      Tools.showWarningMessage("Number of processors missing or equals to 0");
      return false;
    }
    else if(tf_cpu_ratio.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Relative processor speed (CPU ratio) missing");
      return false;
    }
    else if(tf_intra_node_startup.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Intra-node communications startup missing");
      return false;
    }
    else if(tf_intra_node_bandwidth.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Intra-node communications bandwidth missing");
      return false;
    }
    else if(tf_intra_node_in_links.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Intra-node communications input links missing");
      return false;
    }
    else if(tf_intra_node_out_links.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Intra-node communications output links missing");
      return false;
    }
    else if(tf_inter_node_startup.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Inter-node communications startup missing");
      return false;
    }
    else if(tf_inter_node_in_links.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Inter-node communications input links missing");
      return false;
    }
    else if(tf_inter_node_out_links.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Inter-node communications output links missing");
      return false;
    }
    else if(tf_wan_startup.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("Inter-machines (WAN) communications startup missing");
      return false;
    }
    
    /* If heterogenous node checkbox is enabled */
    if (cb_acc.isSelected())
    {
	    if(tf_acc_startup.getText().equalsIgnoreCase(""))
	    {
	      Tools.showWarningMessage("Accelerator startup missing");
	      return false;
	    }
	    else if(tf_acc_mem_startup.getText().equalsIgnoreCase(""))
	    {
	      Tools.showWarningMessage("Accelerator memory startup missing");
	      return false;
	    }
	    else if(tf_acc_bandwidth.getText().equalsIgnoreCase(""))
	    {
	      Tools.showWarningMessage("Accelerator bandwidth missing");
	      return false;
	    }
	    else if(tf_acc_buses.getText().equalsIgnoreCase(""))
	    {
	      Tools.showWarningMessage("Accelerator number of buses missing");
	      return false;
	    }
	    else if(tf_acc_ratio.getText().equalsIgnoreCase(""))
	    {
	      Tools.showWarningMessage("Accelerator speed ratio missing");
	      return false;
	    }
    }

    return true;
  }

  /*
  * El método storeInformation salva la información que aparece en pantalla en
  * la estructura de datos correspondiente.
  *
  * @param: · int index -> índice que indica el elemento al que pertenecen los
  *                        datos a guardar.
  *         · boolean reply -> TRUE si la información debe ser copiada a todos
  *                            los elementos (el usuario ha pulsado el botón "DO
  *                            ALL THE SAME"), FALSE en otro caso.
  *
  * @ret boolean: TRUE si la operación ha concluido con éxito, FALSE en otro caso.
  */
  private boolean storeInformation(int index, boolean reply)
  {
    if(!dataOK())
    {
      return false;
    }

    try
    {
      if(!reply)
      {
        data.nodes_information.node[index].setNode_id(tf_node_id.getText());
      }

      data.nodes_information.node[index].setProcessors(tf_processors.getText());
      data.nodes_information.node[index].setCPURatio(tf_cpu_ratio.getText());

      data.nodes_information.node[index].setIntraNodeStartup(tf_intra_node_startup.getText());
      data.nodes_information.node[index].setIntraNodeBandwidth(tf_intra_node_bandwidth.getText());
      data.nodes_information.node[index].setIntraNodeBuses(tf_intra_node_buses.getText());
      data.nodes_information.node[index].setIntraNodeInLinks(tf_intra_node_in_links.getText());
      data.nodes_information.node[index].setIntraNodeOutLinks(tf_intra_node_out_links.getText());

      data.nodes_information.node[index].setInterNodeStartup(tf_inter_node_startup.getText());
      data.nodes_information.node[index].setInterNodeInLinks(tf_inter_node_in_links.getText());
      data.nodes_information.node[index].setInterNodeOutLinks(tf_inter_node_out_links.getText());

      data.nodes_information.node[index].setWANStartup(tf_wan_startup.getText());
      
      data.nodes_information.node[index].setAcc(cb_acc.isSelected());
      /* If heterogenous node checkbox is enabled */
      if (cb_acc.isSelected())
      {
    	  data.nodes_information.node[index].setAccStartup(tf_acc_startup.getText());
    	  data.nodes_information.node[index].setAccMemStartup(tf_acc_mem_startup.getText());
    	  data.nodes_information.node[index].setAccBandwidth(tf_acc_bandwidth.getText());
    	  data.nodes_information.node[index].setAccBuses(tf_acc_buses.getText());
    	  data.nodes_information.node[index].setAccRatio(tf_acc_ratio.getText());
      }
      

    }
    catch(Exception exc)
    {
      return false;
    }
    
    return true;
  }

  // Constructor de la clase NodeWindow.
  public NodeWindow(Data d, int nodeNumber)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Node information");

    // Añadiendo información.
    fillInformation(nodeNumber);

    // drawline(new Component[] {labelFour});

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Node number"),b_left,tf_number,b_right});
    drawLine(new Component[] {new JLabel("Machine id"),tf_machine_id});
    drawLine(new Component[] {new JLabel("Simulated architecture"),tf_architecture});
    // drawLine(new Component[] {new JLabel("Node id"),tf_id});
    // drawLine(new Component[] {new JLabel("Number of processors"),tf_processors});
    // drawLine(new Component[] {new JLabel("Relative processor speed [%]"),tf_speed});

    drawProcessorPanel();
    drawIntraNodeCommsPanel();
    drawInterNodeCommsPanel();
    drawWANCommsPanel();
    
    drawAccPanel(nodeNumber);

    // drawLine(new Component[] {new JLabel("Startup [s]"),tf_local});
    // drawLine(new Component[] {new JLabel("Bandwidth [MByte/s]"),tf_memory});
    // drawLine(new Component[] {new JLabel("Number of memory links"),tf_mem_buses});
    // drawLine(new Component[] {new JLabel("Inter-nodes communications parameters")});
    // drawLine(new Component[] {new JLabel("Startup [s]"),tf_remote});
    // drawLine(new Component[] {new JLabel("Input links"),tf_input});
    // drawLine(new Component[] {new JLabel("Output links"),tf_output});
    // drawLine(new Component[] {new JLabel("WAN communications parameters")});
    // drawLine(new Component[] {new JLabel("External net latency [s]"),tf_latency});



    drawButtons(new Component[] {b_save,b_same,b_close},25,5);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+30,getHeight());
    pack();
    setVisible(true);
  }


  /*
   * Creates the processor parameters
   */
  private void drawProcessorPanel()
  {
    JPanel processorPanel = new JPanel();
    processorPanel.setLayout(new GridLayout(0,2,5,5));

    processorPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),
                                                              "Processor parameters"));

    processorPanel.add(new JLabel("Number of processors"));
    processorPanel.add(tf_processors);
    processorPanel.add(new JLabel("Relative processor speed [%]"));
    processorPanel.add(tf_cpu_ratio);

    super.addComponent(processorPanel);
  }

  /*
   * Creates the intra-node communication parameters panel
   */
  private void drawIntraNodeCommsPanel()
  {
    JPanel intraNodeCommsPanel = new JPanel();
    intraNodeCommsPanel.setLayout(new GridLayout(0,2,5,5));


    intraNodeCommsPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),
                                                                   "Intra-node communications parameters"));

    // grid.gridwidth = GridBagConstraints.BOTH;
    intraNodeCommsPanel.add(new JLabel("Startup [s]"));
    // grid.gridwidth = GridBagConstraints.REMAINDER;
    intraNodeCommsPanel.add(tf_intra_node_startup);

    // grid.gridwidth = GridBagConstraints.BOTH;
    intraNodeCommsPanel.add(new JLabel("Bandwidth [MByte/s]"));
    // grid.gridwidth = GridBagConstraints.REMAINDER;
    intraNodeCommsPanel.add(tf_intra_node_bandwidth);

    // grid.gridwidth = GridBagConstraints.BOTH;
    intraNodeCommsPanel.add(new JLabel("Buses"));
    // grid.gridwidth = GridBagConstraints.REMAINDER;
    intraNodeCommsPanel.add(tf_intra_node_buses);

    intraNodeCommsPanel.add(new JLabel("Input Links"));
    intraNodeCommsPanel.add(tf_intra_node_in_links);

    intraNodeCommsPanel.add(new JLabel("Output Links"));
    intraNodeCommsPanel.add(tf_intra_node_out_links);

    // drawLine(new Component[] {new JLabel("Intra-node communications parameters")});
    super.addComponent(intraNodeCommsPanel);

    return;
  }


  /*
   * Creates the inter-node communication parameters panel
   */
  private void drawInterNodeCommsPanel()
  {
    JPanel interNodeCommsPanel = new JPanel();
    interNodeCommsPanel.setLayout(new GridLayout(0,2,5,5));
    interNodeCommsPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),
                                                                   "Inter-node communications parameters"));


    interNodeCommsPanel.add(new JLabel("Startup [s]"));
    interNodeCommsPanel.add(tf_inter_node_startup);
    interNodeCommsPanel.add(new JLabel("Input links"));
    interNodeCommsPanel.add(tf_inter_node_in_links);
    interNodeCommsPanel.add(new JLabel("Output links"));
    interNodeCommsPanel.add(tf_inter_node_out_links);

    super.addComponent(interNodeCommsPanel);

    return;
  }

  /*
   * Creates the WAN communication parameters panel
   */
  private void drawWANCommsPanel()
  {
    JPanel WANCommsPanel = new JPanel();
    WANCommsPanel.setLayout(new GridLayout(0,2,5,5));
    WANCommsPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),
                                                                   "WAN communications parameters"));


    WANCommsPanel.add(new JLabel("Startup [s]"));
    WANCommsPanel.add(tf_wan_startup);

    super.addComponent(WANCommsPanel);

    return;
  }
  
  /*
   * Creates the Accelerator parameters panel
   */
  private void drawAccPanel(int nodeid)
  {
    JPanel AccPanel = new JPanel();
    AccPanel.setLayout(new GridLayout(6,2,5,5));
    AccPanel.setBorder(BorderFactory.createTitledBorder(BorderFactory.createEtchedBorder(),
                                                                   "Accelerator parameters"));
    /* Checkbox for enabling accelerator information */
    Boolean enable_acc = data.nodes_information.node[nodeid].getAcc();
    AccPanel.add(new JLabel("Heterogenous node"));
    cb_acc.setSelected(enable_acc);
    AccPanel.add(cb_acc);
    
    cb_acc.addChangeListener(new ChangeListener() {
		
		@Override
		public void stateChanged(ChangeEvent e) {
			// TODO Auto-generated method stub
			if (cb_acc.isSelected())
			{
				//Enables params
				tf_acc_startup.setEnabled(true);
				tf_acc_mem_startup.setEnabled(true);
				tf_acc_bandwidth.setEnabled(true);
				tf_acc_buses.setEnabled(true);
				tf_acc_ratio.setEnabled(true);
			}
			else if (!cb_acc.isSelected())
			{
				//Disable params
				tf_acc_startup.setEnabled(false);
				tf_acc_mem_startup.setEnabled(false);
				tf_acc_bandwidth.setEnabled(false);
				tf_acc_buses.setEnabled(false);
				tf_acc_ratio.setEnabled(false);
			}
		}
	});

    //Accelerator params enabled only if checkbox enabled
    AccPanel.add(new JLabel("Startup [s]"));
    tf_acc_startup.setEnabled(enable_acc);
    AccPanel.add(tf_acc_startup);
    
    AccPanel.add(new JLabel("Memory startup [s]"));
    tf_acc_mem_startup.setEnabled(enable_acc);
    AccPanel.add(tf_acc_mem_startup);
    
    AccPanel.add(new JLabel("Bandwidth [MByte/s]"));
    tf_acc_bandwidth.setEnabled(enable_acc);
    AccPanel.add(tf_acc_bandwidth);
    
    AccPanel.add(new JLabel("Buses"));
    tf_acc_buses.setEnabled(enable_acc);
    AccPanel.add(tf_acc_buses);
    
    AccPanel.add(new JLabel("Relative speed [%]"));
    tf_acc_ratio.setEnabled(enable_acc);
    AccPanel.add(tf_acc_ratio);
    
    super.addComponent(AccPanel);
    return;
  }


  /*
  * El método increase incrementa el valor del contador que indica qué nodo se
  * esta visualizando por pantalla. Si se sobrepasa el número de elementos
  * entonces se volverá al primer elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String increase(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == data.nodes_information.getNumberOfNodes())
    {
      return "1";
    }
    else
    {
      return String.valueOf(++aux);
    }
  }

  /*
  * El método decrease decrementa el valor del contador que indica qué nodo se
  * esta visualizando por pantalla. Si se sobrepasa el número de elementos
  * entonces se volverá al último elemento (cíclico).
  *
  * @param: String stz -> valor actual del contador.
  *
  * @ret String: Nuevo valor del contador.
  */
  private String decrease(String stz)
  {
    int aux = Integer.parseInt(stz);

    if(aux == 1)
    {
      return String.valueOf(data.nodes_information.getNumberOfNodes());
    }
    else
    {
      return String.valueOf(--aux);
    }
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    String mapNodeChangeResult;
    
    if(e.getSource() == b_left)
    {
      if(storeInformation(Integer.parseInt(tf_number.getText())-1, false))
      {
        if ( (mapNodeChangeResult = data.map.changeAtNodes()) != null)
        {
          Tools.showWarningMessage(mapNodeChangeResult+"\n"+
                  "Please check the \"Mapping information\" to verify your task mapping");
        }
        tf_number.setText(decrease(tf_number.getText()));
        fillInformation(Integer.parseInt(tf_number.getText())-1);
      }
    }
    if(e.getSource() == b_right)
    {
      if(storeInformation(Integer.parseInt(tf_number.getText())-1, false))
      {
        if ( (mapNodeChangeResult = data.map.changeAtNodes()) != null)
        {
          Tools.showWarningMessage(mapNodeChangeResult+"\n"+
                  "Please check the \"Mapping information\" to verify your task mapping");
        }
        
        tf_number.setText(increase(tf_number.getText()));
        fillInformation(Integer.parseInt(tf_number.getText())-1);
      }
    }
    else if(e.getSource() == b_save)
    {
      if(storeInformation(Integer.parseInt(tf_number.getText())-1, false))
      {
        if ( (mapNodeChangeResult = data.map.changeAtNodes()) != null)
        {
          Tools.showWarningMessage(mapNodeChangeResult+"\n"+
                  "Please check the \"Mapping information\" to verify your task mapping");
        }
        
        dispose();
      }
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
    else if(e.getSource() == b_same)
    {
      boolean wrongNodeData = false;
      for(int i = data.nodes_information.getNumberOfNodes()-1; i >= 0; i--)
      {
        if(!storeInformation(i,true))
        {
          wrongNodeData = true;
          break;
        }
      }
      if (!wrongNodeData)
      {
        if ( (mapNodeChangeResult = data.map.changeAtNodes()) != null)
        {
          Tools.showWarningMessage(mapNodeChangeResult+"\n"+
                  "Please check the \"Mapping information\" to verify your task mapping");
        }
      }
    }
  }
}
