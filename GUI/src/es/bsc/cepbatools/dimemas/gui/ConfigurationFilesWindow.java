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
import java.io.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana que permite elegir los ficheros de configuración
* extra.
*/
public class ConfigurationFilesWindow extends GUIWindow
{
  public static final long serialVersionUID = 5L;
  
  private JComboBox cb_select = createComboBox();

  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");
  private JButton b_edit = createButton("Edit");

  private JTextField tf_scheduler = new JTextField(25);
  private JTextField tf_fileSys = new JTextField(25);
  private JTextField tf_comm = new JTextField(18);
  private JTextField tf_sensitivity = new JTextField(25);

  /*
  * El método createComboBox genera un selector Swing que permitirá escoger los
  * ficheros de configuración extra.
  *
  * @ret JComboBox: Selector Swing creado.
  */
  private JComboBox createComboBox()
  {
    JComboBox cb = new JComboBox();

    cb.addItem("Browse file");
    cb.addItem("Scheduler");
    cb.addItem("File system");
    cb.addItem("Communication");
    cb.addItem("Sensitivity");
    cb.addActionListener(this);

    return cb;
  }

  // Constructor de la clase ConfigurationFilesWindow.
  public ConfigurationFilesWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Configuration files information");
    tf_scheduler.setText(data.config.getScheduler(false));
    tf_fileSys.setText(data.config.getFileSys(false));
    tf_comm.setText(data.config.getCommunication(false));
    tf_sensitivity.setText(data.config.getSensitivity(false));

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Scheduler"),tf_scheduler});
    drawLine(new Component[] {new JLabel("File system"),tf_fileSys});
    drawLine(new Component[] {new JLabel("Communication"),tf_comm,b_edit});
    drawLine(new Component[] {new JLabel("Sensitivity"),tf_sensitivity});
    drawButtons(new Component[] {b_save,cb_select,b_close},25,5);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+20,getHeight());
    pack();
    setVisible(true);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_save)
    {
      data.config.setScheduler(tf_scheduler.getText());
      data.config.setFileSys(tf_fileSys.getText());
      data.config.setCommunication(tf_comm.getText());
      data.config.setSensitivity(tf_sensitivity.getText());
      dispose();
    }
    else if(e.getSource() == cb_select)
    {
      if(cb_select.getSelectedIndex() == 3 && data.environment.getNumberOfMachines() == 0)
      {
        Tools.showInformationMessage("You must define the number of machines before load this data file.");
      }
      else if(cb_select.getSelectedIndex() != 0)
      {
        Tools.fc.addChoosableFileFilter(new Tools.CFGfilter());
        int result = Tools.fc.showOpenDialog(null);

        if(result == Tools.fc.APPROVE_OPTION)
        {
          if(cb_select.getSelectedIndex() == 1)
          {
            tf_scheduler.setText(Tools.fc.getSelectedFile().getAbsolutePath());
          }
          else if(cb_select.getSelectedIndex() == 2)
          {
            tf_fileSys.setText(Tools.fc.getSelectedFile().getAbsolutePath());
          }
          else if(cb_select.getSelectedIndex() == 3)
          {
            try
            {
              if(data.loadCommunicationData(Tools.fc.getSelectedFile().getAbsolutePath()))
              {
                tf_comm.setText(Tools.fc.getSelectedFile().getAbsolutePath());
              }
            } catch(Exception exc)
              {
                Tools.showInformationMessage(exc.toString());
              }
          }
          else if(cb_select.getSelectedIndex() == 4)
          {
            tf_sensitivity.setText(Tools.fc.getSelectedFile().getAbsolutePath());
          }
        }

        Tools.fc.resetChoosableFileFilters();
      }

      cb_select.setSelectedIndex(0);
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
    else if(e.getSource() == b_edit)
    {
      if(data.environment.getNumberOfMachines() == 0)
      {
        Tools.showInformationMessage("You must specify the number of machines before configure these properties.");
      }
      else
      {
        b_edit.setEnabled(false);
        b_close.setEnabled(false);
        b_save.setEnabled(false);
        cb_select.setEnabled(false);
        new CommunicationWindow(data,tf_comm).addWindowListener(
          new WindowAdapter()
          {
            public void windowClosed(WindowEvent we)
            {
              b_edit.setEnabled(true);
              b_close.setEnabled(true);
              b_save.setEnabled(true);
              cb_select.setEnabled(true);
            }
          }
        );
      }
    }
  }
}
