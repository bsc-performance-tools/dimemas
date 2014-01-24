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
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import data.*;
import tools.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración de la red WAN.
*/
public class WideAreaNetworkWindow extends GUIWindow
{
  public static final long serialVersionUID = 17L;

  private JRadioButton commLin;
  private JRadioButton commLog;
  private JRadioButton commCt;
  private ButtonGroup commGroup = createGroup(true);

  private JRadioButton trafficExp;
  private JRadioButton trafficLin;
  private JRadioButton trafficLog;
  private JRadioButton trafficCt;
  private ButtonGroup trafficGroup = createGroup(false);


  //private JComboBox cb_architecture = createComboBox(true);
  private JComboBox cb_bandwidth = createComboBox(false);

  private JTextField tf_name = new JTextField(10);
  private JTextField tf_machines = new JTextField(21);
  private JTextField tf_connections = new JTextField(21);
  private JTextField tf_max = new JTextField(21);
  private JTextField tf_bandwidth = new JTextField(10);

  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");

  private JComboBox createComboBox(boolean architecture)
  {
    JComboBox cb = new JComboBox();

    cb.addItem("Edit");

    if(architecture)
    {
      /*
      for(int i = 0; i < data.netDB.getNumberOfNetworksInDB(); i++)
      {
        cb.addItem(data.netDB.net[i].getName());
      }
      */
    }
    else
    {
      cb.addItem("1");
      cb.addItem("0.1");
      cb.addItem("0.01");
      cb.addItem("0.001");
      cb.addItem("0.0001");
    }

    cb.addActionListener(this);

    return cb;
  }

  // Método que agrupa las opciones para que solo se pueda escoger una a la vez.
  private ButtonGroup createGroup(boolean communicationGroup)
  {
    ButtonGroup group = new ButtonGroup();

    if(communicationGroup)
    {
      commLog = new JRadioButton("LOG");
      commLin = new JRadioButton("LIN");
      commCt = new JRadioButton("CT");
      group.add(commLog);
      group.add(commLin);
      group.add(commCt);
    }
    else
    {
      trafficExp = new JRadioButton("EXP");
      trafficLog = new JRadioButton("LOG");
      trafficLin = new JRadioButton("LIN");
      trafficCt = new JRadioButton("CT");
      group.add(trafficExp);
      group.add(trafficLog);
      group.add(trafficLin);
      group.add(trafficCt);
    }

    return group;
  }

  // Constructor de la clase WideAreaNetworkWindow.
  public WideAreaNetworkWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Wide area network information");

    // Añadiendo información..
    tf_name.setText(data.wan.getName(false));

    /*
    for(int i = data.netDB.getNumberOfNetworksInDB()-1; i >= 0; i--)
    {
      if(data.netDB.net[i].getLabel().equalsIgnoreCase(tf_name.getText()))
      {
        cb_architecture.setSelectedIndex(i+1);
      }
    }
    */

    tf_machines.setText(data.wan.getMachines());
    tf_connections.setText(data.wan.getDedicated());
    tf_max.setText(data.wan.getMax());
    tf_bandwidth.setText(data.wan.getBandwidth());

    switch(Integer.parseInt(data.wan.getCommunication()))
    {
      case 1: commCt.setSelected(true);
              break;
      case 2: commLin.setSelected(true);
              break;
      case 3: commLog.setSelected(true);
              break;
    }

    switch(Integer.parseInt(data.wan.getTraffic()))
    {
      case 1: trafficExp.setSelected(true);
              break;
      case 2: trafficLog.setSelected(true);
              break;
      case 3: trafficLin.setSelected(true);
              break;
      case 4: trafficCt.setSelected(true);
              break;
    }

