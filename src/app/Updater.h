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

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>

class QTimer;
class QNetworkAccessManager;
class QNetworkReply;

class Updater : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool check MEMBER m_check WRITE setCheck NOTIFY checkChanged)
public:
    explicit Updater(QObject *parent = 0);

    void setCheck(bool check);

signals:
    void checkChanged(bool newCheck) const;
    void updateAvailable(const QString &version, const QUrl &link) const;

public slots:
    void checkForUpdates();

private slots:
    void requestFinished(QNetworkReply *reply);

private:
    bool parseUpdateData(const QByteArray &updateData);

    bool m_check;
    QTimer *m_checkTimer;
    QNetworkAccessManager *m_networkManager;
};

#endif // UPDATER_H
