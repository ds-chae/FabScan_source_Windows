#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fscontroller.h"
#include "fsdialog.h"
#include "fsdefines.h"

#include <QBasicTimer>
#include <QDialogButtonBox>
#include <QFuture>
#include <QtCore>
#include <QtConcurrent/QtConcurrentRun>
#include <QCamera>
#include <QSound>

#include <QMessageBox>

#include <boost/bind.hpp>

#ifdef LINUX
#include <boost/filesystem.hpp>
#endif

#define version_Major 1
#define version_Minor 2

int tick_divider = 0;

int step_increase = 40;
int move_count = 0;
int scanner_state = 0;
int elapse_count = 0;

#define	MAX_TURN_STEPS	16400 //16400
#define	TIMER_ELAPSE	10
#define	TIMER_GAP		200
#define	TIME_PER_STEP	2
#define	TIMER_PER_MOVE	(step_increase * TIME_PER_STEP / TIMER_ELAPSE)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    hwTimer(new QBasicTimer),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setupMenu();
    this->enumerateSerialPorts(); // This includes the first call to FSController::getInstance()
    this->enumerateWebCams();
#if 0 // dschae 20140711
	hwTimer->start(400, this); //timer that checks periodically for attached hardware (camera, arduino) - 400ms
#else
	hwTimer->start(TIMER_ELAPSE, this); //timer that checks periodically for attached hardware (camera, arduino) - 400ms
#endif
	dialog = new FSDialog(this);
    controlPanel = new FSControlPanel(this);
    FSController::getInstance()->mainwindow=this;
    FSController::getInstance()->controlPanel=controlPanel;
    ui->widget->setStyleSheet("border: 1px solid black;");
	this->readConfiguration(false);		//Try and get new config
    applyState(POINT_CLOUD);
    //resolution: Good
    FSController::getInstance()->turntableStepSize = 8*FSController::getInstance()->turntable->degreesPerStep; //Default is "Good" resolution
    FSController::getInstance()->yDpi = 1;
}

MainWindow::~MainWindow()
{
    FSController::getInstance()->scanning=false;	// Terminate any scan in progress
	FSController::getInstance()->laser_0->turnOff();	// Make sure laser is off!
	FSController::getInstance()->laser_1->turnOff();	// Make sure laser is off!
	FSController::getInstance()->destroy();	// We'll find what's hogging the .exe the hard way...
	cv::destroyAllWindows();
	controlPanel->~FSControlPanel();
	delete ui;
	dialog->~FSDialog();
	//qextSerialEnumerator->~QextSerialEnumerator();
	hwTimer->~QBasicTimer();
	qApp->quit();
}

void MainWindow::setupMenu()
{
	QAction* showAboutAction = new QAction("About", this);
    //showAboutAction->setShortcuts(QKeySequence::About);
    connect(showAboutAction,SIGNAL(triggered()),this, SLOT(showAbout()));
    ui->menuFile->addAction(showAboutAction);

    QAction* newPointCloudAction = new QAction("New", this);
    newPointCloudAction->setShortcuts(QKeySequence::New);
    connect(newPointCloudAction,SIGNAL(triggered()),this, SLOT(newPointCloud()));
    ui->menuFile->addAction(newPointCloudAction);

    QAction* openPointCloudAction = new QAction("Open PointCloud...", this);
    openPointCloudAction->setShortcuts(QKeySequence::Open);
    connect(openPointCloudAction,SIGNAL(triggered()),this, SLOT(openPointCloud()));
    ui->menuFile->addAction(openPointCloudAction);

    QAction* savePointCloudAction = new QAction("Save PointCloud...", this);
    savePointCloudAction->setShortcuts(QKeySequence::Save);
    connect(savePointCloudAction,SIGNAL(triggered()),this, SLOT(savePointCloud()));
    ui->menuFile->addAction(savePointCloudAction);

    QAction* exportSTLAction = new QAction("Export .STL...", this);
    connect(exportSTLAction,SIGNAL(triggered()),this, SLOT(exportSTL()));
    ui->menuFile->addAction(exportSTLAction);

    QAction* readConfiguartion = new QAction("Read Configuration", this);
    connect(readConfiguartion,SIGNAL(triggered()),this, SLOT(readConfig()));
    ui->menuFile->addAction(readConfiguartion);

    QAction* showControlPanelAction = new QAction("Control Panel...", this);
    showControlPanelAction->setShortcuts(QKeySequence::Preferences);
    connect(showControlPanelAction,SIGNAL(triggered()),this, SLOT(showControlPanel()));
    ui->menuFile->addAction(showControlPanelAction);
}

