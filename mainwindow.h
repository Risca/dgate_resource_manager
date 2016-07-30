#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <flicplayer.h>

#include <QMainWindow>
#include <QString>

class QDir;
class QModelIndex;
class QSettings;

namespace model {
class Video;
class Music;
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
    FlicPlayer m_Player;

    model::Video* m_VideoModel;
    model::Music* m_MusicModel;
};

#endif // MAINWINDOW_H
