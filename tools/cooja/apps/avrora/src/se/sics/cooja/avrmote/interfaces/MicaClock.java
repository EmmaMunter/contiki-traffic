/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: MicaClock.java,v 1.2 2009/03/11 14:12:19 fros4943 Exp $
 */

package se.sics.cooja.avrmote.interfaces;

import java.util.Collection;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.avrmote.MicaZMote;
import se.sics.cooja.interfaces.Clock;


/**
 * @author Fredrik Osterlind, Joakim Eriksson
 */
@ClassDescription("Cycle clock")
public class MicaClock extends Clock {
  private static Logger logger = Logger.getLogger(MicaClock.class);

  private MicaZMote myMote;

  public MicaClock(MicaZMote mote) {
    myMote = mote;
  }

  public void setTime(long newTime) {
    logger.fatal("Can't change emulated CPU time");
  }

  public long getTime() {
    int time = (int) ((myMote.cycleCounter) / MicaZMote.NR_CYCLES_PER_MSEC);
    return time > 0 ? time : 0;
  }

  public void setDrift(int drift) {
    myMote.cycleDrift = MicaZMote.NR_CYCLES_PER_MSEC * drift;
  }

  public int getDrift() {
    return (int) (myMote.cycleDrift / MicaZMote.NR_CYCLES_PER_MSEC);
  }

  public JPanel getInterfaceVisualizer() {
    return null;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public double energyConsumption() {
    return 0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
