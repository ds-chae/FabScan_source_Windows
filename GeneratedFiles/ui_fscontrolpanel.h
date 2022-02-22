/********************************************************************************
** Form generated from reading UI file 'fscontrolpanel.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FSCONTROLPANEL_H
#define UI_FSCONTROLPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FSControlPanel
{
public:
    QGroupBox *groupBox;
    QPushButton *laserOnButton;
    QPushButton *laserOffButton;
    QPushButton *autoResetButton;
    QPushButton *laserStepLeftButton;
    QPushButton *laserStepRightButton;
    QPushButton *laser1OnButton;
    QPushButton *laser1OffButton;
    QLabel *label_2;
    QComboBox *laserComboBox;
    QGroupBox *groupBox_2;
    QPushButton *fetchFrameButton;
    QLabel *label_3;
    QLineEdit *centerEdit;
    QGroupBox *groupBox_3;
    QPushButton *stepLeftButton;
    QPushButton *stepRightButton;
    QCheckBox *checkBox;
    QLineEdit *degreesEdit;
    QLabel *label;
    QLabel *cameraLabel;
    QPushButton *pushButton_2;
    QPushButton *diffImage;

    void setupUi(QWidget *FSControlPanel)
    {
        if (FSControlPanel->objectName().isEmpty())
            FSControlPanel->setObjectName(QStringLiteral("FSControlPanel"));
        FSControlPanel->resize(751, 556);
        FSControlPanel->setMinimumSize(QSize(580, 360));
        FSControlPanel->setMaximumSize(QSize(1580, 1360));
        FSControlPanel->setAutoFillBackground(true);
        groupBox = new QGroupBox(FSControlPanel);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        groupBox->setGeometry(QRect(10, 90, 121, 171));
        laserOnButton = new QPushButton(groupBox);
        laserOnButton->setObjectName(QStringLiteral("laserOnButton"));
        laserOnButton->setGeometry(QRect(0, 20, 61, 21));
        laserOffButton = new QPushButton(groupBox);
        laserOffButton->setObjectName(QStringLiteral("laserOffButton"));
        laserOffButton->setGeometry(QRect(60, 20, 61, 21));
        autoResetButton = new QPushButton(groupBox);
        autoResetButton->setObjectName(QStringLiteral("autoResetButton"));
        autoResetButton->setEnabled(false);
        autoResetButton->setGeometry(QRect(0, 40, 121, 21));
        laserStepLeftButton = new QPushButton(groupBox);
        laserStepLeftButton->setObjectName(QStringLiteral("laserStepLeftButton"));
        laserStepLeftButton->setEnabled(false);
        laserStepLeftButton->setGeometry(QRect(10, 60, 51, 21));
        laserStepRightButton = new QPushButton(groupBox);
        laserStepRightButton->setObjectName(QStringLiteral("laserStepRightButton"));
        laserStepRightButton->setEnabled(false);
        laserStepRightButton->setGeometry(QRect(60, 60, 50, 21));
        laser1OnButton = new QPushButton(groupBox);
        laser1OnButton->setObjectName(QStringLiteral("laser1OnButton"));
        laser1OnButton->setGeometry(QRect(0, 90, 61, 21));
        laser1OffButton = new QPushButton(groupBox);
        laser1OffButton->setObjectName(QStringLiteral("laser1OffButton"));
        laser1OffButton->setGeometry(QRect(60, 90, 61, 21));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(10, 120, 56, 12));
        laserComboBox = new QComboBox(groupBox);
        laserComboBox->setObjectName(QStringLiteral("laserComboBox"));
        laserComboBox->setGeometry(QRect(10, 140, 101, 22));
        groupBox_2 = new QGroupBox(FSControlPanel);
        groupBox_2->setObjectName(QStringLiteral("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 270, 121, 121));
        fetchFrameButton = new QPushButton(groupBox_2);
        fetchFrameButton->setObjectName(QStringLiteral("fetchFrameButton"));
        fetchFrameButton->setGeometry(QRect(0, 20, 121, 21));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setGeometry(QRect(10, 90, 41, 16));
        centerEdit = new QLineEdit(groupBox_2);
        centerEdit->setObjectName(QStringLiteral("centerEdit"));
        centerEdit->setGeometry(QRect(60, 90, 51, 20));
        groupBox_3 = new QGroupBox(FSControlPanel);
        groupBox_3->setObjectName(QStringLiteral("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, 0, 121, 91));
        stepLeftButton = new QPushButton(groupBox_3);
        stepLeftButton->setObjectName(QStringLiteral("stepLeftButton"));
        stepLeftButton->setGeometry(QRect(10, 50, 51, 21));
        stepRightButton = new QPushButton(groupBox_3);
        stepRightButton->setObjectName(QStringLiteral("stepRightButton"));
        stepRightButton->setGeometry(QRect(60, 50, 50, 21));
        checkBox = new QCheckBox(groupBox_3);
        checkBox->setObjectName(QStringLiteral("checkBox"));
        checkBox->setEnabled(true);
        checkBox->setGeometry(QRect(20, 70, 70, 20));
        degreesEdit = new QLineEdit(groupBox_3);
        degreesEdit->setObjectName(QStringLiteral("degreesEdit"));
        degreesEdit->setGeometry(QRect(10, 20, 100, 22));
        degreesEdit->setLayoutDirection(Qt::LeftToRight);
        degreesEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label = new QLabel(groupBox_3);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(110, 30, 10, 16));
        cameraLabel = new QLabel(FSControlPanel);
        cameraLabel->setObjectName(QStringLiteral("cameraLabel"));
        cameraLabel->setGeometry(QRect(140, 20, 600, 450));
        cameraLabel->setAutoFillBackground(false);
        cameraLabel->setStyleSheet(QStringLiteral("background-color: black; color: white;"));
        cameraLabel->setFrameShape(QFrame::Box);
        pushButton_2 = new QPushButton(FSControlPanel);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setEnabled(false);
        pushButton_2->setGeometry(QRect(10, 330, 121, 21));
        diffImage = new QPushButton(FSControlPanel);
        diffImage->setObjectName(QStringLiteral("diffImage"));
        diffImage->setEnabled(false);
        diffImage->setGeometry(QRect(10, 310, 121, 21));
        cameraLabel->raise();
        groupBox->raise();
        groupBox_2->raise();
        groupBox_3->raise();
        pushButton_2->raise();
        diffImage->raise();

        retranslateUi(FSControlPanel);

        QMetaObject::connectSlotsByName(FSControlPanel);
    } // setupUi

    void retranslateUi(QWidget *FSControlPanel)
    {
        FSControlPanel->setWindowTitle(QApplication::translate("FSControlPanel", "Control Panel", 0));
        groupBox->setTitle(QApplication::translate("FSControlPanel", "Laser", 0));
        laserOnButton->setText(QApplication::translate("FSControlPanel", "On", 0));
        laserOffButton->setText(QApplication::translate("FSControlPanel", "Off", 0));
        autoResetButton->setText(QApplication::translate("FSControlPanel", "Detect Laser", 0));
        laserStepLeftButton->setText(QApplication::translate("FSControlPanel", "<", 0));
        laserStepRightButton->setText(QApplication::translate("FSControlPanel", ">", 0));
        laser1OnButton->setText(QApplication::translate("FSControlPanel", "1 On", 0));
        laser1OffButton->setText(QApplication::translate("FSControlPanel", "1 Off", 0));
        label_2->setText(QApplication::translate("FSControlPanel", "Select", 0));
        laserComboBox->setCurrentText(QString());
        groupBox_2->setTitle(QApplication::translate("FSControlPanel", "Camera", 0));
        fetchFrameButton->setText(QApplication::translate("FSControlPanel", "Fetch Frame", 0));
        label_3->setText(QApplication::translate("FSControlPanel", "Center", 0));
        groupBox_3->setTitle(QApplication::translate("FSControlPanel", "Turntable (degrees)", 0));
        stepLeftButton->setText(QApplication::translate("FSControlPanel", "CW", 0));
        stepRightButton->setText(QApplication::translate("FSControlPanel", "CCW", 0));
        checkBox->setText(QApplication::translate("FSControlPanel", "Enabled", 0));
        degreesEdit->setText(QApplication::translate("FSControlPanel", "10", 0));
        label->setText(QApplication::translate("FSControlPanel", "                         Select a camera from the menu!", 0));
        cameraLabel->setText(QApplication::translate("FSControlPanel", "                                                Select a camera from the menu!", 0));
        pushButton_2->setText(QApplication::translate("FSControlPanel", "Laser Image", 0));
        diffImage->setText(QApplication::translate("FSControlPanel", "Diff Image", 0));
    } // retranslateUi

};

namespace Ui {
    class FSControlPanel: public Ui_FSControlPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FSCONTROLPANEL_H
