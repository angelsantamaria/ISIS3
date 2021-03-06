#!/usr/bin/env python

import os
from optparse import OptionParser

def setisis():
  #Try to see if ISISROOT is set...
  try:
    ISISROOT = os.environ["ISISROOT"]
  except:
    ISISROOT = None

  #... and if it's not give it a default value
  if ISISROOT is None:
    ISISROOT = "/usgs/pkgs/isis3/install"
    os.environ['ISISROOT'] = ISISROOT

  #Check for the ISISDATA directory. If it does not exist use a default
  if os.path.exists("%s/../data" % (ISISROOT)):
    os.environ['ISISDATA'] = "%s/../isis_data" % (ISISROOT)
  else:
    os.environ['ISISDATA'] = "/usgs/cpkgs/isis3/isis_data"

  #Check for the ISISTESTDATA directory. If it does not exist use a default
  if os.path.exists("%s/../testData" % (ISISROOT)):
    os.environ['ISISTESTDATA'] = "%s/../testData" % ISISROOT
  else:
    os.environ['ISISTESTDATA'] = "/usgs/cpkgs/isis3/isis_testData"

  #If PATH is not set, just set it to a default location. Else append
  #the isis path to the end of the current path
  try:
    os.environ['PATH']
  except:
    os.environ['PATH'] = "%s/bin" % (ISISROOT)
  else:
    os.environ['PATH'] = "%s:%s/bin" % (os.environ['PATH'], ISISROOT)

if __name__ == "__main__":
  setisis()
