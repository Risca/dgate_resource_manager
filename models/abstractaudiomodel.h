#ifndef ABSTRACTAUDIOMODEL_H
#define ABSTRACTAUDIOMODEL_H

#include "trackinfo.h"

#include <QAbstractItemModel>
#include <QList>
#include <QString>

class QDataStream;

namespace model {

class AbstractAudioModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit AbstractAudioModel(const QString &filename, QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void processDirectory(const QString &dir);

protected:
    QList<TrackInfo> m_Tracks;
    virtual void parse(QDataStream &stream, const QString &filePath) = 0;

private:
    const QString m_Filename;
};

} // namespace model

#endif // ABSTRACTAUDIOMODEL_H
