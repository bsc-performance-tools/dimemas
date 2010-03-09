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

// Esta clase sirve de base para la mayoría de las ventanas del GUI aportando
// propiedades y métodos comunes a todas ellas.
abstract class GUIWindow extends JFrame implements ActionListener
{
  protected Data data;
  protected Container windowPanel = getContentPane();
  protected GridBagLayout layout = new GridBagLayout();
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

    setIconImage(Toolkit.getDefaultToolkit().getImage(Data.ICON_IMAGE));
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
  abstract public void actionPerformed(ActionEvent e);
}
