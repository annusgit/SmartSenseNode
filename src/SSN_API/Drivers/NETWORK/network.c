

#include "network.h"
#include "SSN_API/Drivers/MESSAGES/messages.h"

void WIZ5500_Reset() {
    /* Reset WIZ5500 for ~460ms */
    PORTSetBits(IOPORT_B, BIT_4);
    PORTClearBits(IOPORT_B, BIT_4);
    PORTSetBits(IOPORT_B, BIT_4);
    PORTClearBits(IOPORT_B, BIT_4);
    delay(2000000);
    PORTSetBits(IOPORT_B, BIT_4);    
}

void open_SPI2() {
/* Basic SPI configuration, pin setup and reset for WIZNet5500 SPI */
    PORTSetPinsDigitalOut(IOPORT_B, BIT_4);  // Pin-B4: Wiz-RST
    PORTSetPinsDigitalIn(IOPORT_B, BIT_13);  // Pin-B13: Wiz-MISO, See page-131 note
    PORTSetPinsDigitalOut(IOPORT_A, BIT_4);  // Pin-A2: Wiz-CS
    // Peripheral Pin Select (PPS) for SPI2
    SDI2R  = 0x0003; // MISO-2
    RPB5R  = 0x0004; // MOSI-2
    // SPI2 configuration settings
    SPI2CON  = 0x00008065; // 8-bit transfer (For Framed Data, do: 0x80008065)
    SPI2CON2 = 0x00001F00;
    SPI2BRG  = 0x00000002;
}

void Ethernet_Reset() {
    WIZ5500_Reset();
    sleep_for_microseconds(100000);
}

void setup_Ethernet(uint32_t delay_loops) {
    open_SPI2();
    
    // Setup Wiz5500 device
    PORTClearBits(IOPORT_A, BIT_4); // Select Wiz5500
    // Wiz5500 Reset
    WIZ5500_Reset();
    delay(delay_loops);
    // Device Ready for SPI communication
    
    // register read/write and chip select call backs for non-framed mode
    reg_wizchip_cs_cbfunc(WIZ5500_select, WIZ5500_deselect);
    reg_wizchip_spi_cbfunc(WIZ5500_read_byte, WIZ5500_write_byte);
    reg_wizchip_spiburst_cbfunc(WIZ5500_read_array, WIZ5500_write_array);
    
    // set basic ethernet parameters
//    WIZ5500_network_information.mac[0] = mac_address[0];
//    WIZ5500_network_information.mac[1] = mac_address[1];
//    WIZ5500_network_information.mac[2] = mac_address[2];
//    WIZ5500_network_information.mac[3] = mac_address[3];
//    WIZ5500_network_information.mac[4] = mac_address[4];
//    WIZ5500_network_information.mac[5] = mac_address[5];

//    WIZ5500_network_information.ip[0] = 192;
//    WIZ5500_network_information.ip[1] = 168;
//    WIZ5500_network_information.ip[2] = 8;
//    WIZ5500_network_information.ip[3] = 2;
//    
//    WIZ5500_network_information.sn[0] = 255;
//    WIZ5500_network_information.sn[1] = 255;
//    WIZ5500_network_information.sn[2] = 255;
//    WIZ5500_network_information.sn[3] = 0;
//    
//    WIZ5500_network_information.gw[0] = 192;
//    WIZ5500_network_information.gw[1] = 168;
//    WIZ5500_network_information.gw[2] = 0;
//    WIZ5500_network_information.gw[3] = 1;
//    
//    WIZ5500_network_information.dns[0] = 0;
//    WIZ5500_network_information.dns[1] = 0;
//    WIZ5500_network_information.dns[2] = 0;
//    WIZ5500_network_information.dns[3] = 0;
    
//    WIZ5500_network_information.dhcp = NETINFO_DHCP;
    WIZ5500_network_information.dhcp = NETINFO_STATIC;
}

