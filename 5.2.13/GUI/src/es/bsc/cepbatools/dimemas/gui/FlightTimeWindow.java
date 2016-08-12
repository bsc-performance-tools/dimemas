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
* Esta clase crea la ventana de configuración correspondiente al FLIGHT TIME.
*/
public class FlightTimeWindow extends GUIWindow
{
  public static final long serialVersionUID = 10L;
  
  private JTextField[][] cells;

  private JScrollPane elements;
  private JPanel buttonPanel;

  private JButton b_close = createButton("Close");
  private JButton b_save = createButton("Save");
  private JButton b_sym = createButton("Symmetric");
  private JButton b_all = createButton("Do all the same");

  // Método que crea los paneles y sus componentes (matriz de FLIGHT TIME y
  // botones) de la ventana de configuración.
  private void createPanels()
  {
    int macs = data.environment.getNumberOfMachines();
    int m = 0;
    int n = 0;
    cells = new JTextField[macs][macs-1];
    JPanel panel = new JPanel(new GridLayout(macs+1,macs+1));

    buttonPanel = new JPanel(new FlowLayout());
    buttonPanel.add(b_save);
    buttonPanel.add(b_sym);
    buttonPanel.add(b_all);
    buttonPanel.add(b_close);

    for(int i = 0; i <= macs; i++)
    {
      for(int j = 0; j <= macs; j++)
      {
        if(i == j)
        {
          panel.add(new JLabel(""));
        }
        else if(i == 0)
        {
          panel.add(new JLabel("Source "+j,JLabel.CENTER));
        }
        else if(j == 0)
        {
          panel.add(new JLabel("Target "+i,JLabel.CENTER));
        }
        else
        {
          cells[m][n] = new JTextField(8);
          cells[m][n].setText("0.0");
          panel.add(cells[m][n]);
          n++;

          if(n == macs-1)
          {
            m++;
            n=0;
          }
        }
      }
    }

    elements = new JScrollPane(panel,ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
  }

  // Método que inicializa los componentes de la ventana para que muestre la
  // información correspondiente a FLIGHT TIME.
  private void fillInfo()
  {
    for(int i = 0; i < data.environment.getNumberOfMachines(); i++)
    {
      for(int j = 0; j < data.environment.getNumberOfMachines()-1; j++)
      {
        cells[i][j].setText(data.environment.ftGetValue(i,j));
      }
    }
  }

  // Constructor de la clase FlightTimeWindow.
  public FlightTimeWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("Flight time information");
    setResizable(true);
    windowPanel.setLayout(new BorderLayout());

    // Añadiendo los componentes a la ventana.
    createPanels();
    windowPanel.add(elements,BorderLayout.CENTER);
    windowPanel.add(buttonPanel,BorderLayout.SOUTH);

    // Añadiendo información.
    fillInfo();

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
    for(int i = 0; i < data.environment.getNumberOfMachines(); i++)
    {
      for(int j = 0; j < data.environment.getNumberOfMachines()-1; j++)
      {
        try
        {
          Double.parseDouble(cells[i][j].getText());
          cells[i][j].setText(Tools.filterForDouble(cells[i][j].getText()));
          data.environment.ftSetValue(i,j,Double.parseDouble(cells[i][j].getText()));
        } catch(NumberFormatException e)
          {
            Tools.showErrorMessage("FLIGHT TIME");
            return false;
          }
      }
    }

    return true;
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
      if(dataOK())
      {
        dispose();
      }
    }
    else if(e.getSource() == b_sym)
    {
      for(int i = 0; i < data.environment.getNumberOfMachines(); i++)
      {
        for(int j = i; j < data.environment.getNumberOfMachines()-1; j++)
        {
          cells[j+1][i].setText(cells[i][j].getText());
        }
      }
    }
    else if(e.getSource() == b_all)
    {
      for(int i = 0; i < data.environment.getNumberOfMachines(); i++)
      {
        for(int j = i; j < data.environment.getNumberOfMachines()-1; j++)
        {
          cells[i][j].setText(cells[0][0].getText());
        }
      }
    }
  }
}
