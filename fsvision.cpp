#include "fsvision.h"
#include "fslaser.h"
#include "fswebcam.h"
#include "fsturntable.h"
#include "fscontroller.h"

#include <assert.h>
#include <QDebug>

class FSLaser;
class FSController;

FSVision::FSVision()
{

}

extern int centerPos;

FSPoint FSVision::convertCvPointToFSPoint(CvPoint cvPoint)
{
  CvSize cvImageSize = cvSize(FSController::config->CAM_IMAGE_WIDTH, FSController::config->CAM_IMAGE_HEIGHT);
  FSSize fsImageSize = FSMakeSize(FSController::config->FRAME_WIDTH,
                                  (double)FSController::config->FRAME_WIDTH*(double)(FSController::config->CAM_IMAGE_HEIGHT/(double)FSController::config->CAM_IMAGE_WIDTH), 0.0f);

  //here we define the origin of the cvImage, we place it in the middle of the frame and in the corner of the two perpendiculair planes
  CvPoint origin;
  origin.x = (cvImageSize.width/2.0f) + centerPos; // dschae
  origin.y = cvImageSize.height*FSController::config->ORIGIN_Y;

  FSPoint fsPoint;
  //translate
  cvPoint.x -= origin.x;
  cvPoint.y -= origin.y;
  //scale
  fsPoint.x = cvPoint.x*fsImageSize.width/cvImageSize.width;
  fsPoint.y = -cvPoint.y*fsImageSize.height/cvImageSize.height;
  fsPoint.z = 0.0f;

  return fsPoint;
}

CvPoint FSVision::convertFSPointToCvPoint(FSPoint fsPoint)
{
  CvSize cvImageSize = cvSize(
              FSController::config->CAM_IMAGE_WIDTH,
              FSController::config->CAM_IMAGE_HEIGHT);
  FSSize fsImageSize = FSMakeSize(
              FSController::config->FRAME_WIDTH,
              FSController::config->FRAME_WIDTH*(
                  FSController::config->CAM_IMAGE_HEIGHT/FSController::config->CAM_IMAGE_WIDTH), 0.0f);
  CvPoint origin;
  origin.x = cvImageSize.width/2.0f;
  origin.y = cvImageSize.height*FSController::config->ORIGIN_Y;

  CvPoint cvPoint;

  cvPoint.x = fsPoint.x*cvImageSize.width/fsImageSize.width;
  cvPoint.y = -fsPoint.y*cvImageSize.height/fsImageSize.height;

  //translate
  cvPoint.x += origin.x;
  cvPoint.y += origin.y;

  return cvPoint;
}

cv::Mat FSVision::diffImage(cv::Mat &laserOff, cv::Mat &laserOn)
{
    unsigned int cols = laserOff.cols;
    unsigned int rows = laserOff.rows;
    cv::Mat bwLaserOff( cols,rows,CV_8U,cv::Scalar(100) );
    cv::Mat bwLaserOn( cols,rows,CV_8U,cv::Scalar(100) );
    cv::Mat diffImage( cols,rows,CV_8U,cv::Scalar(100) );
    cv::Mat result( cols,rows,CV_8UC3,cv::Scalar(100) );
    cv::cvtColor(laserOff, bwLaserOff, CV_RGB2GRAY); //convert to grayscale
    cv::cvtColor(laserOn, bwLaserOn, CV_RGB2GRAY); //convert to grayscale
    cv::subtract(bwLaserOn,bwLaserOff,diffImage); //subtract both grayscales
    cv::cvtColor(diffImage, result, CV_GRAY2RGB); //convert back ro rgb
    return result;
}

cv::Mat FSVision::subLaser(cv::Mat &laserOff, cv::Mat &laserOn, FSFloat threshold)
{
    unsigned int cols = laserOff.cols;
    unsigned int rows = laserOff.rows;
    cv::Mat bwLaserOff( cols,rows,CV_8U,cv::Scalar(100) );
    cv::Mat bwLaserOn( cols,rows,CV_8U,cv::Scalar(100) );
    cv::Mat diffImage( cols,rows,CV_8U,cv::Scalar(100) );
    cv::Mat treshImage( cols,rows,CV_8U,cv::Scalar(100) );
    cv::Mat result( cols,rows,CV_8UC3,cv::Scalar(100) );

    cv::cvtColor(laserOff, bwLaserOff, CV_RGB2GRAY); //convert to grayscale
    cv::cvtColor(laserOn, bwLaserOn, CV_RGB2GRAY); //convert to grayscale
    cv::subtract(bwLaserOn,bwLaserOff,diffImage); //subtract both grayscales
    cv::GaussianBlur(diffImage,diffImage,cv::Size(5,5),3); //gaussian filter
    cv::threshold(diffImage,treshImage,threshold,255,cv::THRESH_BINARY); //apply threshold
    cv::Mat element5(3,3,CV_8U,cv::Scalar(1));
    cv::morphologyEx(treshImage,treshImage,cv::MORPH_OPEN,element5);

    cv::cvtColor(treshImage, result, CV_GRAY2RGB); //convert back ro rgb
    /*cv::namedWindow("laserLine");
    cv::imshow("laserLine", result);
    cv::waitKey(0);
    cv::destroyWindow("laserLine");*/
    return result;
}

cv::Mat FSVision::histogram(cv::Mat &img)
{
    /// Separate the image in 3 places ( B, G and R )

      /// Establish the number of bins
      int histSize = 256;

      /// Set the ranges ( for B,G,R) )
      float range[] = { 0, 256 } ;
      const float* histRange = { range };

      bool uniform = true;
      bool accumulate = false;

      cv::Mat hist;

      /// Compute the histograms:
      //cv::calcHist()
      cv::calcHist(&img, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );

      // Draw the histograms for B, G and R
      int hist_w = 512;
      int hist_h = 400;
      int bin_w = cvRound( (double) hist_w/histSize );

      cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar(0,0,0) );

      /// Normalize the result to [ 0, histImage.rows ]
      cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat() );

      /// Draw for each channel
      for( int i = 1; i < histSize; i++ )
      {
          cv::line( histImage, cv::Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
                           cv::Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
                           cv::Scalar( 255, 0, 0), 2, 8, 0  );
      }
    return histImage;
}

