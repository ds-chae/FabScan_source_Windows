#include "fsserial.h"
#include <QThread>
#include "fscontroller.h"

FSSerial::FSSerial()
{
    serialPortPath = new QString();
}

bool FSSerial::connectToSerialPort()
{
    this->serialPort = new QextSerialPort(*serialPortPath, QextSerialPort::EventDriven);
    serialPort->setBaudRate(BAUD19200);
    serialPort->setFlowControl(FLOW_OFF);
    serialPort->setParity(PAR_NONE);
    serialPort->setDataBits(DATA_8);
    serialPort->setStopBits(STOP_1);

    if (serialPort->open(QIODevice::ReadWrite) == true) {
        connect(serialPort, SIGNAL(readyRead()), this, SLOT(onReadyRead()) );
        connect(serialPort, SIGNAL(dsrChanged(bool)), this, SLOT(onDsrChanged(bool)) );
        if (!(serialPort->lineStatus() & LS_DSR)){
            qDebug() << "warning: device is not turned on";
            return true; // Was false
        }
        qDebug() << "listening for data on" << serialPort->portName();
        return true;
    }else{
        qDebug() << "device failed to open:" << serialPort->errorString();
        return false; // Was true
    }
}

void FSSerial::disconnectFromSerialPort()
{
	if (serialPort->isOpen())
	{
		serialPort->disconnect(SIGNAL(readyRead()), this, SLOT(onReadyRead()));
		serialPort->disconnect(SIGNAL(dsrChanged(bool)), this, SLOT(onDsrChanged(bool)));
		serialPort->close();
	}
}

void FSSerial::onReadyRead()
{
    QByteArray bytes;
    int a = serialPort->bytesAvailable();
    bytes.resize(a);
    serialPort->read(bytes.data(), bytes.size());
    qDebug() << "Bytes read " << bytes.size() << "bytes=" << (int) ((unsigned char) *bytes.data());
	//FSController::getInstance()->Arduino_data = (int) ((unsigned char) *bytes.data());
}

void FSSerial::onDsrChanged(bool)
{
    qDebug("onDsrChanged");
}

void FSSerial::writeChar(char c)
{
    qDebug() << "writing to serial port: " << (int)((unsigned char)c);
    if( serialPortPath->isEmpty() ) return;
    if( !serialPort->isOpen() ) return;
    if( serialPort->isWritable() ){
        //qDebug("is writable");
        //usleep(100000);
        serialPort->write(&c);
    }else{
        //qDebug("is not writable");
    }
}

void FSSerial::writeChars(char* c)
{
    if( serialPortPath->isEmpty() ) return;
    if( !serialPort->isOpen() ) return;
    if( serialPort->isWritable() ){
        //qDebug("is writable");
        //usleep(100000);
        serialPort->write(c);
    }else{
        //qDebug("is not writable");
    }
}

bool FSSerial::waitRead(int time)
{
	bool result;

	result= serialPort->waitForReadyRead(time);
	qDebug ("waitRead :");
	if(result){
		qDebug ("TRUE");
	}
	else qDebug() << "FALSE " << serialPort->errorString();
	return result;
}

// Not sure if this will work...
char FSSerial::readChar(void)
{
	char c = 1; // We don't expect anything other than 211 back from the Arduino, so we'll use these values as flags

    if( serialPortPath->isEmpty() )
		{
			qDebug() << "Serial port no path!";
			return 2;
	}
	if( !serialPort->isOpen() )
		{
			qDebug() << "Serial port not open!";
			return 3;
	}	
    if( serialPort->isReadable() ){
        //usleep(100000);
        serialPort->read(&c, 1); //Store byte back in c, maxLen is 1 byte
		qDebug() << "reading from serial port: " << (int)((unsigned char)c);
    }else{
        qDebug("Serial port is not readable!");
    }
	return c;
}