#ifndef TIMELOGCATEGORYDATA_H
#define TIMELOGCATEGORYDATA_H

#include <QVariantMap>

struct TimeLogCategoryData
{
    Q_GADGET

    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(QVariantMap data MEMBER data)
public:
    explicit TimeLogCategoryData(const QString &name = QString(),
                                 const QVariantMap &data = QVariantMap());

    bool isValid() const;

    QString name;
    QVariantMap data;
};

Q_DECLARE_TYPEINFO(TimeLogCategoryData, Q_MOVABLE_TYPE);

#endif // TIMELOGCATEGORYDATA_H
