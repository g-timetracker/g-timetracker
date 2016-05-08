#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <QObject>

class Notifier : public QObject
{
    Q_OBJECT
public:
    explicit Notifier(QObject *parent = 0);

signals:
    void activateRequested() const;

public slots:
    void requestActivate() const;
};

#endif // NOTIFIER_H
