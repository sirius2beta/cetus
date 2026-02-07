#include "rclcpp/rclcpp.hpp"
#include "more_interfaces/msg/mavlink_packet.hpp"
#include "more_interfaces/msg/marinelink_packet.hpp"
#include "RosInterface.h"
#include "bridge.h"
#include "linkmanager.h"
#include <QCoreApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    QCoreApplication app(argc, argv);
    RosInterface* ros_interface = new RosInterface();
    LinkManager* linkManager = LinkManager::instance();
    linkManager->startAutoConnectedLinks();
    linkManager->init();
    QObject::connect(ros_interface, &QThread::finished, []() {
        QCoreApplication::quit(); 
    });
    QObject::connect(Bridge::instance(), &Bridge::mavlinkToParse, ros_interface, &RosInterface::onMavlinkToParse);
    ros_interface->start();
    int result = app.exec();
    rclcpp::shutdown();
    return result;
}