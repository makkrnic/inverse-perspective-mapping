#include "inverseMapping.h"

using namespace std;
using namespace cv;

void mouseHandler (int event, int x, int y, int flags, void *param) {
  switch (event) {
    case CV_EVENT_LBUTTONDOWN:
      ((CvPoint*)param)->x = x;
      ((CvPoint*)param)->y = y;
      break;
  }
}

Mat calculateTransformationMatrix (const vector<Point2f>& originalPoints, const vector<Point2f>& transformedPoints, bool interpolation = true) {

  cout << "Original points: " << originalPoints << "\n";
  cout << "Transformed points: " << transformedPoints << "\n";
  
  // in Ax = 0
  //Matx<float, 8, 9> A;
  Mat A (8, 9, CV_32F);
  for (int i = 0; i < 4; i++) {
    A.at<float>(2*i, 0) = 0;
    A.at<float>(2*i, 1) = 0;
    A.at<float>(2*i, 2) = 0;
    A.at<float>(2*i, 3) = -originalPoints[i].x;
    A.at<float>(2*i, 4) = -originalPoints[i].y;
    A.at<float>(2*i, 5) = -1;
    A.at<float>(2*i, 6) = transformedPoints[i].y * originalPoints[i].x;
    A.at<float>(2*i, 7) = transformedPoints[i].y * originalPoints[i].y;
    A.at<float>(2*i, 8) = transformedPoints[i].y;
    
    A.at<float>(2*i+1, 0) = originalPoints[i].x;
    A.at<float>(2*i+1, 1) = originalPoints[i].y;
    A.at<float>(2*i+1, 2) = 1;
    A.at<float>(2*i+1, 3) = 0;
    A.at<float>(2*i+1, 4) = 0;
    A.at<float>(2*i+1, 5) = 0;
    A.at<float>(2*i+1, 6) = -transformedPoints[i].x * originalPoints[i].x;
    A.at<float>(2*i+1, 7) = -transformedPoints[i].x * originalPoints[i].y;
    A.at<float>(2*i+1, 8) = -transformedPoints[i].x;
  }

  cout << "A: " << A << "\n";
  SVD svd = SVD((Mat)A, SVD::FULL_UV);
  Mat *transformationMatrix = new Mat(3, 3, CV_32F);;
  cout << "SVD.vt: " << svd.vt << "\n";

  for (int i = 0; i < 9; i++) {
    cout << (svd.vt).at<float>(8, i) << "\n";
    transformationMatrix->at<float>(i) = (svd.vt).at<float>(8, i);
  }

  *transformationMatrix = transformationMatrix->inv();
  return *transformationMatrix;
}

void transformImage (const Mat& originalImage, Mat& transformedImage, const Mat& transformationMatrix, bool enableInterpolation = true) {
  
  Mat transformationMatrixInv = transformationMatrix;
  CvSize transformedSize = transformedImage.size();
  CvSize originalSize = originalImage.size();

  for (int y = 0; y <transformedSize.height; y++) {
    cerr << "Progress: " << (float)((float)y/((float)transformedSize.height - 1) * 100) << " %\n";
    for (int x = 0; x < transformedSize.width; x++) {
      Mat pointTransformed (3, 1, transformationMatrixInv.type());
      pointTransformed.at<float>(0)= x;
      pointTransformed.at<float>(1)= y;
      pointTransformed.at<float>(2)= 1;
      Mat pointOriginal = transformationMatrixInv * (Mat)pointTransformed;

      Point2f originalPoint ( 
        pointOriginal.at<float>(0)/pointOriginal.at<float>(2),
        pointOriginal.at<float>(1)/pointOriginal.at<float>(2)
      );
      
      if (enableInterpolation
          && (originalPoint.x != (int)originalPoint.x || originalPoint.y != (int)originalPoint.y)
          && originalPoint.x < originalSize.width
          && originalPoint.y < originalSize.height
          ) {

        int xOrigInt = (int)originalPoint.x;
        int yOrigInt = (int)originalPoint.y;

        float dx = originalPoint.x - xOrigInt;
        float dy = originalPoint.y - yOrigInt;

        Vec3b point = originalImage.at<Vec3b>(yOrigInt, xOrigInt) * (1 - dx) * (1 - dy)
                    + originalImage.at<Vec3b>(yOrigInt, xOrigInt + 1) * dx * (1 - dy)
                    + originalImage.at<Vec3b>(yOrigInt + 1, xOrigInt) * (1 - dx) * dy
                    + originalImage.at<Vec3b>(yOrigInt + 1, xOrigInt + 1) * dx * dy;

        transformedImage.at<Vec3b>(y, x) = point;
      }
      else {
        transformedImage.at<Vec3b>(y, x) = originalImage.at<Vec3b>(floor(originalPoint.y), floor(originalPoint.x));
      }
    }
  }

  //cout << "AT: " << hex << originalImage.at<float>(100, 100) << "\n";

  //return transformedImage;
}

