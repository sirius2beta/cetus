#include "LinkConfiguration.h"
#include "SerialLink.h"
#include "UDPLink.h"


LinkConfiguration::LinkConfiguration(const QString &name, QObject *parent)
    : QObject(parent)
    , _name(name)
{
}

LinkConfiguration::LinkConfiguration(const LinkConfiguration *copy, QObject *parent)
    : QObject(parent)
    , _link(copy->_link)
    , _name(copy->name())
    , _dynamic(copy->isDynamic())
    , _autoConnect(copy->isAutoConnect())
    , _highLatency(copy->isHighLatency())
{

    Q_ASSERT(!_name.isEmpty());
}

LinkConfiguration::~LinkConfiguration()
{
}

void LinkConfiguration::copyFrom(const LinkConfiguration *source)
{
    Q_ASSERT(source);

    setLink(source->_link.lock());
    setName(source->name());
    setDynamic(source->isDynamic());
    setAutoConnect(source->isAutoConnect());
    setHighLatency(source->isHighLatency());
}

LinkConfiguration *LinkConfiguration::createSettings(int type, const QString &name)
{
    LinkConfiguration *config = nullptr;

    switch (static_cast<LinkType>(type)) {
    case TypeSerial:
        config = new SerialConfiguration(name);
        break;

    case TypeUdp:
        config = new UDPConfiguration(name);
        break;

    case TypeLast:
    default:
        break;
    }

    return config;
}

LinkConfiguration *LinkConfiguration::duplicateSettings(const LinkConfiguration *source)
{
    LinkConfiguration *dupe = nullptr;

    switch(source->type()) {
#ifndef QGC_NO_SERIAL_LINK
    case TypeSerial:
        dupe = new SerialConfiguration(qobject_cast<const SerialConfiguration*>(source));
        break;
#endif
    case TypeUdp:
        dupe = new UDPConfiguration(qobject_cast<const UDPConfiguration*>(source));
        break;

    case TypeLast:
    default:
        break;
    }

    return dupe;
}

void LinkConfiguration::setName(const QString &name)
{
    if (name != _name) {
        _name = name;
        emit nameChanged(name);
    }
}

void LinkConfiguration::setLink(const SharedLinkInterfacePtr link)
{
    if (link.get() != this->link()) {
        _link = link;
        emit linkChanged();

        (void) connect(link.get(), &LinkInterface::disconnected, this, &LinkConfiguration::linkChanged, Qt::QueuedConnection);
    }
}

void LinkConfiguration::setDynamic(bool dynamic)
{
    if (dynamic != _dynamic) {
        _dynamic = dynamic;
        emit dynamicChanged();
    }
}

void LinkConfiguration::setAutoConnect(bool autoc)
{
    if (autoc != _autoConnect) {
        _autoConnect = autoc;
        emit autoConnectChanged();
    }
}

void LinkConfiguration::setHighLatency(bool hl)
{
    if (hl != _highLatency) {
        _highLatency = hl;
        emit highLatencyChanged();
    }
}
