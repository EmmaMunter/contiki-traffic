/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: ESBMote.java,v 1.2 2008/03/17 09:54:19 fros4943 Exp $
 */

package se.sics.cooja.mspmote;

import java.io.File;
import java.util.*;
import org.apache.log4j.Logger;
import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;
import se.sics.cooja.mspmote.interfaces.*;
import se.sics.mspsim.platform.esb.ESBNode;

/**
 * @author Fredrik Osterlind
 */
public class ESBMote extends MspMote {
  private static Logger logger = Logger.getLogger(ESBMote.class);

  public ESBNode esbNode = null;

  public ESBMote() {
    super();
  }

  public ESBMote(MoteType moteType, Simulation sim) {
    super((MspMoteType) moteType, sim);
  }

  protected boolean initEmulator(File fileELF) {
    try {
      esbNode = new ESBNode();
      esbNode.setupNodePorts();
      prepareMote(fileELF, esbNode.getCPU());

    } catch (Exception e) {
      logger.fatal("Error when creating ESB mote: " + e);
      return false;
    }
    return true;
  }

  protected MoteInterfaceHandler createMoteInterfaceHandler() {
    MoteInterfaceHandler moteInterfaceHandler = new MoteInterfaceHandler();

    // Add position interface
    Position motePosition = new Position(this);
    Random random = new Random();
    motePosition.setCoordinates(random.nextDouble()*100, random.nextDouble()*100, random.nextDouble()*100);
    moteInterfaceHandler.addActiveInterface(motePosition);

    // Add log interface
    Log moteLog = new ESBLog(this);
    moteInterfaceHandler.addActiveInterface(moteLog);

    // Add time interface
    Clock moteClock = new MspClock(this);
    moteInterfaceHandler.addActiveInterface(moteClock);

    // Add led interface
    LED moteLed = new ESBLED(this);
    moteInterfaceHandler.addActiveInterface(moteLed);

    // Add button interface
    Button moteButton = new ESBButton(this);
    moteInterfaceHandler.addActiveInterface(moteButton);

    // Add ID interface
    MoteID moteID = new MspMoteID(this);
    moteInterfaceHandler.addActiveInterface(moteID);

    // Add radio interface
    myRadio = new TR1001Radio(this);
    moteInterfaceHandler.addActiveInterface(myRadio);

    return moteInterfaceHandler;
  }

  public String toString() {
    if (getInterfaces().getMoteID() != null) {
      return "ESB Mote, ID=" + getInterfaces().getMoteID().getMoteID();
    } else {
      return "ESB Mote, ID=null";
    }
  }

}

