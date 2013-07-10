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
  Vec3b blankPoint (0, 0, 0);

  for (int y = 0; y <transformedSize.height; y++) {
    //cerr << "Progress: " << (float)((float)y/((float)transformedSize.height - 1) * 100) << " %\n";
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
      
      if (originalPoint.x < 0 || originalPoint.x > originalSize.width
        ||originalPoint.y < 0 || originalPoint.y > originalSize.height) {
        
        transformedImage.at<Vec3b>(y, x) = blankPoint;
      }
      else if (enableInterpolation
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
  string inputImageName;
  
  if (argc < 5) {
    cerr << "Usage: "<< argv[0] << usage << "\n";
    return -1;
  }

  inputImageName = argv[1];
  //inputImageName = "samples/road.avi";

  int outputWidth   = atoi (argv[3]);
  int outputHeight  = atoi (argv[4]);
  //Mat originalImage = imread (inputImageName, CV_LOAD_IMAGE_COLOR);
  Mat originalImage;
  char *outputFileName = argv[2];
  Mat *transformedImage;
  //Mat *transformedImage = new Mat (outputHeight, outputWidth, originalImage.type());

  vector<Point2f> transformedPoints;
  transformedPoints.push_back(Point2f(0, 0));
  transformedPoints.push_back(Point2f(outputWidth, 0));
  transformedPoints.push_back(Point2f(outputWidth, outputHeight));
  transformedPoints.push_back(Point2f(0, outputHeight));
  bool enableInterpolation = argc < 6;


  // video capture

  VideoCapture cap(inputImageName);
  if (!cap.isOpened()) {
    cerr << "error opening default camera\n";
    return -1;
  }

  Mat edges;
  namedWindow ("edges", CV_WINDOW_KEEPRATIO|CV_WINDOW_NORMAL);
  namedWindow ("transformed", CV_WINDOW_KEEPRATIO|CV_WINDOW_NORMAL);
  Mat frame;
  cap >> frame;
  //Canny(edges, edges, 0, 30, 3);
  imshow("edges",frame);
  imshow("transformed",frame);
  originalImage = frame;

  // First, we need to display the image to the user
  namedWindow ("edges", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
  // Next, we have to pick up selected points
  CvPoint selectedPoint;
  cvSetMouseCallback ("edges", mouseHandler, &selectedPoint);
  
  Mat originalImageCopy = originalImage.clone();
  imshow ("edges", originalImageCopy);
  
  vector<Point2f> selectedPoints;// = new vector();
  int pointsClicked;
  CvScalar pointColor = cvScalar (0, 0, 255);
  for (pointsClicked = 0; pointsClicked < 4; pointsClicked++) {
    selectedPoint.x = -1;
    while (selectedPoint.x == -1) {
      waitKey(10);
    }
  
    circle (originalImageCopy, selectedPoint, 5, pointColor, CV_FILLED);
    imshow ("edges", originalImageCopy);
    
    selectedPoints.push_back (selectedPoint);
  
  }
  
  cout << "Points: " << "\n";
  for (int i = 0; i < 4; i++) {
    Point2f p = selectedPoints[i];
    cout << "(" << p.x << ", " << p.y << ")" << "\n";
  }


  Mat transformationMatrixCustom = calculateTransformationMatrix (selectedPoints, transformedPoints);

  transformedImage = new Mat (outputHeight, outputWidth, originalImage.type());

  //string outputFileName = "out.mpg";

  VideoWriter vw (outputFileName, CV_FOURCC('M','J','P','G'), 25, frame.size(), true);
  if (!vw.isOpened()) {
    cerr << "error opening file for writing\n";
    return -3;

  }
  //vw->open
  while (true) {
    //Mat frame;
    cap >> frame;
    //Canny(edges, edges, 0, 30, 3);
    Mat transformedImage (outputHeight, outputWidth, frame.type());

    transformImage (frame, transformedImage, transformationMatrixCustom, enableInterpolation);

    imshow("transformed",transformedImage);
    imshow ("edges", frame);
    resizeWindow ("transformed", 500, 500);
    vw.write(frame);
    if (waitKey(30) == 27) break;
  }

  return 0;
}
