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

#include <QFileSelector>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QVersionNumber>

#include <QLoggingCategory>

#include "ChangeLog.h"

Q_LOGGING_CATEGORY(CHANGELOG_CATEGORY, "ChangeLog", QtInfoMsg)

ChangeLog::ChangeLog(QObject *parent) : QObject(parent)
{

}

bool ChangeLog::checkChanges(const QString &previousVersion, const QString &currentVersion)
{
    QVersionNumber previous = QVersionNumber::fromString(previousVersion);
    QVersionNumber current = QVersionNumber::fromString(currentVersion);
    if (current <= previous) {
        qCInfo(CHANGELOG_CATEGORY) << "Current version isn't newer, than previous"
                                   << previousVersion << currentVersion;
        return false;
    }

    QFileSelector fileSelector;
    fileSelector.setExtraSelectors(QStringList() << QLocale::system().uiLanguages().constFirst().split('-').constFirst());
    const QString path = fileSelector.select(":/changelogs/changelog.json");

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCCritical(CHANGELOG_CATEGORY) << "Fail to open changelog file" << path << file.errorString();
        return false;
    }
    QByteArray data = file.readAll();

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCritical(CHANGELOG_CATEGORY) << "Fail to parse changelog data JSON:"
                                      << data << "error at offset"
                                      << parseError.offset
                                      << parseError.errorString() << parseError.error;
        return false;
    }
    QVariantList content = document.array().toVariantList();

    while (!content.isEmpty()) {
        if (QVersionNumber::fromString(content.constFirst().toMap().value("version").toString()) > current) {
            content.takeFirst();
        } else {
            break;
        }
    }
    QVariantList result;
    while (!content.isEmpty()) {
        if (QVersionNumber::fromString(content.constFirst().toMap().value("version").toString()) > previous) {
            result.append(content.takeFirst());
        } else {
            break;
        }
    }

    if (result.isEmpty()) {
        qCWarning(CHANGELOG_CATEGORY) << "Empty changelog" << previousVersion << currentVersion;
        return false;
    }

    emit changesAvailable(result);

    return true;
}
