#ifndef FSVISION_H
#define FSVISION_H

#include "staticHeaders.h"

class FSLaser;

class FSVision
{
public:
    FSVision();
    static FSPoint convertCvPointToFSPoint(CvPoint cvPoint);
    static CvPoint convertFSPointToCvPoint(FSPoint fsPoint);

    static cv::Mat subLaser(cv::Mat &laserOff, cv::Mat &laserOn, FSFloat threshold);
    static cv::Mat subLaser2(cv::Mat &laserOff, cv::Mat &laserOn);
    static cv::Mat drawHelperLinesToFrame(cv::Mat &frame);
    static cv::Mat drawLaserLineToFrame(cv::Mat &frame);
    static cv::Mat diffImage(cv::Mat &laserOff, cv::Mat &laserOn);

    static void putPointsFromFrameToCloud(
		FSLaser* laser,
            cv::Mat &laserOff,
            cv::Mat &laserOn,
            int dpiVertical,
            FSFloat lowerLimit);

	static void putPointsFromFrameToCloud2(
		FSLaser* laser,
            cv::Mat &laserOff,
            cv::Mat &laserOn,
            int dpiVertical,
            FSFloat lowerLimit);

    static FSPoint detectLaserLine(cv::Mat &laserOff, cv::Mat &laserOn, unsigned int threshold);
    static cv::Mat histogram(cv::Mat &img);

	static FSPoint TestFunction( FSPoint laserPos, int x, int y, int dpiVertical, FSPoint alphaDelta );    //step between vertical points
};

#endif // FSVISION_H