unsigned int SPI2_send(unsigned int data) {
    /* PIC32MX basic SPI Single Byte transfer method for SPI2 */
    SPI2BUF = data;           // write to shift register to begin transmission
    while(!SPI2STATbits.SPIRBF); // wait for transfer to complete
    return SPI2BUF;              // read the shift register value 
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Call back function for W5500 SPI - Theses used to parameter or reg_wizchip_xxx_cbfunc()  //
//////////////////////////////////////////////////////////////////////////////////////////////
void WIZ5500_select(void) {}
void WIZ5500_deselect(void) {}

void WIZ5500_write_byte(uint8_t wb) {
    SPI2_send(wb);    
}

uint8_t WIZ5500_read_byte() {
    return (0xFF & SPI2_send(0)); // dummy transfer to push MISO
}


void WIZ5500_write_array (uint8_t* addrBuf, uint8_t* pBuf, uint16_t len) {
    /* Assume the addrBuf is the starting address array, and len number of 
       and len number of consecutive elements must be read starting from 
       this address */
    unsigned int counter = 0;
    int16_t offset_address;
    for (counter; counter < len; counter++) { // len is the len of data to be written
        // Write three bytes of address and then whatever byte is to be written
        offset_address = (addrBuf[0] << 8) + addrBuf[1] + counter;
        WIZ5500_write_byte((offset_address & 0xFF00) >> 8); // because offset is to be incremented
        WIZ5500_write_byte((offset_address & 0x00FF));
        WIZ5500_write_byte(addrBuf[2]);
        WIZ5500_write_byte(pBuf[counter]);
    }
}


void WIZ5500_read_array (uint8_t* addrBuf, uint8_t* pBuf, uint16_t len) {
    /* Assume the addrBuf is the starting address array, and len number of 
       and len number of consecutive elements must be read starting from 
       this address */
    unsigned int counter = 0;
    int16_t offset_address;
    for (counter; counter < len; counter++) { // len is the len of data to be written
        // Write three bytes of address and then whatever byte is to be written
        offset_address = (addrBuf[0] << 8) + addrBuf[1] + counter;
        WIZ5500_write_byte((offset_address & 0xFF00) >> 8); // because offset is to be incremented
        WIZ5500_write_byte((offset_address & 0x00FF));
        WIZ5500_write_byte(addrBuf[2]);
        pBuf[counter] = WIZ5500_read_byte();
    }
}

/******************************************************************************
 * @brief  Network Init
 * Intialize the network information to be used in WIZCHIP
 *****************************************************************************/
void WIZ5500_network_initiate(void) {
    uint8_t tmpstr[6] = {0,};
	wiz_NetInfo netinfo;

	// Set Network information from netinfo structure
	ctlnetwork(CN_SET_NETINFO, (void*)&WIZ5500_network_information);

#ifdef _MAIN_DEBUG_
	// Get Network information
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);

	if(netinfo.dhcp == NETINFO_DHCP) printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
	else printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);

	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],
			netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	printf("===========================\r\n");
#endif
}

/*******************************************************
 * @ brief Call back for ip assing & ip update from DHCP
 *******************************************************/
void WIZ5500_IP_assigned_callback(void) {
    getIPfromDHCP(WIZ5500_network_information.ip);
    getGWfromDHCP(WIZ5500_network_information.gw);
    getSNfromDHCP(WIZ5500_network_information.sn);
    getDNSfromDHCP(WIZ5500_network_information.dns);
    WIZ5500_network_information.dhcp = NETINFO_DHCP;
    /* Network initialization */
    WIZ5500_network_initiate();      // apply from dhcp
#ifdef _MAIN_DEBUG_
    printf("DHCP LEASED TIME : %ld Sec.\r\n", getDHCPLeasetime());
#endif
}

/************************************
 * @ brief Call back for ip Conflict
 ************************************/
void WIZ5500_IP_conflict_callback(void) {
#ifdef _MAIN_DEBUG_
	printf("CONFLICT IP from DHCP\r\n");
#endif
   //halt or reset or any...
   while(1); // this example is halt.
}

uint8_t Ethernet_get_physical_link_status() {
    return wizphy_getphylink();
}

