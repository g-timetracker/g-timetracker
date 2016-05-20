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

#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QCoreApplication>
#include <QSysInfo>
#include <QVersionNumber>

#include <QLoggingCategory>

#include "Updater.h"

Q_LOGGING_CATEGORY(UPDATER_CATEGORY, "Updater", QtInfoMsg)

const QUrl updateLink("http://g-timetracker.github.io/update/update.json");

const int updateCheckTimeout = 6 * 3600;

Updater::Updater(QObject *parent) :
    QObject(parent),
    m_check(false),
    m_checkTimer(new QTimer(this)),
    m_networkManager(new QNetworkAccessManager(this))
{
    m_checkTimer->setTimerType(Qt::VeryCoarseTimer);
    m_checkTimer->setInterval(updateCheckTimeout * 1000);
    m_checkTimer->setSingleShot(true);
    connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(checkForUpdates()));

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));
}

void Updater::setCheck(bool check)
{
    if (check == m_check) {
        return;
    }

    m_check = check;

    if (m_check) {
        checkForUpdates();
    }

    emit checkChanged(m_check);
}

void Updater::checkForUpdates()
{
    m_networkManager->get(QNetworkRequest(updateLink));
}

void Updater::requestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(UPDATER_CATEGORY) << "Network error:" << reply->error() << reply->errorString();
        m_checkTimer->start();
    } else {
        if (parseUpdateData(reply->readAll())) {
            m_checkTimer->stop();
        } else {
            m_checkTimer->start();
        }
    }

    reply->deleteLater();
}

bool Updater::parseUpdateData(const QByteArray &updateData)
{
    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(updateData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qCCritical(UPDATER_CATEGORY) << "Fail to parse update data JSON:"
                                     << updateData << "error at offset"
                                     << parseError.offset
                                     << parseError.errorString() << parseError.error;
        return false;
    }
    QVariantHash updateContent = document.object().toVariantHash();

    const QString &name = updateContent.value("name").toString();
    if (name.isEmpty()) {
        qCCritical(UPDATER_CATEGORY) << "Empty application name in update data";
        return false;
    } else if (name != QCoreApplication::applicationName()) {
        qCCritical(UPDATER_CATEGORY) << "Wrong application name in update data:" << name;
        return false;
    }

    QVariantMap platformEntry = updateContent.value(QSysInfo::productType()).toMap();
    if (platformEntry.isEmpty()) {
        platformEntry = updateContent.value(QSysInfo::kernelType()).toMap();    // linux
    }
    if (platformEntry.isEmpty()) {
        qCInfo(UPDATER_CATEGORY) << "No update data for current platform"
                                 << QSysInfo::prettyProductName()
                                 << QSysInfo::buildCpuArchitecture();
        return false;
    }

    QVariantMap updateEntry = platformEntry.value(QSysInfo::buildCpuArchitecture()).toMap();
    if (updateEntry.isEmpty()) {
        qCInfo(UPDATER_CATEGORY) << "No update data for current architecture"
                                 << QSysInfo::prettyProductName()
                                 << QSysInfo::buildCpuArchitecture();
        return false;
    }

    const QString &versionString = updateEntry.value("version").toString();
    QVersionNumber updateVersion = QVersionNumber::fromString(versionString);
    QVersionNumber currentVersion = QVersionNumber::fromString(QCoreApplication::applicationVersion());

    if (updateVersion <= currentVersion) {
        qCInfo(UPDATER_CATEGORY) << "No updates available";
        return false;
    }

    qCInfo(UPDATER_CATEGORY) << "New version available:" << versionString;
    emit updateAvailable(versionString, updateEntry.value("info_url").toUrl());

    return true;
}
