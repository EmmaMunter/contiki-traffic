/*
 * Copyright (c) 20068, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: SkyFlash.java,v 1.2 2008/04/03 14:02:47 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.mspmote.SkyMote;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("M25P80 Flash")
public class SkyFlash extends MoteInterface {
  private static Logger logger = Logger.getLogger(SkyFlash.class);

  public int SIZE = 1024*1024;

  private SkyMote mote = null;
  private CoojaM25P80 m24p80 = null;

  public SkyFlash(Mote mote) {
    this.mote = (SkyMote) mote;
    m24p80 = new CoojaM25P80(this.mote.getCPU());
    this.mote.skyNode.setFlash(m24p80);
  }

  /**
   * Write ID header to start of flash.
   *
   * @param id ID
   */
  public void writeIDheader(int id) {
    byte[] idHeader = new byte[4];
    idHeader[0] = (byte) 0xad;
    idHeader[1] = (byte) 0xde;
    idHeader[2] = (byte) (id >> 8);
    idHeader[3] = (byte) (id & 0xff);
    try {
      m24p80.seek(0);
      m24p80.write(idHeader);
    } catch (IOException e) {
      logger.fatal("Exception when writing ID header: " + e);
    }
  }

  public void doActionsBeforeTick() {
  }

  public void doActionsAfterTick() {
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JButton uploadButton = new JButton("Upload file");
    panel.add(uploadButton);
    final JButton downloadButton = new JButton("Store to file");
    panel.add(downloadButton);

    if (GUI.isVisualizedInApplet()) {
      uploadButton.setEnabled(false);
      uploadButton.setToolTipText("Not available in applet mode");
      downloadButton.setEnabled(false);
      downloadButton.setToolTipText("Not available in applet mode");
    }

    uploadButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        byte[] fileData = readDialogFileBytes(null);

        if (fileData != null) {
          if (fileData.length > CoojaM25P80.SIZE) {
            logger.fatal("Too large data file: " + fileData.length + " > " + CoojaM25P80.SIZE);
            return;
          }
          try {
            m24p80.seek(0);
            m24p80.write(fileData);
            logger.info("Done! (" + fileData.length + " bytes written to Flash)");
          } catch (IOException ex) {
            logger.fatal("Exception: " + ex);
          }
        }
      }
    });

    downloadButton.setEnabled(false);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    panel.setMinimumSize(new Dimension(140, 60));
    panel.setPreferredSize(new Dimension(140, 60));

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  /**
   * Opens a file dialog and returns the contents of the selected file or null if dialog aborted.
   *
   * @param parent Dialog parent, may be null
   * @return Binary contents of user selected file
   */
  public static byte[] readDialogFileBytes(Component parent) {
    // Choose file
    File file = null;
    JFileChooser fileChooser = new JFileChooser();
    fileChooser.setCurrentDirectory(new java.io.File("."));
    fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
    fileChooser.setDialogTitle("Select data file");

    if (fileChooser.showOpenDialog(parent) == JFileChooser.APPROVE_OPTION) {
      file = fileChooser.getSelectedFile();
    } else {
      return null;
    }

    // Read file data
    long fileSize = file.length();
    byte[] fileData = new byte[(int) fileSize];

    FileInputStream fileIn;
    DataInputStream dataIn;
    int offset = 0;
    int numRead = 0;
    try {
      fileIn = new FileInputStream(file);
      dataIn = new DataInputStream(fileIn);
      while (offset < fileData.length
          && (numRead = dataIn.read(fileData, offset, fileData.length - offset)) >= 0) {
        offset += numRead;
      }

      dataIn.close();
      fileIn.close();
    } catch (Exception ex) {
      logger.debug("Exception ex: " + ex);
      return null;
    }

    return fileData;
  }

  public double energyConsumptionPerTick() {
    return 0;
  }

}
