#include "fscontroller.h"
#include "fsdialog.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <QFuture>
#include <QTime>
#include <QtConcurrent/QtConcurrentRun>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

using namespace std;
using namespace cv;

#define	DUAL_LASER	1

FSController* FSController::singleton=0;

FSConfiguration* FSController::config = new FSConfiguration();

FSController::FSController()
{
    //Software
    geometries = new GeometryEngine();
    model = new FSModel();
    //Hardware
    serial = new FSSerial();
    webcam = new FSWebCam();
    turntable = new FSTurntable();
    laser_0 = new FSLaser(0, 'd', 'e');
	laser_1 = new FSLaser(1, 'b', 'c');

	vision = new FSVision();

    scanning = false;
    //all in degrees; (only when stepper is attached to laser)
    laserSwipeMin = 30; //18
    laserSwipeMax = 45; //50
    meshComputed = false;

	// Added for OpenCV capture
	webcam->StartX();

	//sysTime = new QTime();
}

FSController* FSController::getInstance()
{
    if (singleton == 0){
        singleton = new FSController();
    }
    return singleton;
}

void FSController::destroy()
{
    if (singleton != 0) {
		// Added to try to get exe to end when mainwindow closed
		singleton->geometries->~GeometryEngine();
		if (singleton->webcam) {
			singleton->webcam->~FSWebCam();}
        delete singleton;
        singleton = 0;
    }
}

void FSController::fetchFrame()
{
    if(webcam->info.portName.isEmpty()){
        mainwindow->showDialog("No webcam found!");
        return;
    } 

    cv::Mat frame;
    frame = FSController::getInstance()->webcam->getFrame();
//      cv::imshow("Extracted Frame",frame);
//      cv::waitKey(0);
    cv::resize(frame,frame,cv::Size(1280,960));
    cv::Mat result = vision->drawHelperLinesToFrame(frame);
    cv::resize(result,result,cv::Size(800,600)); //this is the resolution of the preview
    cv::imshow("Extracted Frame",result);
//    cv::waitKey(1);
//    cvDestroyWindow("Extracted Frame");
}

void FSController::hideFrame()
{
    cvDestroyWindow("Extracted Frame");
}

void FSController::scan()
{/*
    if(webcam->info.portName.isEmpty()){
        mainwindow->showDialog("No webcam selected!");
        return;
    }*/
    QFuture<void> future = QtConcurrent::run(this, &FSController::scanThread);
}

