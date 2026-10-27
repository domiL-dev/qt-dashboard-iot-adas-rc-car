#include "mainwindow.h"
#include <cstring>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qstringview.h>
#include <qwidget.h>
#include "config.private.h"

Dashboard::Dashboard(QWidget *parent) : QMainWindow(parent){
    // Central-Widget
    QWidget *centralWiget = new QWidget(this);
    setCentralWidget(centralWiget);

    // Main-Layout for whole window
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWiget);
   
   // Sidebar left side - feature on/off 
    QVBoxLayout *sidebarLayout = new QVBoxLayout();

    m_enableDebugLogButton = new QPushButton("Enable Debug Log", this);
    m_enableDebugLogButton->setMaximumWidth(150);
    sidebarLayout->addWidget(m_enableDebugLogButton);

    m_enableGpioDebugLogButton = new QPushButton("Enable GPIO Debug Log", this);
    m_enableGpioDebugLogButton->setMaximumWidth(150);
    sidebarLayout->addWidget(m_enableGpioDebugLogButton);

    m_controlButtonGetGpio = new QPushButton("Log GPIO",this);
    m_controlButtonGetGpio->setMaximumWidth(150);
    sidebarLayout->addWidget(m_controlButtonGetGpio);

    sidebarLayout->addStretch();

    // Grid Layout
    QGridLayout *dashboardGrid = new QGridLayout();

    // Grid Elements
    m_logTextEdit = new QPlainTextEdit("Debug Log:");
    m_resetDebugLogText = new QPushButton("clear - Debug Log", this);

    m_statusLabel = new QLabel("MQTT Status (QT): Getrennt", this);
    m_statusPicoLabel = new QLabel("MQTT Status (Pico 2W): Getrennt", this);
    m_statusDriveDirectionLabel = new QLabel("Drive Direction: FORWARD", this);
    m_valueSteeringAngelLabel = new QLabel("Steering Angle: - ", this);

    m_controlButtonAccelerate = new QPushButton("Accelerate", this);
    m_controlButtonReverse = new QPushButton("Reverse", this);
    m_controlButtonLeft = new QPushButton("Left", this);
    m_controlButtonRight = new QPushButton("Right", this);
    m_controlButtonStop = new QPushButton("Break", this);
    m_controlButtonDriveDirectionForward = new QPushButton("Forward",this);
    m_controlButtonDriveDirectionReverse = new QPushButton("Reverse",this);

    QGroupBox *groupBoxPwmLevel = new QGroupBox("PWM Level", this);

    m_radioButton25 = new QRadioButton("25 %");
    m_radioButton50 = new QRadioButton("50 %");
    m_radioButton75 = new QRadioButton("75 %");
    m_radioButton100 = new QRadioButton("100 %");

    QButtonGroup *pwmLevelGroup = new QButtonGroup(this);

    pwmLevelGroup->addButton(m_radioButton25, 25);
    pwmLevelGroup->addButton(m_radioButton50, 50);
    pwmLevelGroup->addButton(m_radioButton75, 75);
    pwmLevelGroup->addButton(m_radioButton100, 100);
    
    QHBoxLayout *pwmGroupLayout = new QHBoxLayout(groupBoxPwmLevel);

    pwmGroupLayout->addWidget(m_radioButton25);
    pwmGroupLayout->addWidget(m_radioButton50);
    pwmGroupLayout->addWidget(m_radioButton75);
    pwmGroupLayout->addWidget(m_radioButton100);


    // 
    m_logTextEdit->setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: black;"                                      // Hintergrund komplett schwarz
        "    color: white;"                                                 // Schriftfarbe komplett weiß
        "    font-family: 'Segoe UI Mono', 'Consolas', 'Courier New', monospace;" // Sehr gut lesbare Monospace-Schriften
        "    font-size: 10pt;"                                              // Etwas größer für bessere Lesbarkeit
        "    line-height: 120%;"                                            // Mehr Abstand zwischen den Zeilen
        "}"
    );

    // setting tab-width to 4 spaces auf
    QFontMetrics metrics(m_logTextEdit->font());
    m_logTextEdit->setTabStopDistance(10 * metrics.horizontalAdvance(' '));

    // Organizing Grid Elements
    dashboardGrid->addWidget(m_logTextEdit,1,5, 10, 3);
    dashboardGrid->addWidget(m_resetDebugLogText, 11, 5, 1, 3);

    dashboardGrid->addWidget(m_statusLabel, 0, 1);
    dashboardGrid->addWidget(m_statusPicoLabel, 1, 1);

    dashboardGrid->addWidget(m_controlButtonAccelerate, 3, 2);
    dashboardGrid->addWidget(m_controlButtonReverse, 4, 2);
    dashboardGrid->addWidget(m_controlButtonLeft, 4, 1);
    dashboardGrid->addWidget(m_controlButtonRight, 4, 3);
    dashboardGrid->addWidget(m_controlButtonStop, 5, 1, 1, 3);
    dashboardGrid->addWidget(m_statusDriveDirectionLabel, 6, 1);
    dashboardGrid->addWidget(m_controlButtonDriveDirectionForward, 6, 2);
    dashboardGrid->addWidget(m_controlButtonDriveDirectionReverse, 6, 3);

    dashboardGrid->addWidget(groupBoxPwmLevel, 7, 1 );
    dashboardGrid->addWidget(m_valueSteeringAngelLabel, 8, 1 );


        mainLayout->addLayout(sidebarLayout);
        mainLayout->addLayout(dashboardGrid);

        // 2. MQTT Client aufsetzen
        m_client = new QMqttClient(this);
        m_client->setHostname(Config::MQTT_HOST); 
        m_client->setPort(Config::MQTT_PORT);

        // 3. Signal-Slot-Verbindungen (Logik)
        connect(m_client, &QMqttClient::stateChanged, this, &Dashboard::updateStatus);
        connect(m_enableDebugLogButton, &QPushButton::clicked, this, &Dashboard::enableDebugLog);
        connect(m_enableGpioDebugLogButton, &QPushButton::clicked, this, &Dashboard::enableGpioDebugLog);
        connect(m_resetDebugLogText, &QPushButton::clicked, this, &Dashboard::resetDebugLog);

        connect(m_controlButtonAccelerate, &QPushButton::pressed, this,[this]()  {Dashboard::controlVehicle(VehicleControlCommand::ACCELERATE);});
        connect(m_controlButtonAccelerate, &QPushButton::released, this,[this]()  {Dashboard::controlVehicle(VehicleControlCommand::STOP_ACCELERATE);});
        connect(m_controlButtonReverse, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::REVERSE);});
        connect(m_controlButtonReverse, &QPushButton::released, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::STOP_REVERSE);});
        connect(m_controlButtonStop, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::BREAK);});
        connect(m_controlButtonStop, &QPushButton::released, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::STOP_BREAK);});
        connect(m_controlButtonLeft, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::LEFT);});
        connect(m_controlButtonLeft, &QPushButton::released, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::STOP_LEFT);});
        connect(m_controlButtonRight, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::RIGHT);});
        connect(m_controlButtonRight, &QPushButton::released, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::STOP_RIGHT);});
        connect(m_controlButtonDriveDirectionForward, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::setFORWARD);});
        connect(m_controlButtonDriveDirectionReverse, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::setREVERSE);});
        connect(m_controlButtonGetGpio, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::getGPIO);});
        connect(m_radioButton25, &QRadioButton::clicked, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::setPWM_LEVEL_25);});
        connect(m_radioButton50, &QRadioButton::clicked, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::setPWM_LEVEL_50);});
        connect(m_radioButton75, &QRadioButton::clicked, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::setPWM_LEVEL_75);});
        connect(m_radioButton100, &QRadioButton::clicked, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::setPWM_LEVEL_100);});

        // subscribe topic after connecting to Mqtt Server
        connect(m_client, &QMqttClient::connected, this, [this]() {
        //m_client->subscribe(QString("mein/dashboard/sensor"));
        m_client->subscribe(QString("/Pico2W/Debug"));
        m_client->subscribe(QString("/drive/direction"));
        m_client->subscribe(QString("/steering/angle"));
        m_client->subscribe(QString("/pico2/online"));
        m_client->subscribe(QString("/online"));
        });
        
        connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
    
            QString topicStr = topic.name();
            QString msgStr = QString::fromUtf8(message);

            if (topicStr == "/steering/angle") {
                updateSteeringAngle(message);
            } 
            else if (topicStr == "/Pico2W/Debug") {
                logDebug(topicStr, msgStr);
            }
            else if(topicStr == "/pico2/online"){
                updatePicoStatus(msgStr);
            }
            else if(topicStr == "/online"){
                updatePicoStatus(msgStr);
            }
            else if(topicStr == "/drive/direction"){
                updateDriveDirectionStatus(msgStr);
            }
        });
