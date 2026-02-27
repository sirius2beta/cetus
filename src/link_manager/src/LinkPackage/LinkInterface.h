#ifndef LINKINTERFACE_H
#define LINKINTERFACE_H

#include "LinkConfiguration.h"
#include <QObject>
#include <memory>
class LinkManager;

class LinkInterface : public QObject
{
    Q_OBJECT

    friend class LinkManager;
public:
    virtual ~LinkInterface();

    Q_INVOKABLE virtual void disconnect() = 0; // Implementations should guard against multiple calls


    virtual bool isConnected() const = 0;
    virtual bool isSecureConnection() const { return false; } ///< Returns true if the connection is secure (e.g. USB, wired ethernet)

    SharedLinkConfigurationPtr linkConfiguration() { return _config; }
    const SharedLinkConfigurationPtr linkConfiguration() const { return _config; }
    uint8_t mavlinkChannel() const;
    bool mavlinkChannelIsSet() const;


    void writeBytesThreadSafe(const char *bytes, int length);
signals:
    void bytesReceived(LinkInterface* link, const QByteArray &data);
    void bytesSent(LinkInterface *link, const QByteArray &data);
    void connected();
    void disconnected();
    void communicationError(const QString &title, const QString &error);
protected:
    explicit LinkInterface(SharedLinkConfigurationPtr &config, QObject *parent = nullptr);

    virtual void _freeMavlinkChannel();
    bool _allocateMavlinkChannel();
    SharedLinkConfigurationPtr _config;
private slots:
    /// Not thread safe if called directly, only writeBytesThreadSafe is thread safe
    virtual void _writeBytes(const QByteArray &bytes) = 0;
private:
    virtual bool _connect() = 0;

    uint8_t _mavlinkChannel = std::numeric_limits<uint8_t>::max();
};

typedef std::shared_ptr<LinkInterface> SharedLinkInterfacePtr;
typedef std::weak_ptr<LinkInterface> WeakLinkInterfacePtr;

#endif // LINKINTERFACE_H