cv::Mat FSVision::subLaser2(cv::Mat &laserOff, cv::Mat &laserOn) //This is the routine used to identify laser points
{
    unsigned int cols = laserOff.cols;
    unsigned int rows = laserOff.rows;
	//Create assorted blank frames...
    cv::Mat bwLaserOff( rows,cols,CV_8U,cv::Scalar(100) );	//B/W image of object only
    cv::Mat bwLaserOn( rows,cols,CV_8U,cv::Scalar(100) );	//B/W image of object with laser line
    cv::Mat tresh2Image( rows,cols,CV_8U,cv::Scalar(0) );	//tresh2Image: Threshold?
    cv::Mat diffImage( rows,cols,CV_8U,cv::Scalar(100) );	//Difference between object with laser line and without (i.e. just the laser line)
    cv::Mat gaussImage( rows,cols,CV_8U,cv::Scalar(0) );	//For Gaussian blur
    cv::Mat laserImage( rows,cols,CV_8U,cv::Scalar(0) );
    cv::Mat result( rows,cols,CV_8UC3,cv::Scalar(0) );

    cv::cvtColor(laserOff, bwLaserOff, CV_RGB2GRAY);//convert to grayscale
    cv::cvtColor(laserOn, bwLaserOn, CV_RGB2GRAY); //convert to grayscale
    cv::subtract(bwLaserOn, bwLaserOff, diffImage); //subtract both grayscales
    tresh2Image = diffImage.clone();	//Copy to tresh2Image

if(0) {
	cv::namedWindow("laserLine");
	cv::imshow("laserLine", diffImage);
	cv::waitKey(0);
}
    // Apply the specified morphology operation
    //cv::imshow("laserLine", FSVision::histogram(diffImage));
    //cv::waitKey(0);
    /*int morph_elem = 0;
    int morph_size = 1;
    cv::Mat element = cv::getStructuringElement(
                morph_elem,
                cv::Size( 2*morph_size + 1, 2*morph_size+1 ),
                cv::Point( morph_size, morph_size ) );
    cv::morphologyEx(diffImage, diffImage, cv::MORPH_OPEN, element);*/
    //cv::imshow("laserLine", diffImage);
    //cv::waitKey(0);

	cv::GaussianBlur(diffImage,gaussImage,cv::Size(15,15),12,12);	//Blur image
    diffImage = diffImage-gaussImage;	//Subtract blurred image (simple unsharp-mask?)
if(0){
	cv::imshow("laserLine", diffImage);
	cv::waitKey(0);
}

//Threshold here
    FSFloat threshold = 10;	//This might need to be played with. 0 = Black, 255 = White
    cv::threshold(diffImage,diffImage,threshold,255,cv::THRESH_TOZERO); //apply threshold-to-zero operation. Should it be threshold-binary?
    //cv::imshow("laserLine", FSVision::histogram(diffImage));
    //cv::waitKey(0);
    //cv::equalizeHist(diffImage,diffImage);
    //cv::imshow("laserLine", FSVision::histogram(diffImage));
    //cv::waitKey(0);
    cv::erode(diffImage,diffImage,cv::Mat(3,3,CV_8U,cv::Scalar(1)) );	//Erode: 'grow' dark regions, 'shrink' light regions
    //cv::imshow("laserLine", diffImage);
    //cv::waitKey(0);

    //cv::Mat element5(3,3,CV_8U,cv::Scalar(1));
    //cv::morphologyEx(diffImage,diffImage,cv::MORPH_OPEN,element5);
    //cv::imshow("laserLine", diffImage);
    //cv::waitKey(0);
    cv::Canny(diffImage,diffImage,20,50);	//Edge detection - 'Canny' algorithm
    //cv::imshow("laserLine", diffImage);
    //cv::waitKey(0);
    //cv::imshow("laserLine", treshImage+diffImage);
    //cv::waitKey(0);
    //cv::destroyWindow("laserLine");

	int *edges;
    edges = new int[cols]; //contains the cols index of the detected edges per row
    for(unsigned int y = 0; y <rows; y++){
        //reset the detected edges
        for(unsigned int j=0; j<cols; j++){ edges[j]=-1; }
        int j=0;
        for(unsigned int x = 0; x<cols; x++){
            if(diffImage.at<uchar>(y,x)>250){
                edges[j]=x;
                j++;
            }
        }
        //iterate over detected edges, take middle of two edges
        for(unsigned int j=0; j<cols-1; j+=2){
            if(edges[j]>=0 && edges[j+1]>=0 && edges[j+1]-edges[j]<40){
                int middle = (int)(edges[j]+edges[j+1])/2;
                //qDebug() << cols << rows << y << middle;
                laserImage.at<uchar>(y,middle) = 255;
            }
        }
    }
    /*cv::namedWindow("laserLine");
    cv::imshow("laserLine", diffImage);
    cv::waitKey(0);
    cv::imshow("laserLine", laserImage);
    cv::waitKey(0);
    cv::imshow("laserLine", laser+treshImage);
    cv::waitKey(0);
    cv::destroyAllWindows();*/
    cv::cvtColor(laserImage, result, CV_GRAY2RGB); //convert back ro rgb

    return result;
}

cv::Mat FSVision::drawHelperLinesToFrame(cv::Mat &frame)
{
    //artifical horizont
    cv::line(frame,
             cv::Point(0,frame.rows*FSController::config->ORIGIN_Y),
             cv::Point(frame.cols,frame.rows*FSController::config->ORIGIN_Y),
             CV_RGB( 0,0,255 ), //Blue
             2);

    //two lines for center of frame
    cv::line(frame,
             cv::Point(frame.cols*0.5f,0),	//Vertical from top...
             cv::Point(frame.cols*0.5f,frame.rows), //... to bottom
             CV_RGB( 255,255,0 ), //Yellow
             1);
    cv::line(frame,
             cv::Point(0,frame.rows*0.5f),	//Horizontally across middle
             cv::Point(frame.cols,frame.rows*0.5f),
             CV_RGB( 255,255,0 ), //Yellow
             1);

    //line showing the upper limit where analyzing starts
    cv::line(frame,
             cv::Point(0,frame.rows-FSController::config->LOWER_ANALYZING_FRAME_LIMIT),
             cv::Point(frame.cols,frame.rows-FSController::config->LOWER_ANALYZING_FRAME_LIMIT),
             CV_RGB( 255,255,0 ), //Yellow
             1);

    //line showing the lower limit where analyzing stops
    cv::line(frame,
             cv::Point(0,FSController::config->UPPER_ANALYZING_FRAME_LIMIT),
             cv::Point(frame.cols,FSController::config->UPPER_ANALYZING_FRAME_LIMIT),
             CV_RGB( 255,255,0 ), //Yellow
             1);
    frame = drawLaserLineToFrame(frame); //Red
    return frame;
}