void MainWindow::
showDialog(QString dialogText)
{
    dialog->setStandardButtons(QDialogButtonBox::Ok);
    dialog->setText(dialogText);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();

	//Below is the added code to display the two cv camera feeds, which attach to the side of MainWindow.ui
		 cv::namedWindow("Extracted Frame",0); //显示实时采集	
	  cv::namedWindow("Laser Frame",0);	//显示处理结果

	   //捕捉鼠标
   //  cv::setMouseCallback("extractedlaserLine1",onMouse,0);

	  	   	   	 HWND hWnd0 = (HWND)cvGetWindowHandle("Axiscan3D");
 HWND hRawWnd0 = ::GetParent(hWnd0);
 if (hRawWnd0 != NULL) {
  BOOL bRet = ::SetWindowPos(hRawWnd0, HWND_TOPMOST, 100, 100, 0, 0, SWP_NOSIZE );
  assert(bRet);

 }

	   	   	 HWND hWnd = (HWND)cvGetWindowHandle("Extracted Frame");
 HWND hRawWnd = ::GetParent(hWnd);
 if (hRawWnd != NULL) {
  BOOL bRet = ::SetWindowPos(hRawWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE |SWP_NOMOVE);
  assert(bRet);

 }

 	   	   	 HWND hWnd2 = (HWND)cvGetWindowHandle("Laser Frame");
 HWND hRawWnd2 = ::GetParent(hWnd2);
 if (hRawWnd2 != NULL) {
  BOOL bRet = ::SetWindowPos(hRawWnd2, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE |SWP_NOMOVE);
  assert(bRet);

 }


	 cv::resizeWindow("Extracted Frame", FSController::getInstance()->mainwindow->height()*3/4,FSController::getInstance()->mainwindow->height()/2-19);
	  cv::resizeWindow("Laser Frame", FSController::getInstance()->mainwindow->height()*3/4,FSController::getInstance()->mainwindow->height()/2-19);
	 cv::moveWindow("Extracted Frame", FSController::getInstance()->mainwindow->x() +FSController::getInstance()->mainwindow->width()+15,FSController::getInstance()->mainwindow->y());
	  cv::moveWindow("Laser Frame", FSController::getInstance()->mainwindow->x() +FSController::getInstance()->mainwindow->width()+15,FSController::getInstance()->mainwindow->y()+FSController::getInstance()->mainwindow->height()/2+19);
}

//===========================================
// Action Methods
//===========================================

void MainWindow::showAbout()
{
	QString aboutString = "Axiscan3D for Windows version "%QChar(0x30+version_Major)%"."%QChar(0x30+version_Minor)%"\n\nRefined by Ian Shillingford";
	this->showDialog(aboutString);
	return;
}
void MainWindow::exportSTL()
{
    if(FSController::getInstance()->model->pointCloud->empty()){
        this->showDialog("PointCloud is empty! Perform a scan, or open a pointcloud.");
        return;
    }
    QFileDialog d(this, "Save File","","STL (*.stl)");
    d.setAcceptMode(QFileDialog::AcceptSave);
    if(d.exec()){

        QString fileName = d.selectedFiles()[0];
        if(fileName.isEmpty() ) return;
        qDebug() << fileName;

        if(!FSController::getInstance()->meshComputed){
            qDebug() << "Computing mesh...";
            this->showDialog("Will now compute surface mesh, this may take a while...");
            FSController::getInstance()->computeSurfaceMesh();
            FSController::getInstance()->meshComputed = true;
        }
        //cout << "Done computing surface mesh, now stl export..." << endl;
		this->setWindowTitle("Axiscan3D - Done computing surface mesh, now stl export...");
        FSController::getInstance()->model->saveToSTLFile(fileName.toStdString());
        this->showDialog("STL export done!");

    }
}