int main (int argc, char *argv[]) {
  const char *usage = " <input_file> <output_file> <output_width> <output_height>";
  char *inputImageName;
  
  if (argc < 5) {
    cerr << "Usage: "<< argv[0] << usage << "\n";
    return -1;
  }

  inputImageName = argv[1];

  int outputWidth   = atoi (argv[3]);
  int outputHeight  = atoi (argv[4]);
  Mat originalImage = imread (inputImageName, CV_LOAD_IMAGE_COLOR);
  char *outputImageName = argv[2];
  //Mat *transformedImage = new Mat (outputHeight, outputWidth, originalImage.type());

  vector<Point2f> transformedPoints;
  transformedPoints.push_back(Point2f(0, 0));
  transformedPoints.push_back(Point2f(outputWidth, 0));
  transformedPoints.push_back(Point2f(outputWidth, outputHeight));
  transformedPoints.push_back(Point2f(0, outputHeight));
  bool enableInterpolation = argc < 6;



  if (!originalImage.data) {
    cerr << "Could not open image: " << inputImageName << "\n";
    return -1;
  }

  // First, we need to display the image to the user
  namedWindow ("Points selection", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
  // Next, we have to pick up selected points
  CvPoint selectedPoint;
  cvSetMouseCallback ("Points selection", mouseHandler, &selectedPoint);

  Mat originalImageCopy = originalImage.clone();
  imshow ("Points selection", originalImageCopy);

  vector<Point2f> selectedPoints;// = new vector();
  int pointsClicked;
  CvScalar pointColor = cvScalar (0, 0, 255);
  for (pointsClicked = 0; pointsClicked < 4; pointsClicked++) {
    selectedPoint.x = -1;
    while (selectedPoint.x == -1) {
      waitKey(10);
    }

    circle (originalImageCopy, selectedPoint, 5, pointColor, CV_FILLED);
    imshow ("Points selection", originalImageCopy);
    
    selectedPoints.push_back (selectedPoint);

  }
  
  cout << "Points: " << "\n";
  for (int i = 0; i < 4; i++) {
    Point2f p = selectedPoints[i];
    cout << "(" << p.x << ", " << p.y << ")" << "\n";
  }

  //selectedPoints.push_back (Point2f (162, 55));
  //selectedPoints.push_back (Point2f (286, 28));
  //selectedPoints.push_back (Point2f (186, 332));
  //selectedPoints.push_back (Point2f (23, 342));


  Mat transformationMatrixCustom = calculateTransformationMatrix (selectedPoints, transformedPoints);

  Mat transformedImage (outputHeight, outputWidth, originalImage.type());
  cerr << "Enable interpolation: " << enableInterpolation << "\n";
  transformImage (originalImage, transformedImage, transformationMatrixCustom, enableInterpolation);
  
  namedWindow ("newimage", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
  imshow ("newimage", transformedImage);

  try {
    imwrite (outputImageName, transformedImage);
  }
  catch (Exception e) {
    cerr << "Error saving image" << "\n";
  }

  char c;
  do {
    c = waitKey(0);
  } while (c != 27);
  
  return 0;
}
