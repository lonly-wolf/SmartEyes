#include "camerahelper.h"
#include <QCameraInfo>
#include <QList>
#include <QByteArray>
#include <QVector>
#include <QDebug>
#include <QVideoProbe>
#include <QVideoSurfaceFormat>

static QString cameraKeyId("0023044922");

CameraHelper::CameraHelper(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<FrameInfo>("FrameInfo");
    m_monitor = new NativeEventMonitor;

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
        if (it.deviceName().toLower().contains(cameraKeyId)) {
            qDebug() << "Available Camera ID:" << it.deviceName();
            appendOneCamera(it, static_cast<Direction>(m_cameraArray.size()));
        }

        }
}

bool CameraHelper::openCamera(const QString &deviceName)
{
    foreach(auto info, m_cameraArray) {
        if (info.deviceName == deviceName) {
            info.camera->searchAndLock(QCamera::LockType::NoLock);

            QCameraViewfinderSettings settings = info.camera->viewfinderSettings();
            settings.setResolution(QSize(1920, 1080));
            settings.setPixelFormat(QVideoFrame::Format_ARGB32);
            settings.setMaximumFrameRate(10);

            info.camera->start();

            info.camera->setViewfinderSettings(settings);

//            auto formats = info.camera->supportedViewfinderPixelFormats();
//            foreach(auto it, formats) {
//                qDebug() << "spuuort format:" << it;
//            }

            QString errorString = info.camera->errorString();
            qWarning() <<errorString;

            QVideoSurfaceFormat format;;
            format.setFrameRate(10);
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
        QCamera *camera = new QCamera(info2, this);

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
                    //qDebug() << "format1:" << frame.pixelFormat();
                    //qDebug() << "format2:" << frame.imageFormatFromPixelFormat(frame.pixelFormat());
                    QImage view(frame.bits(),frame.width(), frame.height(),  QImage::Format_RGB32);
                    emit oneFrameReady(FrameInfo(it.deviceName, it.direction, view));
                    break;
                }
            }
        });

        qDebug() << "Find One Camera:" << info.deviceName << " direction:" << static_cast<int>(direction);
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