void FSController::scanThread() // For units with no laser stepper
{	// This is the main scanning routine. No preparation done in mainwindow.cpp before calling this
	QTime sysTime;
	QString str;
	int slice;
	char c;
	unsigned long delay;

	/*
    //check if camera is connected
    if(webcam->info.portName.isEmpty()){
        mainwindow->showDialog("No webcam selected!");
        return;
    }*/
    //detect laser line
    // this->detectLaserLine(); //Not required for my version2
    //turn off laser stepper (if available)
    this->laser_0->disable();
    this->laser_1->disable();

    scanning = true; //start scanning, if false, scan stops
    FSFloat stepDegrees = turntableStepSize;

    laser_0->turnOn();
    turntable->setDirection(FS_DIRECTION_CW); //CCW
    turntable->enable();

    //iterate over a complete turn of the turntable
	stepDegrees = 2; // dschae
    for(FSFloat i=0; i<360 && scanning==true; i+= stepDegrees){
        //take picture without laser
        laser_0->turnOff();
#if DUAL_LASER
		laser_1->turnOff();
#endif
		QThread::msleep(200); //was 200
        cv::Mat laserOff = webcam->getFrame();
		cv::resize( laserOff,laserOff,cv::Size(1280,960) );
        //take picture with laser
        laser_0->turnOn();
        QThread::msleep(200);	// was 200. Do we need these delays?
        cv::Mat laserOn = webcam->getFrame();
        cv::resize( laserOn,laserOn,cv::Size(1280,960) );
//------- lase 1
		laser_0->turnOff();
#if DUAL_LASER
		laser_1->turnOn();
		QThread::msleep(200);
		cv::Mat laser1On = webcam->getFrame();
		cv::resize(laser1On, laser1On, cv::Size(1280, 960));
#endif
		//--------
		//turn turntable a step
        turntable->turnNumberOfDegrees(stepDegrees);
		slice = (360-i)/stepDegrees;
		mainwindow->setWindowTitle("Axiscan3D - Slices left = "+ str.setNum(slice));
		//here the magic happens
        vision->putPointsFromFrameToCloud2(laser_0, laserOff, laserOn, yDpi, 0);
#if DUAL_LASER
        vision->putPointsFromFrameToCloud2(laser_1, laserOff, laser1On, yDpi, 0);
#endif
		int idegree = (int)(i * 100);
		char fname[MAX_PATH];
		sprintf_s(fname, MAX_PATH, "C:\\scanimage\\img%05d_on0.bmp", idegree);
		imwrite(fname, laserOn);
#if DUAL_LASER
		sprintf_s(fname, MAX_PATH, "C:\\scanimage\\img%05d_on1.bmp", idegree);
		imwrite(fname, laser1On);
#endif
		sprintf_s(fname, MAX_PATH, "C:\\scanimage\\img%05d_off.bmp", idegree);
		imwrite(fname, laserOff);
		//update gui
        geometries->setPointCloudTo(model->pointCloud);
        mainwindow->redraw();
#if 0
// dschae Axiscan3D has no ping-pong function
		FSController::getInstance()->serial->writeChar(MC_FABSCAN_PING);	//Send a 'ping'...
		//Need to time-limit the following...
		delay = 300+stepDegrees*100; //The original delay amount - the bigger the step the longer the delay
		sysTime.start();	//Start timer
		do {
			c = FSController::getInstance()->serial->readChar(); //Get a 'pong'
		} while(((int)((unsigned char)c) != MC_FABSCAN_PONG) && (sysTime.elapsed() < delay)); //c is char, MC_FABSCAN_PONG is int
		//We're now also keeping an eye on the time, and if it takes too long we exit
#endif
	}
    if(scanning) mainwindow->doneScanning();
	laser_0->turnOff();
	mainwindow->setWindowTitle("Axiscan3D");
    scanning = false; //stop scanning
}

void FSController::simulateSlice(FSLaser* laser, FSFloat degree, TCHAR *tsz_onname) // For units with no laser stepper
{	// This is the main scanning routine. No preparation done in mainwindow.cpp before calling this
	char on_filename[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, tsz_onname, lstrlen(tsz_onname)+1, on_filename, MAX_PATH, NULL, NULL);
	simulateSlice(laser, degree, on_filename);
}

FSPoint FSController::TestFunction(int x, int y, FSFloat laserx, FSFloat degree)
{
	FSPoint laserPos;
	laserPos.x = laserx;
	laserPos.y = 11;
	laserPos.z = 36.4;

	FSPoint ad;
	ad.x = 0;
	ad.y = 0;
	ad.z = 0;

	return vision->TestFunction(laserPos, x, y, 1, ad);
}

void FSController::simulateSlice(FSLaser* laser, FSFloat degree, char *on_filename) // For units with no laser stepper
{
	QTime sysTime;
	QString str;
	int slice;
	char c;
	unsigned long delay;


	int lastpos = strlen(on_filename)-6;

	char off_filename[MAX_PATH];
	strcpy(off_filename, on_filename);
	strcpy(off_filename+lastpos, "ff.bmp");

	cv::Mat laserOff = imread(off_filename, CV_LOAD_IMAGE_COLOR);   // Read the file
	cv::Mat laserOn = imread(on_filename, CV_LOAD_IMAGE_COLOR);   // Read the file

//	cv::flip(laserOff,laserOff, 1);
//	cv::flip(laserOn, laserOn, 1);

	cv::imshow("Extracted Frame", laserOn);/* waitKey(1) */;

	cv::resize( laserOff,laserOff,cv::Size(1280,960) );
	cv::resize( laserOn, laserOn, cv::Size(1280,960) );


	FSPoint r;
	r.y = degree;

	turntable->setRotation(r);

	//here the magic happens
	vision->putPointsFromFrameToCloud2(laser, laserOff, laserOn, yDpi, 0);
//	vision->putPointsFromFrameToCloud(laserOff, laserOn, yDpi, 0);
	//update gui
	geometries->setPointCloudTo(model->pointCloud);
	mainwindow->redraw();
}

