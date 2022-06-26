#include "videosurfaceprocessor.h"

VideoSurfaceProcessor::VideoSurfaceProcessor(QObject *parent)
    : QAbstractVideoSurface(parent)
{

}

VideoSurfaceProcessor::~VideoSurfaceProcessor()
{

}

QList<QVideoFrame::PixelFormat> VideoSurfaceProcessor::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    QList<QVideoFrame::PixelFormat> lists;

    lists << QVideoFrame::Format_UYVY << QVideoFrame::Format_Jpeg << QVideoFrame::Format_ARGB32 <<QVideoFrame::Format_RGB32 << QVideoFrame::Format_UYVY;

    return lists;
}

bool VideoSurfaceProcessor::present(const QVideoFrame &frame)
{
    auto newFrame = frame;
    if (!newFrame.isMapped())
        newFrame.map(QAbstractVideoBuffer::MapMode::ReadOnly);

    emit oneFrameReady(newFrame);
    return true;
}
