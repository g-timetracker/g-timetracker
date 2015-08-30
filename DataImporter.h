#ifndef DATAIMPORTER_H
#define DATAIMPORTER_H

#include "TimeLogEntry.h"

class DataImporter
{
public:
    DataImporter();

    bool importFile(const QString &path) const;

private:
    TimeLogEntry parseLine(const QString &line) const;
};

#endif // DATAIMPORTER_H