/*
        connect(m_client, &QMqttClient::connected, this, [this]() {
            // Nach Verbindung ein Topic abonnieren
            auto subscription = m_client->subscribe(QString("mein/dashboard/sensor"));
            if (subscription) {
                connect(subscription, &QMqttSubscription::messageReceived, this, [this](QMqttMessage msg) {
                    m_dataLabel->setText(QString("Sensorwert: %1").arg(QString(msg.payload())));
                });
            }
        });
*/
        // Verbindung herstellen
        m_client->connectToHost(); 
}

//private slots:

    void Dashboard::updateStatus() {
        switch (m_client->state()) {
            case QMqttClient::Disconnected: m_statusLabel->setText("MQTT Status (QT): Getrennt"); break;
            case QMqttClient::Connecting:   m_statusLabel->setText("MQTT Status (QT): Verbinden..."); break;
            case QMqttClient::Connected:    m_statusLabel->setText("MQTT Status (QT): Verbunden"); break;
        }
    }

    void Dashboard::updatePicoStatus(const QString &message){
        if(message == "0"){
            m_statusPicoLabel->setText("MQTT Status (Pico 2W): Getrennt");
        } else if (message == "1") {
            m_statusPicoLabel->setText("MQTT Status (Pico 2W): Verbunden");
        }
    }

    void Dashboard::updateDriveDirectionStatus(const QString &message){
        if(message == "REVERSE"){
            m_statusDriveDirectionLabel->setText("Drive Direction: Reverse");
        } else if (message == "FORWARD") {
            m_statusDriveDirectionLabel->setText("Drive Direction: Forward");
        }
    }

    void Dashboard::updateSteeringAngle(const QString &message){
        m_valueSteeringAngelLabel->setText(QString("Steering Angle: %1").arg(message));
    } 

    void Dashboard::enableDebugLog(){
        if (m_client->state() != QMqttClient::Connected) return;

        m_enableDebugLogState = !m_enableDebugLogState;
        QString payload = m_enableDebugLogState ? "Debug ON" : "Debug OFF";
        
        // Befehl über MQTT senden
        m_client->publish(QString("/set/Debug"), payload.toUtf8());
        m_enableDebugLogButton->setText(m_enableDebugLogState ? "Disable Debug Log" : "Enable Debug Log");
    }

    void Dashboard::enableGpioDebugLog(){
        if (m_client->state() != QMqttClient::Connected) return;

        m_enableGpioDebugLogState = !m_enableGpioDebugLogState;
        QString payload = m_enableGpioDebugLogState ? "Gpio Debug ON" : "Gpio Debug OFF";
        
        m_client->publish(QString("/set/Debug"), payload.toUtf8());
        m_enableGpioDebugLogButton->setText(m_enableGpioDebugLogState ? "Disable Gpio Debug" : "Enable Gpio Debug");
    }

    void Dashboard::resetDebugLog(){
        m_logTextEdit->setPlainText("Debug Log:");
    }

    void Dashboard::logDebug(const QString &topic, const QString &message) {
        // 1. generate Timestamp
        QString timeString = QDateTime::currentDateTime().toString("hh:mm:ss");

        // 2. concatenate Debug Log
        QString logLine = QString("[%1] %2\n").arg(timeString).arg(message);

        // 3. Zeile im Terminal hinten anhängen
        m_logTextEdit->appendPlainText(logLine);

        // 4. Automatisch ganz nach unten scrollen (falls Qt es nicht von alleine tut)
        m_logTextEdit->ensureCursorVisible();
        m_logTextEdit->moveCursor(QTextCursor::End);
    }

    void Dashboard::controlVehicle(enum VehicleControlCommand command) {
        if (m_client->state() != QMqttClient::Connected) return;

        switch (command) {
            case ACCELERATE: 
                m_client->publish(QString("/control/Vehicle"), "A1");
                m_controlButtonAccelerate->setText("Accelerating");
                break;
            case STOP_ACCELERATE: 
                m_client->publish(QString("/control/Vehicle"), "A0");
                m_controlButtonAccelerate->setText("(W) Accelerate");
                break;
            case REVERSE: 
                m_client->publish(QString("/control/Vehicle"), "R1"); 
                m_controlButtonReverse->setText("Reversing");
                break;
            case STOP_REVERSE: 
                m_client->publish(QString("/control/Vehicle"), "R0"); 
                m_controlButtonReverse->setText("(S) Reverse");
                break;
            case BREAK: 
                m_client->publish(QString("/control/Vehicle"), "B1");
                m_controlButtonStop->setText("Breaking");
                break;
            case STOP_BREAK: 
                m_client->publish(QString("/control/Vehicle"), "B0");
                m_controlButtonStop->setText("(B) Break");
                break;
            case LEFT: 
                m_client->publish(QString("/control/Vehicle"), "SL"); 
                m_controlButtonLeft->setText("Left");
                break;
            case STOP_LEFT: 
                m_client->publish(QString("/control/Vehicle"), "SS"); 
                m_controlButtonLeft->setText("(A) Left");
                break;
            case RIGHT: 
                m_client->publish(QString("/control/Vehicle"), "SR"); 
                m_controlButtonRight->setText("(D) Right");
                break;
            case STOP_RIGHT: 
                m_client->publish(QString("/control/Vehicle"), "SS"); 
                m_controlButtonRight->setText("Right");
                break;
            case setFORWARD: 
                m_client->publish(QString("/control/Vehicle"), "SET FORWARD"); 
                break;
            case setREVERSE: 
                m_client->publish(QString("/control/Vehicle"), "SET REVERSE"); 
                break;
            case setPWM_LEVEL_25:
            m_client->publish(QString("/control/Vehicle"), "PWM 25"); 
                break;
            case setPWM_LEVEL_50:
            m_client->publish(QString("/control/Vehicle"), "PWM 50"); 
                break;
            case setPWM_LEVEL_75:
            m_client->publish(QString("/control/Vehicle"), "PWM 75"); 
                break;
            case setPWM_LEVEL_100:
            m_client->publish(QString("/control/Vehicle"), "PWM 100"); 
                break;
            case getGPIO: 
                m_client->publish(QString("/control/Vehicle"), "GET GPIO"); 
                break;
            
            default: break;
        }

    }


