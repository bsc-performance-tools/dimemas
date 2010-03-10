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
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de configuración de los BLOCK FACTORS.
*/
public class BlockFactorsWindow extends GUIWindow
{
  public static final long serialVersionUID = 2L;
  
  JButton b_save = createButton("Save");
  JButton b_close = createButton("Close");

  JPanel buttonPanel;
  JScrollPane infoPanel;
  JTextField[] blockValue;

  // Método que crea los paneles con los componentes correspondientes.
  private void createPanels()
  {
    // Panel de botones.
    buttonPanel = new JPanel(new FlowLayout());
    buttonPanel.add(b_save);
    buttonPanel.add(b_close);

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

    infoPanel = new JScrollPane(top,JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
  }

  // Constructor de la clase BlockFactorsWindow.
  public BlockFactorsWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    windowPanel.setLayout(new BorderLayout());
    setTitle("Block factors window");

    // Añadiendo los componentes a la ventana.
    createPanels();
    windowPanel.add(infoPanel,BorderLayout.CENTER);
    windowPanel.add(buttonPanel,BorderLayout.SOUTH);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+25,500);
    pack();
    setVisible(true);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_close)
    {
      dispose();
    }
    else if(e.getSource() == b_save)
    {
      try
      {
        for(int i = 0; i < data.block.getNumberOfBlocks(); i++)
        {
          data.block.factors[i].setValue(blockValue[i].getText());
        }

        dispose();
      } catch(Exception exc) {}
    }
  }
}
