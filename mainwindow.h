#pragma once

#include <QMainWindow>
#include "camerahelper.h"
#include "frameratecounter.h"
#include <QGraphicsPixmapItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    CameraHelper *m_cameraHelper;
    FrameRateCounter *m_counter;

    QGraphicsPixmapItem *m_imgFront;
    QGraphicsPixmapItem *m_imgBack;
    QGraphicsPixmapItem *m_imgLeft;
    QGraphicsPixmapItem *m_imgRight;

    QMap<CameraHelper::Direction, QGraphicsPixmapItem *> m_imgMap;


    void initialUI();
};
