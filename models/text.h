#ifndef TEXT_H
#define TEXT_H

#include <QAbstractItemModel>
#include <QStringList>

namespace model {

class Text : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit Text(QObject *parent = 0);

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

private:
    QStringList m_TextLines;
    void parse(QDataStream &stream);
};

} // namespace model

#endif // TEXT_H
