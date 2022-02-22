#include "fslaser.h"
#include "fscontroller.h"
#include "fsserial.h"
#include <math.h>

FSLaser::FSLaser(int index, char oncmd, char offcmd)
{
    laserPointPosition = FSMakePoint(14.0f, 0.0f, 0.0f);
//    degreesPerStep = 360.0f/200.0f/16.0f; //the size of a microstep
    degreesPerStep = 360.0f/(410.0 * 40); // dschae - pulley drived table
    direction = FS_DIRECTION_CW;
    rotation = FSMakePoint(0.0f, 0.0f, 0.0f);
	setLaserPosition(index);
    //FSController::getInstance()->controlPanel->setLaserSwipeMaxEditText(rotation.y);
	turnOffCmd = offcmd;
	turnOnCmd = oncmd;
}

void FSLaser::selectStepper()
{
    char c[2];
    c[0] = MC_SELECT_STEPPER;
    c[1] = MC_LASER_STEPPER;
#if 0
	FSController::getInstance()->serial->writeChars(c);
#else
	// Axiscan 3D has no stepper selection
#endif
}

void FSLaser::turnOn()
{
#if 0
	FSController::getInstance()->serial->writeChar(MC_TURN_LASER_ON);
#else
	FSController::getInstance()->serial->writeChar(turnOnCmd);
#endif
}

void FSLaser::turnOff()
{
#if 0
	FSController::getInstance()->serial->writeChar(MC_TURN_LASER_OFF);
#else
	FSController::getInstance()->serial->writeChar(turnOffCmd);
#endif
}

void FSLaser::turnNumberOfSteps(unsigned int steps)
{
	this->selectStepper();
    qDebug()<<"Steps: " << steps;
    unsigned char size = steps/256*2;
    char c[9999]; // c[size]
    unsigned int s = steps;
    for(unsigned int i=0; i<=(steps/256); i++){
        c[2*i]=MC_PERFORM_STEP;
        if(s<256){
            c[2*i+1]=s%256;
        }else{
            c[2*i+1]=255;
            s-=255;
        }
    }
    FSController::getInstance()->serial->writeChars(c);
    laserPointPosition.x = position.x - tan(rotation.y*M_PI/180)*position.z;
    qDebug() << "LaserPositionX"<< laserPointPosition.x;
}

void FSLaser::turnNumberOfDegrees(double degrees)
{
    int steps = (int)(degrees/degreesPerStep);
    //make sure to correctly update rotation in degrees, not steps
    degrees = (double)steps*(double)degreesPerStep;
    qDebug()<<"Steps"<<steps<<"Degrees"<<degrees;
    if(direction==FS_DIRECTION_CCW){
      rotation.y += degrees;
    }else if(direction==FS_DIRECTION_CW){
      rotation.y -= degrees;
    }
    qDebug()<<"computed number of steps";
    turnNumberOfSteps(steps);
    FSController::getInstance()->controlPanel->setLaserAngleText(rotation.y);
}

void FSLaser::turnToAngle(float degrees)
{
    double degreesToTurn = this->rotation.y - degrees;
    if(degreesToTurn<0){
        setDirection(FS_DIRECTION_CCW);
        turnNumberOfDegrees(degreesToTurn*-1);
    }else{
        setDirection(FS_DIRECTION_CW);
        turnNumberOfDegrees(degreesToTurn);
    }
}

void FSLaser::setDirection(FSDirection d)
{
	this->selectStepper();
    direction = d;
    char c = (d==FS_DIRECTION_CW)?MC_SET_DIRECTION_CW:MC_SET_DIRECTION_CCW;
    FSController::getInstance()->serial->writeChar(c);
}

void FSLaser::toggleDirection(){
    FSDirection d = (direction == FS_DIRECTION_CW)?FS_DIRECTION_CCW:FS_DIRECTION_CW;
    setDirection(d);
}

void FSLaser::enable(void)
{
    this->selectStepper();
    FSController::getInstance()->serial->writeChar(MC_TURN_STEPPER_ON);
}

void FSLaser::disable(void)
{
    this->selectStepper();
    FSController::getInstance()->serial->writeChar(MC_TURN_STEPPER_OFF);
}

void FSLaser::setLaserPointPosition(FSPoint p)
{
    laserPointPosition = p;
    double b = position.x - laserPointPosition.x;
    double a = position.z - laserPointPosition.z;
	if(a == 0) a=0.1; //Kludge to avoid divide-by-zero error
    rotation.y = atan(b/a)*180.0/M_PI;
    qDebug() << "Current laser angle: "<<rotation.y;
    FSController::getInstance()->controlPanel->setLaserAngleText(rotation.y);
}

void FSLaser::setLaserPosition(int index) //Load configuration data into structure
{
	position = FSMakePoint(
	    FSController::config->LASER_POS_X,
		FSController::config->LASER_POS_Y,
        FSController::config->LASER_POS_Z);
	if(index != 0) {
		position.x = -position.x; // left line laser
	}

}

FSPoint FSLaser::getLaserPointPosition(void)
{
    return laserPointPosition;
}

FSPoint FSLaser::getPosition()
{
    return position;
}

FSPoint FSLaser::getRotation()
{
    return rotation;
}