cv::Mat FSVision::drawLaserLineToFrame(cv::Mat &frame)
{
    FSLaser* laser = FSController::getInstance()->laser_0;
    CvPoint cvLaserPoint = convertFSPointToCvPoint(laser->getLaserPointPosition());

    FSFloat vertical    = cvLaserPoint.x;
    FSFloat horizontal  = convertFSPointToCvPoint(FSMakePoint(0,0,0)).y;

    cv::Point p1 = cv::Point(vertical, 0);          //top of laser line
    cv::Point p2 = cv::Point(vertical, horizontal); //bottom of laser line
    cv::line(frame, p1, p2, CV_RGB( 255,0,0 ),6);   //draw laser line
    return frame;
}

#if 0
// This is the original subroutine which uses the backboard to calculate laser line
void FSVision::putPointsFromFrameToCloud(
        cv::Mat &laserOff,
        cv::Mat &laserOn,
        int dpiVertical,    //step between vertical points
        FSFloat lowerLimit) //remove points below this limit
{
    //qDebug() << "putPointsFromFrameToCloud";
    //the following lines are just to make to code more readable
    FSModel* model = FSController::getInstance()->model;
    FSLaser* laser = FSController::getInstance()->laser;
    FSTurntable* turntable = FSController::getInstance()->turntable;

    //extract laser line from the two images
    //cv::Mat laserLine = subLaser(laserOff,laserOn,threshold);
    cv::Mat laserLine = subLaser2(laserOff,laserOn);
	cv::imshow("Laser Frame", laserLine);

    //calculate position of laser in cv frame
    //position of the laser line on the back plane in frame/image coordinates
    FSPoint fsLaserLinePosition = laser->getLaserPointPosition();
    //position of the laser line on the back plane in world coordinates
    CvPoint cvLaserLinePosition = convertFSPointToCvPoint(fsLaserLinePosition);

    FSFloat laserPos = cvLaserLinePosition.x; //const over all y

    //laserLine is result of subLaser2, is in RGB
    unsigned int cols = laserLine.cols;
    unsigned int rows = laserLine.rows;
    //create new image in black&white
    cv::Mat bwImage( cols,rows,CV_8U,cv::Scalar(100) );

    //qDebug("still works here");
    //convert from rgb to b&w
    cv::cvtColor(laserLine, bwImage, CV_RGB2GRAY); //convert to grayscale

    //now iterating from top to bottom over bwLaserLine frame
    //no bear outside of these limits :) cutting of top and bottom of frame
    for(int y = FSController::config->UPPER_ANALYZING_FRAME_LIMIT;
        y < bwImage.rows-(FSController::config->LOWER_ANALYZING_FRAME_LIMIT);
        y+=dpiVertical )
    {
        //qDebug() << "checking point at line " << y << laserPos+ANALYZING_LASER_OFFSET;
        //ANALYZING_LASER_OFFSET is the offset where we stop looking for a reflected laser, cos we might catch the non reflected
        //now iteratinf from right to left over bwLaserLine frame
        for(int x = bwImage.cols-1;
            x >= laserPos+FSController::config->ANALYZING_LASER_OFFSET;
            x -= 1){
            //qDebug() << "Pixel value: " << bwImage.at<uchar>(y,x);
            if(bwImage.at<uchar>(y,x)==255){ //check if white=laser-reflection
                //qDebug() << "found point at x=" << x;
                //if (row[x] > 200){
                //we have a white point in the grayscale image, so one edge laser line found
                //no we should continue to look for the other edge and then take the middle of those two points
                //to take the width of the laser line into account

                //position of the reflected laser line on the image coord
                CvPoint cvNewPoint;
                cvNewPoint.x = x;
                cvNewPoint.y = y;	//x and y as in h and v pixels
                //cout << x << ":" << y << endl;

                //convert to world coordinates withouth depth
                FSPoint fsNewPoint = FSVision::convertCvPointToFSPoint(cvNewPoint);
                //cout << fsNewPoint.x << ":" << fsNewPoint.y << ":" << fsNewPoint.z << endl;
                FSLine l1 = computeLineFromPoints(FSCamPosition, fsNewPoint);
                FSLine l2 = computeLineFromPoints(laser->getPosition(), laser->getLaserPointPosition()); //This needs fixing.
				//Instead of laser->getLaserPointPosition() [already called! see line 310!] we should use the centre of the turntable
				//as the endpoint. Perhaps create a "computeLineFromAngle()" where we supply the absolute angle of the laser to the camera.

                FSPoint i = computeIntersectionOfLines(l1, l2);
                fsNewPoint.x = i.x;
                fsNewPoint.z = i.z;


                //At this point we know the depth=z. Now we need to consider the scaling depending on the depth.
                //First we move our point to a camera centered cartesion system.
                fsNewPoint.y -= FSCamPosition.y;
                fsNewPoint.y *= ((FSCamPosition).z - fsNewPoint.z)/(FSCamPosition).z;
                //Redo the translation to the box centered cartesion system.
                fsNewPoint.y += (FSCamPosition).y;
                //cout << __PRETTY_FUNCTION__ << fsNewPoint.y << endl;

                //get color from picture without laser
                FSUChar r = laserOff.at<cv::Vec3b>(y,x)[2];
                FSUChar g = laserOff.at<cv::Vec3b>(y,x)[1];
                FSUChar b = laserOff.at<cv::Vec3b>(y,x)[0];
                fsNewPoint.color = FSMakeColor(r, g, b);

                //turning new point according to current angle of turntable
                //translate coordinate system to the middle of the turntable
                fsNewPoint.z -= FSController::config->TURNTABLE_POS_Z; //7cm radius of turntbale plus 5mm offset from back plane
                FSPoint alphaDelta = turntable->getRotation();
                FSFloat alphaOld = (float)atan(fsNewPoint.z/fsNewPoint.x);
                FSFloat alphaNew = alphaOld+alphaDelta.y*(M_PI/180.0f);
                FSFloat hypotenuse = (float)sqrt(fsNewPoint.x*fsNewPoint.x + fsNewPoint.z*fsNewPoint.z);

                if(fsNewPoint.z < 0 && fsNewPoint.x < 0){
                    alphaNew += M_PI;
                }else if(fsNewPoint.z > 0 && fsNewPoint.x < 0){
                    alphaNew -= M_PI;
                }
                fsNewPoint.z = (float)sin(alphaNew)*hypotenuse;
                fsNewPoint.x = (float)cos(alphaNew)*hypotenuse;

                if(fsNewPoint.y>lowerLimit+0.5 && hypotenuse < 7){ //eliminate points from the grounds, that are not part of the model
                    //qDebug("adding point");
                    model->addPointToPointCloud(fsNewPoint);
                }
                break;
            }
        }
    }
}
#else // dschae testing
// This is the original subroutine which uses the backboard to calculate laser line
void FSVision::putPointsFromFrameToCloud(FSLaser* laser,
        cv::Mat &laserOff,
        cv::Mat &laserOn,
        int dpiVertical,    //step between vertical points
        FSFloat lowerLimit) //remove points below this limit
{
    //qDebug() << "putPointsFromFrameToCloud";
    //the following lines are just to make to code more readable
    FSModel* model = FSController::getInstance()->model;
//    FSLaser* laser_0 = FSController::getInstance()->laser_0;
    FSTurntable* turntable = FSController::getInstance()->turntable;

    //extract laser line from the two images
    cv::Mat laserLine = subLaser(laserOff,laserOn, 20);
    //cv::Mat laserLine = subLaser2(laserOff,laserOn);
	cv::imshow("Laser Frame", laserLine);

    //calculate position of laser in cv frame
    //position of the laser line on the back plane in frame/image coordinates
    FSPoint fsLaserLinePosition = laser->getLaserPointPosition();
    //position of the laser line on the back plane in world coordinates
    CvPoint cvLaserLinePosition = convertFSPointToCvPoint(fsLaserLinePosition);

    FSFloat laserPos = cvLaserLinePosition.x; //const over all y

    //laserLine is result of subLaser2, is in RGB
    unsigned int cols = laserLine.cols;
    unsigned int rows = laserLine.rows;
    //create new image in black&white
    cv::Mat bwImage( cols,rows,CV_8U,cv::Scalar(100) );

    //qDebug("still works here");
    //convert from rgb to b&w
    cv::cvtColor(laserLine, bwImage, CV_RGB2GRAY); //convert to grayscale

    //now iterating from top to bottom over bwLaserLine frame
    //no bear outside of these limits :) cutting of top and bottom of frame
    for(int y = FSController::config->UPPER_ANALYZING_FRAME_LIMIT;
        y < bwImage.rows-(FSController::config->LOWER_ANALYZING_FRAME_LIMIT);
        y+=dpiVertical )
    {
        //qDebug() << "checking point at line " << y << laserPos+ANALYZING_LASER_OFFSET;
        //ANALYZING_LASER_OFFSET is the offset where we stop looking for a reflected laser, cos we might catch the non reflected
        //now iteratinf from right to left over bwLaserLine frame
        for(int x = bwImage.cols-1;
            x >= laserPos+FSController::config->ANALYZING_LASER_OFFSET;
            x -= 1){
            //qDebug() << "Pixel value: " << bwImage.at<uchar>(y,x);
            if(bwImage.at<uchar>(y,x)==255){ //check if white=laser-reflection
                //qDebug() << "found point at x=" << x;
                //if (row[x] > 200){
                //we have a white point in the grayscale image, so one edge laser line found
                //no we should continue to look for the other edge and then take the middle of those two points
                //to take the width of the laser line into account

                //position of the reflected laser line on the image coord
                CvPoint cvNewPoint;
                cvNewPoint.x = x;
                cvNewPoint.y = y;	//x and y as in h and v pixels
                //cout << x << ":" << y << endl;

                //convert to world coordinates withouth depth
                FSPoint fsNewPoint = FSVision::convertCvPointToFSPoint(cvNewPoint);
                //cout << fsNewPoint.x << ":" << fsNewPoint.y << ":" << fsNewPoint.z << endl;
                FSLine l1 = computeLineFromPoints(FSCamPosition, fsNewPoint);
                FSLine l2 = computeLineFromPoints(laser->getPosition(), laser->getLaserPointPosition()); //This needs fixing.
				//Instead of laser->getLaserPointPosition() [already called! see line 310!] we should use the centre of the turntable
				//as the endpoint. Perhaps create a "computeLineFromAngle()" where we supply the absolute angle of the laser to the camera.

                FSPoint i = computeIntersectionOfLines(l1, l2);
                fsNewPoint.x = i.x;
                fsNewPoint.z = i.z;


                //At this point we know the depth=z. Now we need to consider the scaling depending on the depth.
                //First we move our point to a camera centered cartesion system.
                fsNewPoint.y -= FSCamPosition.y;
                fsNewPoint.y *= ((FSCamPosition).z - fsNewPoint.z)/(FSCamPosition).z;
                //Redo the translation to the box centered cartesion system.
                fsNewPoint.y += (FSCamPosition).y;
                //cout << __PRETTY_FUNCTION__ << fsNewPoint.y << endl;

                //get color from picture without laser
                FSUChar r = laserOff.at<cv::Vec3b>(y,x)[2];
                FSUChar g = laserOff.at<cv::Vec3b>(y,x)[1];
                FSUChar b = laserOff.at<cv::Vec3b>(y,x)[0];
                fsNewPoint.color = FSMakeColor(r, g, b);

                //turning new point according to current angle of turntable
                //translate coordinate system to the middle of the turntable
                fsNewPoint.z -= FSController::config->TURNTABLE_POS_Z; //7cm radius of turntbale plus 5mm offset from back plane
                FSPoint alphaDelta = turntable->getRotation();
                FSFloat alphaOld = (float)atan(fsNewPoint.z/fsNewPoint.x);
                FSFloat alphaNew = alphaOld+alphaDelta.y*(M_PI/180.0f);
                FSFloat hypotenuse = (float)sqrt(fsNewPoint.x*fsNewPoint.x + fsNewPoint.z*fsNewPoint.z);

                if(fsNewPoint.z < 0 && fsNewPoint.x < 0){
                    alphaNew += M_PI;
                }else if(fsNewPoint.z > 0 && fsNewPoint.x < 0){
                    alphaNew -= M_PI;
                }
                fsNewPoint.z = (float)sin(alphaNew)*hypotenuse;
                fsNewPoint.x = (float)cos(alphaNew)*hypotenuse;

                if(fsNewPoint.y>lowerLimit+0.5 && hypotenuse < 7){ //eliminate points from the grounds, that are not part of the model
                    //qDebug("adding point");
                    model->addPointToPointCloud(fsNewPoint);
                }
                break;
            }
        }
    }
}
#endif

