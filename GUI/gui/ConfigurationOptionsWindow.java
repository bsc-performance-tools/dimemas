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
 * Título:      dimemas
 * Descripcion:
 * Copyright:   Copyright (c) 2001
 * Empresa:
 * @author      Óscar Bardillo Luján
 * @version     1.0
 */

import data.*;
import tools.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana con el listado de módulos configurables de una
* arquitectura a simular con Dimemas.
*/
public class ConfigurationOptionsWindow extends GUIWindow
{
  public static final long serialVersionUID = 6L;

  private final String CONFIG_BUTTON_TEXT = "Config!";
  private final String STATUS_OPEN        = "Open";
  private final String STATUS_DEFAULT     = "Default";
  private final String STATUS_MODIFIED    = "Altered";

  /* JGG: tamaño de los JTextFields para que se visualicen correctamente */
  private final int   TXTFIELD_COLS      = 10;

  private JButton b_wan          = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_dedicated    = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_environment  = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_node         = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_mapping      = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_config_files = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_file_sys     = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_block        = createButton(CONFIG_BUTTON_TEXT);
  private JButton b_close        = createButton("Close window");

  private JLabel lbl_wan          = createLabel();
  private JLabel lbl_dedicated    = createLabel();
  private JLabel lbl_environment  = createLabel();
  private JLabel lbl_node         = createLabel();
  private JLabel lbl_mapping      = createLabel();
  private JLabel lbl_config_files = createLabel();
  private JLabel lbl_file_sys     = createLabel();
  private JLabel lbl_block        = createLabel();

  // Método que refresca la pantalla indicando qué módulos han sido configurados.
  // DEFAULT -> Valores por defecto/no configurado
  // MODIFIED -> módulo configurado
  public void refresh()
  {
    if(b_wan.isEnabled())
    {
      if(!data.wan.defaultValues())
      {
        lbl_wan.setText(STATUS_MODIFIED);
        lbl_wan.setForeground(Color.blue);
      }
      else
      {
        lbl_wan.setText(STATUS_DEFAULT);
      }
    }

    if(b_dedicated.isEnabled())
    {
      if(!data.dedicated.defaultValues())
      {
        lbl_dedicated.setText(STATUS_MODIFIED);
        lbl_dedicated.setForeground(Color.blue);
      }
      else
      {
        lbl_dedicated.setText(STATUS_DEFAULT);
      }
    }

    if(b_environment.isEnabled())
    {
      if(!data.environment.defaultValues())
      {
        lbl_environment.setText(STATUS_MODIFIED);
        lbl_environment.setForeground(Color.blue);
      }
      else
      {
        lbl_environment.setText(STATUS_DEFAULT);
      }
    }

    if(b_node.isEnabled())
    {
      if(!data.nodes_information.defaultValues())
      {
        lbl_node.setText(STATUS_MODIFIED);
        lbl_node.setForeground(Color.blue);
      }
      else
      {
        lbl_node.setText(STATUS_DEFAULT);
      }
    }

    if(b_mapping.isEnabled())
    {
      if(!data.map.defaultValues())
      {
        lbl_mapping.setText(STATUS_MODIFIED);
        lbl_mapping.setForeground(Color.blue);
      }
      else
      {
        lbl_mapping.setText(STATUS_DEFAULT);
      }
    }

    if(b_config_files.isEnabled())
    {
      if(!data.config.defaultValues())
      {
        lbl_config_files.setText(STATUS_MODIFIED);
        lbl_config_files.setForeground(Color.blue);
      }
      else
      {
        lbl_config_files.setText(STATUS_DEFAULT);
      }
    }

    if(b_file_sys.isEnabled())
    {
      if(!data.fileSys.defaultValues())
      {
        lbl_file_sys.setText(STATUS_MODIFIED);
        lbl_file_sys.setForeground(Color.blue);
      }
      else
      {
        lbl_file_sys.setText(STATUS_DEFAULT);
      }
    }

    if(b_block.isEnabled())
    {
      if(!data.block.defaultValues())
      {
        lbl_block.setText(STATUS_MODIFIED);
        lbl_block.setForeground(Color.blue);
      }
      else
      {
        lbl_block.setText(STATUS_DEFAULT);
      }
    }
  }

