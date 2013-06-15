#!/usr/bin/env python2

import sys
from PIL import Image
from pylab import *
import matplotlib
from numpy import *

def showPointsSelection(image):
  imshow(image)
  print ("Please select 4 points")
  points = ginput(4)
  print (points)

  return points

def calculateHMatrix (originalPoints, transformedPoints):
  print ("calculating H matrix")
  
  orig = np.array([tuple(i) for i in originalPoints]);
  
  print (orig)
  transformed = np.array([tuple(i) for i in transformedPoints]);
  print (transformed)

  A = []
  for i in range (0,4):
    A.append([0, 0, 0, -orig[i][0], -orig[i][1], -1, transformed[i][1] * orig[i][0], transformed[i][1] * orig[i][1], transformed[i][1]])
    A.append([orig[i][0], orig[i][1], 1, 0, 0, 0, -transformed[i][0] * orig[i][0], -transformed[i][0] * orig[i][1], -transformed[i][0]])

  A = np.array(A)
  print ("A: ")
  print (A)

  u, d, vt = linalg.svd(A)
  print ("u:"); print (u);
  print ("d: "); print (d);
  print ("vt: "); print (vt);
  h = vt[-1]
  print ("H as vector: ", str(h))
  
  H = np.matrix([
    [h[0],h[1],h[2]],
    [h[3],h[4],h[5]],
    [h[6],h[7],h[8]]])

  return H

def transformImage ((width, height), originalImage, transformationMatrix):

  Hinv = linalg.inv(transformationMatrix)
  transformedImage = array(Image.new (originalImage.mode, (width,height)))

  originalArray = array(originalImage)
  for y in range(0, height):
    print ("Progress %.2lf %%" % (float(y)/float(height-1) * 100))
    for x in range (0, width):
      pointTransformed = np.matrix([[x], [y], [1]])
      pointOriginal = Hinv * pointTransformed
     
      t = [float(pointOriginal[0][0]/pointOriginal[2][0]),
        float(pointOriginal[1][0]/pointOriginal[2][0]),
        1]

      #print ("Point transformed: " +str(pointTransformed) + ", Point original: " + str (t) + '\n==============\n')
      
      xOrig = t[0]
      yOrig = t[1]
      
      transformedImage[y][x] = originalArray[yOrig][xOrig]

  return transformedImage

if __name__ == '__main__':
  if len(sys.argv) < 3 or len (sys.argv) == 4 or len (sys.argv) > 5:
    sys.stderr.write ("Usage: " + str(sys.argv[0]) + " input_file output_file [output_width output_height]\n")
    sys.stderr.write ("Dimensions are in pixels with default values 500x500\n")
    sys.exit (-1)

  inputFile = sys.argv[1]
  outputFile = sys.argv[2]

  if len(sys.argv) == 5:
    transformedWidth = int(sys.argv[3])
    transformedHeight = int(sys.argv[4])
  else:
    transformedWidth  = 500
    transformedHeight = 500
    
  
  sys.stderr.write ("Using: Input file: " + inputFile + "\nOutput file: " + outputFile + "\nResolution: " + str(transformedWidth) + "x" + str(transformedHeight) + "\n")


  transformedPoints = [(0,0), (transformedWidth,0), (transformedWidth,transformedHeight), (0,transformedHeight)]
  originalImage = Image.open(inputFile)
  originalPoints = showPointsSelection(array(originalImage))
  
  #originalPoints = [(0,0), (transformedWidth,0), (transformedWidth,transformedHeight), (0,transformedHeight)]

  hMatrix = calculateHMatrix (originalPoints, transformedPoints)
  
  transformedImage = transformImage ((transformedWidth, transformedHeight), originalImage, hMatrix)
  Image.fromarray(transformedImage).save (outputFile)

  #print ("Displaying image")
  #imshow (transformedImage)
  #ginput (3)
