#include "rclcpp/rclcpp.hpp"
#include "more_interfaces/msg/mavlink_packet.hpp"
#include "more_interfaces/msg/marinelink_packet.hpp"
#include "RosInterface.h"
#include <QCoreApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    QCoreApplication app(argc, argv);
    RosInterface* ros_interface = new RosInterface();
    QObject::connect(ros_interface, &QThread::finished, []() {
        QCoreApplication::quit(); 
    });
    ros_interface->start();
    int result = app.exec();
    rclcpp::shutdown();
    return result;
}