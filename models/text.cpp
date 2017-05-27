#include "text.h"

#include <QDataStream>
#include <QFile>

#include <vector>

#include <inttypes.h>

namespace {
/*
 * Code for extracting text from Death gate was taken from
 * http://www.ifarchive.org/indexes/if-archiveXutilities.html
 *
 * This text below was part of the original source code.
 */
// Legend Entertainment string file parser.
// Use to decompress and disassemble string file from all Legend adventure
// games from Timequest to Callahan's Crosstime Saloon.
// Command line:
//   legend-str <string file> [-version]
// String file always has extension .dat and the name ends in "str." For Eric
// the Unready it is ERICSTR.DAT, for Companions of Xanth it is XANTHSTR.DAT, etc.
// There are four different versions of file format. It is impossible for the
// utility to figure out exact version number so it has to be specified on
// the command line. If no number is specified, version 2 is assumed.
// Version 1 was used by two oldest games -- Spellcasting 101 and Timequest.
// Version 2 was used by the other two Spellcasting games, Eric the Unready, and
// the first Gateway game.
// Version 3 was used first in Gateway 2 and then continued through the rest
// of the games except CCS which is the only existing game to use version 4.
// I've tried multiple versions of the same games and did not find any
// discrepancies but that does not mean that none existed. If you find any games
// that did not match listed version numbers, let me know.
// This code is not particularly clean or reliable. Use it on your own risk.
// It may crash or hang if wrong version is specified or in case of any other
// error.
// The main purpose is to document and analyze the file format.
//
// Written by Vasyl Tsvirkunov (vasyl at pacbell dot net) 7/2005

// Newest header format: GW2, Xanth, Hoboken, Shannara, Mission Critical, Callahan's Crosstime Salloon, Quandaries
// Version 4 is used only by CCS.
struct cHeader
{
    uint16_t stringCount;
    uint16_t streamSize;
    uint16_t flags; // ?
};

typedef std::vector<uint16_t> tHuffmanTree;
typedef std::vector<uint8_t> tStream;
typedef std::vector<uint16_t> tSizeTable;

uint16_t decomp(QDataStream& stream, uint16_t unitcount, uint16_t* huffmantable, uint16_t total, uint8_t* destination, uint16_t* dictionaryOffsets, uint8_t* dictionary)
{
    uint16_t uncompsize = 0;
    uint8_t bitcounter = 0;
    uint16_t treecode;
    uint8_t buffer;

    while(total) {
        treecode = unitcount-2;

internalloop:
        if (!total) {
            continue;
        }

        if(!bitcounter) {
            stream >> buffer;
            total--;

            bitcounter = 8;
        }

        treecode = huffmantable[(buffer & 1) | treecode]; // note: word table

        buffer >>= 1;

        --bitcounter;

        if ((treecode & 0x8000) == 0) { // internal node?
            goto internalloop;
        }

        treecode = treecode ^ 0xffff; // leaf

        if (treecode >= 0x80) { // shorthand?
            treecode -= 0x80; // older version used the second half for shorthands
            uint8_t* runptr = &dictionary[dictionaryOffsets[treecode]];

            while(*runptr) {
                *destination++ = *runptr++;
                uncompsize ++;
            }
        }
        else {
            *destination++ = uint8_t(treecode);
            uncompsize++;
        }
    }

    return uncompsize;
}

} // anonymous namespace

namespace model {

Text::Text(QObject *parent)
    : QAbstractItemModel(parent)
{
}

QVariant Text::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section == 0) {
            return "Text";
        }
        if (orientation == Qt::Vertical && section < rowCount()) {
            return QString::number(section + 1);
        }
    }

    return QVariant();
}

QModelIndex Text::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column) : QModelIndex();
}

QModelIndex Text::parent(const QModelIndex &) const
{
    return QModelIndex();
}

int Text::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_TextLines.count();
}

int Text::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant Text::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    if (row >= rowCount()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_TextLines.at(row);
        break;

    default:
        break;
    }

    return QVariant();
}

void Text::processDirectory(const QString &dir)
{
    QFile file(dir + "/DGATESTR.DAT");
    if (!file.open(QFile::ReadOnly)) {
        return;
    }

    QDataStream stream(&file);
    parse(stream);

    emit dataChanged(createIndex(0, 0),
                     createIndex(rowCount() - 1, 0));
}

void Text::parse(QDataStream &stream)
{
    uint16_t s;

    stream.setByteOrder(QDataStream::LittleEndian);
    // Read header. It starts from "segment" count, then contains appropriate number
    // of entries describing each segment.
    uint16_t segmentCount;
    stream >> segmentCount;

    std::vector<uint16_t> stringCounts;
    stringCounts.resize(segmentCount);
    for (s = 0; s < segmentCount; s++) {
        cHeader h;
        stream >> h.stringCount >> h.streamSize >> h.flags;
        stringCounts[s] = h.stringCount;
    }

    // Starting with version 2 there are tables of string sizes, one table per segment, one entry per string.
    // S101 and TQ don't have this table, strings are terminated implicitly.
    std::vector<tSizeTable> compressedTextSizeTables;
    compressedTextSizeTables.resize(segmentCount);

    for (s = 0; s < segmentCount; s++) {
        compressedTextSizeTables[s].resize(stringCounts[s]);
        stream.readRawData(reinterpret_cast<char*>(&compressedTextSizeTables[s][0]),
                           stringCounts[s] * 2);
    }

    // Huffman tree.
    // Encoded as table of entry pairs. First two uint8_ts is number of entries (version>=2) or pairs (version==1)
    uint16_t huffmantablesize;
    stream >> huffmantablesize;
    tHuffmanTree huffmantable;
    huffmantable.resize(huffmantablesize);
    stream.readRawData(reinterpret_cast<char*>(&huffmantable[0]), huffmantablesize * 2);

    // "Shorthand" table.
    uint16_t offsetsInShorthandsSize;
    stream >> offsetsInShorthandsSize;
    std::vector<uint16_t> offsetsInShorthands;
    offsetsInShorthands.resize(offsetsInShorthandsSize);
    stream.readRawData(reinterpret_cast<char*>(&offsetsInShorthands[0]), offsetsInShorthandsSize * 2);

    uint16_t shorthandsize;
    stream >> shorthandsize;
    std::vector<uint8_t> shorthand;
    shorthand.resize(shorthandsize);
    stream.readRawData(reinterpret_cast<char*>(&shorthand[0]), shorthandsize);

    // Encoded strings
    char buffer[65536];
    for(uint16_t i=0; i<segmentCount; i++) {
        for(uint16_t s=0; s<stringCounts[i]; s++) {
            uint16_t dsize = decomp(stream,
                                    huffmantablesize,
                                    &huffmantable[0],
                                    compressedTextSizeTables[i][s],
                                    reinterpret_cast<uint8_t*>(buffer),
                                    &offsetsInShorthands[0],
                                    &shorthand[0]);
            buffer[dsize] = 0;
            m_TextLines.append(buffer);
        }
    }
}

} // namespace model
