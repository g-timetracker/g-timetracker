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

#include <QCoreApplication>

#include "TimeLogDefaultCategories.h"

namespace {

struct comment_tr {
    const char *source;
    const char *comment;
};

struct DefaultCategory {
    DefaultCategory(const QUuid &uuid, const char *name, const comment_tr &comment) :
        uuid(uuid), name(name), comment(comment) { }

    QUuid uuid;
    const char *name;
    comment_tr comment;
};

const DefaultCategory categories[] = {
    DefaultCategory(QUuid("{e52c78a9-fac4-4cc2-8a9d-6596ab9973c5}"),
                    QT_TRANSLATE_NOOP("default categories", "Sleep"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{324a9e29-3a12-40e2-bb58-b49aab5b1e8b}"),
                    QT_TRANSLATE_NOOP("default categories", "Work"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{93cdcff9-bb97-4312-99bb-35081278cd52}"),
                    QT_TRANSLATE_NOOP("default categories", "Routine"),
                    QT_TRANSLATE_NOOP3("default categories", "Everyday tasks", "Routine"))
    ,
    DefaultCategory(QUuid("{4ab94db3-a038-43db-b54b-35e30b1b3dc5}"),
                    QT_TRANSLATE_NOOP("default categories", "Routine > Hygiene"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{28471057-a5a2-4f7d-9dea-cf004387f19d}"),
                    QT_TRANSLATE_NOOP("default categories", "Routine > Meal"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{1d4fc48d-7ead-4877-a174-009d390a25d5}"),
                    QT_TRANSLATE_NOOP("default categories", "Housekeeping"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{e027da49-6a5c-4360-a8f1-7729ec2e80ec}"),
                    QT_TRANSLATE_NOOP("default categories", "Housekeeping > Cooking"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{5cbef4b5-8e9f-41c8-9bf7-5ee2d37d38c8}"),
                    QT_TRANSLATE_NOOP("default categories", "Housekeeping > Shopping"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{917901b6-1281-4771-b71a-587e1bf05d8f}"),
                    QT_TRANSLATE_NOOP("default categories", "Housekeeping > Tidying Up"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{0436ffa4-e52b-4879-a772-df33c2b75789}"),
                    QT_TRANSLATE_NOOP("default categories", "Healthcare"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{d09cb091-88a2-4fee-a8e5-f4e5197ed33a}"),
                    QT_TRANSLATE_NOOP("default categories", "Family"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{8b402eac-91af-4928-b96f-02da474e3408}"),
                    QT_TRANSLATE_NOOP("default categories", "Sport"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{48dfa3f5-327e-43bd-8e6b-fb6f3e348b57}"),
                    QT_TRANSLATE_NOOP("default categories", "Recreation"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{11c3e23c-6270-4079-a1f5-8c81060eaafe}"),
                    QT_TRANSLATE_NOOP("default categories", "Recreation > Books"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{5d6a9018-8c2c-4a09-b633-f3bd722c34c0}"),
                    QT_TRANSLATE_NOOP("default categories", "Recreation > Movies"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{93752a2f-e39f-4055-89e0-790397e761c8}"),
                    QT_TRANSLATE_NOOP("default categories", "Recreation > Video games"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{39619238-1f3c-4322-b9c6-b0c267252be3}"),
                    QT_TRANSLATE_NOOP("default categories", "Hobby"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{0613f185-10b8-4332-b7fc-1f44074e4254}"),
                    QT_TRANSLATE_NOOP("default categories", "Resources"),
                    QT_TRANSLATE_NOOP3("default categories", "Management of resources", "Resources"))
    ,
    DefaultCategory(QUuid("{3e812eeb-0f95-4228-a747-afcae5a593bb}"),
                    QT_TRANSLATE_NOOP("default categories", "Resources > Time"),
                    QT_TRANSLATE_NOOP3("default categories", "Time spent on time tracking :)", "Resources > Time"))
    ,
    DefaultCategory(QUuid("{cc667931-834a-4eda-a621-bd2e4d9fdac4}"),
                    QT_TRANSLATE_NOOP("default categories", "Resources > Finances"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{866d8237-865c-462a-91a4-1094c5bc38a0}"),
                    QT_TRANSLATE_NOOP("default categories", "Moving"),
                    QT_TRANSLATE_NOOP3("default categories", "Transport and other location changes", "Moving"))
    ,
    DefaultCategory(QUuid("{483f8def-d84c-4cdb-81c7-60a1e8e799a6}"),
                    QT_TRANSLATE_NOOP("default categories", "Maintenance"),
                    QT_TRANSLATE_NOOP3("default categories", "Tuning, repairing and keepeng things to work", "Maintenance"))
    ,
    DefaultCategory(QUuid("{c8b3cdff-e9ee-4c87-ad2b-0f7ef0b451ce}"),
                    QT_TRANSLATE_NOOP("default categories", "Maintenance > Computer"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{3b2416a1-7e45-4947-bf7c-8c4b8f528eb0}"),
                    QT_TRANSLATE_NOOP("default categories", "Maintenance > Gadgets"),
                    comment_tr())
    ,
    DefaultCategory(QUuid("{323a7ff1-e686-4d3b-9f90-00f86737ec22}"),
                    QT_TRANSLATE_NOOP("default categories", "Training"),
                    QT_TRANSLATE_NOOP3("default categories", "Education and skills acquisition/mastering", "Training"))
    ,
    DefaultCategory(QUuid("{81b4b9b4-315e-4310-a664-c32f188e859a}"),
                    QT_TRANSLATE_NOOP("default categories", "Unknown"),
                    QT_TRANSLATE_NOOP3("default categories", "Not tracked time", "Unknown"))
};

}

QVector<TimeLogCategory> TimeLogDefaultCategories::defaultCategories()
{
    QVector<TimeLogCategory> result;

    for (const DefaultCategory &item: categories) {
        QVariantMap data;
        QString comment(QCoreApplication::translate("default categories", item.comment.source,
                                                    item.comment.comment));
        if (!comment.isEmpty()) {
            data.insert("comment", comment);
        }
        QString name(QCoreApplication::translate("default categories", item.name));
        TimeLogCategoryData categoryData(name, data);
        TimeLogCategory category(item.uuid, categoryData);

        result.append(category);
    }

    return result;
}
