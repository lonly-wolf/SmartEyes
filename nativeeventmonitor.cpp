#include "nativeeventmonitor.h"
#include "windows.h"
#include "Dbt.h"
#include <QDebug>

NativeEventMonitor::NativeEventMonitor(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DeviceState>("DeviceState");
}

NativeEventMonitor::~NativeEventMonitor()
{

}

bool NativeEventMonitor::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    MSG *msg = reinterpret_cast<MSG *>(message);
    int type = msg->message;

    if (type == DBT_DEVICEARRIVAL) {
        qDebug() <<"USB Connected!";
        emit useDeviceStateChanged(QString(), DeviceState::Entered_State);
    } else if (type == DBT_DEVICEREMOVECOMPLETE) {
        qDebug() <<"USB Removed!";
        emit useDeviceStateChanged(QString(), DeviceState::Removed_State);
    }
    return false;
}
