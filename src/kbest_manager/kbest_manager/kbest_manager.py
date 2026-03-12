import rclpy
from rclpy.node import Node
import subprocess
import threading
import time
import re

from more_interfaces.msg import KBestValues

class KBestReaderNode(Node):
    def __init__(self):
        super().__init__('kbest_reader')
        self.kbest_values_publisher = self.create_publisher(KBestValues, '/sensor/kbest_values', 10)

        self.router_ip = "192.168.1.21"

        self.local_rssi_oid = '.1.3.6.1.4.1.17455.70.701.2.8.1.4.1'
        self.remote_rssi_oid = '.1.3.6.1.4.1.17455.70.701.2.8.1.9.1'
        self.tx_rate_oid = '.1.3.6.1.4.1.17455.70.701.2.8.1.5.1'
        self.rx_rate_oid = '.1.3.6.1.4.1.17455.70.701.2.8.1.6.1'
        
        self.local_rssi = 0
        self.remote_rssi = 0
        self.tx_rate = 0
        self.rx_rate = 0
        threading.Thread(target = self.reader, daemon = True).start() # start the reader thread

    def kbest_values_callback(self, msg):
        self.get_logger().info(f"Received KBestValues: {msg}")
    
    def reader(self):
        while(True):
            try:
                snmp_read_local_rssi_cmd = ['snmpget', '-v', '2c', '-c', 'public', self.router_ip, self.local_rssi_oid]
                snmp_read_local_rssi_result = subprocess.run(snmp_read_local_rssi_cmd, capture_output=True, text=True, timeout = 5)
                stdout = snmp_read_local_rssi_result.stdout
                match = re.search(r'(-?\d+)(?=dBm)', stdout)
                if(match):
                    extracted_value = int(match.group(1))
                    self.local_rssi = extracted_value
            except:
                self.local_rssi = -1
            
            try:
                snmp_read_remote_rssi_cmd = ['snmpget', '-v', '2c', '-c', 'public', self.router_ip, self.remote_rssi_oid]
                snmp_read_remote_rssi_result = subprocess.run(snmp_read_remote_rssi_cmd, capture_output=True, text=True, timeout = 5)
                
                stdout = snmp_read_remote_rssi_result.stdout
                match = re.search(r'(-?\d+)(?=dBm)', stdout)
                if(match):
                    extracted_value = int(match.group(1))
                    self.remote_rssi = extracted_value
            except:
                self.remote_rssi = -1

            try:
                snmp_read_tx_rate_cmd = ['snmpget', '-v', '2c', '-c', 'public', self.router_ip, self.tx_rate_oid]
                snmp_read_tx_rate_result = subprocess.run(snmp_read_tx_rate_cmd, capture_output=True, text=True, timeout = 5)

                stdout = snmp_read_tx_rate_result.stdout
                match = re.search(r'(\d+\.\d+)(?=Mbps)', stdout)
                if(match):
                    extracted_value = float(match.group(1))
                    self.tx_rate = extracted_value
            except:
                self.tx_rate = -1

            try:
                snmp_read_rx_rate_cmd = ['snmpget', '-v', '2c', '-c', 'public', self.router_ip, self.rx_rate_oid]
                snmp_read_rx_rate_result = subprocess.run(snmp_read_rx_rate_cmd, capture_output=True, text=True, timeout = 5)

                stdout = snmp_read_rx_rate_result.stdout
                match = re.search(r'(\d+\.\d+)(?=Mbps)', stdout)
                if(match):
                    extracted_value = float(match.group(1))
                    self.rx_rate = extracted_value
            except:
                self.rx_rate = -1
            msg = KBestValues()
            msg.kbest_boat_rssi = self.local_rssi
            msg.kbest_ground_rssi = self.remote_rssi
            msg.tx_rate = self.tx_rate
            msg.rx_rate = self.rx_rate
            self.kbest_values_publisher.publish(msg)
            time.sleep(1)

def main(args=None):
    rclpy.init(args=args)
    node = KBestReaderNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()