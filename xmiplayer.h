#ifndef XMIPLAYER_H
#define XMIPLAYER_H

#include "waveplayer.h"

class XmiPlayer : public WavePlayer
{
    Q_OBJECT
public:
    explicit XmiPlayer(QObject *parent = 0);
    ~XmiPlayer();

public slots:
    void play(const QModelIndex &index);

};

#endif // XMIPLAYER_H
