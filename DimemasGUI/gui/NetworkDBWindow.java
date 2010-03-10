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
import tools.Tools;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana que muestra las redes predefinidas existentes en la
* base de datos.
*/
public class NetworkDBWindow extends GUIWindow
{
  public static final long serialVersionUID = 16L;
  
  private JButton b_ok = createButton("OK");
  private JButton b_add = createButton("Add network");
  private JButton[] b_delete;
  private JButton[] b_modify;
  private boolean modify = false;

  // Método que crea un panel con los botones de la ventana.
  private JPanel createButtonPanel()
  {
    JPanel panel = new JPanel();

    panel.add(b_ok);
    panel.add(b_add);

    return panel;
  }

  // Método que crea el panel que contendrá la información de las redes
  // predefinidas.
  private JScrollPane createInfoPanel()
  {
    JPanel panel = new JPanel(new GridLayout(data.netDB.getNumberOfNetworksInDB()+1,6,10,1));

    b_delete = new JButton[data.netDB.getNumberOfNetworksInDB()];
    b_modify = new JButton[data.netDB.getNumberOfNetworksInDB()];
    panel.add(new JLabel(""));
    panel.add(new JLabel(""));
    panel.add(new JLabel("NAME",JLabel.CENTER));
    panel.add(new JLabel("IDENTIFIER",JLabel.CENTER));
    panel.add(new JLabel("BANDWIDTH [MByte/s]",JLabel.CENTER));
    panel.add(new JLabel("BUSES",JLabel.CENTER));

    for(int i = 0; i < data.netDB.getNumberOfNetworksInDB(); i++)
    {
      b_delete[i] = createButton("Delete");
      b_modify[i] = createButton("Modify");
      panel.add(b_delete[i]);
      panel.add(b_modify[i]);
      panel.add(new JLabel(data.netDB.net[i].getName(),JLabel.CENTER));
      panel.add(new JLabel(data.netDB.net[i].getLabel(),JLabel.CENTER));
      panel.add(new JLabel(data.netDB.net[i].getBandwidth(),JLabel.CENTER));
      panel.add(new JLabel(data.netDB.net[i].getBuses(),JLabel.CENTER));
    }

    return new JScrollPane(panel,ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
  }

  // Constructor de la clase NetworkDBWindow.
  public NetworkDBWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setBounds(25,150,450,175);
    setResizable(true);
    setTitle("Network database");
    windowPanel.setLayout(new BorderLayout());

    // Añadiendo los componentes a la ventana.
    windowPanel.add("Center",createInfoPanel());
    windowPanel.add("South",createButtonPanel());

    // Más propiedades de ventana.
    setVisible(true);
  }

  // Método que actualiza la información que aparece en pantalla (al borrar o
  // añadir un elemento).
  private void updateWindow()
  {
    windowPanel.removeAll();
    windowPanel.add("Center",createInfoPanel());
    windowPanel.add("South",createButtonPanel());
    setVisible(true);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_ok && !modify)
    {
      dispose();
    }
    else if(e.getSource() == b_add)
    {
      new NetworkDBAddWindow(data,-1).addWindowListener(
        new WindowAdapter()
        {
          public void windowClosed(WindowEvent we)
          {
            updateWindow();
          }
        }
      );
    }
    else if(!modify) // No se esta modificando ningún elemento ahora mismo.
    {
      for(int i = 0; i < data.netDB.getNumberOfNetworksInDB(); i++)
      {
        if(e.getSource() == b_delete[i])        // Borrar elemento.
        {
          if(Tools.showConfirmationMessage("Do you want to delete this item?"))
          {
            data.netDB.deleteElement(i);
            updateWindow();
          }

          break;
        }
        else if(e.getSource() == b_modify[i])   // Modificar elemento.
        {
          if(Tools.showConfirmationMessage("Do you want to modify this item?"))
          {
            modify = true;
            new NetworkDBAddWindow(data,i).addWindowListener(
              new WindowAdapter()
              {
                public void windowClosed(WindowEvent we)
                {
                  modify = false;
                  updateWindow();
                }
              }
            );
          }

          break;
        } // END if(modificar elemento)
      } // END for
    } // END if(no se esta modificando ningún elemento ahora mismo)
  }
}