  // Método que crea los campos de texto que indican el estado (configurado o
  // no) de un módulo determinado.
  private JLabel createLabel()
  {
    /* JGG: La inicilización fija el tamaño en columnas, para la correcta
     * visualización */
    JLabel lbl = new JLabel(STATUS_DEFAULT, JLabel.LEFT);

    lbl.setBorder(null);
    // lbl.setEditable(false);

    return lbl;
  }

  // Constructor de la clase ConfigurationOptionsWindow.
  public ConfigurationOptionsWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Configuration window");

    // Añadiendo los componentes a la ventana.
    refresh();
    drawLine(new Component[] {b_wan,new JLabel("WAN information"),lbl_wan});
    drawLine(new Component[] {b_dedicated,new JLabel("Dedicated connections"),lbl_dedicated});
    drawLine(new Component[] {b_environment,new JLabel("Environment information"),lbl_environment});
    drawLine(new Component[] {b_node,new JLabel("Node information"),lbl_node});
    drawLine(new Component[] {b_mapping,new JLabel("Mapping information"),lbl_mapping});
    drawLine(new Component[] {b_config_files,new JLabel("Config files"),lbl_config_files});
    drawLine(new Component[] {b_file_sys,new JLabel("File system parameters"),lbl_file_sys});
    drawLine(new Component[] {b_block,new JLabel("Block factors"),lbl_block});
    drawButtons(new Component[] {b_close},5,5);

    // Más propiedades de ventana.

