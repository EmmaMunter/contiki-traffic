/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: Node.java,v 1.4 2010/09/14 10:38:12 nifi Exp $
 *
 * -----------------------------------------------------------------
 *
 * Node
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2010/09/14 10:38:12 $
 *           $Revision: 1.4 $
 */

package se.sics.contiki.collect;
import java.util.ArrayList;
import java.util.Hashtable;

/**
 *
 */
public class Node implements Comparable<Node> {

  private SensorDataAggregator sensorDataAggregator;
  private ArrayList<SensorData> sensorDataList = new ArrayList<SensorData>();
  private ArrayList<Link> links = new ArrayList<Link>();

  private final String id;
  private final String name;

  private int x = -1, y = -1;

  private Hashtable<String,Object> objectTable;

  private long lastActive;

  public Node(String nodeID) {
    this.id = nodeID;
    this.name = nodeID;
    sensorDataAggregator = new SensorDataAggregator(this);
  }

  public final String getID() {
    return id;
  }

  public final String getName() {
    return name;
  }

  public int getX() {
    return x;
  }

  public int getY() {
    return y;
  }

  public void setLocation(int x, int y) {
    this.x = x;
    this.y = y;
  }

  public boolean hasLocation() {
    return x >= 0 && y >= 0;
  }

  public long getLastActive() {
    return lastActive;
  }

  public void setLastActive(long lastActive) {
    this.lastActive = lastActive;
  }

  @Override
  public int compareTo(Node o) {
    return name.compareTo(o.name);
  }

  public String toString() {
    return name;
  }


  // -------------------------------------------------------------------
  // Attributes
  // -------------------------------------------------------------------

  public Object getAttribute(String key) {
    return getAttribute(key, null);
  }

  public Object getAttribute(String key, Object defaultValue) {
    if (objectTable == null) {
      return null;
    }
    Object val = objectTable.get(key);
    return val == null ? defaultValue : val;
  }

  public void setAttribute(String key, Object value) {
    if (objectTable == null) {
      objectTable = new Hashtable<String,Object>();
    }
    objectTable.put(key, value);
  }

  public void clearAttributes() {
    if (objectTable != null) {
      objectTable.clear();
    }
  }


  // -------------------------------------------------------------------
  // SensorData
  // -------------------------------------------------------------------

  public SensorDataAggregator getSensorDataAggregator() {
    return sensorDataAggregator;
  }

  public SensorData[] getAllSensorData() {
    return sensorDataList.toArray(new SensorData[sensorDataList.size()]);
  }

  public void removeAllSensorData() {
    sensorDataList.clear();
    sensorDataAggregator.clear();
  }

  public SensorData getSensorData(int index) {
    return sensorDataList.get(index);
  }

  public int getSensorDataCount() {
    return sensorDataList.size();
  }

  public boolean addSensorData(SensorData data) {
    if (sensorDataList.size() > 0) {
      SensorData last = sensorDataList.get(sensorDataList.size() - 1);
      // TODO should check seqno!
      if (data.getNodeTime() <= last.getNodeTime()) {
        // Sensor data already added
        System.out.println("SensorData: ignoring (time " + (data.getNodeTime() - last.getNodeTime())
            + "msec): " + data);
        return false;
      }
    }
    sensorDataList.add(data);
    sensorDataAggregator.addSensorData(data);
    return true;
  }


  // -------------------------------------------------------------------
  // Links
  // -------------------------------------------------------------------

  public Link getLink(Node node) {
    for(Link l: links) {
      if (l.node == node) {
        return l;
      }
    }

    // Add new link
    Link l = new Link(node);
    links.add(l);
    return l;
  }

  public Link getLink(int index) {
    return links.get(index);
  }

  public int getLinkCount() {
    return links.size();
  }

  public void removeLink(Node node) {
    for (int i = 0, n = links.size(); i < n; i++) {
      Link l = links.get(i);
      if (l.node == node) {
        links.remove(i);
        break;
      }
    }
  }

  public void clearLinks() {
    links.clear();
  }

}
