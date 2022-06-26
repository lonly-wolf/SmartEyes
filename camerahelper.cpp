#include "camerahelper.h"
#include <QCameraInfo>
#include <QList>
#include <QByteArray>
#include <QVector>
#include <QDebug>
#include <QVideoProbe>
#include <QVideoSurfaceFormat>

static QString cameraKeyId("vid_2304");

CameraHelper::CameraHelper(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<FrameInfo>("FrameInfo");
    m_monitor = new NativeEventMonitor;
    m_cameraSwitchTimer = new QTimer(this);
    m_cameraSwitchTimer->setInterval(1);
    m_cameraSwitchTimer->setSingleShot(true);

    connect(m_cameraSwitchTimer, &QTimer::timeout, this, [this](){
        if (m_cameraArray.size() == 0)
            return;

        if (m_currentIndex >= m_cameraArray.size())
            m_currentIndex = 0;

        QString deviceName = m_cameraArray[m_currentIndex].deviceName;

        foreach(auto info, m_cameraArray) {
            if (info.deviceName != deviceName) {
                info.camera->stop();
                info.processor->stop();
            }
        };

        foreach(auto info, m_cameraArray) {
            if (info.deviceName == deviceName) {
                QVideoSurfaceFormat format;;
                format.setFrameRate(30);
                format.setFrameSize(QSize(1920, 1080));
                info.camera->start();
                info.processor->start(format);
            }
        };

        m_currentIndex ++;
    });

    connect(m_monitor, &NativeEventMonitor::useDeviceStateChanged, this, &CameraHelper::onUSBStateChanged);
}

CameraHelper::~CameraHelper()
{
    delete m_monitor;
    releaseAllCamera();
}

void CameraHelper::initialCameraList()
{
    releaseAllCamera();

    // load current exist camera
    QList<QCameraInfo> infos = QCameraInfo::availableCameras();
    foreach(auto it, infos) {
        qDebug()  << it.deviceName();
        appendOneCamera(it, static_cast<Direction>(infos.indexOf(it)));

        }
}

bool CameraHelper::openCamera(const QString &deviceName)
{
    foreach(auto info, m_cameraArray) {
        if (info.deviceName != deviceName) {
            info.processor->stop();
        }
    }

    foreach(auto info, m_cameraArray) {
        if (info.deviceName == deviceName) {
            info.camera->searchAndLock(QCamera::LockType::NoLock);

            QCameraViewfinderSettings settings = info.camera->viewfinderSettings();
            settings.setResolution(QSize(1920, 1080));
            settings.setPixelFormat(QVideoFrame::Format_Jpeg);
            info.camera->setViewfinderSettings(settings);
            info.camera->start();
            QString errorString = info.camera->errorString();
            qWarning() <<errorString;

            QVideoSurfaceFormat format;;
            format.setFrameRate(30);
            format.setFrameSize(QSize(1920, 1080));
            info.processor->start(format);
            info.camera->unlock();
            return true;
        }
    }

    return false;
}

bool CameraHelper::openCamera(const QVector<QString> &deviceNameArray)
{
    bool result = true;
    foreach(auto it, deviceNameArray)
        result &= openCamera(it);
    return result;
}

bool CameraHelper::closeCamera(const QString &deviceName)
{
    foreach(auto it, m_cameraArray) {
        if (it.deviceName == deviceName) {
            it.camera->stop();
            return true;
        }
    }

    return false;
}

bool CameraHelper::closeCamera(const QVector<QString> &deviceNameArray)
{
    bool result = true;
    foreach(auto it, deviceNameArray)
        result &= closeCamera(it);
    return result;
}

QVector<QString> CameraHelper::avaliableDeviceNames() const
{
    QVector<QString> names;
    QList<QCameraInfo> infos = QCameraInfo::availableCameras();
    foreach(auto it, infos) {
        names.append(it.deviceName());
    }

    return names;
}

void CameraHelper::setCameraType(const QString &deviceName, Direction direction)
{
    for (int i = 0; i < m_cameraArray.size(); ++i) {
        auto it = m_cameraArray[i];
        if (it.deviceName == deviceName) {
            it.direction = direction;
            m_cameraArray[i] = it;
            return;
        }
    }
}

CameraHelper::Direction CameraHelper::cameraType(const QString &deviceName)
{
    foreach(auto it, m_cameraArray) {
        if (it.deviceName == deviceName)
            return it.direction;
    }
    return Direction::Front;
}

QString CameraHelper::deviceName(Direction direction) const
{
    foreach(auto it, m_cameraArray) {
        if (it.direction == direction)
            return it.deviceName;
    }
    return QString();
}

void CameraHelper::setAutoOpenEnabed(bool enabled)
{
    m_autoOpenEnabled = enabled;
}

bool CameraHelper::autoOpenEnabled() const
{
    return m_autoOpenEnabled;
}

void CameraHelper::appendOneCamera(const QCameraInfo &info2, Direction direction)
{
    if (info2.deviceName().toLower().contains(cameraKeyId)) {
        QCamera *camera = new QCamera(info2.deviceName().toUtf8(), this);

        CameraInfo info;
        info.camera =camera;
        info.deviceName = info2.deviceName();
        info.direction = direction;
        info.processor = new VideoSurfaceProcessor(this);
        camera->setCaptureMode(QCamera::CaptureMode::CaptureViewfinder);

        camera->setViewfinder(info.processor);

        connect(info.processor, &VideoSurfaceProcessor::oneFrameReady, this, [this](const QVideoFrame &frame){
            VideoSurfaceProcessor *d = reinterpret_cast<VideoSurfaceProcessor *>(sender());
            foreach(auto it, m_cameraArray) {
                if (it.processor == d) {
                    QImage view(frame.bits(),frame.width(), frame.height(), QImage::Format_ARGB32);
                    emit oneFrameReady(FrameInfo(it.deviceName, it.direction, view));
                    m_cameraSwitchTimer->start();
                }
            }
        });

        m_cameraArray.append(info);

        // Auto Open
        if (m_autoOpenEnabled)
            openCamera(info2.deviceName());
}
}

void CameraHelper::removeOneCamera(const QString &deviceName)
{
    int searchIndex = -1;
    foreach(auto it, m_cameraArray) {
        if (it.deviceName == deviceName) {
            it.camera->stop();
            it.camera->deleteLater();
            it.processor->deleteLater();
            searchIndex = m_cameraArray.indexOf(it);
            break;
        }
    }

    m_cameraArray.removeAt(searchIndex);
}

void CameraHelper::onUSBStateChanged()
{
    initialCameraList();
}

void CameraHelper::releaseAllCamera()
{
    foreach(auto it, m_cameraArray) {
        it.camera->unlock();
        it.camera->deleteLater();
        it.processor->deleteLater();
    }

    m_cameraArray.clear();
}
