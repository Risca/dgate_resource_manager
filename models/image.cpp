#include "image.h"

#include "treeitem.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QStringList>

namespace model {

const QSet<QString> Image::g_KnownImageFiles = QSet<QString>()
        << "DGATE000.PIC"
        << "DGATE001.PIC"
        << "DGATE002.PIC"
        << "DGATE003.PIC"
        << "DGATE004.PIC"
        << "DGATE005.PIC"
        << "DGATE006.PIC"
        << "DGATE007.PIC"
        << "DGATE008.PIC"
        << "DGATE009.PIC"
        << "DGATE010.PIC"
        << "DGATE011.PIC"
        << "DGATE012.PIC"
        << "DGATE013.PIC"
        << "DGATE014.PIC"
        << "DGATE015.PIC"
        << "DGATE016.PIC"
        << "DGATE017.PIC"
        << "DGATE018.PIC"
        << "DGATE019.PIC"
        << "DGATE020.PIC"
        << "DGATE021.PIC"
        << "DGATE022.PIC"
        << "DGATE023.PIC"
        << "DGATE024.PIC"
        << "DGATE025.PIC"
        << "DGATE026.PIC"
        << "DGATE027.PIC"
        << "DGATE028.PIC"
        << "DGATE040.PIC"
        << "DGATE050.PIC"
        << "DGATE051.PIC"
        << "DGATE052.PIC"
        << "DGATE053.PIC"
        << "DGATE054.PIC"
        << "DGATE055.PIC"
        << "DGATE056.PIC"
        << "DGATE057.PIC"
        << "DGATE058.PIC"
        << "DGATE060.PIC"
        << "DGATE061.PIC"
        << "DGATE062.PIC"
        << "DGATE063.PIC"
        << "DGATE064.PIC"
        << "DGATE066.PIC"
        << "DGATE067.PIC"
        << "DGATE068.PIC"
        << "DGATE069.PIC"
        << "DGATE070.PIC"
        << "DGATE071.PIC"
        << "DGATE075.PIC"
        << "DGATE076.PIC"
        << "DGATE077.PIC"
        << "DGATE078.PIC"
        << "DGATE080.PIC"
        << "DGATE081.PIC"
        << "DGATE082.PIC"
        << "DGATE083.PIC"
        << "DGATE084.PIC"
        << "DGATE085.PIC"
        << "DGATE086.PIC";

Image::Image(QObject *parent)
    : QAbstractItemModel(parent)
{
    TreeItem::ItemData headers;
    headers[COLUMN_OFFSET] = "Offset";
    headers[COLUMN_IMG_OFF] = "Image offset";
    headers[COLUMN_FLAGS] = "Flags";
    headers[COLUMN_WIDTH] = "Width";
    headers[COLUMN_HEIGHT] = "Height";
    headers[COLUMN_PALETTE] = "Palette";
    m_RootItem = new TreeItem(headers);
}

Image::~Image()
{
    delete m_RootItem;
}

QVariant Image::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            return m_RootItem->data(section);
        }
        if (orientation == Qt::Vertical && section < rowCount()) {
            return QString::number(section + 1);
        }
    }

    return QVariant();
}

QModelIndex Image::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    TreeItem *parentItem;
    if (!parent.isValid()) {
        parentItem = m_RootItem;
    }
    else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    TreeItem *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    else {
        return QModelIndex();
    }
}

QModelIndex Image::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem *parentItem = childItem->parentItem();

    if (parentItem == m_RootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int Image::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    TreeItem *parentItem;
    if (!parent.isValid()) {
        parentItem = m_RootItem;
    }
    else {
        parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }
    return parentItem->childCount();
}

int Image::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_RootItem->columnCount();
    }
    else {
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    }
}

QVariant Image::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        return item->data(index.column());
    }
    else if (role == Qt::UserRole && index.column() == 0 && !index.parent().isValid()) {
        // hack to get full path to file
        return m_Dir + QDir::separator() + item->data(index.column()).toString();
    }
    else {
        return QVariant();
    }
}

void Image::processDirectory(const QString &dir)
{
    m_Dir = dir;
    QStringList filenames = QDir(dir).entryList(QDir::Files);
    QSet<QString> files(filenames.begin(), filenames.end());
    QStringList knownFiles = files.intersect(g_KnownImageFiles).values();
    std::sort(knownFiles.begin(), knownFiles.end());
    foreach (const QString &filename, knownFiles) {
        TreeItem::ItemData itemData;
        itemData[0] = filename;
        // hacky workaround to make stuff show up
        for (int i = 1; i < COLUMN_END; ++i) {
            itemData[i] = "";
        }
        TreeItem *item = new TreeItem(itemData, m_RootItem);
        m_RootItem->appendChild(item);

        QFile file(dir + QDir::separator() + filename);
        if (file.open(QFile::ReadOnly)) {
            QDataStream stream(&file);
            parse(stream, item);
        }
    }

    emit dataChanged(index(0,0),index(rowCount() - 1, columnCount()));
}

void Image::parse(QDataStream &stream, TreeItem *parent)
{
    quint32 offset;

    stream.setByteOrder(QDataStream::LittleEndian);
    stream >> offset;
    while (offset != 0 && !stream.atEnd()) {
        quint16 flags, width, height;
        qint32 imageOffset = 0;

        stream >> flags >> width >> height;
        if (flags & FlagCoordinates) {
            imageOffset += 4;
        }
        if (flags & FlagPalette) {
            imageOffset += 3 * 256;
        }

        TreeItem::ItemData itemData;
        itemData[COLUMN_OFFSET] = offset;
        itemData[COLUMN_IMG_OFF] = imageOffset;
        itemData[COLUMN_FLAGS] = flags;
        itemData[COLUMN_WIDTH] = width;
        itemData[COLUMN_HEIGHT] = height;
        itemData[COLUMN_PALETTE] = !!(flags & FlagPalette);

        parent->appendChild(new TreeItem(itemData, parent));

        // Align to next offset
        stream >> flags; // read 2 bytes of padding
        stream >> offset;
    }
}

} // namespace model