void Ethernet_Save_MAC(uint8_t* this_mac) {
    WIZ5500_network_information.mac[0] = this_mac[0];
    WIZ5500_network_information.mac[1] = this_mac[1];
    WIZ5500_network_information.mac[2] = this_mac[2];
    WIZ5500_network_information.mac[3] = this_mac[3];
    WIZ5500_network_information.mac[4] = this_mac[4];
    WIZ5500_network_information.mac[5] = this_mac[5];
}

void Ethernet_Save_Static_IP(uint8_t* this_IP) {
    uint8_t i; for(i=0; i<4; i++) {
        WIZ5500_network_information.ip[i] = this_IP[i];
    }
}

void Ethernet_Save_Subnet_Mask(uint8_t* this_subnet) {
    uint8_t i; for(i=0; i<4; i++) {
        WIZ5500_network_information.sn[i] = this_subnet[i];
    }
}

void Ethernet_Save_Gateway_Address(uint8_t* this_gateway) {
    uint8_t i; for(i=0; i<4; i++) {
        WIZ5500_network_information.gw[i] = this_gateway[i];
    }    
}

void setup_TIMER2_with_interrupt(float delay_time) {
    IEC0CLR = 0x0200;       // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;       // clear timer 2 int flag, IFS0<9>
    IPC2CLR = 0x001f;       // clear timer 2 priority/subpriority fields 
    IPC2SET = 0x0010;       // set timer 2 int priority = 4, IPC2<4:2>
    IEC0SET = 0x0200;       // enable timer 2 int, IEC0<9>
    // Turn on 16-bit Timer2, set prescaler to 1:256 (frequency is Pbclk / 256)
    T2CON   = 0x8060;       // this prescaler reduces the input clock frequency by 64    
    
    PR2 = setPR2(delay_time);  // 1ms timer interrupt
}

void stop_TIMER2_with_interrupt() {
    IEC0CLR = 0x0200;       // disable timer 2 interrupt, IEC0<9>
    IFS0CLR = 0x0200;       // clear timer 2 int flag, IFS0<9>
    IEC0SET = 0x0000;       // disable timer 2 int, IEC0<9>
}

void __ISR(_TIMER_2_VECTOR, IPL4SOFT) Timer2IntHandler(void){
    // Do stuff...
    msTicks++; /* increment counter necessary in Delay()*/
	////////////////////////////////////////////////////////
	// SHOULD BE Added DHCP Timer Handler your 1s tick timer
	if(msTicks % 1000 == 0)	{
        DHCP_time_handler();
        /* Give the Ethernet Indication */
        No_Ethernet_LED_INDICATE();
        // printf("LOG: DHCP Timer Interrupt\n");
    }
	//////////////////////////////////////////////////////
    // Clear interrupt flag 
    IFS0CLR = 0x0200;       // clear timer 2 interrupt flag, IFS0<9>
}

void Ethernet_get_IP_from_DHCP() {
    uint8_t tmp, memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
    uint16_t my_dhcp_retry = 0;

    setup_TIMER2_with_interrupt(0.001);
    
    /* wizchip initialize*/
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1) {
       printf("LOG: -> WIZCHIP Initialized fail.\r\n");
       while(1);
    }
    printf("LOG: -> Wizchip initialized successfully\n");

    /* PHY link status check */
    do {
       if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
          printf("LOG: -> Unknown PHY Link status.\r\n");
    } while(tmp == PHY_LINK_OFF);
    printf("LOG: -> Physical Link OK\n");
    
    /* Network initialization */
    WIZ5500_network_initiate(); // Static netinfo setting
    
    // Set MAC address before initiating DHCP
	setSHAR(WIZ5500_network_information.mac);
    
    // Step-1: initiate dhcp
    DHCP_init(SOCK_DHCP, gDATABUF);
    reg_dhcp_cbfunc(WIZ5500_IP_assigned_callback, WIZ5500_IP_assigned_callback, WIZ5500_IP_conflict_callback);
    
    // printf("LOG: -> DHCP Requesting IP\n");
    
    /* DHCP Request IP Loop */
    int dhcp_status, request_started = 0;
    while(1) {
        dhcp_status = DHCP_run();
		
        switch(dhcp_status) {
            
			case DHCP_IP_ASSIGN:
                printf("LOG: -> DHCP IP Assigned\n");
                break;
                
			case DHCP_IP_CHANGED:
				printf("LOG: -> DHCP IP Changed\n");
				break;
                
			case DHCP_IP_LEASED:
				// TO DO YOUR NETWORK APPs.
                printf("LOG: -> DHCP Standby\n");
				break;
                
			case DHCP_FAILED:
				my_dhcp_retry++;
				if(my_dhcp_retry > MY_MAX_DHCP_RETRY) {
					#ifdef _MAIN_DEBUG_
					printf("LOG: -> DHCP %d Failed\r\n", my_dhcp_retry);
					#endif
					my_dhcp_retry = 0;
					DHCP_stop();      // if restart, recall DHCP_init()
					WIZ5500_network_initiate();   // apply the default static network and print out netinfo to serial
				}
				break;
                
			default:
                // printf("LOG: -> Default Case. Return code: %d\n", dhcp_status);
                if (request_started > 0) {
                    if (request_started % 500 == 0)
                        printf(".");
                }
                else
                    printf("LOG: -> DHCP Requesting IP");
                request_started++;
				break;
		}
        
        if (dhcp_status == DHCP_IP_LEASED) {
            printf("\n");
            stop_TIMER2_with_interrupt();
            break;
        }
    }
}