void MainWindow::showControlPanel()
{
    controlPanel->show();
    controlPanel->raise();
    controlPanel->activateWindow();
}

TCHAR  filelist[1000][MAX_PATH];
int   filecount = 0;
int   fileindex = 0;

extern int activeLaser;

void MainWindow::DoSimulate()
{
	if(fileindex < filecount) { // in simulation mode
//dschae		FSFloat degree = (fileindex * 360.0) / 410.0;
		TCHAR* ep = filelist[fileindex];
		while(*ep) ep++;
		while(*(ep-1) != '\\') ep--;
		int slice_it = 0;
		if(ep[11] == '0' && (activeLaser & 1)) slice_it = 1;
		if(ep[11] == '1' && (activeLaser & 2)) slice_it = 1;
		if(slice_it) {
			int idegree = 0;
			for(int i = 3; i < 8; i++) idegree = idegree * 10 + (ep[i] - '0');
			FSFloat degree = (float)idegree / 100.0;

			FSLaser* laser = (ep[11] == '0') ?
				FSController::getInstance()->laser_0 : FSController::getInstance()->laser_1;

			FSController::getInstance()->simulateSlice(laser, degree, filelist[fileindex]);	//Call scanning thread
		}
		fileindex++;
		if(fileindex >= filecount) {
			doneScanning();
			setWindowTitle("Axiscan3D");
			FSController::getInstance()->scanning = false; //stop scanning
		}
	}
}

void MainWindow::on_testButton_clicked()
{
	FSPoint p1 = FSController::getInstance()->TestFunction(612, 387, 34.0, 0.0);
	FSPoint p2 = FSController::getInstance()->TestFunction(1280-612, 387, -34.0, 0.0);
}

#define	MOVE_DISK	0
#define	LASER_0_ON	1
#define	LASER_1_ON	2
#define	LASER_OFF	3

int img_state = MOVE_DISK;
int step_count = 0;

cv::Mat laserOn0Image;
cv::Mat laserOn1Image;
cv::Mat laserOffImage;

char on0_fname[MAX_PATH];
char on1_fname[MAX_PATH];
char off_fname[MAX_PATH];

void ScanTimer()
{

	cv::Mat frame_copy = FSController::getInstance()->webcam->getFrame();
	switch(img_state) {
	case MOVE_DISK : // turn table command issued
		move_count--;
		if(move_count <= 0) {
	        FSController::getInstance()->laser_0->turnOn();
			img_state = LASER_0_ON;
		}
		break;
	case LASER_0_ON :
		elapse_count += TIMER_ELAPSE;
		if(elapse_count >= TIMER_GAP) {
			sprintf_s(on0_fname, MAX_PATH, "C:\\scanimage\\img%05d_on0.bmp", step_count);
			imwrite(on0_fname, frame_copy);
			frame_copy.copyTo(laserOn0Image);
			FSController::getInstance()->laser_0->turnOff();
			FSController::getInstance()->laser_1->turnOn();
			img_state = LASER_1_ON;
			elapse_count = 0;
		}
		break;
	case LASER_1_ON :
		elapse_count += TIMER_ELAPSE;
		if(elapse_count >= TIMER_GAP) {
			sprintf_s(on1_fname, MAX_PATH, "C:\\scanimage\\img%05d_on1.bmp", step_count);
			imwrite(on1_fname, frame_copy);
			frame_copy.copyTo(laserOn1Image);
			FSController::getInstance()->laser_1->turnOff();
			img_state = LASER_OFF;
			elapse_count = 0;
		}
		break;
	case LASER_OFF :
		elapse_count += TIMER_ELAPSE;
		if(elapse_count >= TIMER_GAP) {
			sprintf_s(off_fname, MAX_PATH, "C:\\scanimage\\img%05d_off.bmp", step_count);
			imwrite(off_fname, frame_copy);
			frame_copy.copyTo(laserOffImage);

			FSController::getInstance()->simulateSlice(FSController::getInstance()->laser_0, (step_count * 360.0) / MAX_TURN_STEPS, on0_fname);
			FSController::getInstance()->simulateSlice(FSController::getInstance()->laser_1, (step_count * 360.0) / MAX_TURN_STEPS, on1_fname);

			elapse_count = 0;
			step_count += step_increase;
			if(step_count < MAX_TURN_STEPS) {
				FSController::getInstance()->turntable->turnSteps(step_increase);
				img_state = MOVE_DISK;
				move_count = TIMER_PER_MOVE;
			} else {
				scanner_state = 3;
				step_count = 0;
			}
		}
	}
//getframe usage:        cv::Mat laserOff = webcam->getFrame();
//		mainwindow->setWindowTitle("Axiscan3D - Slices left = "+ str.setNum(slice));
}