    setBounds(500,150,getWidth()+35,getHeight());
    pack();
    setVisible(true);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_wan)
    {
      new WideAreaNetworkWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowOpened(WindowEvent we)
          {
            b_wan.setEnabled(false);
            lbl_wan.setText(STATUS_OPEN);
            lbl_wan.setForeground(Color.red);
          }

          public void windowClosed(WindowEvent we)
          {
            b_wan.setEnabled(true);
            lbl_wan.setForeground(Color.black);

            if(data.wan.defaultValues())
            {
              lbl_wan.setText(STATUS_DEFAULT);
            }
            else
            {
              lbl_wan.setText(STATUS_MODIFIED);
              lbl_wan.setForeground(Color.blue);
            }
          }
        }
      );
    }
    else if(e.getSource() == b_dedicated)
    {
      if(data.wan.getDedicated().equalsIgnoreCase("0"))
      {
        Tools.showInformationMessage("You must specify the number of dedicated connections before configure them.");
      }
      else
      {
        new DedicatedConnectionWindow(data).addWindowListener(
          new WindowAdapter()
          {
            public void windowOpened(WindowEvent we)
            {
              b_dedicated.setEnabled(false);
              lbl_dedicated.setText(STATUS_OPEN);
              lbl_dedicated.setForeground(Color.red);
            }

            public void windowClosed(WindowEvent we)
            {
              b_dedicated.setEnabled(true);
              lbl_dedicated.setForeground(Color.black);

              if(data.dedicated.defaultValues())
              {
                lbl_dedicated.setText(STATUS_DEFAULT);
              }
              else
              {
                lbl_dedicated.setText(STATUS_MODIFIED);
                lbl_dedicated.setForeground(Color.blue);
              }
            }
          }
        );
      }
    }
    else if(e.getSource() == b_environment)
    {

      if(data.environment.getNumberOfMachines() == 0)
      {
        Tools.showInformationMessage("No machines defined. Using a single machine environment with default values\n"+
                                     "Check the parameters in 'Environment Information' window");
        data.wan.initialValues();
        data.environment.setNumberOfMachines(1);
        try
        {
          data.environment.changeAtMachines();
        }
        catch (Exception exc)
        {
          Tools.showErrorMessage("Error creating default machine");
          return;
        }
      }



      new EnvironmentWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowOpened(WindowEvent we)
          {
            b_environment.setEnabled(false);
            lbl_environment.setText(STATUS_OPEN);
            lbl_environment.setForeground(Color.red);
          }

          public void windowClosed(WindowEvent we)
          {
            b_environment.setEnabled(true);
            lbl_environment.setForeground(Color.black);

            if(data.environment.defaultValues())
            {
              lbl_environment.setText(STATUS_DEFAULT);
            }
            else
            {
              lbl_environment.setText(STATUS_MODIFIED);
              lbl_environment.setForeground(Color.blue);
            }
          }
        }
      );
    }
    else if(e.getSource() == b_node)
    {
      if(data.nodes_information.getNumberOfNodes() == 0)
      {
        Tools.showInformationMessage("You must specify the number of nodes before configure them.");
      }
      else
      {
        new NodeWindow(data,0).addWindowListener(
          new WindowAdapter()
          {
            public void windowOpened(WindowEvent we)
            {
              b_node.setEnabled(false);
              lbl_node.setText(STATUS_OPEN);
              lbl_node.setForeground(Color.red);
            }

            public void windowClosed(WindowEvent we)
            {
              b_node.setEnabled(true);
              lbl_node.setForeground(Color.black);

              if(data.nodes_information.defaultValues())
              {
                lbl_node.setText(STATUS_DEFAULT);
              }
              else
              {
                lbl_node.setText(STATUS_MODIFIED);
                lbl_node.setForeground(Color.blue);
              }
            }
          }
        );
      }
    }
    else if(e.getSource() == b_mapping)
    {
      if(data.nodes_information.getNumberOfNodes() == 0)
      {
        Tools.showInformationMessage("You must specify the number of nodes before configure task-mapping.");
      }
      else if(data.map.getTasks() == 0)
      {
        Tools.showInformationMessage("You must specify the number of tasks before configure task-mapping.");
      }
      else
      {
        new MappingWindow(data).addWindowListener(
          new WindowAdapter()
          {
            public void windowOpened(WindowEvent we)
            {
              b_mapping.setEnabled(false);
              lbl_mapping.setText(STATUS_OPEN);
              lbl_mapping.setForeground(Color.red);
            }

            public void windowClosed(WindowEvent we)
            {
              b_mapping.setEnabled(true);
              lbl_mapping.setForeground(Color.black);

              if(data.map.defaultValues())
              {
                lbl_mapping.setText(STATUS_DEFAULT);
              }
              else
              {
                lbl_mapping.setText(STATUS_MODIFIED);
                lbl_mapping.setForeground(Color.blue);
              }
            }
          }
        );
      }
    }
    else if(e.getSource() == b_config_files)
    {
      new ConfigurationFilesWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowOpened(WindowEvent we)
          {
            b_config_files.setEnabled(false);
            lbl_config_files.setText(STATUS_OPEN);
            lbl_config_files.setForeground(Color.red);
          }

          public void windowClosed(WindowEvent we)
          {
            b_config_files.setEnabled(true);
            lbl_config_files.setForeground(Color.black);

            if(data.config.defaultValues())
            {
              lbl_config_files.setText(STATUS_DEFAULT);
            }
            else
            {
              lbl_config_files.setText(STATUS_MODIFIED);
              lbl_config_files.setForeground(Color.blue);
            }
          }
        }
      );
    }
    else if(e.getSource() == b_file_sys)
    {
      new FileSystemWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowOpened(WindowEvent we)
          {
            b_file_sys.setEnabled(false);
            lbl_file_sys.setText(STATUS_OPEN);
            lbl_file_sys.setForeground(Color.red);
          }

          public void windowClosed(WindowEvent we)
          {
            b_file_sys.setEnabled(true);
            lbl_file_sys.setForeground(Color.black);

            if(data.fileSys.defaultValues())
            {
              lbl_file_sys.setText(STATUS_DEFAULT);
            }
            else
            {
              lbl_file_sys.setText(STATUS_MODIFIED);
              lbl_file_sys.setForeground(Color.blue);
            }
          }
        }
      );
    }
    else if(e.getSource() == b_block)
    {
      new BlockFactorsWindow(data).addWindowListener(
        new WindowAdapter()
        {
          public void windowOpened(WindowEvent we)
          {
            b_block.setEnabled(false);
            lbl_block.setText(STATUS_OPEN);
            lbl_block.setForeground(Color.red);
          }

          public void windowClosed(WindowEvent we)
          {
            b_block.setEnabled(true);
            lbl_block.setForeground(Color.black);

            if(data.block.defaultValues())
            {
              lbl_block.setText(STATUS_DEFAULT);
            }
            else
            {
              lbl_block.setText(STATUS_MODIFIED);
              lbl_block.setForeground(Color.blue);
            }
          }
        }
      );
    }
    else if(e.getSource() == b_close)
    {
      if(!b_wan.isEnabled() || !b_dedicated.isEnabled() ||
         !b_environment.isEnabled() || !b_node.isEnabled() ||
         !b_mapping.isEnabled() || !b_config_files.isEnabled() ||
         !b_file_sys.isEnabled() || !b_block.isEnabled())
      {
        Tools.showInformationMessage("You should close all the windows opened to configure the Target Machine before exit.");
      }
      else
      {
        dispose();
      }
    }
  }
}
