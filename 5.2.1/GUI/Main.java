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

/**
 * Título:       dimemas
 * Descripcion:
 * Copyright:    Copyright (c) 2001
 * Empresa:
 * @author Óscar Bardillo Luján
 * @version 1.0
 */

import gui.*; 
import data.Data;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class Main extends JFrame
{
  public static final long serialVersionUID = 1000L;

  JTextField currentConfigurationFile = new JTextField("");
  Data data                           = new Data(currentConfigurationFile);
  OptionsMenu menu                    = new OptionsMenu(data);
  Container panelPrincipal            = getContentPane();

  public Main()
  {
    super("DIMEMAS");

    setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getClassLoader().getResource((Data.ICON_IMAGE))));          
    setResizable(false);
    setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

    setJMenuBar(menu);
    panelPrincipal.setLayout(new BorderLayout());
    panelPrincipal.add("West",new JLabel("  Current configuration file: "));
    panelPrincipal.add("Center",currentConfigurationFile);

    setBounds(25,50,600,100);
    setResizable(false);
    setVisible(true);
  }

  public static void main(String[] args)
  {
    if (!Data.checkConfigurationFiles())
    {
      System.exit(-1);
    }

    final JFrame ventanaPrincipal = new Main();

    ventanaPrincipal.addWindowListener(new WindowAdapter()
      {
        @Override
        public void windowClosing(WindowEvent e)
        {
          if ( 0 ==
            JOptionPane.showConfirmDialog(
              null,
              "Are you sure you want to quit?",
              "Dimemas Question",
              JOptionPane.YES_NO_OPTION
            )
          )
          {
            System.exit(0);
          }
          else
          {
            ventanaPrincipal.setVisible(true);
          }
        }
      });
  }
}
