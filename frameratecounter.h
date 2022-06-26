#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>

class FrameRateCounter : public QObject
{
    Q_OBJECT
public:
    explicit FrameRateCounter(QObject *parent = nullptr);
    ~FrameRateCounter();
    void startCount();
    void stopCount();
    void oneFrameReady();
    double currentFrame();

signals:
    void frameChanged(double frame);

private:
    QElapsedTimer m_timer;
    QTimer *m_frameUpdateTimer;
    qint64 m_counts{0};
};