void MainWindow::timerEvent(QTimerEvent *e)
{
	bool result;

	Q_UNUSED(e);
	//this->enumerateSerialPorts();
	//this->enumerateWebCams();

	switch(scanner_state) {
	case 0 :
		// do original checking etc.
		break;
	case 1 : // requested to start scanning;
		scanner_state = 2;
		break;
	case 2 :
		ScanTimer();
		break;
	case 3 :
		scanner_state = 0;
		break;
	case 4 :
		break;
	case 5 :
		DoSimulate();
		return;
	}

	tick_divider++;
	if(tick_divider < 400/TIMER_ELAPSE)
		return;
	tick_divider = 0;

	if(FSController::getInstance()->scanning) {
		//Don't interupt Arduino if we're scanning!
	} else {
		//Added for the OpenCV webcam windows. They update each timer period
	
		cv::namedWindow("Extracted Frame",0);	

		cv::namedWindow("Laser Frame",0);	

		cv::resizeWindow("Extracted Frame", FSController::getInstance()->mainwindow->height()*3/4,FSController::getInstance()->mainwindow->height()/2-19);
		cv::resizeWindow("Laser Frame", FSController::getInstance()->mainwindow->height()*3/4,FSController::getInstance()->mainwindow->height()/2-19);
		cv::moveWindow("Extracted Frame", FSController::getInstance()->mainwindow->x() +FSController::getInstance()->mainwindow->width()+15,FSController::getInstance()->mainwindow->y());
		cv::moveWindow("Laser Frame", FSController::getInstance()->mainwindow->x() +FSController::getInstance()->mainwindow->width()+15,FSController::getInstance()->mainwindow->y()+FSController::getInstance()->mainwindow->height()/2+19);

		if (FSController::getInstance()->scanning == false) //Don't interupt Arduino if we're scanning!
		{
			result = arduinoStatus(); //See if Arduino still responding properly
		}
	}
}

//===========================================
// Menu Methods
//===========================================

void MainWindow::onSelectSerialPort()
{
	FSController *fs;
	bool dat, result;

    QAction* action=qobject_cast<QAction*>(sender());
    if(!action) return;

	fs = FSController::getInstance(); //Because there are so many references in this function

	if(fs->serial->serialPortPath != action->iconText()) //Check if this port already open (if same serial port clicked)
	{	// This is not the same serial port. Do we already have a port open?
		if(fs->serial->serialPortPath->size())	//i.e. is the path name more than zero characters long?
		{	//Yes, disconnect it
			fs->serial->disconnectFromSerialPort();
		}
		//set new path
		fs->serial->serialPortPath->clear();
		fs->serial->serialPortPath->append(action->iconText());
		//this->enumerateSerialPorts();
		fs->serial->connectToSerialPort();
	}
	// Here we've opened an active serial port. We'll experiment with receiving data...
	result = arduinoStatus();
	qDebug() << "Port status: " << *fs->serial->serialPortPath;
}

