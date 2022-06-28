#pragma once
#include <QObject>
#include <QImage>
#include <QCameraInfo>
#include <QVector>
#include "nativeeventmonitor.h"
#include <videosurfaceprocessor.h>
#include <QTimer>


class CameraHelper : public QObject
{
    Q_OBJECT
public:
    enum class Direction {Left, Front, Back, Right};
    struct FrameInfo {
        FrameInfo()
        {

        }
        FrameInfo(const QString &name, Direction direction, const QImage &img)
            : deviceName(name)
            , direction(direction)
            , image(img)
        {

        }

        QString deviceName;
        Direction direction;
        QImage image;
    };

    explicit CameraHelper(QObject *parent = nullptr);
    ~CameraHelper();

    void initialCameraList();
    bool openCamera(const QString &deviceName);
    bool openCamera(const QVector<QString> &deviceNameArray);
    bool closeCamera(const QString &deviceName);
    bool closeCamera(const QVector<QString> &deviceNameArray);
    QVector<QString> avaliableDeviceNames() const;

    void setCameraType(const QString &deviceName, Direction direction);
    Direction cameraType(const QString &deviceName);
    QString deviceName(Direction direction) const;
    void setAutoOpenEnabed(bool enabled);
    bool autoOpenEnabled() const;

signals:
    void oneFrameReady(const CameraHelper::FrameInfo &frame);

private:
    struct CameraInfo {
        QCamera *camera {nullptr};
        VideoSurfaceProcessor *processor {nullptr};
        Direction direction;
        QString deviceName;

        bool operator ==(const CameraInfo &other) {
            return deviceName == other.deviceName;
        }
    };

    QMap<CameraHelper::Direction, QGraphicsVideoItem *> m_imgMap;
    QVector<CameraInfo> m_cameraArray;
    bool m_autoOpenEnabled {true};
    NativeEventMonitor *m_monitor;
    int m_currentIndex {0};

    void appendOneCamera(const QCameraInfo &info2, Direction direction);
    void removeOneCamera(const QString &deviceName);
    void onUSBStateChanged();
    void releaseAllCamera();
};
