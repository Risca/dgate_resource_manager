#ifndef VIDEO_H
#define VIDEO_H

#include <QAbstractListModel>
#include <QDir>
#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>

namespace model {

class Video : public QAbstractListModel
{
    Q_OBJECT

public:
    Video(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

public slots:
    void processDirectory(const QString &dir);

private:
    QDir m_Dir;
    QStringList m_Videos;
    static const QSet<QString> g_KnownVideoFiles;
};

} // namespace model

#endif // VIDEO_H
