#include "mainwindow.h"
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

    m_controlDebugInfoButton = new QPushButton("Debug Info", this);
    m_controlDebugInfoButton->setMaximumWidth(150);
    sidebarLayout->addWidget(m_controlDebugInfoButton);

    sidebarLayout->addStretch();

    // Grid Layout
    QGridLayout *dashboardGrid = new QGridLayout();

    // Grid Elements
    m_logTextEdit = new QPlainTextEdit("Debug Log:");
    m_resetDebugLogText = new QPushButton("clear - Debug Log", this);

    m_statusLabel = new QLabel("Status: Getrennt", this);
    m_controlButtonAccelerate = new QPushButton("Accelerate", this);
    m_controlButtonReverse = new QPushButton("Reverse", this);
    m_controlButtonLeft = new QPushButton("Left", this);
    m_controlButtonRight = new QPushButton("Right", this);

    // Organizing Grid Elements
    dashboardGrid->addWidget(m_logTextEdit,1,5, 10, 3);
    dashboardGrid->addWidget(m_resetDebugLogText, 11, 5, 1, 3);

    dashboardGrid->addWidget(m_statusLabel, 0, 1);
    dashboardGrid->addWidget(m_controlButtonAccelerate, 3, 2);
    dashboardGrid->addWidget(m_controlButtonReverse, 4, 2);
    dashboardGrid->addWidget(m_controlButtonLeft, 4, 1);
    dashboardGrid->addWidget(m_controlButtonRight, 4, 3);


        mainLayout->addLayout(sidebarLayout);
        mainLayout->addLayout(dashboardGrid);

        // 2. MQTT Client aufsetzen
        m_client = new QMqttClient(this);
        m_client->setHostname(Config::MQTT_HOST); 
        m_client->setPort(Config::MQTT_PORT);

        // 3. Signal-Slot-Verbindungen (Logik)
        connect(m_client, &QMqttClient::stateChanged, this, &Dashboard::updateStatus);
        connect(m_controlDebugInfoButton, &QPushButton::clicked, this, &Dashboard::activateDebugLog);
        connect(m_resetDebugLogText, &QPushButton::clicked, this, &Dashboard::resetDebugLog);

        connect(m_controlButtonAccelerate, &QPushButton::pressed, this,[this]()  {Dashboard::controlVehicle(VehicleControlCommand::ACCELERATE);});
        connect(m_controlButtonAccelerate, &QPushButton::released, this,[this]()  {Dashboard::controlVehicle(VehicleControlCommand::STOP_DRIVE);});
        connect(m_controlButtonReverse, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::REVERSE);});
        connect(m_controlButtonReverse, &QPushButton::released, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::STOP_DRIVE);});
        connect(m_controlButtonLeft, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::LEFT);});
        connect(m_controlButtonLeft, &QPushButton::released, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::STOP_STEERING);});
        connect(m_controlButtonRight, &QPushButton::pressed, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::RIGHT);});
        connect(m_controlButtonRight, &QPushButton::released, this, [this](){Dashboard::controlVehicle(VehicleControlCommand::STOP_STEERING);});

        // subscribe topic after connecting to Mqtt Server
        connect(m_client, &QMqttClient::connected, this, [this]() {
        m_client->subscribe(QString("mein/dashboard/sensor"));
        m_client->subscribe(QString("/Pico2W/Debug"));
        });
        
        connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
    
            QString topicStr = topic.name();
            QString msgStr = QString::fromUtf8(message);

            if (topicStr == "/Pico2W/Debug") {
            // Hier befüllen Sie direkt Ihr neues Terminal aus dem letzten Schritt!
                logDebug(topicStr, msgStr);
            
                //m_logTextEdit->appendPlainText(QString("[%1] %2").arg(topicStr, msgStr));
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
            case QMqttClient::Disconnected: m_statusLabel->setText("Status: Getrennt"); break;
            case QMqttClient::Connecting:   m_statusLabel->setText("Status: Verbinden..."); break;
            case QMqttClient::Connected:    m_statusLabel->setText("Status: Verbunden"); break;
        }
    }

    void Dashboard::activateDebugLog(){
        if (m_client->state() != QMqttClient::Connected) return;

        m_debugInfoState = !m_debugInfoState;
        QString payload = m_debugInfoState ? "ON" : "OFF";
        
        // Befehl über MQTT senden
        m_client->publish(QString("activate/Debug"), payload.toUtf8());
        m_controlDebugInfoButton->setText(m_debugInfoState ? "Debug Info (on)" : "Debug Info (off)");
    }

    void Dashboard::resetDebugLog(){
        m_logTextEdit->setPlainText("Debug Log:");
    }

    void Dashboard::logDebug(const QString &topic, const QString &message) {
        // 1. Zeitstempel generieren (optional, aber sehr nützlich für Terminal-Logs)
        QString timeString = QDateTime::currentDateTime().toString("hh:mm:ss");

        // 2. Log-Zeile zusammenbauen
        QString logLine = QString("[%1] %2: %3").arg(timeString).arg(topic).arg(message);

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
                m_client->publish(QString("/control/Vehicle"), "ACCELERATE");
                m_controlButtonAccelerate->setText("Accelerate Pressed");
                break;
            case REVERSE: 
                m_client->publish(QString("/control/Vehicle"), "REVERSE"); 
                m_controlButtonReverse->setText("Reverse Pressed");
                break;
            case LEFT: 
                m_client->publish(QString("/control/Vehicle"), "LEFT"); 
                m_controlButtonLeft->setText("Left Pressed");
                break;
            case RIGHT: 
                m_client->publish(QString("/control/Vehicle"), "RIGHT"); 
                m_controlButtonRight->setText("Right Pressed");
                break;
            case STOP_DRIVE: 
                m_client->publish(QString("/control/Vehicle"), "STOP DRIVE");
                m_controlButtonAccelerate->setText("Accelerate");
                m_controlButtonReverse->setText("Reverse");
                break;
            case STOP_STEERING: 
                m_client->publish(QString("/control/Vehicle"), "STOP STEERING"); 
                m_controlButtonLeft->setText("Left");
                m_controlButtonRight->setText("Right");
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
            default: QMainWindow::keyReleaseEvent(event); // allow other keys to be processed normally
        }
    }