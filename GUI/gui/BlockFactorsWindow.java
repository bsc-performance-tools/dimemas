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
import java.util.ArrayList;

/*
* Esta clase crea la ventana de configuración de los Module Ratios.
*/
public class BlockFactorsWindow extends GUIWindow
{
  public static final long serialVersionUID = 2L;

  JButton b_add   = createButton("Add Module");
  JButton b_save  = createButton("Save");
  JButton b_close = createButton("Close");

  JPanel      buttonPanel;
  JScrollPane infoPanel;

  int DefinedModules = 0;

  ArrayList<JTextField> ModuleTypes   = new ArrayList<JTextField>();
  ArrayList<JTextField> ModuleValues  = new ArrayList<JTextField>();
  ArrayList<JTextField> ModuleRatios  = new ArrayList<JTextField>();
  ArrayList<JButton>    DeleteButtons = new ArrayList<JButton>();

  // Método que crea los paneles con los componentes correspondientes.
  private void createPanels()
  {
    // Panel de botones.
    buttonPanel = new JPanel(new FlowLayout());
    buttonPanel.add(b_add);
    buttonPanel.add(b_save);
    buttonPanel.add(b_close);

    // JPanel top = new JPanel(new GridLayout(data.block.getNumberOfBlocks(), 4));
    JPanel top = new JPanel(new GridLayout(0, 4));

    top.add(new JLabel("Event Type"));
    top.add(new JLabel("Event Value"));
    top.add(new JLabel("Ratio"));
    top.add(new JLabel(""));

    if (data.block.getNumberOfBlocks() != 0)
    {
      data.block.startIterator();

      while (data.block.hasNext() )
      {
        ModuleTypes.add(new JTextField(10));
        ModuleTypes.get(DefinedModules).setText(data.block.getType());
        top.add(ModuleTypes.get(DefinedModules));

        ModuleValues.add(new JTextField(4));
        ModuleValues.get(DefinedModules).setText(data.block.getValue());
        top.add(ModuleValues.get(DefinedModules));

        ModuleRatios.add(new JTextField(4));
        ModuleRatios.get(DefinedModules).setText(data.block.getRatio());
        top.add(ModuleRatios.get(DefinedModules));

        DeleteButtons.add(createButton("Delete"));
        top.add(DeleteButtons.get(DefinedModules));

        DefinedModules++;
      }
    }

    if (DefinedModules == 0)
    {
      ModuleTypes.add(new JTextField(10));
      top.add(ModuleTypes.get(DefinedModules));

      ModuleValues.add(new JTextField(4));
      top.add(ModuleValues.get(DefinedModules));

      ModuleRatios.add(new JTextField(4));
      top.add(ModuleRatios.get(DefinedModules));

      DeleteButtons.add(createButton("Delete"));
      top.add(DeleteButtons.get(DefinedModules));

      DefinedModules++;
    }

    /* JGG (2012/04/02) DEPRECATED!
    // Panel de información de BLOCK FACTORS (Nombre -- Valor).
    JPanel top = new JPanel(new GridLayout(data.block.getNumberOfBlocks(),2));
    blockValue = new JTextField[data.block.getNumberOfBlocks()];

    for(int i = 0; i < data.block.getNumberOfBlocks(); i++)
    {
      top.add(new JLabel(data.block.factors[i].getId() + " - " + data.block.factors[i].getName()));
      blockValue[i] = new JTextField(18);
      blockValue[i].setText(data.block.factors[i].getValue());
      top.add(blockValue[i]);
    }
    */

    infoPanel = new JScrollPane(top,JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
  }

  // Constructor de la clase BlockFactorsWindow.
  public BlockFactorsWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    windowPanel.setLayout(new BorderLayout());
    setTitle("Modules ratio window");

    // Añadiendo los componentes a la ventana.
    createPanels();
    windowPanel.add(infoPanel,BorderLayout.CENTER);
    windowPanel.add(buttonPanel,BorderLayout.SOUTH);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+25,500);
    pack();
    setVisible(true);
  }

  // Add new module information
  public void addNewBlock()
  {
    if (ModuleTypes.get(DefinedModules-1).getText().equals("") &&
        ModuleValues.get(DefinedModules-1).getText().equals("") &&
        ModuleRatios.get(DefinedModules-1).getText().equals(""))
    {
      return;
    }

    JPanel top = new JPanel(new GridLayout(0, 4));

    top.add(new JLabel("Event Type"));
    top.add(new JLabel("Event Value"));
    top.add(new JLabel("Ratio"));
    top.add(new JLabel(""));

    for (int i = 0; i  < DefinedModules; i++)
    {
      top.add(ModuleTypes.get(i));
      top.add(ModuleValues.get(i));
      top.add(ModuleRatios.get(i));
      top.add(DeleteButtons.get(i));
      DeleteButtons.get(i).setEnabled(true);
    }

    ModuleTypes.add(new JTextField(10));
    top.add(ModuleTypes.get(DefinedModules));

    ModuleValues.add(new JTextField(4));
    top.add(ModuleValues.get(DefinedModules));

    ModuleRatios.add(new JTextField(4));
    top.add(ModuleRatios.get(DefinedModules));

    DeleteButtons.add(createButton("Delete"));
    DeleteButtons.get(DefinedModules).setEnabled(false);
    top.add(DeleteButtons.get(DefinedModules));

    DefinedModules++;

    infoPanel.setViewportView(top);

    setBounds(25,150,getWidth()+25,500);
    pack();
  }

  public void deleteBlock(int Order)
  {

    ModuleTypes.remove(Order);
    ModuleValues.remove(Order);
    ModuleRatios.remove(Order);
    DeleteButtons.remove(Order);

    DefinedModules--;

    JPanel top = new JPanel(new GridLayout(0, 4));

    top.add(new JLabel("Event Type"));
    top.add(new JLabel("Event Value"));
    top.add(new JLabel("Ratio"));
    top.add(new JLabel(""));

    for (int i = 0; i  < DefinedModules; i++)
    {
      top.add(ModuleTypes.get(i));
      top.add(ModuleValues.get(i));
      top.add(ModuleRatios.get(i));
      top.add(DeleteButtons.get(i));
    }

    if (DefinedModules == 0)
    {
      ModuleTypes.add(new JTextField(10));
      top.add(ModuleTypes.get(DefinedModules));

      ModuleValues.add(new JTextField(4));
      top.add(ModuleValues.get(DefinedModules));

      ModuleRatios.add(new JTextField(4));
      top.add(ModuleRatios.get(DefinedModules));

      DeleteButtons.add(createButton("Delete"));
      top.add(DeleteButtons.get(DefinedModules));

      DefinedModules++;
    }

    infoPanel.setViewportView(top);

    setBounds(25,150,getWidth()+25,500);
    pack();
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_add)
    {
      addNewBlock();
    }
    if(e.getSource() == b_close)
    {
      dispose();
    }
    else if(e.getSource() == b_save)
    {
      try
      {
        data.block.destroyFactors();

        for (int i = 0; i < DefinedModules; i++)
        {
          if (ModuleTypes.get(i).getText().equals("")  ||
              ModuleValues.get(i).getText().equals("") ||
              ModuleRatios.get(i).getText().equals(""))
          {
            Tools.showErrorDialog("Incomplete module definition. It won't be saved");
          }
          else
          {
            data.block.addModuleRatio(ModuleTypes.get(i).getText(),
                                      ModuleValues.get(i).getText(),
                                      ModuleRatios.get(i).getText());
          }
        }

        dispose();
      } catch(Exception exc) {}
    }
    else
    {
      for (int i = 0; i  < DefinedModules; i++)
      {
        if (e.getSource() == DeleteButtons.get(i))
        {
          deleteBlock(i);
        }
      }
    }
  }
}
