#pragma once

#include "staticHeaders.h"

#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <set>
#include <fstream>
#include <iostream>

using namespace std;
using namespace cv;
/*
 *  CurveCSS.h
 *  CurveMatching
 *
 *  Created by Roy Shilkrot on 11/28/12.
 *
 */

#pragma mark Curves Utilities
//template<typename T, typename V>
void PolyLineSplit(const vector<cv::Point2d >& pl,vector<double>& contourx, vector<double>& contoury) {
	contourx.resize(pl.size()); 
	contoury.resize(pl.size());
	
	for (int j=0; j<pl.size(); j++) 
	{ 
		contourx[j] = (double)(pl[j].x); 
		contoury[j] = (double)(pl[j].y); 
	}
}

//template<typename T, typename V>
void PolyLineMerge(vector<cv::Point2d >& pl, const vector<double>& contourx, const vector<double>& contoury) {
	assert(contourx.size()==contoury.size());
	pl.resize(contourx.size());
	for (int j=0; j<contourx.size(); j++) {
		pl[j].x = (double)(contourx[j]);
		pl[j].y = (double)(contoury[j]);
	}
}

//template<typename T, typename V>
void ConvertCurve(const vector<cv::Point2d >& curve, vector<cv::Point2d >& output) {
	output.clear();
	for (int j=0; j<curve.size(); j++) {
		output.push_back(cv::Point2d(curve[j].x,curve[j].y));
	}
}

void ResampleCurve(const vector<double>& curvex, const vector<double>& curvey,
				   vector<double>& resampleX, vector<double>& resampleY,
				   int N, bool isOpen = false
				   );

//template<typename T>
void drawOpenCurve(Mat& img, const vector<cv::Point2d >& curve, Scalar color, int thickness) {
	vector<cv::Point2d> curve2i;
	ConvertCurve(curve, curve2i);
	for (int i=0; i<curve2i.size()-1; i++) {
		line(img, curve2i[i], curve2i[i+1], color, thickness);
	}
}

#pragma mark CSS Image

void ComputeCurveCSS(const vector<double>& curvex, 
					 const vector<double>& curvey, 
					 vector<double>& kappa, 
					 vector<double>& smoothX,vector<double>& smoothY,
					 double sigma = 1.0,
					 bool isOpen = false);

vector<int> FindCSSInterestPoints(const vector<double>& kappa);

vector<int> ComputeCSSImageMaximas(const vector<double>& contourx_, const vector<double>& contoury_,
								   vector<double>& contourx, vector<double>& contoury, bool isClosedCurve = true);

template<typename T>
void ComputeCurveCSS(const vector<cv::Point2d >& curve, 
					 vector<double>& kappa, 
					 vector<cv::Point2d >& smooth,
					 double sigma,
					 bool isOpen = false
					 ) 
{
	vector<double> contourx(curve.size()),contoury(curve.size());
	PolyLineSplit(curve, contourx, contoury);
	
	vector<double> smoothx, smoothy;
	ComputeCurveCSS(contourx, contoury, kappa, smoothx, smoothy, sigma, isOpen);
	
	PolyLineMerge(smooth, smoothx, smoothy);	
}

#pragma mark Curve Segments

