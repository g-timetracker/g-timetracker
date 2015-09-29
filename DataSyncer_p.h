#ifndef DATASYNCER_P
#define DATASYNCER_P

#include "DataSyncer.h"

class DataSyncerSingleton: public DataSyncer
{
    Q_OBJECT
public:
    explicit DataSyncerSingleton(QObject *parent = 0) : DataSyncer(parent) { }
};

#endif // DATASYNCER_P