//This is my Version2 subroutine which ignores the backboard and uses the turntable centre to calculate the laser line
#if 0
void FSVision::putPointsFromFrameToCloud2(
        cv::Mat &laserOff,
        cv::Mat &laserOn,
        int dpiVertical,    //step between vertical points
        FSFloat lowerLimit) //remove points below this limit
{
    //qDebug() << "putPointsFromFrameToCloud";
    //the following lines are just to make to code more readable
    FSModel* model = FSController::getInstance()->model;
    FSLaser* laser = FSController::getInstance()->laser;
    FSTurntable* turntable = FSController::getInstance()->turntable;
    FSWebCam* webcam = FSController::getInstance()->webcam;

	//FSPoint laserPoint = FSMakePoint(0,0,FSController::config->TURNTABLE_POS_Z); //End laser point is at turntable centre
	// Moved to a better location- creation of fscontroller thread


    //extract laser line from the two images
    //cv::Mat laserLine = subLaser(laserOff,laserOn,threshold);
    cv::Mat laserLine = subLaser2(laserOff,laserOn);

    //calculate position of laser in cv frame
    //position of the laser line on the back plane in frame/image coordinates
    // FSPoint fsLaserLinePosition = laser->getLaserPointPosition();
    //position of the laser line on the back plane in world coordinates
    // CvPoint cvLaserLinePosition = convertFSPointToCvPoint(fsLaserLinePosition);

    // FSFloat laserPos = cvLaserLinePosition.x; //const over all y

    //laserLine is result of subLaser2, is in RGB
    unsigned int cols = laserLine.cols;
    unsigned int rows = laserLine.rows;
    //create new image in black&white
    cv::Mat bwImage( cols,rows,CV_8U,cv::Scalar(100) );
	//Show for the user...
//	cv::imshow("Extracted Frame",bwImage);

    //qDebug("still works here");
    //convert from rgb to b&w
    cv::cvtColor(laserLine, bwImage, CV_RGB2GRAY); //convert to grayscale

	cv::imshow("Laser Frame", laserLine);

    //now iterating from top to bottom over bwLaserLine frame
    //no bear outside of these limits :) cutting of top and bottom of frame
    for(int y = FSController::config->UPPER_ANALYZING_FRAME_LIMIT;
        y < bwImage.rows-(FSController::config->LOWER_ANALYZING_FRAME_LIMIT);
        y += dpiVertical )
    {
        //qDebug() << "checking point at line " << y << laserPos+ANALYZING_LASER_OFFSET;
        //ANALYZING_LASER_OFFSET is the offset where we stop looking for a reflected laser, cos we might catch the non reflected
        //now iteratinf from right to left over bwLaserLine frame
        for(int x = bwImage.cols-1;
            x >= /*laserPos+*/FSController::config->ANALYZING_LASER_OFFSET;
            x -= 1){
            //qDebug() << "Pixel value: " << bwImage.at<uchar>(y,x);
            if(bwImage.at<uchar>(y,x)==255){ //check if white=laser-reflection
                //qDebug() << "found point at x=" << x;
                //if (row[x] > 200){
                //we have a white point in the grayscale image, so one edge laser line found
                //no we should continue to look for the other edge and then take the middle of those two points
                //to take the width of the laser line into account

                //position of the reflected laser line on the image coord
                CvPoint cvNewPoint;
                cvNewPoint.x = x;
                cvNewPoint.y = y;	//x and y as in horizontal and vertical pixels
                //cout << x << ":" << y << endl;

                //convert to world coordinates withouth depth
                FSPoint fsNewPoint = FSVision::convertCvPointToFSPoint(cvNewPoint);
                //cout << fsNewPoint.x << ":" << fsNewPoint.y << ":" << fsNewPoint.z << endl;
                FSLine l1 = computeLineFromPoints(FSCamPosition, fsNewPoint);
                //FSLine l2 = computeLineFromPoints(laser->getPosition(), laser->getLaserPointPosition()); //Try calculating from turntable centre
				FSLine l2 = computeLineFromPoints(laser->getPosition(), FSMakePoint(0,0,0));

                FSPoint i = computeIntersectionOfLines(l1, l2);
                fsNewPoint.x = i.x;
                fsNewPoint.z = i.z;


                //At this point we know the depth=z. Now we need to consider the scaling depending on the depth.
                //First we move our point to a camera centered cartesion system.
                fsNewPoint.y -= (FSCamPosition).y;
                fsNewPoint.y *= ((FSCamPosition).z - fsNewPoint.z)/(FSCamPosition).z;
                //Redo the translation to the box centered cartesion system.
                fsNewPoint.y += (FSCamPosition).y;
                //cout << __PRETTY_FUNCTION__ << fsNewPoint.y << endl;

                //get color from picture without laser
                FSUChar r = laserOff.at<cv::Vec3b>(y,x)[2];
                FSUChar g = laserOff.at<cv::Vec3b>(y,x)[1];
                FSUChar b = laserOff.at<cv::Vec3b>(y,x)[0];
                fsNewPoint.color = FSMakeColor(r, g, b);

                //turning new point according to current angle of turntable
                //translate coordinate system to the middle of the turntable
                fsNewPoint.z -= FSController::config->TURNTABLE_POS_Z; //7cm radius of turntbale plus 5mm offset from back plane
                FSPoint alphaDelta = turntable->getRotation();
                FSFloat alphaOld = (float)atan(fsNewPoint.z/fsNewPoint.x);
                FSFloat alphaNew = alphaOld+alphaDelta.y*(M_PI/180.0f);
                FSFloat hypotenuse = (float)sqrt(fsNewPoint.x*fsNewPoint.x + fsNewPoint.z*fsNewPoint.z);

                if(fsNewPoint.z < 0 && fsNewPoint.x < 0){
                    alphaNew += M_PI;
                }else if(fsNewPoint.z > 0 && fsNewPoint.x < 0){
                    alphaNew -= M_PI;
                }
                fsNewPoint.z = (float)sin(alphaNew)*hypotenuse;
                fsNewPoint.x = (float)cos(alphaNew)*hypotenuse;

                if(fsNewPoint.y>lowerLimit+0.5 && hypotenuse < 7){ //eliminate points from the grounds, that are not part of the model
                    //qDebug("adding point");
                    model->addPointToPointCloud(fsNewPoint);
                }
                break;
            }
        }
    }
}
#else // dschae testing
void FSVision::putPointsFromFrameToCloud2(
	FSLaser* laser,
        cv::Mat &laserOff,
        cv::Mat &laserOn,
        int dpiVertical,    //step between vertical points
        FSFloat lowerLimit) //remove points below this limit
{
    //qDebug() << "putPointsFromFrameToCloud";
    //the following lines are just to make to code more readable
    FSModel* model = FSController::getInstance()->model;
//    FSLaser* laser_0 = FSController::getInstance()->laser_0;
    FSTurntable* turntable = FSController::getInstance()->turntable;
    FSWebCam* webcam = FSController::getInstance()->webcam;

	//FSPoint laserPoint = FSMakePoint(0,0,FSController::config->TURNTABLE_POS_Z); //End laser point is at turntable centre
	// Moved to a better location- creation of fscontroller thread


    //extract laser line from the two images
    //cv::Mat laserLine = subLaser(laserOff,laserOn,threshold);
    cv::Mat laserLine = subLaser2(laserOff, laserOn);

    //calculate position of laser in cv frame
    //position of the laser line on the back plane in frame/image coordinates
    // FSPoint fsLaserLinePosition = laser->getLaserPointPosition();
    //position of the laser line on the back plane in world coordinates
    // CvPoint cvLaserLinePosition = convertFSPointToCvPoint(fsLaserLinePosition);

    // FSFloat laserPos = cvLaserLinePosition.x; //const over all y

    //laserLine is result of subLaser2, is in RGB
    unsigned int cols = laserLine.cols;
    unsigned int rows = laserLine.rows;
    //create new image in black&white
    cv::Mat bwImage( cols,rows,CV_8U,cv::Scalar(100) );
	//Show for the user...
//	cv::imshow("Extracted Frame",bwImage);

    //qDebug("still works here");
    //convert from rgb to b&w
    cv::cvtColor(laserLine, bwImage, CV_RGB2GRAY); //convert to grayscale

	cv::imshow("Laser Frame", laserLine);

	int flip_x = 0;
	FSPoint laserPos = laser->getPosition();
/*
	if(laserPos.x < 0) {
		flip_x = 1;
		laserPos.x = -laserPos.x;
	}
*/
    //now iterating from top to bottom over bwLaserLine frame
    //no bear outside of these limits :) cutting of top and bottom of frame
    for(int y = FSController::config->UPPER_ANALYZING_FRAME_LIMIT;
        y < bwImage.rows-(FSController::config->LOWER_ANALYZING_FRAME_LIMIT);
        y += dpiVertical )
    {
        //qDebug() << "checking point at line " << y << laserPos+ANALYZING_LASER_OFFSET;
        //ANALYZING_LASER_OFFSET is the offset where we stop looking for a reflected laser, cos we might catch the non reflected
        //now iteratinf from right to left over bwLaserLine frame
#if 0 // ?????? ?? ?? ??????
		int xpos = -1;
		if(flip_x) {
			for(int x = 0; x < bwImage.cols - FSController::config->ANALYZING_LASER_OFFSET; x++) {
				if(bwImage.at<uchar>(y,x) == 255) { //check if white=laser-reflection
					xpos = bwImage.cols - x;
					break;
				}
			}
		} else {
			for(int x = bwImage.cols-1; x >= FSController::config->ANALYZING_LASER_OFFSET; x -= 1) {
				if(bwImage.at<uchar>(y,x) == 255) { //check if white=laser-reflection
					xpos = x;
					break;
				}
			}
		}

		if(xpos != -1) {
			//qDebug() << "found point at x=" << x;
            //if (row[x] > 200){
            //we have a white point in the grayscale image, so one edge laser line found
            //no we should continue to look for the other edge and then take the middle of those two points
            //to take the width of the laser line into account

            //position of the reflected laser line on the image coord
            CvPoint cvNewPoint;
            cvNewPoint.x = xpos;
            cvNewPoint.y = y;	//x and y as in horizontal and vertical pixels
            //cout << x << ":" << y << endl;

			//convert to world coordinates withouth depth
            FSPoint fsNewPoint = FSVision::convertCvPointToFSPoint(cvNewPoint);
            //cout << fsNewPoint.x << ":" << fsNewPoint.y << ":" << fsNewPoint.z << endl;
            FSLine l1 = computeLineFromPoints(FSCamPosition, fsNewPoint);
            //FSLine l2 = computeLineFromPoints(laser->getPosition(), laser->getLaserPointPosition()); //Try calculating from turntable centre
//				FSLine l2 = computeLineFromPoints(laser->getPosition(), FSMakePoint(0,0,0));
			FSLine l2 = computeLineFromPoints(laserPos, FSMakePoint(0,0,0));

            FSPoint i = computeIntersectionOfLines(l1, l2);
            fsNewPoint.x = i.x;
            fsNewPoint.z = i.z;


            //At this point we know the depth=z. Now we need to consider the scaling depending on the depth.
            //First we move our point to a camera centered cartesion system.
            fsNewPoint.y -= (FSCamPosition).y;
            fsNewPoint.y *= (FSCamPosition.z - fsNewPoint.z)/(FSCamPosition).z;
            //Redo the translation to the box centered cartesion system.
            fsNewPoint.y += (FSCamPosition).y;
            //cout << __PRETTY_FUNCTION__ << fsNewPoint.y << endl;

            //get color from picture without laser
            FSUChar r = laserOff.at<cv::Vec3b>(y,xpos)[2];
            FSUChar g = laserOff.at<cv::Vec3b>(y,xpos)[1];
            FSUChar b = laserOff.at<cv::Vec3b>(y,xpos)[0];
            if(flip_x) {
				r = laserOff.at<cv::Vec3b>(y,bwImage.cols-xpos)[2];
				g = laserOff.at<cv::Vec3b>(y,bwImage.cols-xpos)[1];
				b = laserOff.at<cv::Vec3b>(y,bwImage.cols-xpos)[0];
			}
			fsNewPoint.color = FSMakeColor(r, g, b);

            //turning new point according to current angle of turntable
            //translate coordinate system to the middle of the turntable
            fsNewPoint.z -= FSController::config->TURNTABLE_POS_Z; //7cm radius of turntbale plus 5mm offset from back plane
            FSPoint alphaDelta = turntable->getRotation();
            FSFloat alphaOld = (float)atan(fsNewPoint.z/fsNewPoint.x);
            FSFloat alphaNew = alphaOld + alphaDelta.y * (M_PI/180.0f);
            FSFloat hypotenuse = (float)sqrt(fsNewPoint.x*fsNewPoint.x + fsNewPoint.z*fsNewPoint.z);

            if(fsNewPoint.z < 0 && fsNewPoint.x < 0){
                alphaNew += M_PI;
            }else if(fsNewPoint.z > 0 && fsNewPoint.x < 0){
                alphaNew -= M_PI;
            }
            fsNewPoint.z = (float)sin(alphaNew)*hypotenuse;
            fsNewPoint.x = (float)cos(alphaNew)*hypotenuse;

            if(fsNewPoint.y > lowerLimit+0.5 && hypotenuse < 7){ //eliminate points from the grounds, that are not part of the model
                //qDebug("adding point");
                if(flip_x)
					fsNewPoint.x = -fsNewPoint.x;
				model->addPointToPointCloud(fsNewPoint);
            }
        }
#else
		for(int x = bwImage.cols-1;
            x >= /*laserPos+*/FSController::config->ANALYZING_LASER_OFFSET;
            x -= 1){
            //qDebug() << "Pixel value: " << bwImage.at<uchar>(y,x);
            if(bwImage.at<uchar>(y,x)==255){ //check if white=laser-reflection
                //qDebug() << "found point at x=" << x;
                //if (row[x] > 200){
                //we have a white point in the grayscale image, so one edge laser line found
                //no we should continue to look for the other edge and then take the middle of those two points
                //to take the width of the laser line into account

                //position of the reflected laser line on the image coord
                CvPoint cvNewPoint;
                cvNewPoint.x = x;
                cvNewPoint.y = y;	//x and y as in horizontal and vertical pixels
                //cout << x << ":" << y << endl;

				//convert to world coordinates withouth depth
                FSPoint fsNewPoint = FSVision::convertCvPointToFSPoint(cvNewPoint);
                //cout << fsNewPoint.x << ":" << fsNewPoint.y << ":" << fsNewPoint.z << endl;
                FSLine l1 = computeLineFromPoints(FSCamPosition, fsNewPoint);
                //FSLine l2 = computeLineFromPoints(laser->getPosition(), laser->getLaserPointPosition()); //Try calculating from turntable centre
//				FSLine l2 = computeLineFromPoints(laser->getPosition(), FSMakePoint(0,0,0));
				FSLine l2 = computeLineFromPoints(laserPos, FSMakePoint(0,0,0));

                FSPoint i = computeIntersectionOfLines(l1, l2);
                fsNewPoint.x = i.x;
                fsNewPoint.z = i.z;


                //At this point we know the depth=z. Now we need to consider the scaling depending on the depth.
                //First we move our point to a camera centered cartesion system.
                fsNewPoint.y -= (FSCamPosition).y;
                fsNewPoint.y *= (FSCamPosition.z - fsNewPoint.z)/(FSCamPosition).z;
                //Redo the translation to the box centered cartesion system.
                fsNewPoint.y += (FSCamPosition).y;
                //cout << __PRETTY_FUNCTION__ << fsNewPoint.y << endl;

                //get color from picture without laser
                FSUChar r = laserOff.at<cv::Vec3b>(y,x)[2];
                FSUChar g = laserOff.at<cv::Vec3b>(y,x)[1];
                FSUChar b = laserOff.at<cv::Vec3b>(y,x)[0];
                fsNewPoint.color = FSMakeColor(r, g, b);

                //turning new point according to current angle of turntable
                //translate coordinate system to the middle of the turntable
                fsNewPoint.z -= FSController::config->TURNTABLE_POS_Z; //7cm radius of turntbale plus 5mm offset from back plane
                FSPoint alphaDelta = turntable->getRotation();
                FSFloat alphaOld = (float)atan(fsNewPoint.z/fsNewPoint.x);
                FSFloat alphaNew = alphaOld+alphaDelta.y*(M_PI/180.0f);
                FSFloat hypotenuse = (float)sqrt(fsNewPoint.x*fsNewPoint.x + fsNewPoint.z*fsNewPoint.z);

                if(fsNewPoint.z < 0 && fsNewPoint.x < 0){
                    alphaNew += M_PI;
                }else if(fsNewPoint.z > 0 && fsNewPoint.x < 0){
                    alphaNew -= M_PI;
                }
                fsNewPoint.z = (float)sin(alphaNew)*hypotenuse;
                fsNewPoint.x = (float)cos(alphaNew)*hypotenuse;

                if(fsNewPoint.y>lowerLimit+0.5 && hypotenuse < 7){ //eliminate points from the grounds, that are not part of the model
if(fsNewPoint.y > 6.592293 && fsNewPoint.y < 6.592294) {
	fsNewPoint.y = 6.592293;
}
                    //qDebug("adding point");
                    model->addPointToPointCloud(fsNewPoint);
                }
                break;
            }
        }
#endif
	}
}
#endif

