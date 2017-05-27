#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <flicplayer.h>
#include "xmiplayer.h"
#include "waveplayer.h"

#include <QMainWindow>
#include <QString>

class QDir;
class QModelIndex;
class QSettings;

namespace model {
class Video;
class Music;
class Voice;
class Text;
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

signals:
    void directoryOpened(const QString& dir);

private:
    Ui::MainWindow *ui;
    QSettings *m_Settings;
    QString m_LastDir;
    FlicPlayer m_FlicPlayer;
    XmiPlayer m_XmiPlayer;
    WavePlayer m_WavePlayer;

    model::Video* m_VideoModel;
    model::Music* m_MusicModel;
    model::Voice* m_VoiceModel;
    model::Text* m_TextModel;
};

#endif // MAINWINDOW_H