void Ethernet_set_Static_IP(uint8_t* static_IP, uint8_t* subnet_mask, uint8_t* gateway) {
    WIZ5500_network_information.dhcp = NETINFO_STATIC;
    uint8_t tmp, memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
    uint16_t my_dhcp_retry = 0;
    
    /* wizchip initialize*/
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1) {
       printf("LOG: -> WIZCHIP Initialized fail.\r\n");
       while(1);
    }
    printf("LOG: -> Wizchip initialized successfully\n");

    /* PHY link status check */
    do {
       if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1)
          printf("LOG: -> Unknown PHY Link status.\r\n");
    } while(tmp == PHY_LINK_OFF);
    printf("LOG: -> Physical Link OK\n");
    
    /* Network initialization */
    WIZ5500_network_initiate(); // Static netinfo setting
    
    // Set network credentials, MAC address, IP, subnet mask and gateway
    Ethernet_Save_Static_IP(static_IP);
    Ethernet_Save_Subnet_Mask(subnet_mask);
    Ethernet_Save_Gateway_Address(gateway);
    WIZ5500_network_initiate(); // Static netinfo setting
}

int32_t Send_Message_Over_UDP(uint8_t socket_number, uint8_t* message, uint8_t message_byte_length, char* destination_ip, uint16_t destination_port) {
//    uint16_t Messages_to_send_in_buffer = is_Message_to_be_transmitted(socket_number);
//    if(Messages_to_send_in_buffer) {
//        printf("Seems we had some pending messages: %d\n", Messages_to_send_in_buffer);
//    }
    return sendto(socket_number, message, message_byte_length, destination_ip, destination_port);
}

uint16_t is_Message_Received_Over_UDP(uint8_t socket_number) {
//    unsigned int received_size = getSn_RX_RSR(socket_number);
//    unsigned int number_of_received_messages = received_size / 1;
    return getSn_RX_RSR(socket_number); //number_of_received_messages;
}

uint16_t is_Message_to_be_transmitted(uint8_t socket_number) {
    return getSn_TX_FSR(socket_number);
}

uint8_t Recv_Message_Over_UDP(uint8_t socket_number, char* message, uint8_t message_byte_length, char* destination_ip, uint16_t destination_port) {
    // Check the Sn_IR(RECV) Interrupt bit. 
    // This is issued whenever data is received from a peer.
    // This function returns zero if there was no data in recv buffer or 0 data size received
//    printf("Received Buffer Size: %d %d\r\n", getSn_IR(socket_number), getSn_IMR(socket_number));
        // equal to zero means set
        // Clear the recv interrupt bit
        int32_t data_size = recvfrom(socket_number, message, message_byte_length, destination_ip, &destination_port);
//        setSn_IR(socket_number, new_recv_buffer_val);
//        printf("Received Message\r\n");
    return data_size;
}

