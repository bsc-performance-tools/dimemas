/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

package es.bsc.cepbatools.dimemas.gui;

import es.bsc.cepbatools.dimemas.data.Data;
import es.bsc.cepbatools.dimemas.data.MappingData;
import es.bsc.cepbatools.dimemas.tools.Tools;

/**
 *
 * @author jgonzale
 */
public class PredefinedMapsWindow extends GUIWindow {
  
  public static final long serialVersionUID = 14L;
  
  private int[]         currentMap;
  private int           currentMapInfo;
  private int           nTasksPerNode;
  
  /**
   * Creates new form NewMappingWindow
   */
  public PredefinedMapsWindow(Data d)
  {
    super(d);
    
    initComponents();

    currentMap     = data.map.getMap();
    currentMapInfo = data.map.getMapInfo();
    nTasksPerNode  = data.map.getNTasksPerNode();
    
    if (data.map.getTasks() <= 0)
    {
      l_unknownTaskNum.setVisible(true);
      l_unknownTaskNum1.setVisible(true);
    }
    else
    {
      l_unknownTaskNum.setVisible(false);
      l_unknownTaskNum1.setVisible(false);
    }
    
    setState();
    
    pack();
    setVisible(true);
  }

  /**
   * This method is called from within the constructor to initialize the form.
   * WARNING: Do NOT modify this code. The content of this method is always
   * regenerated by the Form Editor.
   */
  @SuppressWarnings("unchecked")
  // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
  private void initComponents() {

    rb_predefinedMaps = new javax.swing.ButtonGroup();
    p_radioButtos = new javax.swing.JPanel();
    rb_filledNodes = new javax.swing.JRadioButton();
    rb_interleaved = new javax.swing.JRadioButton();
    tf_nTasksPerNode = new javax.swing.JTextField();
    rb_nTasksPerNode = new javax.swing.JRadioButton();
    rb_irregularMap = new javax.swing.JRadioButton();
    p_buttons = new javax.swing.JPanel();
    b_close = new javax.swing.JButton();
    b_save = new javax.swing.JButton();
    p_labels = new javax.swing.JPanel();
    l_unknownTaskNum1 = new javax.swing.JLabel();
    l_unknownTaskNum = new javax.swing.JLabel();

    setTitle("Predefined Maps");
    setResizable(false);

    rb_predefinedMaps.add(rb_filledNodes);
    rb_filledNodes.setFont(rb_filledNodes.getFont());
    rb_filledNodes.setText("Fill Nodes");
    rb_filledNodes.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        rb_filledNodesActionPerformed(evt);
      }
    });

    rb_predefinedMaps.add(rb_interleaved);
    rb_interleaved.setFont(rb_interleaved.getFont());
    rb_interleaved.setText("Interleaved");
    rb_interleaved.setToolTipText("One tasks is assigned to each node in a cyclic way");
    rb_interleaved.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        rb_interleavedActionPerformed(evt);
      }
    });

    tf_nTasksPerNode.setColumns(2);
    tf_nTasksPerNode.setFont(tf_nTasksPerNode.getFont());
    tf_nTasksPerNode.setText("1");
    tf_nTasksPerNode.setToolTipText("Press 'Enter' to confirm the value");
    tf_nTasksPerNode.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        tf_nTasksPerNodeActionPerformed(evt);
      }
    });

    rb_predefinedMaps.add(rb_nTasksPerNode);
    rb_nTasksPerNode.setFont(rb_nTasksPerNode.getFont());
    rb_nTasksPerNode.setText("'n' tasks per node");
    rb_nTasksPerNode.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        rb_nTasksPerNodeActionPerformed(evt);
      }
    });

    rb_predefinedMaps.add(rb_irregularMap);
    rb_irregularMap.setText("Irregular map");
    rb_irregularMap.setToolTipText("Map modified manually, not selectable");
    rb_irregularMap.setEnabled(false);

    javax.swing.GroupLayout p_radioButtosLayout = new javax.swing.GroupLayout(p_radioButtos);
    p_radioButtos.setLayout(p_radioButtosLayout);
    p_radioButtosLayout.setHorizontalGroup(
      p_radioButtosLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(p_radioButtosLayout.createSequentialGroup()
        .addContainerGap()
        .addGroup(p_radioButtosLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
          .addGroup(p_radioButtosLayout.createSequentialGroup()
            .addGroup(p_radioButtosLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
              .addComponent(rb_irregularMap)
              .addComponent(rb_interleaved)
              .addComponent(rb_filledNodes))
            .addContainerGap())
          .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, p_radioButtosLayout.createSequentialGroup()
            .addGap(0, 0, Short.MAX_VALUE)
            .addComponent(rb_nTasksPerNode)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addComponent(tf_nTasksPerNode, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))))
    );
    p_radioButtosLayout.setVerticalGroup(
      p_radioButtosLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(p_radioButtosLayout.createSequentialGroup()
        .addComponent(rb_filledNodes)
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
        .addGroup(p_radioButtosLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
          .addComponent(rb_nTasksPerNode)
          .addComponent(tf_nTasksPerNode, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
        .addComponent(rb_interleaved)
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
        .addComponent(rb_irregularMap)
        .addContainerGap())
    );

    b_close.setFont(b_close.getFont());
    b_close.setText("Close");
    b_close.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        b_closeActionPerformed(evt);
      }
    });

    b_save.setFont(b_save.getFont());
    b_save.setText("Save");
    b_save.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(java.awt.event.ActionEvent evt) {
        b_saveActionPerformed(evt);
      }
    });

    javax.swing.GroupLayout p_buttonsLayout = new javax.swing.GroupLayout(p_buttons);
    p_buttons.setLayout(p_buttonsLayout);
    p_buttonsLayout.setHorizontalGroup(
      p_buttonsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(p_buttonsLayout.createSequentialGroup()
        .addGap(28, 28, 28)
        .addComponent(b_save, javax.swing.GroupLayout.PREFERRED_SIZE, 96, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
        .addComponent(b_close, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
    );
    p_buttonsLayout.setVerticalGroup(
      p_buttonsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(p_buttonsLayout.createSequentialGroup()
        .addContainerGap()
        .addGroup(p_buttonsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
          .addComponent(b_close)
          .addComponent(b_save))
        .addContainerGap())
    );

    javax.swing.GroupLayout p_labelsLayout = new javax.swing.GroupLayout(p_labels);
    p_labels.setLayout(p_labelsLayout);
    p_labelsLayout.setHorizontalGroup(
      p_labelsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGap(0, 14, Short.MAX_VALUE)
    );
    p_labelsLayout.setVerticalGroup(
      p_labelsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGap(0, 40, Short.MAX_VALUE)
    );

    l_unknownTaskNum1.setForeground(new java.awt.Color(255, 0, 0));
    l_unknownTaskNum1.setText("Verify the mapping in the simulator");

    l_unknownTaskNum.setForeground(new java.awt.Color(255, 0, 0));
    l_unknownTaskNum.setText("NOTE: unknown number of tasks.");

    javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
    getContentPane().setLayout(layout);
    layout.setHorizontalGroup(
      layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(layout.createSequentialGroup()
        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
          .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
            .addComponent(p_labels, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
            .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
              .addComponent(l_unknownTaskNum1)
              .addComponent(l_unknownTaskNum)))
          .addGroup(layout.createSequentialGroup()
            .addContainerGap()
            .addComponent(p_buttons, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
          .addGroup(layout.createSequentialGroup()
            .addGap(37, 37, 37)
            .addComponent(p_radioButtos, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
        .addContainerGap())
    );
    layout.setVerticalGroup(
      layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
      .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
        .addContainerGap()
        .addComponent(p_radioButtos, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
        .addComponent(p_buttons, javax.swing.GroupLayout.PREFERRED_SIZE, 47, javax.swing.GroupLayout.PREFERRED_SIZE)
        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
          .addGroup(layout.createSequentialGroup()
            .addComponent(p_labels, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addGap(0, 0, Short.MAX_VALUE))
          .addGroup(layout.createSequentialGroup()
            .addComponent(l_unknownTaskNum, javax.swing.GroupLayout.PREFERRED_SIZE, 17, javax.swing.GroupLayout.PREFERRED_SIZE)
            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(l_unknownTaskNum1)))
        .addContainerGap())
    );
  }// </editor-fold>//GEN-END:initComponents

  private void b_closeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_b_closeActionPerformed
    // TODO add your handling code here:
   // closeEvent();
   dispose();
  }//GEN-LAST:event_b_closeActionPerformed

      private void rb_nTasksPerNodeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_rb_nTasksPerNodeActionPerformed

    MappingData.MapResult result = data.map.new MapResult();

    // ("'n' Tasks per Node Selected");
    try
    {
      nTasksPerNode = Integer.parseInt(tf_nTasksPerNode.getText());
    }
    catch (NumberFormatException e)
    {
      Tools.showErrorMessage("Wrong tasks per node value");
      return;
    }

    result = data.map.mapNTasksPerNode(nTasksPerNode);
    
    if (result.errorMessage != null)
    {
      Tools.showErrorMessage(result.errorMessage);
    }
    
    currentMap     = result.generatedMap;
    currentMapInfo = Data.N_TASKS_PER_NODE_MAP;  
    setState();
  }//GEN-LAST:event_rb_nTasksPerNodeActionPerformed

  private void rb_interleavedActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_rb_interleavedActionPerformed
    MappingData.MapResult result = data.map.new MapResult();
    
    // System.out.println("Interleaved selected");
    
    result = data.map.mapInterleaved();
    
    if ( result.errorMessage != null)
    {
      Tools.showErrorDialog(result.errorMessage);
    }

    currentMap     = result.generatedMap;
    currentMapInfo = Data.INTERLEAVE_MAP;
    setState();
  }//GEN-LAST:event_rb_interleavedActionPerformed

  private void rb_filledNodesActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_rb_filledNodesActionPerformed
    MappingData.MapResult result = data.map.new MapResult();
    
    // ("Fill Nodes Selected");
    
    result = data.map.mapFillingNodes();

    if (result.errorMessage != null)
    {
      Tools.showErrorMessage(result.errorMessage);
    }

    currentMap     = result.generatedMap;
    currentMapInfo = Data.FILL_NODE_MAP;
    setState();
  }//GEN-LAST:event_rb_filledNodesActionPerformed

  private void tf_nTasksPerNodeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_tf_nTasksPerNodeActionPerformed
    
    MappingData.MapResult result = data.map.new MapResult();

    // ("'n' Tasks per Node Selected");
    try
    {
      nTasksPerNode = Integer.parseInt(tf_nTasksPerNode.getText());
    }
    catch (NumberFormatException e)
    {
      Tools.showErrorMessage("Wrong tasks per node value");
      return;
    }

    result = data.map.mapNTasksPerNode(nTasksPerNode);
    
    if (result.errorMessage != null)
    {
      Tools.showErrorMessage(result.errorMessage);
    }

    currentMap     = result.generatedMap;
    currentMapInfo = Data.N_TASKS_PER_NODE_MAP;
    setState();
  }//GEN-LAST:event_tf_nTasksPerNodeActionPerformed

  private void b_saveActionPerformed(java.awt.event.ActionEvent evt) 
  {
    /*GEN-FIRST:event_b_saveActionPerformed
     Check a possible value change in the text field that indicates the
       tasks per node */
    data.map.setMapInfo(currentMapInfo);
    data.map.setMap(currentMap);
    data.map.setNTasksPerNode(nTasksPerNode);
   
    if(data.map.getMapInfo() == Data.NO_MAP)
    {
        Tools.showWarningMessage("Please select the mapping first");
        this.setVisible(true);
    }
    else
    {
        dispose();
    }
    if (rb_nTasksPerNode.isSelected())
    {
      try
      {
        nTasksPerNode = Integer.parseInt(tf_nTasksPerNode.getText());
      }
      catch (NumberFormatException e)
      {
        Tools.showErrorMessage("Wrong tasks per node value");
        return;
      }
      dispose();
    }
    data.map.setMapInfo(currentMapInfo);
    data.map.setMap(currentMap);
    data.map.setNTasksPerNode(nTasksPerNode);
  }//GEN-LAST:event_b_saveActionPerformed

  private void setState()
  {
    switch(currentMapInfo)
    {
      case Data.UNKNOWN_MAP:
        rb_irregularMap.setSelected(true);
        break;
      case Data.FILL_NODE_MAP:
        rb_filledNodes.setSelected(true);
        break;
      case Data.N_TASKS_PER_NODE_MAP:
        rb_nTasksPerNode.setSelected(true);
        tf_nTasksPerNode.setText(String.valueOf(nTasksPerNode));
        break;
      case Data.INTERLEAVE_MAP:
        rb_interleaved.setSelected(true);
        break;
      default:
        // ("Unknow map "+data.map.getMapInfo());
        break;
    }
  }
  
 /* private void closeEvent()
  {
    // ("Closing event!");
    if (data.map.getMapInfo() == Data.NO_MAP)
    {
      Tools.showWarningMessage("Please select a map before closing");
      this.setVisible(true);
    }
    else
    {
      dispose();
    }
  }*/
  
  // Variables declaration - do not modify//GEN-BEGIN:variables
  private javax.swing.JButton b_close;
  private javax.swing.JButton b_save;
  private javax.swing.JLabel l_unknownTaskNum;
  private javax.swing.JLabel l_unknownTaskNum1;
  private javax.swing.JPanel p_buttons;
  private javax.swing.JPanel p_labels;
  private javax.swing.JPanel p_radioButtos;
  private javax.swing.JRadioButton rb_filledNodes;
  private javax.swing.JRadioButton rb_interleaved;
  private javax.swing.JRadioButton rb_irregularMap;
  private javax.swing.JRadioButton rb_nTasksPerNode;
  private javax.swing.ButtonGroup rb_predefinedMaps;
  private javax.swing.JTextField tf_nTasksPerNode;
  // End of variables declaration//GEN-END:variables
}