FSPoint FSVision::TestFunction( FSPoint laserPos, int x, int y, int dpiVertical, FSPoint alphaDelta )    //step between vertical points
{
	//position of the reflected laser line on the image coord

	CvPoint cvNewPoint;
    cvNewPoint.x = x;
    cvNewPoint.y = y;	//x and y as in horizontal and vertical pixels
    //cout << x << ":" << y << endl;

	//convert to world coordinates withouth depth
    FSPoint fsNewPoint = FSVision::convertCvPointToFSPoint(cvNewPoint);
    //cout << fsNewPoint.x << ":" << fsNewPoint.y << ":" << fsNewPoint.z << endl;
    FSLine l1 = computeLineFromPoints(FSCamPosition, fsNewPoint);
    //FSLine l2 = computeLineFromPoints(laser->getPosition(), laser->getLaserPointPosition()); //Try calculating from turntable centre
//				FSLine l2 = computeLineFromPoints(laser->getPosition(), FSMakePoint(0,0,0));
	FSLine l2 = computeLineFromPoints(laserPos, FSMakePoint(0,0,0));

    FSPoint i = computeIntersectionOfLines(l1, l2);
    fsNewPoint.x = i.x;
    fsNewPoint.z = i.z;


    //At this point we know the depth=z. Now we need to consider the scaling depending on the depth.
    //First we move our point to a camera centered cartesion system.
    fsNewPoint.y -= (FSCamPosition).y;
    fsNewPoint.y *= (FSCamPosition.z - fsNewPoint.z)/(FSCamPosition).z;
    //Redo the translation to the box centered cartesion system.
    fsNewPoint.y += (FSCamPosition).y;
    //cout << __PRETTY_FUNCTION__ << fsNewPoint.y << endl;

    //turning new point according to current angle of turntable
    //translate coordinate system to the middle of the turntable
    fsNewPoint.z -= FSController::config->TURNTABLE_POS_Z; //7cm radius of turntbale plus 5mm offset from back plane
//    FSPoint alphaDelta = turntable->getRotation();
    FSFloat alphaOld = (float)atan(fsNewPoint.z/fsNewPoint.x);
    FSFloat alphaNew = alphaOld+alphaDelta.y*(M_PI/180.0f);
    FSFloat hypotenuse = (float)sqrt(fsNewPoint.x*fsNewPoint.x + fsNewPoint.z*fsNewPoint.z);

    if(fsNewPoint.z < 0 && fsNewPoint.x < 0){
        alphaNew += M_PI;
    }else if(fsNewPoint.z > 0 && fsNewPoint.x < 0){
        alphaNew -= M_PI;
    }
    fsNewPoint.z = (float)sin(alphaNew)*hypotenuse;
    fsNewPoint.x = (float)cos(alphaNew)*hypotenuse;
	fsNewPoint.x = (fsNewPoint.x * 1) / 1;

	return fsNewPoint;
}