void Recv_Message_Over_MQTT(uint8_t* messagetorecv){
    int rc = 0;    
//	MQTTOptions.showtopics = 1;	
    printf("Subscribing to %s\r\n", TopicToSubscribeTo);
	rc = MQTTSubscribe(&Client_MQTT, TopicToSubscribeTo, MQTTOptions.qos, messageArrivedoverMQTT);
    printf("Messagetorecieve %s\r\n",Messagetorecieve);
    messagetorecv=*Messagetorecieve;
    printf("Messagetorecv %s\r\n",messagetorecv);
	printf("Subscribed %d\r\n", rc);
}

void SetupMQTTOptions(opts_struct* MQTTOptions,char* cliendId ,enum QoS x,int showtopics,char* MQTT_IP){
    strcpy(MQTTOptions->clientid,cliendId);
    MQTTOptions->nodelimiter=0; 
    strcpy(MQTTOptions->delimiter,"\n");
    MQTTOptions->qos=x; 
    strcpy(MQTTOptions->username,"NULL");        
    strcpy(MQTTOptions->password,"NULL");  
    strcpy(MQTTOptions->host,MQTT_IP);  
    MQTTOptions->port=MQTTPort; 
    MQTTOptions->showtopics=showtopics;
}

void SetupMQTTMessage(MQTTMessage* Message_MQTT,uint8_t* payload ,enum QoS x){
//    printf("%d\n",Message_MQTT->payloadlen);
    Message_MQTT->qos=x;
    Message_MQTT->retained=0;
    Message_MQTT->dup=0;
    Message_MQTT->id=1;
    Message_MQTT->payload=payload;
//    printf("In SetupMQTTMessage=%d \n",payload);
    Message_MQTT->payloadlen=strlen(payload);
//    printf("SetupMQTTMessage %d\n",Message_MQTT->payloadlen);
}
void SetupMQTTData(MQTTPacket_connectData* MQTT_DataPacket){
    	
    MQTT_DataPacket->willFlag = 0;
	MQTT_DataPacket->MQTTVersion = 3;
	MQTT_DataPacket->clientID.cstring = MQTTOptions.clientid;
	MQTT_DataPacket->username.cstring = MQTTOptions.username;
	MQTT_DataPacket->password.cstring = MQTTOptions.password;

	MQTT_DataPacket->keepAliveInterval = 180;
	MQTT_DataPacket->cleansession = 1;
}


void Send_Message_Over_MQTT(uint8_t* messagetosend){        
    int rc = 0;    
    SetupMQTTMessage(&Message_MQTT,messagetosend,QOS1 );    
    rc= MQTTPublish(&Client_MQTT, TopicToPublishTo, &Message_MQTT);
    printf("Published %d\r\n", rc);
//    return rc;
}

