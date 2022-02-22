#ifndef FSCONTROLLER_H
#define FSCONTROLLER_H

#include "staticHeaders.h"
#include "mainwindow.h"
#include "fscontrolpanel.h"
#include "geometryengine.h"
#include "fsmodel.h"
#include "fsserial.h"
#include "fswebcam.h"
#include "fsturntable.h"
#include "fslaser.h"
#include "fsvision.h"
#include "fsconfiguration.h"

class GeometryEngine;
class FSModel;
class FSController;
/*class FSSerial;
class FSWebCam;
class MainWindow;
class FSLaser;*/

class FSController
{
    private:
        static FSController* singleton;
        FSController();

    public:
        MainWindow* mainwindow;
        FSControlPanel* controlPanel;
        GeometryEngine* geometries;
        FSModel* model;
        FSSerial* serial;
        FSWebCam* webcam;
        FSTurntable* turntable;
        FSLaser* laser_0; // right, default leasr
        FSLaser* laser_1; // left, additional laser
        FSVision* vision;

        static FSConfiguration* config;


        //Singleton Pattern
        static FSController* getInstance();
        static void destroy();

        //FabScan Functionality
        void fetchFrame();
        void hideFrame();

        void scan();
        void scanThread();
		void simulateSlice(FSLaser* laser, FSFloat degree, TCHAR* onname);
		void simulateSlice(FSLaser* laser, FSFloat degree, char *onname);
        void scanThread2();
        bool detectLaserLine(FSLaser* laser);
        void computeSurfaceMesh();
        cv::Mat diffImage(int index);

        bool scanning; //wether we are scanning or not, used to interrupt scanning
        bool meshComputed; //wether the surface meshhas already been computed from the point cloud or not
        double laserSwipeMin;
        double laserSwipeMax;
        double laserStepSize;
        double turntableStepSize;
        double yDpi;
		//double Arduino_data; //Data sent back by the Arduino motor driver

	FSPoint TestFunction(int x, int y, FSFloat laserx, FSFloat degree);
};

#endif // FSCONTROLLER_H
