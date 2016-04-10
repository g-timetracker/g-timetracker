#include "TimeLogCategoryData.h"

TimeLogCategoryData::TimeLogCategoryData(const QString &name, const QVariantMap &data) :
    name(name),
    data(data)
{

}

bool TimeLogCategoryData::isValid() const
{
    return (!name.isEmpty());
}
