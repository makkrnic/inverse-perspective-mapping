#!/usr/bin/env python2

from PIL import Image
from pylab import *
import matplotlib

def showPointsSelection(imgPath):
  im = array(Image.open(imgPath))
  imshow(im)
  print ("Please select 4 points")
  points = ginput(4)
  print (points)


if __name__ == '__main__':
  print ("hello")
  showPointsSelection('sample.jpg')