bool MainWindow::arduinoStatus()
{
	char c;
	FSController *fs = FSController::getInstance();
#if 0
// Axiscan3D has no status functions
	qDebug() << "Arduino Status...";
	if(!fs->serial->serialPortPath->isNull())	//Only perform if we have a port selected
	{
		fs->serial->writeChar(MC_FABSCAN_PING);	//Send a 'ping'...
		QThread::msleep(100); //Give it time to answer (it's slower than we are)
		c = fs->serial->readChar(); //Get a 'pong'
		if((int)((unsigned char)c) == MC_FABSCAN_PONG) //c is char, MC_FABSCAN_PONG is int
		{
			//Arduino answered back properly
			ui->Serial_check->setChecked(true);
			return true;
		}
		else
		{
			//Didn't answer or incorrect response - wrong port or Arduino has 'disappeared'
			ui->Serial_check->setChecked(false);
			return false;
		}
	}
	else
	{
		//No serial port selected or found
		ui->Serial_check->setChecked(false);
		return false;
	}
#else
	return false;
#endif
}

void MainWindow::onSelectWebCam()
{
    QAction* action=qobject_cast<QAction*>(sender());
    if(!action) return;
    FSController::getInstance()->webcam->info.portName=action->iconText(); //eigentlich doppelt gemoppelt, das hier kann weg muss jedoch gekukt werden
    FSController::getInstance()->webcam->setCamera(action->data().toByteArray());
    this->enumerateWebCams();
}

void MainWindow::openPointCloud()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File","","Files (*.pcd) ;; PLY (*.ply)");
    if(fileName.isEmpty() ) return;
    if(fileName.endsWith(".pcd", Qt::CaseInsensitive) ){
        FSController::getInstance()->model->loadPointCloudFromPCD(fileName.toStdString());
    }else if(fileName.endsWith(".ply", Qt::CaseInsensitive) ){
        FSController::getInstance()->model->loadPointCloudFromPLY(fileName.toStdString());
    }
    ui->widget->drawState = 0;
    ui->widget->updateGL();
    applyState(POINT_CLOUD);
    FSController::getInstance()->meshComputed=false;
}

void MainWindow::savePointCloud()
{
    QFileDialog d(this, "Save File","","PCD (*.pcd) ;; PLY (*.ply) ;; ASCII triplets (*.asc)");
    d.setAcceptMode(QFileDialog::AcceptSave);
    if(d.exec()){
        QString fileName = d.selectedFiles()[0];
        //fileName.append(d.selectedNameFilter());
        if(fileName.isEmpty() ) return;
        qDebug() << fileName;
        if(fileName.endsWith(".pcd", Qt::CaseInsensitive) ){
            qDebug() << "Save as pcd file.";
            FSController::getInstance()->model->savePointCloudAsPCD(fileName.toStdString());
        }else if(fileName.endsWith(".ply", Qt::CaseInsensitive) ){
            qDebug() << "Save as ply file.";
            FSController::getInstance()->model->savePointCloudAsPLY(fileName.toStdString());
		}else if(fileName.endsWith(".asc", Qt::CaseInsensitive) ){	// This is just a PCD file with a .asc extension - for MeshLab
            qDebug() << "Save as asc file.";
            FSController::getInstance()->model->savePointCloudAsPCD(fileName.toStdString());
        }
    }

    ui->widget->drawState = 0;
    ui->widget->updateGL();
}

void MainWindow::newPointCloud()
{
    FSController::getInstance()->model->pointCloud->clear();
    FSController::getInstance()->model->surfaceMesh.polygons.clear();
    ui->widget->updateGL();
    applyState(POINT_CLOUD);
    FSController::getInstance()->meshComputed=false;
}

void MainWindow::readConfig()
{
	this->readConfiguration(true);
}