//template<typename T, typename V>
void GetCurveSegments(const vector<cv::Point2d >& curve, const vector<int>& interestPoints, vector<vector<cv::Point2d > >& segments, bool closedCurve = true) {
	if (closedCurve) {
		segments.resize(interestPoints.size());
	} else {
		segments.resize(interestPoints.size()+1);
	}

	for (int i = (closedCurve)?0:1; i<segments.size()-1; i++) {
		int intpt_idx = (closedCurve)?i:i-1;
		segments[i].clear();
		for (int j=interestPoints[intpt_idx]; j<interestPoints[intpt_idx+1]; j++) {
			segments[i].push_back(cv::Point2d(curve[j].x,curve[j].y));
		}
	}
	if (closedCurve) {
		//put in the segment that passes the 0th point
		segments.back().clear();
		for (int j=interestPoints.back(); j<curve.size(); j++) {
			segments.back().push_back(cv::Point2d(curve[j].x,curve[j].y));
		}
		for (int j=0; j<interestPoints[0]; j++) {
			segments.back().push_back(cv::Point2d(curve[j].x,curve[j].y));
		}
	} else {
		//put in the segment after the last point
		segments.back().clear();
		for (int j=interestPoints.back(); j<curve.size(); j++) {
			segments.back().push_back(cv::Point2d(curve[j].x,curve[j].y));
		}
		//put in the segment before the 1st point
		segments.front().clear();
		for (int j=0; j<interestPoints[0]; j++) {
			segments.front().push_back(cv::Point2d(curve[j].x,curve[j].y));
		}
	}
	for (int i=0; i<segments.size(); i++) {
		vector<double> x,y;
		cout <<"segments[i].size() " << segments[i].size() << endl;
		PolyLineSplit(segments[i], x, y); ResampleCurve(x, y, x, y, 50,true); PolyLineMerge(segments[i], x, y);
	}
}
//template<typename T, typename V>
void GetCurveSegmentsWithCSSImage(vector<cv::Point2d >& curve, vector<int>& interestPoints, vector<vector<cv::Point2d > >& segments, bool closedCurve = true) {
	vector<double> contourx(curve.size()),contoury(curve.size());
	PolyLineSplit(curve, contourx, contoury);
	
	vector<double> smoothx, smoothy;
	interestPoints = ComputeCSSImageMaximas(contourx, contoury, smoothx, smoothy);
	
	PolyLineMerge(curve, smoothx, smoothy);
	
	double minx,maxx; minMaxLoc(smoothx, &minx, &maxx);
	double miny,maxy; minMaxLoc(smoothy, &miny, &maxy);
	Mat drawing(maxy,maxx,CV_8UC3,Scalar(0));
	RNG rng(time(NULL));
	Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
//	vector<vector<cv::Point2d > > contours(1,curve);
//	drawContours( drawing, contours, 0, color, 2, 8);
	drawOpenCurve(drawing, curve, color, 2);
	
	for (int m=0; m<interestPoints.size() ; m++) {
		circle(drawing, curve[interestPoints[m]], 5, Scalar(0,255), CV_FILLED);
	}
	imshow("curve interests", drawing);
	waitKey();
	
	GetCurveSegments(curve, interestPoints, segments, closedCurve);
}

#pragma mark Matching

double MatchTwoSegments(const vector<Point2d>& a, const vector<Point2d>& b);
double MatchCurvesSmithWaterman(const vector<vector<Point2d> >& a, const vector<vector<Point2d> >& b, vector<Point>& traceback); 
double AdaptedMatchCurvesSmithWaterman(const vector<vector<Point2d> >& a, const vector<vector<Point2d> >& b, vector<Point>& traceback);





//
//class atfx
//{
//public:
//	atfx(void);
//	~atfx(void);
//
////	short mat_image[1280][960] ;
////	short  mat_image_without_borders[1280][960];
//
//	 cv::Mat sketch;
//  cv::Mat step_image;
// cv::Mat binarizedImage;
//  cv::Mat gaussianImage;
//  cv::Mat thinnedImage;
//  int sigma_max;
//  int best_sigma;
//  int estimated_automatic_width;
//
//  void setInputSketch(cv::Mat image);
//
//  cv::Mat applyBinarization();
//
//    cv::Mat applyGaussian(double sigma);
//
//	  cv::Mat applyThinning();
//
//  int calculateRequiredThinningIterations(cv::Mat image) ;
//    cv::Point pointClockwiseIndex(int index);
//
//	  int blackToWhiteTransitionCount(cv::Mat image, cv::Point p) ;
//	    double getSensitivityMeasurement(cv::Mat sketch, cv::Mat skeleton) ;
//
//
//    cv::Mat getATFImage();
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//};

