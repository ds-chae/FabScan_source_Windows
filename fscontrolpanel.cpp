#include "fscontrolpanel.h"
#include "ui_fscontrolpanel.h"
#include "fscontroller.h"
#include "fslaser.h"
#include "fsturntable.h"
#include "fsserial.h"

#include <QDebug>
#include <QFuture>
#include <QtConcurrent/QtConcurrentRun>

int activeLaser = 3;

int centerPos = 20;

FSControlPanel::FSControlPanel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FSControlPanel)
{
    ui->setupUi(this);
    this->installEventFilter(this);
	QObject::connect(FSController::getInstance()->webcam, SIGNAL(cameraFrame(QImage)),this, SLOT(on_cameraFrame(QImage)));

	ui->laserComboBox->addItem("Laser 0");
	ui->laserComboBox->addItem("Laser 1");
	ui->laserComboBox->addItem("Both");
	ui->laserComboBox->setCurrentIndex(2);

	QString s;
	s.sprintf("%d", centerPos);
	ui->centerEdit->setText(s);
}

FSControlPanel::~FSControlPanel()
{
    delete ui;
}

void FSControlPanel::on_fetchFrameButton_clicked() // Fetch Frame button
{
    FSController::getInstance()->fetchFrame();
}

void FSControlPanel::on_laserOnButton_clicked()
{
    FSController::getInstance()->laser_0->turnOn();
}

void FSControlPanel::on_laserOffButton_clicked()
{
    FSController::getInstance()->laser_0->turnOff();
}

void FSControlPanel::on_laser1OnButton_clicked()
{
    FSController::getInstance()->laser_1->turnOn();
}

void FSControlPanel::on_laser1OffButton_clicked()
{
    FSController::getInstance()->laser_1->turnOff();
}

void FSControlPanel::on_checkBox_stateChanged(int state)
{
    if(state==2){
        FSController::getInstance()->turntable->enable();
    }else{
        FSController::getInstance()->turntable->disable();
    }
}

void FSControlPanel::on_stepLeftButton_clicked()
{
    FSController::getInstance()->turntable->setDirection(FS_DIRECTION_CW);
    QString str = ui->degreesEdit->text();
    double degrees = str.toDouble();
    FSController::getInstance()->turntable->turnNumberOfDegrees(degrees);
}

void FSControlPanel::on_stepRightButton_clicked()
{
    FSController::getInstance()->turntable->setDirection(FS_DIRECTION_CCW);
    QString str = ui->degreesEdit->text();
    double degrees = str.toDouble();
    FSController::getInstance()->turntable->turnNumberOfDegrees(degrees);
}

void FSControlPanel::on_autoResetButton_clicked() //This is "Detect Laser"
{
    if(FSController::getInstance()->webcam->info.portName.isEmpty()){
        FSController::getInstance()->mainwindow->showDialog("No webcam found!");
        return;
    }
    // FSController::getInstance()->detectLaserLine(); //Not required for my Version2
    cv::Mat shot = FSController::getInstance()->webcam->getFrame();
    cv::resize( shot,shot,cv::Size(1280,960) );
    //shot = FSController::getInstance()->vision->drawLaserLineToFrame(shot);
    cv::resize(shot,shot,cv::Size(800,600));
    cv::imshow("Laser Frame",shot);
//    cv::waitKey(0);
//    cvDestroyWindow("Laser Frame");
    this->raise();
    this->focusWidget();
    this->setVisible(true);
}

void FSControlPanel::on_laserEnable_stateChanged(int state)
{
    if(state==2){
        FSController::getInstance()->laser_0->enable();
    }else{
        FSController::getInstance()->laser_0->disable();
    }
}

void FSControlPanel::on_laserStepLeftButton_clicked()
{
    FSController::getInstance()->laser_0->setDirection(FS_DIRECTION_CCW);
    FSController::getInstance()->laser_0->turnNumberOfDegrees(2.0);
}

void FSControlPanel::on_laserStepRightButton_clicked()
{
    FSController::getInstance()->laser_0->setDirection(FS_DIRECTION_CW);
    FSController::getInstance()->laser_0->turnNumberOfDegrees(2.0);
}

void FSControlPanel::on_diffImage_clicked()
{
    if(FSController::getInstance()->webcam->info.portName.isEmpty()){
        FSController::getInstance()->mainwindow->showDialog("No webcam found!");
        return;
    }
    cv::Mat shot = FSController::getInstance()->diffImage(0);
    cv::resize(shot,shot,cv::Size(800,600));
    cv::imshow("Laser Frame",shot);
//    cv::waitKey(0);
//    cvDestroyWindow("Laser Frame");
    this->raise();
    this->focusWidget();
    this->setVisible(true);
}

void FSControlPanel::on_laserSwipeMaxEdit_returnPressed()
{
//    FSController::getInstance()->laserSwipeMax = (ui->laserSwipeMaxEdit->text()).toDouble();
}

void FSControlPanel::setLaserAngleText(double angle)
{
    QString a = QString("Angle: %1º").arg(angle);
//    this->ui->laserAngle->setText(a);
}

void FSControlPanel::on_laserSwipeMinEdit_returnPressed()
{
//    FSController::getInstance()->laserSwipeMin = (ui->laserSwipeMinEdit->text()).toDouble();
}

void FSControlPanel::on_pushButton_2_clicked() //This is "Laser Image"
{
	if(FSController::getInstance()->webcam->info.portName.isEmpty()){
        FSController::getInstance()->mainwindow->showDialog("No webcam found!");
        return;
    }

	FSController::getInstance()->laser_0->enable();
    FSController::getInstance()->laser_0->turnOff();
    QThread::msleep(200);
    cv::Mat laserOffFrame = FSController::getInstance()->webcam->getFrame();
    FSController::getInstance()->laser_0->turnOn();
    QThread::msleep(200);
    cv::Mat laserOnFrame = FSController::getInstance()->webcam->getFrame();
    cv::resize( laserOnFrame,laserOnFrame,cv::Size(1280,960) );
    cv::resize( laserOffFrame,laserOffFrame,cv::Size(1280,960) );

    qDebug() << "pressed";
    cv::Mat shot = FSVision::subLaser2(laserOffFrame, laserOnFrame);
    cv::resize(shot,shot,cv::Size(800,600));
    cv::imshow("Laser Frame",shot);
//    cv::waitKey(0);
//    cvDestroyWindow("Laser Frame");
    this->raise();
    this->focusWidget();
    this->setVisible(true);
}

void FSControlPanel::on_cameraFrame(QImage frame)
{
	// We've received a frame from CV VideoCapture::read() via fswebcam
	// It is a QImage structure.
	int width;

	width = ui->cameraLabel->width();	//We have to scale it down to fit the preview window, so get the window's width
	ui->cameraLabel->setPixmap(QPixmap::fromImage(frame.scaledToWidth(width, Qt::FastTransformation))); // Show it
}

void FSControlPanel::on_laserComboBox_currentIndexChanged(const QString &arg1)
{
    if(arg1.compare("Laser 0") == 0)
		activeLaser = 1;
    if(arg1.compare("Laser 1") == 0)
		activeLaser = 2;
    if(arg1.compare("Both") == 0)
		activeLaser = 3;
}

void FSControlPanel::on_centerEdit_textChanged(const QString &arg1)
{
	centerPos = arg1.toInt();
}
