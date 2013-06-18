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

int main (int argc, char *argv[]) {
  const char *usage = " <input_file>";
  char *inputImageName;
  
  if (argc < 2) {
    cerr << "Usage: "<< argv[0] << usage << "\n";
    return -1;
  }

  inputImageName = argv[1];

  Mat originalImage = imread (inputImageName, CV_LOAD_IMAGE_COLOR);

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
      waitKey(1);
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


  
  //waitKey (0);

  return 0;
}