/*

public class zd_thin_a
{
  Mat sketch;
  Mat step_image;
  Mat binarizedImage;
  Mat gaussianImage;
  Mat thinnedImage;
  int sigma_max;
  int best_sigma;
  int estimated_automatic_width;

  void setInputSketch(Mat image)
  {
    this.sketch = image;
    this.step_image = image;
  }

  Mat applyBinarization() {
   Size size =this.step_image.size();
    Mat rgb = Mat.zeros(size, step_image.type());

    for (int i = 0; i < this.step_image.height(); i++) {
      for (int j = 0; j < this.step_image.width(); j++) {
        rgb.put (i, j,this.step_image.get( i, j));
      }
    }

    Mat gray =Mat.zeros(size,0); //opencv_core.cvCreateImage(size, 8, 1);
  //  opencv_imgproc.cvCvtColor(rgb, gray, 7);
    Imgproc.cvtColor(rgb, gray, Imgproc.COLOR_RGB2GRAY, 1);
    
    
    this.binarizedImage =new Mat();// opencv_core.cvCreateImage(size, 8, 1);
   
    Imgproc.threshold(gray, this.binarizedImage, 0.0D, 255.0D, 8);
   // opencv_imgproc.cvThreshold(gray, this.binarizedImage, 0.0D, 255.0D, 8);

   // opencv_core.cvReleaseImage(rgb);
   // opencv_core.cvReleaseImage(gray);

    this.step_image = this.binarizedImage;

    return this.binarizedImage;
  }

  Mat applyGaussian(double sigma)
  {
    this.gaussianImage =new Mat();// opencv_core.cvCreateImage(opencv_core.cvGetSize(this.step_image), this.step_image.depth(), this.step_image.nChannels());
    double sigma_from_width = 0.3D * (sigma * 0.5D - 1.0D) + 0.4D;
    Size ksize= new Size( sigma_from_width, sigma_from_width );
	Imgproc.blur(this.step_image, this.gaussianImage, ksize) ;//.cvSmooth(this.step_image, this.gaussianImage, 2, 0, 0, sigma_from_width, sigma_from_width);
    this.step_image = this.gaussianImage;
    return this.gaussianImage;
  }

  Mat applyThinning()
  {
    this.thinnedImage = this.step_image;

    zd_thin alg = new zd_thin();
    alg.setImage(this.thinnedImage);
    this.step_image = alg.getThinnedImage();
    return this.step_image;
  }

  int calculateRequiredThinningIterations(Mat image) {
    int[][] mat_image = new int[image.height()][image.width()];
    int[][] mat_image_without_borders = new int[image.height()][image.width()];

    for (int i = 0; i < image.height(); i++) {
      for (int j = 0; j < image.width(); j++)
      {
        if (image.get( i, j)[0]< 127.0D)
          mat_image[i][j] = 1;
        else {
          mat_image[i][j] = 0;
        }

        mat_image_without_borders[i][j] = 0;
      }

    }

    int removing_iterations = 0;
    boolean more_blacks;
    do { more_blacks = false;

      for (int i = 1; i < image.height() - 1; i++) {
        for (int j = 1; j < image.width() - 1; j++) {
          if (mat_image[i][j] == 1) {
            more_blacks = true;

            boolean is_border_pixel = false;
            for (int x = i - 1; x <= i + 1; x++) {
              for (int y = j - 1; y <= j + 1; y++) {
                if (mat_image[x][y] == 0) {
                  is_border_pixel = true;
                }
              }
            }

            if (is_border_pixel)
              mat_image_without_borders[i][j] = 0;
            else
              mat_image_without_borders[i][j] = 1;
          }
          else {
            mat_image_without_borders[i][j] = 0;
          }
        }
      }

      for (int i = 1; i < image.height() - 1; i++) {
        for (int j = 1; j < image.width() - 1; j++) {
          mat_image[i][j] = mat_image_without_borders[i][j];
        }
      }
      removing_iterations++; }
    while (more_blacks);

    return removing_iterations * 2 + 1;
  }

  Point pointClockwiseIndex(int index) {
    switch (index) {
    case 0:
      return new Point(0, 0);
    case 1:
      return new Point(-1, 0);
    case 2:
      return new Point(-1, 1);
    case 3:
      return new Point(0, 1);
    case 4:
      return new Point(1, 1);
    case 5:
      return new Point(1, 0);
    case 6:
      return new Point(1, -1);
    case 7:
      return new Point(0, -1);
    case 8:
      return new Point(-1, -1);
    }

    return new Point(-2, -2);
  }

  int blackToWhiteTransitionCount(Mat image, Point p) {
    short counter = 0;
    for (int index = 1; index < 9; index++) {
      Point first_point = pointClockwiseIndex(index);
      boolean first_condition = image.get((int)(p.x + first_point.x),(int)( p.y + first_point.y))[0] == 0.0D;
      Point second_point;
      //Point second_point;
      if (index + 1 == 9)
        second_point = pointClockwiseIndex(1);
      else {
        second_point = pointClockwiseIndex(index + 1);
      }
      boolean second_condition = image.get((int)( p.x + second_point.x),(int)( p.y + second_point.y))[0] == 255.0D;

      if ((first_condition) && (second_condition)) {
        counter = (short)(counter + 1);
      }
    }

    return counter;
  }

  double getSensitivityMeasurement(Mat sketch, Mat skeleton) {
    double SM = 0.0D;
    double total_black_pixel_count = 1.0D;

    for (int i = 1; i < skeleton.height() - 1; i++) {
      for (int j = 1; j < skeleton.width() - 1; j++) {
        if (skeleton.get( i, j)[0] == 0.0D) {
          total_black_pixel_count += 1.0D;
          if ((blackToWhiteTransitionCount(skeleton, new Point(i, j)) > 2) || (sketch.get( i, j)[0] != 0.0D) || (blackToWhiteTransitionCount(skeleton, new Point(i, j)) == 0)) {
            SM += 1.0D;
          }
        }
      }
    }

    double ratio = SM / total_black_pixel_count;

    double toReturn = Math.floor(ratio) + Math.floor(ratio * 1000.0D) / 1000.0D;

    return toReturn;
  }

  public Mat getATFImage()
  {
    int estimated_width = calculateRequiredThinningIterations(this.sketch);

    List image_list = new ArrayList();

    List SM_list = new ArrayList();

    applyBinarization();

    applyThinning();
    SM_list.add(Double.valueOf(getSensitivityMeasurement(this.sketch, this.step_image)));

    image_list.add(this.step_image);

    this.step_image = this.sketch;

    List sigma_list = new ArrayList();
    sigma_list.add(Integer.valueOf(0));

    for (int index = 1; index <= estimated_width; index++)
    {
      int width = index * 2 + 1;
      int i = index;

      applyGaussian(width);

      sigma_list.add(Integer.valueOf(i));

      applyBinarization();

      applyThinning();

      SM_list.add(Double.valueOf(getSensitivityMeasurement(this.sketch, this.step_image)));

      image_list.add(this.step_image);

      if (this.gaussianImage != null) {
        // opencv_core.cvReleaseImage(this.gaussianImage);
      }

      this.step_image = this.sketch;
    }

    int SM_index_min = 0;
    int N = SM_list.size();
    for (Iterator i$ = sigma_list.iterator(); i$.hasNext(); ) { int sigma_index = ((Integer)i$.next()).intValue();
      if (((Double)SM_list.get(sigma_index)).doubleValue() < ((Double)SM_list.get(SM_index_min)).doubleValue()) {
        SM_index_min = sigma_index;
      }
    }

    for (int index = 0; index < N; index++) {
      if (index != SM_index_min) {
        Mat image_to_release = (Mat)image_list.get(index);
       // opencv_core.cvReleaseImage(image_to_release);
      }
    }

    Mat image = (Mat)image_list.get(SM_index_min);
    return image;
  }
}

*/