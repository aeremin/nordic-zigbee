import os
import requests
import serial
import subprocess
import time
import unittest

def buildAndUpload():
  makefile_location = 'light_bulb/pca10056/blank/armgcc'
  print('-- Building firmware --')
  out = subprocess.check_output(['bash', '-c', 'make'], cwd=makefile_location).decode('utf-8')
  print(out)
  print('-- Uploading firmware --')
  out = subprocess.check_output(['nrfjprog', '--snr', '683044303', '--program', '_build/nrf52840_xxaa.hex',  '--chiperase', '--reset'], cwd=makefile_location).decode('utf-8')
  print(out)

def waitUntilStringInUart(waitFor):
  with serial.Serial('COM11', 115200) as ser:
    while True:
      line = ser.readline().decode('ascii', 'ignore')
      if line:
        print('Received: ' + line, end='')
        if waitFor in line:
          return
      time.sleep(0.1)


class HueHelper:
  def baseUrl(self):
    return 'http://hue-bridge-dev/api/dPLA71l4sx-TQrimFTPEG0Kq1E50ilTnyIhi514Y'

  def searchForNewLights(self):
    r = requests.post(self.baseUrl() + '/lights')
    r.raise_for_status()

  def getNewLights(self):
    r = requests.get(self.baseUrl() + '/lights/new')
    r.raise_for_status()
    lights = []
    for key in r.json():
      if key != 'lastscan':
        lights.append((key, r.json()[key]))
    return lights

  def getAllLights(self):
    r = requests.get(self.baseUrl() + '/lights')
    r.raise_for_status()
    lights = []
    for key in r.json():
        lights.append((key, r.json()[key]))
    return lights

  def removeAllLights(self):
    for id, _ in self.getAllLights():
      r = requests.delete(self.baseUrl() + '/lights/' + id)
      r.raise_for_status()

  def turnOfAndOn(self, id):
    r = requests.put(self.baseUrl() + '/lights/%s/state' % id, json={'on': False})
    r.raise_for_status()
    time.sleep(0.5)
    r = requests.put(self.baseUrl() + '/lights/%s/state' % id, json={'on': True})
    r.raise_for_status()

def waitUntil(somepredicate, timeout=5, period=1):
  deadline = time.time() + timeout
  while time.time() < deadline:
    if somepredicate():
      return True
    time.sleep(period)
  return False


class TestLightIsDiscoverable(unittest.TestCase):
  @classmethod
  def setUpClass(cls):
    cls.helper = HueHelper()
    cls.helper.removeAllLights()
    buildAndUpload()
    cls.helper.searchForNewLights()

  @classmethod
  def tearDownClass(cls):
    cls.helper.removeAllLights()

  def test_01_JoiningNetwork(self):
    waitUntilStringInUart('Joined network successfully')

  def test_02_LightDiscoveredByHue(self):
    self.assertTrue(waitUntil(self.helper.getNewLights))
    self.assertTrue(waitUntil(self.helper.getAllLights))

  def test_03_Blink(self):
    id, _ = self.helper.getAllLights()[0]
    self.helper.turnOfAndOn(id)
    waitUntilStringInUart('ON/OFF')



if __name__ == '__main__':
  unittest.main()
