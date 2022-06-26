#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QCameraInfo>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initialUI();
    m_cameraHelper = new CameraHelper(this);
    m_counter = new FrameRateCounter(this);

    connect(m_cameraHelper, &CameraHelper::oneFrameReady, this, [this](const CameraHelper::FrameInfo &info){
        qDebug() << "One Frame Ready!" << info.image.size();
        m_imgMap[info.direction]->setPixmap(QPixmap::fromImage(info.image));
        m_counter->oneFrameReady();
    });
    connect(m_counter, &FrameRateCounter::frameChanged, this, [this](double rate){
        ui->statusbar->showMessage(QString("Current rate:%1").arg(rate));
    });

    m_cameraHelper->initialCameraList();
    m_counter->startCount();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialUI()
{
    m_imgMap.clear();
    m_imgFront = new QGraphicsPixmapItem;
    m_imgBack = new QGraphicsPixmapItem;
    m_imgLeft= new QGraphicsPixmapItem;
    m_imgRight = new QGraphicsPixmapItem;

    m_imgMap[CameraHelper::Direction::Front] = m_imgFront;
    m_imgMap[CameraHelper::Direction::Back] = m_imgBack;
    m_imgMap[CameraHelper::Direction::Left] = m_imgLeft;
    m_imgMap[CameraHelper::Direction::Right] = m_imgRight;

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_3->setScene(new QGraphicsScene(this));
    ui->graphicsView_4->setScene(new QGraphicsScene(this));

    ui->graphicsView->scene()->addItem(m_imgFront);
    ui->graphicsView_2->scene()->addItem(m_imgBack);
    ui->graphicsView_3->scene()->addItem(m_imgLeft);
    ui->graphicsView_4->scene()->addItem(m_imgRight);
}