void messageArrivedoverMQTT(MessageData* md){//,char* Messagetorecv) {
	unsigned char testbuffer[BUFFER_SIZE];
	MQTTMessage* message = md->message; 
    printf(":::::\n");
//    clear_array(testbuffer,20);
	if (MQTTOptions.showtopics) {
		memcpy(testbuffer,(char*)message->payload,(int)message->payloadlen);
		*(testbuffer + (int)(message->payloadlen) + 1) = "\n";
                
        uint8_t received_message_id;

        uint32_t TimeOFDayTick;

        received_message_id = decipher_received_message(testbuffer, params);
    
        received_message_id = received_message_id - '0';  // ASCii value of '0' is 48 so 52-48 will return 4.

//        printf("Received message ID is %d\n",received_message_id);

        // based on which message was received (received_message_id), we extract and save the data
        switch (received_message_id) {
            case SET_MAC_MESSAGE_ID:
                printf("<- SET_MAC MESSAGE RECEIVED: %X:%X:%X:%X:%X:%X\n", params[0], params[1], params[2], params[3], params[4], params[5]);
                printf("Resetting Controller Now...\n");
                // write the new MAC addresses to designated location in EEPROM
                EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_MAC_LOC, params, EEPROM_MAC_SIZE);
                // reset the SSN from software
                SoftReset();
                while(1);
                break;

            case SET_TIMEOFDAY_MESSAGE_ID:
                TimeOFDayTick = get_uint32_from_bytes(params);
                printf("<- SET_TIMEOFDAY MESSAGE RECEIVED: %d\n", TimeOFDayTick);
                // assign incoming clock time to SSN Global Clock (Pseudo Clock because we don't have an RTCC)
                set_ssn_time(TimeOFDayTick);
//                return 1;
                break;

                            
//            case SET_CONFIG_MESSAGE_ID:
//                // write the new config to designated location in EEPROM
//                EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_CONFIG_LOC, params, EEPROM_CONFIG_SIZE);
//                // Copy received configurations to the SSN_CONFIG array
//                int i; for (i = 0; i < EEPROM_CONFIG_SIZE; i++) {
//                    SSN_CONFIG[i] = params[i];
//                }
//                // Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
//                for (i = 0; i < NO_OF_MACHINES; i++) {
//                    /* Get the parameters from the Configurations */
//                    SSN_CURRENT_SENSOR_RATINGS[i]    = SSN_CONFIG[3*i+0];
//                    SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[3*i+1];
//                    SSN_CURRENT_SENSOR_MAXLOADS[i]   = SSN_CONFIG[3*i+2];
//                }
//                // save new reporting interval
//                *SSN_REPORT_INTERVAL = SSN_CONFIG[EEPROM_CONFIG_SIZE-1];
//                printf("LOG: Received New Current Sensor Configuration from SSN Server: \n"
//                    "     >> S1-Rating: %03d A | M1-Threshold: %03d A | M1-Maxload: %03d A |\n"
//                    "     >> S2-Rating: %03d A | M2-Threshold: %03d A | M2-Maxload: %03d A |\n"
//                    "     >> S3-Rating: %03d A | M3-Threshold: %03d A | M3-Maxload: %03d A |\n"
//                    "     >> S4-Rating: %03d A | M4-Threshold: %03d A | M4-Maxload: %03d A |\n"
//                    "     >> Reporting Interval: %d sec\n", 
//                    SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
//                    SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
//                    SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
//                    SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], *SSN_REPORT_INTERVAL);
//                // Reset Machine States 
//                for (i = 0; i < NO_OF_MACHINES; i++) {
//                    Machine_status[i] = SENSOR_NOT_CONNECTED;
//                }
////                return 1;
//                break;

            // Only for debugging, will be removed
            // This message will clear the EEPROM of our SSN
            case DEBUG_EEPROM_CLEAR_MESSAGE_ID:
                // stop the global timer
                stop_Global_Clock();
                printf("(DEBUG): Clearing EEPROM Now...\n");
                // Clear EEPROM and reset node
                EEPROM_Clear();
                // reset the SSN
                printf("(DEBUG): Resetting Controller Now...\n");
                SoftReset();
                while(1);
                break;

            // Only for debugging, will be removed
            // This message will reset our SSN
            case DEBUG_RESET_SSN_MESSAGE_ID:
                // stop the global timer
                stop_Global_Clock();
                // reset the SSN
                printf("(DEBUG): Resetting Controller Now...\n");
                sleep_for_microseconds(1000000);
                SoftReset();
                while(1);
                break;

            default:
                break;
        }
		printf("testbuffer %s\r\n",testbuffer); 
//        printf("%s\r\n",message->payload);
        clear_array(testbuffer,100);
//        printf("testbuffer %s\r\n",testbuffer); 

	}
	if (MQTTOptions.nodelimiter)
		printf("%.*s", (int)message->payloadlen, (char*)message->payload);
	else
    {
        printf("%.*s%s", (int)message->payloadlen, (char*)message->payload, MQTTOptions.delimiter);
//        *Messagetorecieve=message->pay load;              
//        printf("Message->Payload %s\r\n",message->payload);                
//        printf("Messagetorecieve %s\r\n",*Messagetorecieve);
        
    }

    }
        
//    *Messagetorecieve=testbuffer;              
//            
//    printf("test buffer %s\r\n",*Messagetorecieve);

//    return Messagetorecv;
//}
//    %.* s    payloadlen, payload
//    %.* s %s payloadlen, payload, delimiter
