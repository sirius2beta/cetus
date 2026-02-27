#include "LinkInterface.h"
#include "LinkManager.h"
#include <QDebug>
#include <QTimer>
LinkInterface::LinkInterface(SharedLinkConfigurationPtr &config, QObject *parent)
    : QObject{parent}
    , _config(config)
{

}

LinkInterface::~LinkInterface()
{

}

uint8_t LinkInterface::mavlinkChannel() const
{
    if(!mavlinkChannelIsSet()){
        qDebug()<<"mavlink channel not set";
    }
    return _mavlinkChannel;
}

bool LinkInterface::mavlinkChannelIsSet() const
{
    return (LinkManager::invalidMavlinkChannel() != _mavlinkChannel);
}

bool LinkInterface::_allocateMavlinkChannel()
{
    Q_ASSERT(!mavlinkChannelIsSet());

    if (mavlinkChannelIsSet()) {
        qDebug() << "already have" << _mavlinkChannel;
        return true;
    }

    _mavlinkChannel = LinkManager::instance()->allocateMavlinkChannel();

    if (!mavlinkChannelIsSet()) {
        qDebug() << "failed";
        return false;
    }

    qDebug() << "_allocateMavlinkChannel" << _mavlinkChannel;


    return true;
}

void LinkInterface::_freeMavlinkChannel()
{
    qDebug() << _mavlinkChannel;

    if (!mavlinkChannelIsSet()) {
        return;
    }

    LinkManager::instance()->freeMavlinkChannel(_mavlinkChannel);
    _mavlinkChannel = LinkManager::invalidMavlinkChannel();
}

void LinkInterface::writeBytesThreadSafe(const char *bytes, int length)
{
    const QByteArray data(bytes, length);

    (void) QMetaObject::invokeMethod(this, [this, data] {
        _writeBytes(data);
    }, Qt::AutoConnection);
}