void MainWindow::readConfiguration(bool mode)
{
    if(FSController::config->readConfiguration(mode)){
		if(mode){
			this->showDialog("Successfully read configuration file!");
		}
		else{
			this->setWindowTitle("Axiscan3D - Successfully read configuration file!");
		}
	}
	else{
		if(mode){
			this->showDialog("Configuration file not found or corrupt!");
		}
		else{
			this->setWindowTitle("Axiscan3D - Using default configuration!");
		}
	}
	// We need to transfer stepper info into FSController now
	FSController::getInstance()->turntable->degreesPerStep = 360.0f/ FSController::config->TURNTABLE_STEPS_360;
	FSController::getInstance()->turntableStepSize = FSController::getInstance()->turntable->degreesPerStep;
}

void MainWindow::enumerateSerialPorts()
{
	FSController *fs = FSController::getInstance(); // We now instantiate FSController up here.
	bool status;

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();	// Get list of serial ports available
    ui->menuSerialPort->clear();	//Clear list in FabScan menu

    foreach (QextPortInfo info, ports) {
        if(!info.portName.isEmpty()){	//If there is a port name...
            QAction* ac = new QAction(info.portName, this); //... add this to the menu list...
            ac->setCheckable(true);	//...which the user can select
            connect(ac,SIGNAL(triggered()),this, SLOT(onSelectSerialPort())); //Really, this only needs to be done once, not for each port
            if(fs->serial->serialPortPath->compare(info.portName)==0){
                ac->setChecked(true);	//If ?
            }
            ui->menuSerialPort->addAction(ac);
			// Let's see if we can auto-detect the right port-
			// We have a port name in info.portName, so let's see if it's already in use (ours shouldn't be)...
			fs->serial->serialPortPath->clear();
			fs->serial->serialPortPath->append(info.portName);
			qDebug() << *fs->serial->serialPortPath;
			if(fs->serial->connectToSerialPort())
			{	//We have opened a serial port
#if 0
				status = arduinoStatus();
				if(status)
				{
					// We found the correct port
					ac->setChecked(true); //Mark it as selected
					return;
				}
				else
				{
					fs->serial->serialPortPath->clear();	//For the last port tested: if not Arduino then remove name from path
				}
				fs->serial->disconnectFromSerialPort();
#else
				// Axiscan3D has no status yet. just skip, return OK
				ac->setChecked(true); //Mark it as selected
				return;

#endif
			}
        }
    }
}

void MainWindow::enumerateWebCams()
{
	//Try using OpenCV to enumerate (but unfortunately not name) available cameras
	int n=0;

	if (FSController::getInstance()->webcam->imageCaptureCv.isOpened()) n=1;
	if(n==0){	//If there are no cameras...
       QAction* a = new QAction("No camera found.", this);
       a->setEnabled(false);
       ui->menuCamera->clear();
       ui->menuCamera->addAction(a);
       return;
    }

    ui->menuCamera->clear();
    QString description = "Detected";
    QAction *videoDeviceAction = new QAction(description, this);
    videoDeviceAction->setCheckable(false);
    videoDeviceAction->setData(QVariant(1));

     FSController::getInstance()->webcam->info.portName=description; //This tells the main routines that we have a camera
        /*if(description.contains("Logitech")){
            FSController::getInstance()->webcam->info.portName=description; //eigentlich doppelt gemoppelt, das hier kann weg muss jedoch gekukt werden
            FSController::getInstance()->webcam->setCamera(videoDeviceAction->data().toByteArray());
        }*/

//        if (FSController::getInstance()->webcam->info.portName.compare(description)==0) {
            //cameraDevice = deviceName;
            videoDeviceAction->setChecked(true);
//        }
        ui->menuCamera->addAction(videoDeviceAction);
		this->setWindowTitle("Camera found!");
		ui->Camera_check->setChecked(true);
    }


