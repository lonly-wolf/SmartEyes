#pragma once

#include <QAbstractNativeEventFilter>
#include <QObject>

class NativeEventMonitor : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    enum class DeviceState {Entered_State, Removed_State};
    NativeEventMonitor(QObject *parent = nullptr);
    ~NativeEventMonitor();

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

signals:
    void useDeviceStateChanged(const QString &deviceName, NativeEventMonitor::DeviceState state);
};
