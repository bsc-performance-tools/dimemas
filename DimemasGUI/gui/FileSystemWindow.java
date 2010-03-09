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
* Esta clase crea la ventana de configuración correspondiente al sistema de
* ficheros.
*/
public class FileSystemWindow extends GUIWindow
{
  public static final long serialVersionUID = 9L;
  
  private JButton b_save = createButton("Save");
  private JButton b_close = createButton("Close");

  private JTextField tf_diskLatency = new JTextField(15);
  private JTextField tf_diskBandwidth = new JTextField(15);
  private JTextField tf_blockSize = new JTextField(15);
  private JTextField tf_requests = new JTextField(15);
  private JTextField tf_hitRatio = new JTextField(15);

  // Constructor de la clase FileSystemWindow.
  public FileSystemWindow(Data d)
  {
    super(d);

    // Propiedades de la ventana.
    setTitle("File system information");

    // Obteniendo información.
    tf_diskLatency.setText(data.fileSys.getLatency());
    tf_diskBandwidth.setText(data.fileSys.getBandwidth());
    tf_blockSize.setText(data.fileSys.getSize());
    tf_requests.setText(data.fileSys.getReq());
    tf_hitRatio.setText(data.fileSys.getHit());

    // Añadiendo los componentes a la ventana.
    drawLine(new Component[] {new JLabel("Disk latency [s]"),tf_diskLatency});
    drawLine(new Component[] {new JLabel("Disk bandwidth [MByte/s]"),tf_diskBandwidth});
    drawLine(new Component[] {new JLabel("Block size [Bytes]"),tf_blockSize});
    drawLine(new Component[] {new JLabel("Concurrent requests"),tf_requests});
    drawLine(new Component[] {new JLabel("Hit ratio"),tf_hitRatio});
    drawButtons(new Component[] {b_save,b_close},60,5);

    // Más propiedades de ventana.
    setBounds(25,150,getWidth()+25,getHeight());
    pack();
    setVisible(true);
  }

  // En este método se ejecutan las respuestas del GUI a las acciones del
  // usuario al interactuar con los elementos de la ventana.
  public void actionPerformed(ActionEvent e)
  {
    if(e.getSource() == b_save)
    {
      try
      {
        data.fileSys.setLatency(tf_diskLatency.getText());
        data.fileSys.setBandwidth(tf_diskBandwidth.getText());
        data.fileSys.setSize(tf_blockSize.getText());
        data.fileSys.setReq(tf_requests.getText());
        data.fileSys.setHit(tf_hitRatio.getText());
        dispose();
      } catch(Exception exc) {}
    }
    else if(e.getSource() == b_close)
    {
      dispose();
    }
  }
}
