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

Mat *calculateTransformationMatrix (const vector<CvPoint>& originalPoints, const vector<CvPoint>& transformedPoints, bool interpolation = true) {
  
  // in Ax = 0
  Matx<double, 8, 9> A;
  for (int i = 0; i < 4; i++) {
    ((Mat)A).at<double>(2*i, 0) = 0;
    ((Mat)A).at<double>(2*i, 1) = 0;
    ((Mat)A).at<double>(2*i, 2) = 0;
    ((Mat)A).at<double>(2*i, 3) = -originalPoints[i].x;
    ((Mat)A).at<double>(2*i, 4) = -originalPoints[i].y;
    ((Mat)A).at<double>(2*i, 5) = -1;
    ((Mat)A).at<double>(2*i, 6) = transformedPoints[i].y * originalPoints[i].x;
    ((Mat)A).at<double>(2*i, 7) = transformedPoints[i].y * originalPoints[i].y;
    ((Mat)A).at<double>(2*i, 8) = transformedPoints[i].y;
    
    ((Mat)A).at<double>(2*i+1, 0) = originalPoints[i].x;
    ((Mat)A).at<double>(2*i+1, 1) = originalPoints[i].y;
    ((Mat)A).at<double>(2*i+1, 2) = 1;
    ((Mat)A).at<double>(2*i+1, 3) = 0;
    ((Mat)A).at<double>(2*i+1, 4) = 0;
    ((Mat)A).at<double>(2*i+1, 5) = 0;
    ((Mat)A).at<double>(2*i+1, 6) = -transformedPoints[i].x * originalPoints[i].x;
    ((Mat)A).at<double>(2*i+1, 7) = -transformedPoints[i].x * originalPoints[i].y;
    ((Mat)A).at<double>(2*i+1, 8) = -transformedPoints[i].x;
  }

  SVD svd = SVD((Mat)A, SVD::FULL_UV);
  Mat *transformationMatrix = new Mat(3, 3, CV_32F);;

  for (int i = 0; i < 9; i++) {
    cout << (svd.vt).at<double>(i, svd.vt.size().width -1) << "\n";
    transformationMatrix->at<float>(i) = (svd.vt).at<double>(i, svd.vt.size().width -1);
  }

  
  return transformationMatrix;
}

Mat *transformImage (const Mat& originalImage, int width, int height, const Mat& transformationMatrix, bool enableInterpolation = true) {
  
  Mat *transformedImage = new Mat (height, width, originalImage.type());
  Mat transformationMatrixInv = transformationMatrix.inv();

  for (int y = 0; y < height; y++) {
    //cerr << "Progress: " << (float)((float)y/(float)height * 100) << "\n";
    for (int x = 0; x < width; x++) {
      //Mat pointTransformed (3, 1, transformationMatrix.type());
      //pointTransformed.at<float>(0)= x;
      //pointTransformed.at<float>(1)= y;
      //pointTransformed.at<float>(2)= 1;
      Matx<float, 3, 1> pointTransformed (x, y, 1);
      Mat pointOriginal = transformationMatrixInv * (Mat)pointTransformed;

      double t[2] = { 
        ((Mat)pointOriginal).at<float>(0)/((Mat)pointOriginal).at<float>(2),
        ((Mat)pointOriginal).at<float>(1)/((Mat)pointOriginal).at<float>(2)
      };

      cout << "(t1, t2): " << t[0] << t[1] << "\n";


      transformedImage->at<float>(y, x) = originalImage.at<float>((int)t[1], (int)t[0]);
    }
  }

  return transformedImage;
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
  //Mat *transformedImage = new Mat (outputHeight, outputWidth, originalImage.type());

  vector<CvPoint> transformedPoints;
  transformedPoints.push_back(cvPoint(0, 0));
  transformedPoints.push_back(cvPoint(outputWidth, 0));
  transformedPoints.push_back(cvPoint(outputWidth, outputHeight));
  transformedPoints.push_back(cvPoint(0, outputHeight));



  if (!originalImage.data) {
    cerr << "Could not open image: " << inputImageName << "\n";
    return -1;
  }

  // First, we need to display the image to the user
  namedWindow ("Points selection", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
  // Next, we have to pick up selected points
  CvPoint selectedPoint;
  cvSetMouseCallback ("Points selection", mouseHandler, &selectedPoint);

  imshow ("Points selection", originalImage);

  vector<CvPoint> selectedPoints;// = new vector();
  int pointsClicked;
  CvScalar pointColor = cvScalar (0, 0, 255);
  for (pointsClicked = 0; pointsClicked < 4; pointsClicked++) {
    selectedPoint.x = -1;
    while (selectedPoint.x == -1) {
      waitKey(10);
    }

    circle (originalImage, selectedPoint, 5, pointColor, CV_FILLED);
    imshow ("Points selection", originalImage);
    
    selectedPoints.push_back (selectedPoint);

  }

  cout << "Points: " << "\n";
  for (int i = 0; i < 4; i++) {
    CvPoint p = selectedPoints[i];
    cout << "(" << p.x << ", " << p.y << ")" << "\n";
  }

  Mat *transformationMatrix = calculateTransformationMatrix (selectedPoints, transformedPoints);
  Mat *transformedImage = transformImage (originalImage, outputWidth, outputHeight, *transformationMatrix);

  //namedWindow ("Result", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
  //imshow ("Result", *transformedImage);

  waitKey(0);
  
  return 0;
}
