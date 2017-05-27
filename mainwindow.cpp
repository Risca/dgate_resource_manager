#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "models/image.h"
#include "models/music.h"
#include "models/video.h"
#include "models/voice.h"
#include "models/text.h"

#include "hexnumberdelegate.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_Settings(new QSettings("--secret--", "dgate resource manager", this)),
    m_LastDir(m_Settings->value("lastdir", QDir::homePath()).toString()),
    m_ImageModel(new model::Image(this)),
    m_VideoModel(new model::Video(this)),
    m_MusicModel(new model::Music(this)),
    m_VoiceModel(new model::Voice(this)),
    m_TextModel(new model::Text(this))
{
    ui->setupUi(this);

    ui->imageTreeView->setModel(m_ImageModel);
    ui->videoListView->setModel(m_VideoModel);
    ui->musicListView->setModel(m_MusicModel);
    ui->voiceListView->setModel(m_VoiceModel);
    ui->textListView->setModel(m_TextModel);

    ui->imageTreeView->setItemDelegateForColumn(model::Image::COLUMN_OFFSET,
                                                new HexNumberDelegate(this));
    ui->imageTreeView->setItemDelegateForColumn(model::Image::COLUMN_IMG_OFF,
                                                new HexNumberDelegate(this));
    ui->imageTreeView->setItemDelegateForColumn(model::Image::COLUMN_FLAGS,
                                                new HexNumberDelegate(this));

    connect(this, SIGNAL(directoryOpened(QString)), this, SLOT(setWindowTitle(QString)));
    connect(ui->actionOpen_folder, SIGNAL(triggered(bool)), this, SLOT(openDir()));
    connect(&m_FlicPlayer, SIGNAL(frameReady(QImage)), ui->widget, SLOT(present(QImage)));
    connect(&m_PicRender, SIGNAL(frameReady(QImage)), ui->widget, SLOT(present(QImage)));

    connect(this, SIGNAL(directoryOpened(QString)), m_ImageModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_VideoModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_MusicModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_VoiceModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_TextModel, SLOT(processDirectory(QString)));

    connect(ui->imageTreeView, SIGNAL(doubleClicked(QModelIndex)), &m_PicRender, SLOT(play(QModelIndex)));
    connect(ui->videoListView, SIGNAL(doubleClicked(QModelIndex)), &m_FlicPlayer, SLOT(play(QModelIndex)));
    connect(ui->musicListView, SIGNAL(doubleClicked(QModelIndex)), &m_XmiPlayer, SLOT(play(QModelIndex)));
    connect(ui->musicListView, SIGNAL(doubleClicked(QModelIndex)), &m_WavePlayer, SLOT(stop()));
    connect(ui->voiceListView, SIGNAL(doubleClicked(QModelIndex)), &m_WavePlayer, SLOT(play(QModelIndex)));
    connect(ui->voiceListView, SIGNAL(doubleClicked(QModelIndex)), &m_XmiPlayer, SLOT(stop()));

    if (!m_LastDir.isEmpty()) {
        emit directoryOpened(m_LastDir);
    }
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