FSPoint FSVision::detectLaserLine( cv::Mat &laserOff, cv::Mat &laserOn, unsigned int threshold )
{
    unsigned int cols = laserOff.cols;
    unsigned int rows = laserOff.rows;
    cv::Mat laserLine = subLaser2(laserOff, laserOn);
    std::vector<cv::Vec4i> lines;
    double deltaRho = 1;
    double deltaTheta = M_PI/2;
    int minVote = 20;
    double minLength = 50;
    double maxGap = 10;
    cv::Mat laserLineBW( cols, rows, CV_8U, cv::Scalar(0) );
    cv::cvtColor(laserLine, laserLineBW, CV_RGB2GRAY); //convert to grayscale

    /*cv::namedWindow("Detected Lines with HoughP");
    cv::imshow("Detected Lines with HoughP",laserLineBW);
    cv::waitKey(0);*/
    //cvDestroyWindow("Detected Lines with HoughP");

    cv::HoughLinesP( laserLineBW,	//8-bit single-channel image
                     lines,			//This will hold the output vector(s)
                     deltaRho,		//Distance resolution, =1
                     deltaTheta,	//Angle resolution, = pi/2, i.e. vertical
                     minVote,		//Threshold, = 20
                     minLength,		//Minimum length of a valid line, = 50
                     maxGap );		//Maximum gap between segments on the same line to still be a valid line, = 10

    //should at least detect the laser line
    qDebug() << "detected"<<lines.size()<<"lines";
    if(lines.size()==0){
        qDebug("Did not detect any laser line, did you select a SerialPort form the menu?");
        FSPoint p = FSMakePoint(0.0,0.0,0.0);
        return(p);
    }
    //assert(lines.size()>0);
    //for(int i=0;i<lines.size();i++){
    cv::Point p1;
    cv::Point p2;
    for(int i=0;i<lines.size();i++){
        qDebug() << "drawing line "<<lines[i][0]<<lines[i][1]<<lines[i][2]<<lines[i][3];
        //int i = 0;
        p1.x = lines[i][0];
        p1.y = lines[i][1];
        p2.x = lines[i][2];
        p2.y = lines[i][3];
        cv::line(laserLine, p1, p2, CV_RGB( 255,0,0 ),1);   //draw laser line
     }



    /*cv::namedWindow("Detected Lines with HoughP");
    cv::imshow("Detected Lines with HoughP",laserLine);
    cv::waitKey(0);
    cvDestroyWindow("Detected Lines with HoughP");*/

    FSPoint p = convertCvPointToFSPoint(p1);
    return p;
}