void MainWindow::on_scanButton_clicked()
{
	QString str;

	if(FSController::getInstance()->webcam->info.portName.isEmpty()){
		showDialog("No webcam found!");
		return;
	}

    //doneScanning();
    //QFuture<void> future = QtConcurrent::run(FSController::getInstance(), &FSController::scanThread);
    bool s = FSController::getInstance()->scanning;
    if (s==false){ //If we are not scanning (i.e. we want to start scanning)...
        applyState(SCANNING);
        // dschae -- scanner_state = 1; // request to start scanning
		FSController::getInstance()->scanThread();	//Call scanning thread
    }else{		// If user clicks button to abort a scan in progress...
        applyState(POINT_CLOUD);
        this->ui->scanButton->setText("Start Scan");
//		doneScanning();
		FSController::getInstance()->laser_0->turnOff();	// Make sure laser is off!
		FSController::getInstance()->laser_1->turnOff();	// Make sure laser is off!
        FSController::getInstance()->scanning = false;
		scanner_state = 3; // request to stop scanning
    }

}

void MainWindow::on_pushButton_clicked()
{
	QString str;

	filecount = 0;
	fileindex = 0;
	
	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFile(L"C:\\scanimage\\img*.bmp", &wfd);
	while(hFind != INVALID_HANDLE_VALUE) {
		if(wfd.cFileName[10] == 'n') {
			wsprintf(filelist[filecount], L"C:\\scanimage\\%s", wfd.cFileName);
			filecount++;
		}
		if(!FindNextFile(hFind, &wfd)) {
			FindClose(hFind);
			break;
		}
	}

	applyState(SCANNING);
	scanner_state = 5;
}

void MainWindow::doneScanning()
{
	scanner_state = 0;
    QSound::play("done.wav");
    this->ui->scanButton->setText("Start Scan");
    FSController::getInstance()->laser_0->disable();
    FSController::getInstance()->laser_1->disable();
    FSController::getInstance()->turntable->disable();
    applyState(POINT_CLOUD);
}

void MainWindow::redraw()
{
    //ui->widget->drawState = 0;
    ui->widget->updateGL();
}

void MainWindow::applyState(FSState s)
{
    state = s;
    switch(state){
    case SCANNING:
        this->ui->widget->drawState=0;
        this->ui->scanButton->setText("Stop Scan");
        break;
    case POINT_CLOUD:
        this->ui->scanButton->setText("Start Scan");
        //the following lines are uncommented since we do not support showing the mesh anymore but just compute and save it
        /*if(FSController::getInstance()->meshComputed){
            this->ui->toggleViewButton->setText("Show SurfaceMesh");
        }else{
            this->ui->toggleViewButton->setText("Compute SurfaceMesh");
        }*/
        break;
    case SURFACE_MESH:
        this->ui->scanButton->setText("Start Scan");
        break;
    }
}

void MainWindow::on_resolutionComboBox_currentIndexChanged(const QString &arg1)
{
    if(arg1.compare("Best")==0){
        //laserStepSize = 2*laser->degreesPerStep;
        FSController::getInstance()->turntableStepSize = FSController::getInstance()->turntable->degreesPerStep; //Here we allow maximum number of slices
		//For the geared stepper there are 2037 slices which can require 4GB
		if((360/FSController::getInstance()->turntableStepSize)>1000 /* A rough estimate */) // But if there are too many slices to fit in memory...
		{
			FSController::getInstance()->turntableStepSize = 2*FSController::getInstance()->turntable->degreesPerStep; //... we reduce the number of slices
		}
        FSController::getInstance()->yDpi = 1;
    }
    if(arg1.compare("Good")==0){
        FSController::getInstance()->turntableStepSize = 8*FSController::getInstance()->turntable->degreesPerStep;
        FSController::getInstance()->yDpi = 1;
    }
    if(arg1.compare("Normal")==0){
        FSController::getInstance()->turntableStepSize = 2*16*FSController::getInstance()->turntable->degreesPerStep;
        FSController::getInstance()->yDpi = 5;
    }
    if(arg1.compare("Poor")==0){
        FSController::getInstance()->turntableStepSize = 10*16*FSController::getInstance()->turntable->degreesPerStep;
        FSController::getInstance()->yDpi = 10;
    }
}

