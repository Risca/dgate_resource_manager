#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "models/image.h"
#include "models/music.h"
#include "models/palette.h"
#include "models/text.h"
#include "models/video.h"
#include "models/voice.h"

#include "cellborderdelegate.h"
#include "hexnumberdelegate.h"

#include <QDebug>
#include <QFileDialog>
#include <QHeaderView>
#include <QListWidgetItem>
#include <QModelIndexList>
#include <QSettings>

namespace {

QModelIndexList toModelIndexList(const QListWidget *w)
{
    QModelIndexList list;
    int rows = w->count();
    for (int row = 0; row < rows; ++row) {
        auto *item = w->item(row);
        list << item->data(Qt::UserRole).toModelIndex();
    }
    return list;
}

QListWidgetItem* ItemInListWidget(const QListWidget *list, const QModelIndex& wanted)
{
    const int rows = list->count();
    for (int row = 0; row < rows; ++row) {
        auto *item = list->item(row);
        auto index = item->data(Qt::UserRole).toModelIndex();
        if (index == wanted)
            return item;
    }
    return nullptr;
}

} // anonymous namespace

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_Settings(new QSettings("--secret--", "dgate resource manager", this)),
    m_LastDir(m_Settings->value("lastdir", QDir::homePath()).toString()),
    m_ImageModel(new model::Image(this)),
    m_VideoModel(new model::Video(this)),
    m_MusicModel(new model::Music(this)),
    m_VoiceModel(new model::Voice(this)),
    m_TextModel(new model::Text(this)),
    m_PaletteModel(new model::Palette(this))
{
    ui->setupUi(this);

    ui->imageTreeView->setModel(m_ImageModel);
    ui->overlayImageTreeView->setModel(m_ImageModel);
    ui->videoListView->setModel(m_VideoModel);
    ui->musicListView->setModel(m_MusicModel);
    ui->voiceListView->setModel(m_VoiceModel);
    ui->textListView->setModel(m_TextModel);
    ui->paletteView->setModel(m_PaletteModel);

    ui->imageTreeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->overlayImageTreeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    ui->paletteView->setItemDelegate(new CellBorderDelegate(this));
    foreach (QTreeView* view, QList<QTreeView*>() << ui->imageTreeView
                                                  << ui->overlayImageTreeView)
    {
        view->setItemDelegateForColumn(model::Image::COLUMN_OFFSET, new HexNumberDelegate(this));
        view->setItemDelegateForColumn(model::Image::COLUMN_IMG_OFF, new HexNumberDelegate(this));
        view->setItemDelegateForColumn(model::Image::COLUMN_FLAGS, new HexNumberDelegate(this));
    }

    connect(this, SIGNAL(directoryOpened(QString)), this, SLOT(setWindowTitle(QString)));
    connect(ui->actionOpen_folder, SIGNAL(triggered(bool)), this, SLOT(openDir()));
    connect(&m_FlicPlayer, SIGNAL(frameReady(QImage)), ui->displaySurface, SLOT(present(QImage)));
    connect(&m_PicRender, SIGNAL(frameReady(QImage)), ui->displaySurface, SLOT(present(QImage)));
    connect(&m_PicRender, SIGNAL(frameReady(QImage)), m_PaletteModel, SLOT(setPalette(QImage)));
    connect(ui->zoomSlider, SIGNAL(valueChanged(int)), ui->displaySurface, SLOT(resize(int)));
    connect(ui->displaySurface, SIGNAL(scaleFactorChanged(int)), ui->zoomSlider, SLOT(setValue(int)));

    connect(this, SIGNAL(directoryOpened(QString)), m_ImageModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_VideoModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_MusicModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_VoiceModel, SLOT(processDirectory(QString)));
    connect(this, SIGNAL(directoryOpened(QString)), m_TextModel, SLOT(processDirectory(QString)));

    connect(ui->imageTreeView, SIGNAL(activated(QModelIndex)), &m_PicRender, SLOT(render(QModelIndex)));
    connect(ui->imageTreeView, SIGNAL(activated(QModelIndex)), &m_FlicPlayer, SLOT(stop()));
    connect(ui->overlayGroupBox, SIGNAL(clicked(bool)), &m_PicRender, SLOT(enableOverlay(bool)));
    connect(ui->overlayImageTreeView, SIGNAL(activated(QModelIndex)), this, SLOT(addOrRemoveOverlay(QModelIndex)));
    connect(ui->enabledOverlays, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(removeOverlay(QListWidgetItem*)));
    connect(ui->videoListView, SIGNAL(activated(QModelIndex)), &m_FlicPlayer, SLOT(play(QModelIndex)));
    connect(ui->musicListView, SIGNAL(activated(QModelIndex)), &m_XmiPlayer, SLOT(play(QModelIndex)));
    connect(ui->musicListView, SIGNAL(activated(QModelIndex)), &m_WavePlayer, SLOT(stop()));
    connect(ui->voiceListView, SIGNAL(activated(QModelIndex)), &m_WavePlayer, SLOT(play(QModelIndex)));
    connect(ui->voiceListView, SIGNAL(activated(QModelIndex)), &m_XmiPlayer, SLOT(stop()));

    connect(ui->displaySurface, SIGNAL(mouseMoved(int,int)), this, SLOT(indicatePixelPosition(int,int)));
    connect(ui->displaySurface, SIGNAL(mouseMoved(int,int)), &m_PicRender, SLOT(selectPixel(int,int)));
    connect(&m_PicRender, SIGNAL(colorIndexSelected(int)), m_PaletteModel, SLOT(selectPaletteColor(int)));
    connect(&m_PicRender, SIGNAL(colorIndexSelected(int)), this, SLOT(indicateColorIndex(int)));

    if (!m_LastDir.isEmpty()) {
        emit directoryOpened(m_LastDir); //clazy:exclude=incorrect-emit
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

void MainWindow::indicatePixelPosition(int x, int y)
{
    m_PixelPosition = QPoint(x, y);
    QString position;
    QTextStream(&position) << "(" << x << ", " << y << ")";
    ui->statusBar->showMessage(position);
}

void MainWindow::indicateColorIndex(int index)
{
    QString text;
    QTextStream(&text) << "(" << m_PixelPosition.x() << ", " << m_PixelPosition.y() << ", " << index << ")";
    ui->statusBar->showMessage(text);
}

void MainWindow::addOrRemoveOverlay(const QModelIndex &index)
{
    if (!ui->overlayGroupBox->isChecked() || !index.parent().isValid()) {
        return;
    }

    int flags = index.sibling(index.row(), model::Image::COLUMN_FLAGS).data().toInt();
    if ((flags & model::Image::FlagCoordinates) == 0) {
        qWarning() << "no coordinates in image header";
        return;
    }

    auto *item = ItemInListWidget(ui->enabledOverlays, index);
    if (!item) {
        QString filename = index.parent().data().toString();
        quint32 offset = index.sibling(index.row(), model::Image::COLUMN_OFFSET).data().toUInt();
        auto *item = new QListWidgetItem(filename + " @ 0x" + QString::number(offset, 16));
        item->setData(Qt::UserRole, index);
        ui->enabledOverlays->addItem(item);

        m_PicRender.overlay(toModelIndexList(ui->enabledOverlays));
    }
    else
        removeOverlay(item);
}

void MainWindow::removeOverlay(QListWidgetItem *item)
{
    delete item;
    m_PicRender.overlay(toModelIndexList(ui->enabledOverlays));
}
