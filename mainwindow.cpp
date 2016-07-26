#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(this, SIGNAL(playFile(QString)), &m_Player, SLOT(play(QString)));
    connect(&m_Player, SIGNAL(frameReady(QImage)), ui->widget, SLOT(present(QImage)));
    emit playFile("/home/risca/.dosbox/C_Drive/SPEL/DGATE/DGATE000.Q");
}

MainWindow::~MainWindow()
{
    delete ui;
}