//protected slots:
    void Dashboard::keyPressEvent(QKeyEvent *event) {
        if (event->isAutoRepeat()) {
            return;
        }

        if (m_client->state() != QMqttClient::Connected) return;

        switch (event->key()) {
            case Qt::Key_W: m_controlButtonAccelerate->pressed(); break;
            case Qt::Key_S: m_controlButtonReverse->pressed(); break;
            case Qt::Key_A: m_controlButtonLeft->pressed(); break;
            case Qt::Key_D: m_controlButtonRight->pressed(); break;
            case Qt::Key_B: m_controlButtonStop->pressed(); break;
            case Qt::Key_R: m_controlButtonDriveDirectionReverse->pressed(); break;
            case Qt::Key_F: m_controlButtonDriveDirectionForward->pressed(); break;
            default: QMainWindow::keyPressEvent(event); // allow other keys to be processed normally
        }
    }

    void Dashboard::keyReleaseEvent(QKeyEvent *event) {
        if (event->isAutoRepeat()) {
            return;
        }

        if (m_client->state() != QMqttClient::Connected) return;

        switch (event->key()) {
            case Qt::Key_W: m_controlButtonAccelerate->released(); break;
            case Qt::Key_S: m_controlButtonReverse->released(); break;
            case Qt::Key_A: m_controlButtonLeft->released(); break;
            case Qt::Key_D: m_controlButtonRight->released(); break;
            case Qt::Key_B: m_controlButtonStop->released(); break;
            default: QMainWindow::keyReleaseEvent(event); // allow other keys to be processed normally
        }
    }