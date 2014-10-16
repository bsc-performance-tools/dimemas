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

import data.Data;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/*
* Esta clase crea la ventana de información de Dimemas.
*/
public class AboutWindow extends JFrame implements ActionListener
{
  public static final long serialVersionUID = 1L;

  private JButton b_close = createButton("Close");
  private Container panelPrincipal = getContentPane();

  /*
  * El método createButton genera un botón Swing con la etiqueta facilitada
  * @name y, además, añade al botón creado un ActionListener para poder capturar
  * las acciones del usuario sobre ese botón.
  *
  * @param: String name -> etiqueta que aperecerá en el botón.
  *
  * @ret JButton: Botón Swing con ActionListener.
  */
  private JButton createButton(String name)
  {
    JButton bt = new JButton(name);

    bt.addActionListener(this);

    return bt;
  }

  /*
  * El método aboutPanel crea un panel que contendrá la información referente a
  * Dimemas.
  *
  * @ret JPanel: Panel Swing con el texto referente a la información sobre
  *              Dimemas.
  */
  private JPanel aboutPanel()
  {
    JPanel panel = new JPanel(new GridLayout(7,1));

    // panel.setBorder(BorderFactory.createEtchedBorder(1));

    panel.add(new JLabel(""));

    panel.add(
      new JLabel(
        "DIMEMAS is a performance analysis tool for message passing",
        JLabel.CENTER));

    panel.add(
      new JLabel(
        "programs.  It is designed,  developed  and  maintained  by  the",
        JLabel.CENTER));

    panel.add(
      new JLabel(
        "Barcelona Supercomputing Center - Centro Nacional de Supercomputación",
        JLabel.CENTER));

    panel.add(new JLabel(""));

    panel.add(new JLabel("Contact:  tools@bsc.es",JLabel.CENTER));

    panel.add(new JLabel(""));

    return panel;
  }

  // Constructor de la clase AboutWindow.
  public AboutWindow()
  {
    // Propiedades de la ventana.
    setTitle("About DIMEMAS");
    setResizable(false);
    setLocation(200,200);
    setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getClassLoader().getResource(Data.BSC_LOGO)));
    panelPrincipal.setLayout(new BorderLayout());

    // Se añaden los elementos de la ventana.
    panelPrincipal.add("North",new JLabel(new ImageIcon(getClass().getClassLoader().getResource(Data.BSC_LOGO))));
    panelPrincipal.add("Center",aboutPanel());
    panelPrincipal.add("South",b_close);

    // Más propiedades de ventana.
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
  }
}
