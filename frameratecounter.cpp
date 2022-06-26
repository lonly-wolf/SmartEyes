#include "frameratecounter.h"

FrameRateCounter::FrameRateCounter(QObject *parent) : QObject(parent)
{
    m_frameUpdateTimer = new QTimer(this);
    m_frameUpdateTimer->setInterval(1000);

    connect(m_frameUpdateTimer, &QTimer::timeout, this, [this](){
        emit frameChanged(currentFrame());
    });
}

FrameRateCounter::~FrameRateCounter()
{

}

void FrameRateCounter::startCount()
{
    m_counts = 0;
    m_timer.start();
    m_frameUpdateTimer->start();
}

void FrameRateCounter::stopCount()
{
    m_frameUpdateTimer->stop();
}

void FrameRateCounter::oneFrameReady()
{
    m_counts ++;
}

double FrameRateCounter::currentFrame()
{
    return m_counts / (m_timer.elapsed() / 1000.000);
}
