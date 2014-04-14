#include "QsLog.h"
#include "MainWindow.h"
#include "SerialLink.h"
#include "QGCMAVLinkLogPlayer.h"
#include "QGC.h"
#include "ui_QGCMAVLinkLogPlayer.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

QGCMAVLinkLogPlayer::QGCMAVLinkLogPlayer(MAVLinkProtocol* mavlink, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QGCMAVLinkLogPlayer),
    m_logLink(NULL),
    m_mavlink(mavlink),
    m_logLoaded(false)
{
    ui->setupUi(this);
    ui->horizontalLayout->setAlignment(Qt::AlignTop);
    // Setup buttons
    connect(ui->selectFileButton, SIGNAL(clicked()), this, SLOT(loadLogButtonClicked()));
    connect(ui->playButton, SIGNAL(clicked()), this, SLOT(playButtonClicked()));
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(speedSliderValueChanged(int)));
    //connect(ui->positionSlider, SIGNAL(valueChanged(int)), this, SLOT(jumpToSliderVal(int)));
    //connect(ui->positionSlider, SIGNAL(sliderPressed()), this, SLOT(pause()));

    //setAccelerationFactorInt(49);
    ui->speedSlider->setValue(49);
    ui->positionSlider->setValue(ui->positionSlider->minimum());

    ui->playButton->setEnabled(false);
    ui->speedSlider->setEnabled(true);
    ui->positionSlider->setEnabled(true);
    ui->speedLabel->setEnabled(false);
    ui->logFileNameLabel->setEnabled(false);
    ui->logStatsLabel->setEnabled(false);
}

QGCMAVLinkLogPlayer::~QGCMAVLinkLogPlayer()
{
    storeSettings();
    delete ui;
}
void QGCMAVLinkLogPlayer::storeSettings()
{
    // Nothing to store
}
void QGCMAVLinkLogPlayer::loadLogButtonClicked()
{
    if (m_logLoaded)
    {
        if (m_logLink)
        {
            //Stop the mavlink, schedule for deletion
            if (m_logLink->isRunning())
            {
                m_logLink->stop();
            }
            else
            {
                m_logLink->deleteLater();
                m_logLink = 0;
                m_logLoaded = false;
            }
        }
        else
        {
            m_logLoaded = false;
        }
        return;
    }


    QString fileName = QFileDialog::getOpenFileName(this, tr("Specify MAVLink log file name to replay"), QGC::MAVLinkLogDirectory(), tr("MAVLink Telemetry log (*.tlog)"));
    if (fileName == "")
    {
        //No file selected/cancel clicked
        return;
    }
    m_logLoaded = true;
    m_logLink = new TLogReplyLink(this);
    connect(m_logLink,SIGNAL(logProgress(qint64,qint64)),this,SLOT(logProgress(qint64,qint64)));


    m_logLink->setLog(fileName);
    connect(m_logLink,SIGNAL(bytesReceived(LinkInterface*,QByteArray)),m_mavlink,SLOT(receiveBytes(LinkInterface*,QByteArray)));
    connect(m_logLink,SIGNAL(terminated()),this,SLOT(logLinkTerminated()));
    m_logLink->connect();
}
void QGCMAVLinkLogPlayer::logProgress(qint64 pos,qint64 total)
{
    ui->positionSlider->setValue(((double)pos / (double)total) * 100);
}

void QGCMAVLinkLogPlayer::playButtonClicked()
{

}
void QGCMAVLinkLogPlayer::logLinkTerminated()
{
    if (m_logLink->toBeDeleted())
    {
        //Log loop has terminated with the intention of unloading the sim link
        m_logLink->deleteLater();
        m_logLink = 0;
        m_logLoaded = false;
    }
}

void QGCMAVLinkLogPlayer::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
void QGCMAVLinkLogPlayer::speedSliderValueChanged(int value)
{
    if (m_logLink)
    {
        m_logLink->setSpeed(value);
    }
}
