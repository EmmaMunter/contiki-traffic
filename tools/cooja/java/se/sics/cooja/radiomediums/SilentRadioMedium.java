/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: SilentRadioMedium.java,v 1.2 2007/01/09 09:47:10 fros4943 Exp $
 */

package se.sics.cooja.radiomediums;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;

/**
 * Silent radio. No data is ever transferred through this medium.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("No radio traffic")
public class SilentRadioMedium extends RadioMedium {

  public SilentRadioMedium(Simulation simulation) {
  }

  public void registerMote(Mote mote, Simulation sim) {
    // Do nothing
  }

  public void unregisterMote(Mote mote, Simulation sim) {
    // Do nothing
  }

  public void registerRadioInterface(Radio radio, Position position, Simulation sim) {
    // Do nothing
  }

  public void unregisterRadioInterface(Radio radio, Simulation sim) {
    // Do nothing
  }

  public void addRadioMediumObserver(Observer observer) {
    // Do nothing
  }
  
  public Observable getRadioMediumObservable() {
    // Return empty observable
    return new Observable();
  }

  public void deleteRadioMediumObserver(Observer observer) {
    // Do nothing
  }

  public RadioConnection[] getLastTickConnections() {
    return null;
  }

  public void setConnectionLogger(ConnectionLogger connection) {
    // Do nothing
  }

  public Collection<Element> getConfigXML() {
    return null;
  }
  
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

}