    // Añadiendo los componentes a la ventana.
    // drawLine(new Component[] {new JLabel("Wide Area Network name"),cb_architecture,tf_name});
    drawLine(new Component[] {new JLabel("Number of machines"),tf_machines});
    drawLine(new Component[] {new JLabel("Number of dedicated connections"),tf_connections});
    drawLine(new Component[] {new JLabel("Function of traffic"),trafficExp,trafficLog,trafficLin,trafficCt});
    drawLine(new Component[] {new JLabel("Max traffic value"),tf_max});
    drawLine(new Component[] {new JLabel("External net bandwidth [MByte/s]"),cb_bandwidth,tf_bandwidth});
    drawLine(new Component[] {new JLabel("Communication group model"),commLog,commLin,commCt});
    drawButtons(new Component[] {b_save,b_close},70,5);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+25,getHeight());
    pack();
    setVisible(true);
  }

  /*
  * El método dataOK comprueba algunos aspectos que deben cumplir los datos que
  * se muestran en ese momento en pantalla (no puede haber campos vacíos, el
  * valor 0 no esta permitido en algunos casos, etc).
  */
  private boolean dataOK()
  {
    int mac;
    int con;

    try
    {
      mac = Integer.parseInt(tf_machines.getText());
    } catch(Exception e)
      {
        Tools.showErrorMessage("MACHINES");
        return false;
      }

    try
    {
      con = Integer.parseInt(tf_connections.getText());
    } catch(Exception e)
      {
        Tools.showErrorMessage("DEDICATED CONNECTIONS");
        return false;
      }

    if(con >= 1 && mac <= 1)
    {
      Tools.showInformationMessage("At least two machines are necessary to create dedicated connecions.");
      return false;
    }
    if(tf_machines.getText().equalsIgnoreCase("") ||
       tf_machines.getText().equalsIgnoreCase("0"))
    {
      Tools.showWarningMessage("NUMBER OF MACHINES");
      return false;
    }
    else if(tf_connections.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("DEDICATED CONNECTIONS");
      return false;
    }
    else if(tf_max.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("MAX TRAFFIC VALUE");
      return false;
    }
    else if(tf_bandwidth.getText().equalsIgnoreCase(""))
    {
      Tools.showWarningMessage("EXTERNAL BANDWIDTH");
      return false;
    }
    else
    {
      return true;
    }
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_save)
    {
      if(dataOK())
      {
        try
        {
          data.wan.setDedicated(tf_connections.getText());
          data.wan.setMachines(tf_machines.getText());
          data.wan.setName(tf_name.getText());
          data.wan.setMax(tf_max.getText());
          data.wan.setBandwidth(tf_bandwidth.getText());

          if(trafficExp.isSelected())
          {
            data.wan.setTraffic(data.wan.TRAFFIC_FUNCTION_EXP);
          }
          else if(trafficLog.isSelected())
          {
            data.wan.setTraffic(data.wan.TRAFFIC_FUNCTION_LOG);
          }
          else if(trafficLin.isSelected())
          {
            data.wan.setTraffic(data.wan.TRAFFIC_FUNCTION_LIN);
          }
          else if(trafficCt.isSelected())
          {
            data.wan.setTraffic(data.wan.TRAFFIC_FUNCTION_CT);
          }

          if(commLog.isSelected())
          {
            data.wan.setCommunication(data.wan.COMM_GROUP_LOG);
          }
          else if(commLin.isSelected())
          {
            data.wan.setCommunication(data.wan.COMM_GROUP_LIN);
          }
          else if(commCt.isSelected())
          {
            data.wan.setCommunication(data.wan.COMM_GROUP_CT);
          }

          int storedConnections = data.dedicated.getNumberOfConnections();
          int actualConnections = Integer.parseInt(tf_connections.getText());

          if(actualConnections != storedConnections)
          {
            data.dedicated.setNumberOfConnections(actualConnections);
            data.dedicated.changeAtConnections();
          }

          int storedMachines = data.environment.getNumberOfMachines();
          int actualMachines = Integer.parseInt(tf_machines.getText());

          if(actualMachines != storedMachines)
          {
            data.environment.setNumberOfMachines(actualMachines);
            data.environment.changeAtMachines();
          }

          /*
          if(cb_architecture.getSelectedIndex() == 0)
          {
          */
            for(int i = data.environment.getNumberOfMachines()-1; i >= 0; i--)
            {
              data.environment.machine[i].setArchitecture(data.environment.DEFAULT_ARCHITECTURE);
              data.environment.machine[i].setBuses(data.environment.DEFAULT_BUSES);
            }
          /*
          }
          else if(cb_architecture.getSelectedIndex() != 0)
          {
            for(int i = data.environment.getNumberOfMachines()-1; i >= 0; i--)
            {
              data.environment.machine[i].setArchitecture
                (data.netDB.net[cb_architecture.getSelectedIndex()-1].getName());
              data.environment.machine[i].setBuses
                (data.netDB.net[cb_architecture.getSelectedIndex()-1].getBuses());
            }
          }
          */

          dispose();
        } catch(Exception exc) {}
      }
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
    /*else if(e.getSource() == cb_architecture)
    {
      if(cb_architecture.getSelectedIndex() != 0)
      {
        tf_name.setEditable(false);
        tf_name.setText(data.netDB.net[cb_architecture.getSelectedIndex()-1].getLabel());
        tf_bandwidth.setText(data.netDB.net[cb_architecture.getSelectedIndex()-1].getBandwidth());
      }
      else
      {
        tf_name.setEditable(true);
        tf_name.setText(data.wan.DEFAULT_NAME);
        tf_bandwidth.setText(data.wan.DEFAULT_BANDWIDTH);
      }
    }
    */
    else if(e.getSource() == cb_bandwidth)
    {
      if(cb_bandwidth.getSelectedIndex() != 0)
      {
        tf_bandwidth.setText((String)cb_bandwidth.getSelectedItem());
      }
      else
      {
        tf_bandwidth.setText(data.wan.DEFAULT_BANDWIDTH);
      }
    }
  }
}
