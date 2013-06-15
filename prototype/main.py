#!/usr/bin/env python2

from PIL import Image
from pylab import *
import matplotlib
from numpy import *

def showPointsSelection(imgPath):
  im = array(Image.open(imgPath))
  imshow(im)
  print ("Please select 4 points")
  points = ginput(4)
  print (points)

  return points

def calculateHMatrix (originalPoints, transformedPoints):
  print ("calculating H matrix")
  
  orig = np.array([tuple(i) for i in originalPoints]);
  transformed = np.array([tuple(i) for i in transformedPoints]);

  A = []
  for i in range (0,4):
    A.append([0, 0, 0, -orig[i][0], -orig[i][1], -1, transformed[i][1] * orig[i][0], transformed[i][1] * orig[i][1], transformed[i][1]])
    A.append([orig[i][0], orig[i][1], 1, 0, 0, 0, transformed[i][0] * orig[i][0], transformed[i][0] * orig[i][1], transformed[i][0]])

  A = np.array(A)
  print (A)

  u, d, vt = linalg.svd(A)
  print (u)
  print (d)
  print (vt)

  h = vt[0]
  
  H = np.array([
    [h[0],h[1],h[2]],
    [h[3],h[4],h[5]],
    [h[6],h[7],h[7]]]

  return H

if __name__ == '__main__':
  print ("hello")
  transformedPoints = [(0,0), (100,0), (100,100), (0,100)]
  originalPoints = showPointsSelection('sample.jpg')

  hMatrix = calculateHMatrix (originalPoints, transformedPoints)
