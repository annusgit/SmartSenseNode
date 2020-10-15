
#include "Connection.h"

uint8_t SetupConnectionWithDHCP(uint8_t* SSN_MAC_ADDRESS, uint8_t UDP_SOCKET) {
    // Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
    Ethernet_Save_MAC(SSN_MAC_ADDRESS);
    
    // Get IP from DHCP, will only return once we have an IP
    Ethernet_get_IP_from_DHCP();
    
    // Our main UDP socket is defined now
    return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}

uint8_t SetupConnectionWithStaticIP(uint8_t UDP_SOCKET, uint8_t* SSN_MAC_ADDRESS, uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway) {
    // Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
    Ethernet_Save_MAC(SSN_MAC_ADDRESS);
    // Setup static ip for the SSN
    Ethernet_set_Static_IP(static_IP, subnet_mask, gateway);
    // Our main UDP socket is defined now
    return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}

uint8_t ResetConnection(uint8_t* SSN_MAC_ADDRESS, uint8_t UDP_SOCKET) {
    // Reset the device first
    Ethernet_Reset();
    
    // Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
    Ethernet_Save_MAC(SSN_MAC_ADDRESS);
    
    // Get IP from DHCP, will only return once we have an IP
    // Ethernet_get_IP_from_DHCP();
    // Initiate the Network again
    WIZ5500_network_initiate();
    
    // return a new socket from the W5500
    return socket(UDP_SOCKET, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
}

struct MQTTClient SetupConnectionWithMQTTClient(uint8_t *MQTT_IP,uint8_t* SSN_MAC_ADDRESS, uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway,char* cliendId){	       
    unsigned char tempBuffer[BUFFER_SIZE] = {};
    Ethernet_Save_MAC(SSN_MAC_ADDRESS);
    Ethernet_set_Static_IP(static_IP, subnet_mask, gateway);
    NewNetwork(&n, TCP_SOCKET);
	ConnectNetwork(&n, MQTT_IP, MQTTPort);
    MQTTClientInit(&Client_MQTT, &n, 1000, MQTT_buf, 100, tempBuffer, 2048);    
    int rc = 0;    
    SetupMQTTOptions(&MQTTOptions,cliendId ,QOS0,1,MQTT_IP);
    SetupMQTTData(&MQTT_DataPacket);    
	rc = MQTTConnect(&Client_MQTT, &MQTT_DataPacket);
	printf("Connected %d\r\n", rc);
    return Client_MQTT;
}
