#!/usr/bin/env python2

import sys
from PIL import Image
from pylab import *
import matplotlib
from numpy import *
import time

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

def transformImage ((width, height), originalImage, transformationMatrix, enableInterpolation = True):

  Hinv = linalg.inv(transformationMatrix)
  transformedImage = array(Image.new (originalImage.mode, (width,height)))
  (originalWidth, originalHeight) = originalImage.size

  originalArray = array(originalImage)
  for y in range(0, height):
    print ("Progress %.2lf %%" % (float(y)/float(height-1) * 100))
    for x in range (0, width):
      pointTransformed = np.matrix([[x], [y], [1]])
      pointOriginal = Hinv * pointTransformed
     
      t = [float(pointOriginal[0][0]/pointOriginal[2][0]),
        float(pointOriginal[1][0]/pointOriginal[2][0]),
        1]

      print ("Point transformed: " +str(pointTransformed) + ", Point original: " + str (t) + '\n==============\n')
      
      xOrig = t[0]
      yOrig = t[1]
      

      if (enableInterpolation
        and (xOrig != int(xOrig) or yOrig != int(yOrig))
        and xOrig - 1 >= 0 and xOrig + 1 <= originalWidth
        and yOrig - 1 >= 0 and yOrig + 1 <= originalHeight):

        #print ("Interpolating (%f, %f)" % (xOrig, yOrig))

        xOrigInt = int (xOrig)
        yOrigInt = int (yOrig)

        dx = xOrig - xOrigInt
        dy = yOrig - yOrigInt

        #print ("dx: %f; dy: %f" % (dx, dy))

        point = (originalArray[yOrigInt][xOrigInt] * (1-dx)*(1-dy)
              + originalArray[yOrigInt][xOrigInt+1]*dx*(1-dy)
              + originalArray[yOrigInt+1][xOrigInt]*(1-dx)*dy
              + originalArray[yOrigInt+1][xOrigInt+1]*dx*dy)
        #print (point)

        transformedImage[y][x] = point

        #sys.stdin.readline()
      else:
        #print ("Not interpolating (%f, %f)" % (xOrig, yOrig))
        transformedImage[y][x] = originalArray[int(yOrig)][int(xOrig)]

  return transformedImage

if __name__ == '__main__':
  timeStart = time.clock()

  if len(sys.argv) < 3 or len (sys.argv) == 4 or len (sys.argv) > 6:
    sys.stderr.write ("Usage: " + str(sys.argv[0]) + " <input_file> <output_file> <output_width> <output_height> [--no-interpolation]\n")
    sys.stderr.write ("\tDimensions are in pixels.\n\t-no-interpolation disables interpolation.\n")
    sys.exit (-1)

  inputFile = sys.argv[1]
  outputFile = sys.argv[2]
  transformedWidth = int(sys.argv[3])
  transformedHeight = int(sys.argv[4])

  if len(sys.argv) == 6:
    enableInterpolation = False
  else:
    enableInterpolation = True
    
  
  sys.stderr.write ("Using: Input file: " + inputFile + "\nOutput file: " + outputFile + "\nResolution: " + str(transformedWidth) + "x" + str(transformedHeight) + "\n")


  transformedPoints = [(0,0), (transformedWidth,0), (transformedWidth,transformedHeight), (0,transformedHeight)]
  originalImage = Image.open(inputFile)
  originalPoints = showPointsSelection(array(originalImage))
  
  #originalPoints = [(0,0), (transformedWidth,0), (transformedWidth,transformedHeight), (0,transformedHeight)]

  timeStartMatrixCalculation = time.clock()
  hMatrix = calculateHMatrix (originalPoints, transformedPoints)
  timeEndMatrixCalculation = time.clock()
  
  timeStartTransformation = time.clock()
  transformedImage = transformImage ((transformedWidth, transformedHeight), originalImage, hMatrix, enableInterpolation)
  timeEndTransformation = time.clock()
  Image.fromarray(transformedImage).save (outputFile)

  timeEnd = time.clock()
  #print ("Displaying image")
  #imshow (transformedImage)
  #ginput (3)
  
  print ("CPU time for transformation matrix calculation: %f" % (timeEndMatrixCalculation - timeStartMatrixCalculation))
  print ("CPU time for transformation: %f." % (timeEndTransformation - timeStartTransformation))
  print ("Total CPU time: %f.\n" % (timeEnd - timeStart))
