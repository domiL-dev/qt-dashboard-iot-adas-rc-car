#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QtMqtt/QMqttClient>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QDateTime>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qsizepolicy.h>

class Dashboard : public QMainWindow
{
    Q_OBJECT;

private:
    QMqttClient *m_client;

    QLabel *m_statusLabel;
    QLabel *m_statusPicoLabel;
    QLabel *m_statusDriveDirectionLabel;
    QLabel *m_valueSteeringAngelLabel;

    QPlainTextEdit *m_logTextEdit;

    QPushButton *m_controlButtonAccelerate;
    QPushButton *m_controlButtonReverse;
    QPushButton *m_controlButtonLeft;
    QPushButton *m_controlButtonRight;
    QPushButton *m_controlButtonStop;
    QPushButton *m_controlButtonDriveDirectionForward;
    QPushButton *m_controlButtonDriveDirectionReverse;
    QPushButton *m_controlButtonGetGpio;

    //set PWM Level


    QRadioButton *m_radioButton25;
    QRadioButton *m_radioButton50;
    QRadioButton *m_radioButton75;
    QRadioButton *m_radioButton100;


    //left sidebar
    QPushButton *m_enableDebugLogButton;
    QPushButton *m_resetDebugLogText;
    bool m_enableDebugLogState = false;

    QPushButton *m_enableGpioDebugLogButton;
    bool m_enableGpioDebugLogState = false;

    enum VehicleControlCommand {
        ACCELERATE,
        STOP_ACCELERATE,
        REVERSE,
        STOP_REVERSE,
        LEFT,
        STOP_LEFT,
        RIGHT,
        STOP_RIGHT,
        BREAK,
        STOP_BREAK,
        setFORWARD,
        setREVERSE,
        setPWM_LEVEL_25,
        setPWM_LEVEL_50,
        setPWM_LEVEL_75,
        setPWM_LEVEL_100,
        getGPIO
    };


    void controlVehicle(enum VehicleControlCommand command);


public:
    Dashboard(QWidget *parent = nullptr); 

    // Dashboard(QWidget *parent = nullptr) : QMainWindow(parent) {
    //     // 1. UI-Elemente rein per Code erstellen
    //     auto *centralWidget = new QWidget(this);
    //     auto *mainLayout = new QVBoxLayout(centralWidget);
    //     auto *statusLayout = new QHBoxLayout();

    //     m_statusLabel = new QLabel("Status: Getrennt", this);
    //     m_dataLabel = new QLabel("Sensorwert: --", this);
    //     m_controlButton = new QPushButton("Gerät einschalten", this);
    //     m_controlButtonAccelerate = new QPushButton("Accelerate", this);
    //     m_controlButtonReverse = new QPushButton("Reverse", this);
    //     m_controlButtonLeft = new QPushButton("Left", this);
    //     m_controlButtonRight = new QPushButton("Right", this);

    //     statusLayout->addWidget(m_statusLabel);
    //     mainLayout->addLayout(statusLayout);
    //     mainLayout->addWidget(m_dataLabel);
    //     mainLayout->addWidget(m_controlButton);
    //     mainLayout->addWidget(m_controlButtonAccelerate);
    //     mainLayout->addWidget(m_controlButtonReverse);
    //     mainLayout->addWidget(m_controlButtonLeft);
    //     mainLayout->addWidget(m_controlButtonRight);
    //     setCentralWidget(centralWidget);

    //     // 2. MQTT Client aufsetzen
    //     m_client = new QMqttClient(this);
    //     m_client->setHostname("192.168.178.140"); // localhost
    //     m_client->setPort(1883);

    //     // 3. Signal-Slot-Verbindungen (Logik)
    //     connect(m_client, &QMqttClient::stateChanged, this, &Dashboard::updateStatus);
    //     connect(m_controlButton, &QPushButton::clicked, this, &Dashboard::toggleDevice);
        
    //     connect(m_controlButtonAccelerate, &QPushButton::clicked, this, &Dashboard::accelerateVehicle);
    //     connect(m_controlButtonReverse, &QPushButton::clicked, this, &Dashboard::reverseVehicle);
    //     connect(m_controlButtonLeft, &QPushButton::clicked, this, &Dashboard::leftVehicle);
    //     connect(m_controlButtonRight, &QPushButton::clicked, this, &Dashboard::rightVehicle);

    //     connect(m_client, &QMqttClient::connected, this, [this]() {
    //         // Nach Verbindung ein Topic abonnieren
    //         auto subscription = m_client->subscribe(QString("mein/dashboard/sensor"));
    //         if (subscription) {
    //             connect(subscription, &QMqttSubscription::messageReceived, this, [this](QMqttMessage msg) {
    //                 m_dataLabel->setText(QString("Sensorwert: %1").arg(QString(msg.payload())));
    //             });
    //         }
    //     });

    //     // Verbindung herstellen
    //     m_client->connectToHost();
    // }

private slots:
    void updateStatus();
    void updatePicoStatus(const QString &message);
    void updateSteeringAngle(const QString &message);
    void updateDriveDirectionStatus(const QString &message);
    void enableDebugLog();
    void enableGpioDebugLog();
    void resetDebugLog();
    void logDebug(const QString &topic, const QString &message);


protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // MAINWINDOW_H
