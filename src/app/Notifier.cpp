#include "Notifier.h"

Notifier::Notifier(QObject *parent) : QObject(parent)
{

}

void Notifier::requestActivate() const
{
    emit activateRequested();
}
