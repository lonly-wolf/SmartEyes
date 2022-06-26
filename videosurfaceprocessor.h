#pragma once

#include <QAbstractVideoSurface>

class VideoSurfaceProcessor : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoSurfaceProcessor(QObject *parent = nullptr);
    ~VideoSurfaceProcessor();

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

    bool present(const QVideoFrame &frame);

signals:
    void oneFrameReady(const QVideoFrame &frame);
};
