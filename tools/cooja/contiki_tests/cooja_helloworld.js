/* Script is called once for every node log output. */
/* Input variables: Mote mote, int id, String msg. */

log.log('MOTE=' + mote + '\n');
log.log('ID=' + id + '\n');
log.log('TIME=' + mote.getSimulation().getSimulationTime() + '\n');
log.log('MSG=' + msg + '\n');

if (msg.startsWith('Hello, world')) {
  log.log('TEST OK\n'); /* Report test success */
  /* To increase test run speed, close the simulator when done */
  mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
}
