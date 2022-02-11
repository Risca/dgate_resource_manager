#include "video.h"

#include <QDir>

#include <algorithm>

namespace model {

const QSet<QString> Video::g_KnownVideoFiles = QSet<QString>()
        << "DGATE000.Q"
        << "DGATE001.Q"
        << "DGATE002.Q"
        << "DGATE003.Q"
        << "DGATE004.Q"
        << "DGATE005.Q"
        << "DGATE006.Q"
        << "DGATE007.Q"
        << "DGATE008.Q"
        << "DGATE009.Q"
        << "DGATE010.Q"
        << "DGATE011.Q"
        << "DGATE012.Q"
        << "DGATE013.Q"
        << "ABARRACH.FLI"
        << "ARIANUS.FLI"
        << "BALTAZAR.FLI"
        << "BROTHOOD.FLI"
        << "CHELESTR.FLI"
        << "DECKRUN.FLI"
        << "GLOWFIN.FLI"
        << "JOURNAL.FLI"
        << "KICKSEY.FLI"
        << "KLEI_FIN.FLI"
        << "KLEIDEAD.FLI"
        << "KL_SCPTR.FLI"
        << "KL_THRON.FLI"
        << "NECROMAC.FLI"
        << "POETRY.FLI"
        << "PRYAN.FLI"
        << "PRYANS.FLI"
        << "SUNDER.FLI"
        << "TRIAN.FLI"
        << "ZIFNAB.FLI"
        << "ZINGER.FLI";

Video::Video(QObject *parent) :
    QAbstractListModel(parent)
{

}

int Video::rowCount(const QModelIndex &) const
{
    return m_Videos.count();
}

QVariant Video::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= rowCount()) {
        return QVariant();
    }

    const QString& file = m_Videos.at(row);
    switch (role) {
    case Qt::DisplayRole:
        return file;
        break;

    case Qt::UserRole:
        return m_Dir.absoluteFilePath(file);

    default:
        break;
    }
    return QVariant();
}

QVariant Video::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return "Filename";
        }
        if (orientation == Qt::Vertical && section < rowCount()) {
            return QString::number(section + 1);
        }
    }

    return QVariant();
}

void Video::processDirectory(const QString &dir)
{
    m_Dir = QDir(dir);
    QStringList filenames = m_Dir.entryList(QDir::Files);
    QSet<QString> files = QSet<QString>(filenames.begin(), filenames.end());
    m_Videos = files.intersect(g_KnownVideoFiles).values();
    std::sort(m_Videos.begin(), m_Videos.end());
    emit dataChanged(createIndex(0, 0),
                     createIndex(rowCount() - 1, 0));
}

} // namespace model
