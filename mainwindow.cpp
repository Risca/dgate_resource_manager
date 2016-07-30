#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <models/music.h>
#include <models/video.h>

#include <QDebug>
#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_Settings(new QSettings("--secret--", "dgate resource manager", this)),
    m_LastDir(m_Settings->value("lastdir", QDir::homePath()).toString()),
    m_VideoModel(new model::Video(this)),
    m_MusicModel(new model::Music(this))
{
    ui->setupUi(this);

    ui->videoListView->setModel(m_VideoModel);
    ui->musicListView->setModel(m_MusicModel);

    connect(this, SIGNAL(directoryOpened(QString)), this, SLOT(setWindowTitle(QString)));
    connect(ui->videoListView, SIGNAL(doubleClicked(QModelIndex)), &m_Player, SLOT(play(QModelIndex)));
    connect(&m_Player, SIGNAL(frameReady(QImage)), ui->widget, SLOT(present(QImage)));
    connect(ui->actionOpen_folder, SIGNAL(triggered(bool)), this, SLOT(openDir()));
    connect(this, SIGNAL(directoryOpened(QString)), m_VideoModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_MusicModel, SLOT(processDirectory(QString)));

    emit directoryOpened(m_LastDir);
}

MainWindow::~MainWindow()
{
    m_Settings->setValue("lastdir", m_LastDir);
    delete m_Settings;
    m_Settings = 0;
    delete ui;
    ui = 0;
}

void MainWindow::openDir()
{
    QString dir;

    dir = QFileDialog::getExistingDirectory(this, "Open Death Gate directory", m_LastDir);
    if (dir.isEmpty()) {
        return;
    }
    m_LastDir = dir;

    emit directoryOpened(dir);
}
