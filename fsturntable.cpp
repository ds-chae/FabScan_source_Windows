#include "fsturntable.h"
#include "fscontroller.h"

FSTurntable::FSTurntable()
{
    degreesPerStep = 360.0f/ FSController::config->TURNTABLE_STEPS_360; //200.0f/16.0f; //the size of a microstep
    direction = FS_DIRECTION_CW;
    rotation = FSMakePoint(0.0f, 0.0f, 0.0f);
}

/* void FSTurntable::turnNumberOfSteps(unsigned int steps) //This routine is crap!
{	// 255 is for splitting number of steps into a number of bytes for transmission to controller.
	// '255' (not 256) because step value of '0' is useless, so only '1'-'255' is useable.
    unsigned char size = ((steps/255)+1)*2; // Number of bytes to send - like "Step",[#steps],"Step",[#steps], etc
	char *c;
    c =new char [size]; //char c[size]: create an array of bytes to send
    unsigned int s = steps;
    for(unsigned int i=0; i<=steps/255; i++){
        c[2*i]=MC_PERFORM_STEP;	//First of pair is "Step" command
        if(s<255){ // 
            c[2*i+1]=s%255; //Second is the number of steps. Here we are sending the 'last' step value
        }else{
            c[2*i+1]=255;	//Maximum count per byte is 255
            s-=255;
        }
    }
    this->selectStepper();
    FSController::getInstance()->serial->writeChars(c);
} */

extern int turn_direction;

void FSTurntable::turnNumberOfSteps(unsigned int steps)
{	// 255 is for splitting number of steps into a number of bytes for transmission to controller.
	// Arduino accepts "Step" followed byte of number of steps (i.e. 0 - 255)
#if 0
// Axiscan3D has move function. direction is determined by sign of movement.
    char c[2];	//Just two bytes, "Step" and the amount
    unsigned int s = steps;
    this->selectStepper();
    do
	{
        c[0]=MC_PERFORM_STEP;	//First of pair is "Step" command
        if(s<255){ // 
            c[1]=s%255; //Second is the number of steps. Here we are sending the 'last' step value
			s = 0;	// To signal the end (and it's mathematically correct anyway!)
        }else{
            c[1]=255;	//Maximum count per byte is 255
            s-=255;
        }
		FSController::getInstance()->serial->writeChars(c);
    } while(s > 0);  
#else
	int signed_steps = steps;
	signed_steps *= turn_direction;
	char cmdbuf[16];
	sprintf_s(cmdbuf, 16, "1f%da", signed_steps);
	for(int i = 0; cmdbuf[i]; i++) {
		FSController::getInstance()->serial->writeChar(cmdbuf[i]);
	}
#endif
}

void FSTurntable::turnSteps(int steps)
{
	char cmdbuf[16];

	sprintf_s(cmdbuf, 16, "%da", steps);
	for(int i = 0; cmdbuf[i]; i++)
		FSController::getInstance()->serial->writeChar(cmdbuf[i]);
}

void FSTurntable::turnNumberOfDegrees(double degrees)
{
	degreesPerStep = 360.0f/ FSController::config->TURNTABLE_STEPS_360; //This seems to get rounded-down (by 1) at some point in the calc
    int steps = (int)(degrees/degreesPerStep);
	qDebug() << "Steps calc: " << steps;
    if(direction==FS_DIRECTION_CW){
      rotation.y -= degrees;
    }else if(direction==FS_DIRECTION_CCW){
      rotation.y += degrees;
    }
    turnNumberOfSteps(steps);
}

int turn_direction = 1;

void FSTurntable::setDirection(FSDirection d)
{
#if 0
	// Axiscan3D has no direction command.
	// direction is determined by sign
	this->selectStepper();
    direction = d;
    char c = (d==FS_DIRECTION_CW)?MC_SET_DIRECTION_CW:MC_SET_DIRECTION_CCW;
    FSController::getInstance()->serial->writeChar(c);
#else
	turn_direction = (d == FS_DIRECTION_CW) ? 1 : -1;
#endif
}

void FSTurntable::toggleDirection(){
    FSDirection d = (direction == FS_DIRECTION_CW)?FS_DIRECTION_CCW:FS_DIRECTION_CW;
    setDirection(d);
}

void FSTurntable::selectStepper()
{
#if 0
//	Axiscan3D has no select stepper function
	char c[2];
    c[0] = MC_SELECT_STEPPER;
    c[1] = MC_TURNTABLE_STEPPER;
    FSController::getInstance()->serial->writeChars(c);
#endif
}

void FSTurntable::enable(void)
{
	this->selectStepper();
#if 0
    FSController::getInstance()->serial->writeChar(MC_TURN_STEPPER_ON);
#else
    FSController::getInstance()->serial->writeChar('1');
    FSController::getInstance()->serial->writeChar('f');
#endif
}

void FSTurntable::disable(void)
{
	this->selectStepper();
#if 0
    FSController::getInstance()->serial->writeChar(MC_TURN_STEPPER_OFF);
#else
    FSController::getInstance()->serial->writeChar('0');
    FSController::getInstance()->serial->writeChar('f');
#endif
}

void FSTurntable::setRotation(FSPoint r)
{
    rotation = r;
}

FSPoint FSTurntable::getRotation()
{
    return rotation;
}
