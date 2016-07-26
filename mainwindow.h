#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <flicplayer.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void playFile(const QString& filename);

private:
    Ui::MainWindow *ui;
    FlicPlayer m_Player;
};

#endif // MAINWINDOW_H