void FSController::scanThread2() // For units with a laser stepper
{/*
    if(webcam->info.portName.isEmpty()){
        mainwindow->showDialog("No webcam selected!");
        return;
    }*/
    scanning = true; //start scanning

    qDebug() << "done with turn to angle";
    //laser->turnNumberOfDegrees( laser->getRotation().y - LASER_SWIPE_MIN );
    turntable->setDirection(FS_DIRECTION_CW);
    for(FSFloat j=0; j<360 && scanning==true; j+=turntableStepSize){
        turntable->disable();
        laser_0->turnOn();
        laser_0->enable();
        laser_0->turnToAngle(laserSwipeMin);
        QThread::msleep(2500);
        laser_0->setDirection(FS_DIRECTION_CCW);
        for(FSFloat i=laserSwipeMin; i<laserSwipeMax && scanning==true; i+=laserStepSize){
            qDebug() << i;
            laser_0->turnOff();
            QThread::msleep(200);
            cv::Mat laserOff = webcam->getFrame();
            cv::resize( laserOff,laserOff,cv::Size(1280,960) );

            laser_0->turnOn();
            QThread::msleep(200);
            cv::Mat laserOn = webcam->getFrame();
            cv::resize( laserOn,laserOn,cv::Size(1280,960) );

            vision->putPointsFromFrameToCloud(laser_0, laserOff, laserOn, 5, 0);
            geometries->setPointCloudTo(model->pointCloud);
            mainwindow->redraw();
            laser_0->turnNumberOfDegrees(laserStepSize);
            QThread::msleep(laserStepSize*100);
        }
        laser_0->disable();
        turntable->enable();
        turntable->turnNumberOfDegrees(turntableStepSize);
        std::string name;
        name.append(boost::lexical_cast<std::string>(j));
        name.append(".ply");
        //model->savePointCloudAsPLY(name);
        //model->pointCloud->clear();
        QThread::msleep(turntableStepSize*100);
    }
    if(scanning) mainwindow->doneScanning();
    scanning = false; //stop scanning
}

cv::Mat FSController::diffImage(int index)
{
	FSLaser* laser = index == 0 ? laser_0 : laser_1;
    laser->turnOff();

    QThread::msleep(200);
    cv::Mat laserOff = webcam->getFrame();
    cv::resize( laserOff,laserOff,cv::Size(1280,960) );

    laser->turnOn();
    QThread::msleep(200);
    cv::Mat laserOn = webcam->getFrame();
    cv::resize( laserOn, laserOn, cv::Size(1280,960) );

    return vision->diffImage(laserOff,laserOn);
}

bool FSController::detectLaserLine(FSLaser* laser)
{
    unsigned int threshold = 40;
    laser->turnOff();
    QThread::msleep(200);
    cv::Mat laserOffFrame = webcam->getFrame();
    laser->turnOn();
    QThread::msleep(200);
    cv::Mat laserOnFrame = webcam->getFrame();
    cv::resize( laserOnFrame,laserOnFrame,cv::Size(1280,960) );
    cv::resize( laserOffFrame,laserOffFrame,cv::Size(1280,960) );

    qDebug("images loaded, now detecting...");
    FSPoint p = vision->detectLaserLine( laserOffFrame, laserOnFrame, threshold );
    if(p.x == 0.0){return false;}
    laser->setLaserPointPosition(p);
    return true;
}

void FSController::computeSurfaceMesh()
{
	qDebug("Doing computeSurfaceMesh...");
    //model->convertPointCloudToSurfaceMesh();
    //geometries->setSurfaceMeshTo(model->surfaceMesh,model->pointCloud);
	qDebug("...via model->convertPointCloudToSurfaceMesh3()");
    model->convertPointCloudToSurfaceMesh3();	// convertPointCloudToSurfaceMesh2() is Mac-specific
	qDebug("Now calling model->convertPolygons2Triangles()");
    model->convertPolygons2Triangles();
    //cout << "FSController:computesurfaceMesh: convert done, now setting" << endl;
	qDebug("Now calling geometries->setSurfaceMeshTo()");
    geometries->setSurfaceMeshTo(model->surfaceMesh,model->pointCloud);
	qDebug("Now calling mainWindow->redraw()");
    mainwindow->redraw();
}
