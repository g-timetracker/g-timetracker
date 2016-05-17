/**
 ** This file is part of the G-TimeTracker project.
 ** Copyright 2015-2016 Nikita Krupenko <krnekit@gmail.com>.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

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
