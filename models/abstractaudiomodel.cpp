#include "abstractaudiomodel.h"

#include <QDataStream>
#include <QDebug>
#include <QDir>
#include <QFile>

namespace model {

AbstractAudioModel::AbstractAudioModel(const QString &filename, QObject *parent)
    : QAbstractItemModel(parent),
      m_Filename(filename)
{
}

QVariant AbstractAudioModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return "Track name";
        }
        if (orientation == Qt::Vertical && section < rowCount()) {
            return QString::number(section + 1);
        }
    }

    return QVariant();
}

QModelIndex AbstractAudioModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

QModelIndex AbstractAudioModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int AbstractAudioModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_Tracks.count();
}

int AbstractAudioModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant AbstractAudioModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    if (row >= rowCount()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return QString::number(row + 1);
        break;

    case Qt::UserRole:
        return QVariant::fromValue<TrackInfo>(m_Tracks[row]);

    default:
        break;
    }
    return QVariant();
}

void AbstractAudioModel::processDirectory(const QString &dir)
{
    QFile file(dir + QDir::separator() + m_Filename);
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QDataStream fileStream(&file);
    parse(fileStream, file.fileName());

    emit dataChanged(createIndex(0, 0),
                     createIndex(rowCount() - 1, 0));
}

} // namespace model
