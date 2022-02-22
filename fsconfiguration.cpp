#include "fsconfiguration.h"
#include "fscontroller.h"

FSPoint FSCamPosition;

FSConfiguration::FSConfiguration()
{
    this->setDefaultValues();
}

void FSConfiguration::setDefaultValues()
{
    //to make the scanning process faster we ommit the lower and hight part of the cvImage
    //as there is no object anyway.  The lower limit is defined by the turning table lower bound
    //units are pixels, seen from the top, resp from the bottom
    UPPER_ANALYZING_FRAME_LIMIT = 0;
    LOWER_ANALYZING_FRAME_LIMIT = 30;

    //as the actual position in the frame differs a little from calculated laserline we stop a little befor as we might catch the real non reflected laser line which creates noise
    ANALYZING_LASER_OFFSET = 90;

    //defining the origin in the cvFrame
    //the position of intersection of back plane with ground plane in cvFrame in percent
    //check the yellow laser line to calibrate, the yellow laser line should touch the bottom plane
    //#define ORIGIN_Y 0.825
    ORIGIN_Y= 0.75;

    /********************************/
    /*       CAMERA DEFINES         */
    /********************************/

    //logitech c270
    FRAME_WIDTH = 26.6f; //in cm. the width of what the camera sees, ie place a measure tool at the back-plane and see how many cm the camera sees.
    CAM_IMAGE_WIDTH = 1280.0f;
    CAM_IMAGE_HEIGHT = 960.0f; //here I am not sure, i think it is 960, (old=853.0f)

    /********************************/
    /*    HARDWARE SETUP DEFINES    */
    /********************************/

    //position of the laser
//dschae    LASER_POS_X = 14.0f; //precise by construction
	LASER_POS_X = 26.0f; //precise by construction
    LASER_POS_Y = 6.4f;  //not needed/used for calculations
//dschae    LASER_POS_Z = 28.8f; //precise by construction
    LASER_POS_Z = 15.0f; //precise by construction

    LASER_SWIPE_MIN = 18.0f;
    LASER_SWIPE_MAX = 52.0f;

    //position of the c270
	FSCamPosition.x = 0.0f; //precise by construction
	FSCamPosition.y = 5.57f;
//dschae    CAM_POS_Z = 30.9f;
	FSCamPosition.z = 15.0f;

    //position of the turntable
    TURNTABLE_POS_X = 0.0f; //not used by calculations
    TURNTABLE_POS_Y = 0.0f; //not used by calculations
    TURNTABLE_POS_Z = 7.5f; //precise by construction

	TURNTABLE_STEPS_360 = 410 * 40; // dschae 200*16;
}

void FSConfiguration::read(const FileNode& node)                          //Read serialization for this class
{
    node["UPPER_ANALYZING_FRAME_LIMIT"] >> UPPER_ANALYZING_FRAME_LIMIT;
    node["LOWER_ANALYZING_FRAME_LIMIT"] >> LOWER_ANALYZING_FRAME_LIMIT;
    node["ANALYZING_LASER_OFFSET"] >> ANALYZING_LASER_OFFSET;
    node["ORIGIN_Y"] >> ORIGIN_Y;
    node["FRAME_WIDTH"] >> FRAME_WIDTH;
    node["CAM_IMAGE_WIDTH"] >> CAM_IMAGE_WIDTH;
    node["CAM_IMAGE_HEIGHT"] >> CAM_IMAGE_HEIGHT;
    node["LASER_POS_X"] >> LASER_POS_X;
    node["LASER_POS_Y"] >> LASER_POS_Y;
    node["LASER_POS_Z"] >> LASER_POS_Z;
    node["LASER_SWIPE_MIN"] >> LASER_SWIPE_MIN;
    node["LASER_SWIPE_MAX"] >> LASER_SWIPE_MAX;
	node["CAM_POS_Y"] >> FSCamPosition.y;
	node["CAM_POS_Z"] >> FSCamPosition.z;
    node["TURNTABLE_POS_X"] >> TURNTABLE_POS_X;
    node["TURNTABLE_POS_Y"] >> TURNTABLE_POS_Y;
    node["TURNTABLE_POS_Z"] >> TURNTABLE_POS_Z;

	// Added to suit different turntable stepper arrangements
	node["TURNTABLE_STEPS_360"] >> TURNTABLE_STEPS_360;
	// Special case to avoid crash!
	if(TURNTABLE_STEPS_360 == 0){
			TURNTABLE_STEPS_360 = 410 * 40; // dschae 200*16;
	} //Seems if the configuration.xml doesn't have a TURNTABLE_STEPS_360 entry it crashes!

    interprate();
}

bool FSConfiguration::readConfiguration(bool mode)
{

    boost::filesystem::path p;
    p = boost::filesystem::current_path();

    //this is platform specific code, needs to be changed for other platforms then mac!
    p/="configuration.xml";
    string resPath;
	if(mode || !(boost::filesystem::exists(p)))  {	//Only ask for file location if the user initiated 'Load Configuration'
        //not in the same directory,
		//use 'else' to bring up Windows or Qt file browser.
		QFileDialog dialog;
		dialog.setFileMode(QFileDialog::ExistingFile);
		dialog.setNameFilter("*.xml");
		QStringList filename;
		if (dialog.exec())
		{
			filename=dialog.selectedFiles();
		}
		QString file = filename.at(0);
		resPath= file.toStdString();
    }
	else {
		//if(boost::filesystem::exists(p)){
        //config file is in same directory as fabscan executable
        resPath = "./configuration.xml";
    	//Next command does not like a null respath
		//}
		//else {
		//	resPath = "./";
		//}
	}
    FileStorage fs(resPath, FileStorage::READ);
    if(!fs.isOpened()){
//        cout << "Could not read configuration from file " << resPath << endl;
        return false;
    }
    fs["Configuration"] >> *this;
    fs.release();

    if(!this->goodInput){
//        cout << "Invalid input detected." << endl;
        return false;
    }
	FSController::getInstance()->laser_0->setLaserPosition(0);
	FSController::getInstance()->laser_1->setLaserPosition(1);
	// Moved from fsvision, as this is now effectively a constant
	FSController::getInstance()->laser_0->setLaserPointPosition(FSMakePoint(0,0,TURNTABLE_POS_Z)); //End laser point is at turntable centre
	FSController::getInstance()->laser_1->setLaserPointPosition(FSMakePoint(0,0,TURNTABLE_POS_Z)); //End laser point is at turntable centre
    return true;
}

void FSConfiguration::interprate()
{
//    cout << CAM_IMAGE_WIDTH << " " << CAM_IMAGE_HEIGHT << endl;
    this->goodInput = true;
}
