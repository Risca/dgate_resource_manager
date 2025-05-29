#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "players/picturerender.h"
#include "players/flicplayer.h"
#include "players/xmiplayer.h"
#include "players/waveplayer.h"

#include <QMainWindow>
#include <QString>

class QDir;
class QListWidgetItem;
class QModelIndex;
class QSettings;

namespace model {
class Image;
class Video;
class Music;
class Voice;
class Text;
class Palette;
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void openDir();
    void indicatePixelPosition(int x, int y);
    void indicateColorIndex(int index);
    void addOrRemoveOverlay(const QModelIndex &index);
    void removeOverlay(QListWidgetItem *item);

signals:
    void directoryOpened(const QString& dir);

private:
    Ui::MainWindow *ui;
    QSettings *m_Settings;
    QString m_LastDir;
    PictureRender m_PicRender;
    FlicPlayer m_FlicPlayer;
    XmiPlayer m_XmiPlayer;
    WavePlayer m_WavePlayer;

    model::Image* m_ImageModel;
    model::Video* m_VideoModel;
    model::Music* m_MusicModel;
    model::Voice* m_VoiceModel;
    model::Text* m_TextModel;
    model::Palette* m_PaletteModel;

    QPoint m_PixelPosition;
};

#endif // MAINWINDOW_H
