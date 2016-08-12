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
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

// Esta clase sirve de base para la mayoría de las ventanas del GUI aportando
// propiedades y métodos comunes a todas ellas.
abstract class GUIWindow extends JFrame implements ActionListener
{
  protected Data data;
  protected Container windowPanel   = getContentPane();
  protected GridBagLayout layout    = new GridBagLayout();
  protected GridBagConstraints grid = new GridBagConstraints();

  /*
  * El método createButton crea un botón Swing con el nombre @name y, además, se
  * le añade un ActionListener para poder controlar las acciones del usuario
  * sobre el botón.
  *
  * @param: String name -> etiqueta que aparecerá en el botón.
  *
  * @ret JButton: Botón Swing creado.
  */
  protected JButton createButton(String name)
  {
    JButton bt = new JButton(name);

    bt.addActionListener(this);

    return bt;
  }

  /*
  * El método addComponent permite añadir un componente al panel de la ventana.
  *
  * @param: Component comp -> componente a añadir.
  */
  protected void addComponent(Component comp)
  {
    /* JGG: Fijado el comportamiento de los componentes en caso que su tamaño
     * se más pequeño o más grande de la zona visible */
    grid.anchor = GridBagConstraints.CENTER;
    grid.fill   = GridBagConstraints.HORIZONTAL;
    layout.setConstraints(comp, grid);
    windowPanel.add(comp);
  }

  // Método que crea un panel con los botones requeridos.
  protected void drawButtons(Component[] buttons, int hGap, int vGap)
  {
    JPanel panel = new JPanel(new FlowLayout(FlowLayout.CENTER,hGap,vGap));

    for(int i = 0; i < buttons.length; i++)
    {
      panel.add(buttons[i]);
    }

    addComponent(panel);
  }

  // Método que crea un panel con los componentes facilitados.
  protected void drawLine(Component[] line)
  {

    int i;

    for(i = 0; i < line.length-1; i++)
    {
      grid.gridwidth = GridBagConstraints.BOTH;
      addComponent(line[i]);
    }

    grid.gridwidth = GridBagConstraints.REMAINDER;
    addComponent(line[i]);
  }

  // Constructor de la clase GUIWindow.
  public GUIWindow(Data d)
  {
    data = d;

    // Propiedades de ventana generales.

    setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getClassLoader().getResource(Data.ICON_IMAGE)));
    setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
    getRootPane().setBorder(BorderFactory.createRaisedBevelBorder());
    windowPanel.setLayout(layout);
    setResizable(true);
    grid.insets = new Insets(5,5,5,5);
    grid.weighty = 0.5;
    grid.weightx = 0.5;
  }

  // Todas las clases que deriven de la clase GUIWindow deberán incluir este
  // método.
  public void actionPerformed(ActionEvent e) {}
}
