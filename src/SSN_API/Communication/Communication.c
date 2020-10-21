
#include "Communication.h"


bool SendMessage(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* message_to_send, uint8_t ssn_message_to_send_size) {
    int32_t send_message_status = Send_Message_Over_UDP(SSN_Socket, message_to_send, ssn_message_to_send_size, SSN_SERVER_IP, SSN_SERVER_PORT);
    if (send_message_status==ssn_message_to_send_size) {
        printf("-> %d-Byte Message Sent to IP: %d:%d:%d:%d @ PORT:%d\n", send_message_status, SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3], SSN_SERVER_PORT);
        return true;
    }
    else {
        printf("-> Error : %d\n", send_message_status);
        return false;
    }
}
bool SendMessageMQTT(uint8_t* messagetosend, uint8_t ssn_message_to_send_size) {
//    int8_t messagetosendsize=strlen(messagetosend);
    printf("In sendmessageMQTT %d\n",ssn_message_to_send_size);
//    int8_t send_message_status =
    Send_Message_Over_MQTT(messagetosend, ssn_message_to_send_size);    
        
//    printf("In sendmessage MQTT== %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n\n",messagetosend[0],messagetosend[1],messagetosend[2],messagetosend[3],messagetosend[4],messagetosend[5],messagetosend[6],messagetosend[7],messagetosend[8],messagetosend[9],messagetosend[10],messagetosend[11],messagetosend[12],messagetosend[13],messagetosend[14]);
    printf("In sendmessage MQTT== %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n\n",messagetosend[15],messagetosend[16],messagetosend[17],messagetosend[18],messagetosend[19],messagetosend[20],messagetosend[21],messagetosend[22],messagetosend[23],messagetosend[24],messagetosend[25],messagetosend[26],messagetosend[27],messagetosend[28],messagetosend[29]);
    printf("In sendmessage MQTT== %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n\n",messagetosend[30],messagetosend[31],messagetosend[32],messagetosend[33],messagetosend[34],messagetosend[35],messagetosend[36],messagetosend[37],messagetosend[38],messagetosend[39],messagetosend[40],messagetosend[41],messagetosend[42],messagetosend[43],messagetosend[44]);
//    printf("In sendmessage MQTT== %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n\n",messagetosend[45],messagetosend[46],messagetosend[47],messagetosend[48],messagetosend[49],messagetosend[50],messagetosend[51],messagetosend[52],messagetosend[53],messagetosend[54],messagetosend[55],messagetosend[56],messagetosend[57],messagetosend[58],messagetosend[59],messagetosend[60]);
//    printf("In sendmessageMQTT %d\n",send_message_status);

//    if (send_message_status==ssn_message_to_send_size) {
//        
    printf("-> %d-Byte Message Sent\n", ssn_message_to_send_size);
//                //" to IP: %d:%d:%d:%d @ PORT:%d\n", SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], SSN_SERVER_IP[3], SSN_SERVER_PORT);
        return true;
//    }
//    else {
//        printf("-> Error : %d\n", send_message_status);
//        return false;
//    }
}
void ReceiveMessageMQTT() {        
    clear_array(message_to_recv, max_recv_message_size);           
    Recv_Message_Over_MQTT(message_to_recv);    
}
void Send_GETMAC_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size); 
    uint8_t ssn_message_to_send_size = construct_get_mac_message(message_to_send, NodeID);
#ifdef _UDP
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
#endif
#ifdef _MQTT
    SendMessageMQTT(message_to_send, ssn_message_to_send_size);
#endif        
}

void Send_GETCONFIG_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size);
    uint8_t ssn_message_to_send_size = construct_get_configuration_message(message_to_send, NodeID);
#ifdef _UDP
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
#endif
#ifdef _MQTT
    SendMessageMQTT(message_to_send, ssn_message_to_send_size);
#endif 
}

void Send_ACKCONFIG_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* SSN_CONFIG) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size);
    uint8_t ssn_message_to_send_size = construct_ack_configuration_message(message_to_send, NodeID, SSN_CONFIG);
#ifdef _UDP
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
#endif
#ifdef _MQTT
    SendMessageMQTT(message_to_send, ssn_message_to_send_size);
#endif 
}

void Send_GETTimeOfDay_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    /* Clear the message array */
    clear_array(message_to_send, max_send_message_size);
    uint8_t ssn_message_to_send_size = construct_get_timeofday_message(message_to_send, NodeID);
#ifdef _UDP
    SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
#endif
#ifdef _MQTT
    SendMessageMQTT(message_to_send, ssn_message_to_send_size);
#endif 
}

bool Send_STATUSUPDATE_Message(uint8_t* NodeID, uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes, 
        float* Machine_load_currents, uint8_t* Machine_load_percentages, uint8_t* Machine_status, uint8_t Machine_status_flag, uint32_t* Machine_status_duration, 
        uint32_t* Machine_status_timestamp, uint32_t ssn_uptime_in_seconds, uint8_t abnormal_activity) {
    /* Clear the message array but we can't because if we do, this will throw an error at the server end */
    //clear_array(message_to_send, max_send_message_size);
    // Finally, construct the full status update message structure
    uint8_t ssn_message_to_send_size = construct_status_update_message(message_to_send, NodeID, temperature_bytes, relative_humidity_bytes, Machine_load_currents, Machine_load_percentages, 
            Machine_status, Machine_status_flag, Machine_status_duration, Machine_status_timestamp, ssn_uptime_in_seconds, abnormal_activity);
    if(ssn_message_to_send_size!=STATUS_UPDATE_MESSAGE_Size) {
        // This is not possible but still..
        printf("(Message BAD) ");
    }
#ifdef _UDP
    return SendMessage(SSN_Socket, SSN_SERVER_IP, SSN_SERVER_PORT, message_to_send, ssn_message_to_send_size);
#endif
#ifdef _MQTT
    return SendMessageMQTT(message_to_send, ssn_message_to_send_size);
#endif 
}

void Receive_MAC(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    
    /* Clear the message array */
    clear_array(message_to_recv, max_recv_message_size);
    
    uint32_t Received_Message_Bytes_in_Buffer;
    uint8_t received_message_id, received_message_status;
    

    // check how many bytes in RX buffer of Ethernet, if it is not empty (non-zero number returned), we should read it
    Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);
    // if there are more than one messages in buffer, we want to receive all of them

        while (Received_Message_Bytes_in_Buffer) {
//         read the message from buffer
        received_message_status = Recv_Message_Over_UDP(SSN_Socket, message_to_recv, max_recv_message_size, SSN_SERVER_IP, SSN_SERVER_PORT);        // Parse and make sense of the message
//         'params' array stores and organizes whatever data we have received in the message
//         this might be a new MAC address, or new Sensor Configurations, or Time of Day, etc.
        received_message_id = decipher_received_message(message_to_recv, params);


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
        // See if there is another message in the buffer so we can do this all over again
        Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);            
    }
}

uint8_t Receive_CONFIG(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT, uint8_t* SSN_CONFIG, uint8_t* SSN_REPORT_INTERVAL, uint8_t* SSN_CURRENT_SENSOR_RATINGS, 
        uint8_t* SSN_CURRENT_SENSOR_THRESHOLDS, uint8_t* SSN_CURRENT_SENSOR_MAXLOADS, uint8_t* Machine_status) {
    
    /* Clear the message array */
    clear_array(message_to_recv, max_recv_message_size);
    
    uint32_t Received_Message_Bytes_in_Buffer;
    uint8_t received_message_id, received_message_status;
    
    // check how many bytes in RX buffer of Ethernet, if it is not empty (non-zero number returned), we should read it
    Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);

    // if there are more than one messages in buffer, we want to receive all of them
    while (Received_Message_Bytes_in_Buffer) {
        // read the message from buffer
        received_message_status = Recv_Message_Over_UDP(SSN_Socket, message_to_recv, max_recv_message_size, SSN_SERVER_IP, SSN_SERVER_PORT);

        // Parse and make sense of the message
        // 'params' array stores and organizes whatever data we have received in the message
        // this might be a new MAC address, or new Sensor Configurations, or Time of Day, etc.
        received_message_id = decipher_received_message(message_to_recv, params);

        // based on which message was received (received_message_id), we extract and save the data
        switch (received_message_id) {
            case SET_CONFIG_MESSAGE_ID:
                // write the new config to designated location in EEPROM
                EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_CONFIG_LOC, params, EEPROM_CONFIG_SIZE);
                // Copy received configurations to the SSN_CONFIG array
                int i; for (i = 0; i < EEPROM_CONFIG_SIZE; i++) {
                    SSN_CONFIG[i] = params[i];
                }
                // Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
                for (i = 0; i < NO_OF_MACHINES; i++) {
                    /* Get the parameters from the Configurations */
                    SSN_CURRENT_SENSOR_RATINGS[i]    = SSN_CONFIG[3*i+0];
                    SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[3*i+1];
                    SSN_CURRENT_SENSOR_MAXLOADS[i]   = SSN_CONFIG[3*i+2];
                }
                // save new reporting interval
                *SSN_REPORT_INTERVAL = SSN_CONFIG[EEPROM_CONFIG_SIZE-1];
                printf("LOG: Received New Current Sensor Configuration from SSN Server: \n"
                    "     >> S1-Rating: %03d A | M1-Threshold: %03d A | M1-Maxload: %03d A |\n"
                    "     >> S2-Rating: %03d A | M2-Threshold: %03d A | M2-Maxload: %03d A |\n"
                    "     >> S3-Rating: %03d A | M3-Threshold: %03d A | M3-Maxload: %03d A |\n"
                    "     >> S4-Rating: %03d A | M4-Threshold: %03d A | M4-Maxload: %03d A |\n"
                    "     >> Reporting Interval: %d sec\n", 
                    SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
                    SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
                    SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
                    SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], *SSN_REPORT_INTERVAL);
                // Reset Machine States 
                for (i = 0; i < NO_OF_MACHINES; i++) {
                    Machine_status[i] = SENSOR_NOT_CONNECTED;
                }
                return 1;
                break;

            // Only for debugging, will be removed
            // This message will clear the EEPROM of our SSN
            case DEBUG_EEPROM_CLEAR_MESSAGE_ID:
                // stop the global timer
                //stop_Global_Clock();
                DisableGlobalInterrupt();
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
                //stop_Global_Clock();
                DisableGlobalInterrupt();
                // reset the SSN
                printf("(DEBUG): Resetting Controller Now...\n");
                sleep_for_microseconds(1000000);
                SoftReset();
                while(1);
                break;

            default:
                break;
        }
        // See if there is another message in the buffer so we can do this all over again
        Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);
    }
    return 0;
}

uint8_t Receive_TimeOfDay(uint8_t SSN_Socket, uint8_t* SSN_SERVER_IP, uint16_t SSN_SERVER_PORT) {
    
    /* Clear the message array */
    clear_array(message_to_recv, max_recv_message_size);
    
    uint32_t Received_Message_Bytes_in_Buffer;
    uint8_t received_message_id, received_message_status;
    uint32_t TimeOFDayTick;
    
    // check how many bytes in RX buffer of Ethernet, if it is not empty (non-zero number returned), we should read it
    Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);

    // if there are more than one messages in buffer, we want to receive all of them
    while (Received_Message_Bytes_in_Buffer) {

        // read the message from buffer
        received_message_status = Recv_Message_Over_UDP(SSN_Socket, message_to_recv, max_recv_message_size, SSN_SERVER_IP, SSN_SERVER_PORT);

        // Parse and make sense of the message
        // 'params' array stores and organizes whatever data we have received in the message
        // this might be a new MAC address, or new Sensor Configurations, or Time of Day, etc.
        received_message_id = decipher_received_message(message_to_recv, params);

        // based on which message was received (received_message_id), we extract and save the data
        switch (received_message_id) {
            case SET_TIMEOFDAY_MESSAGE_ID:
                TimeOFDayTick = get_uint32_from_bytes(params);
                printf("<- SET_TIMEOFDAY MESSAGE RECEIVED: %d\n", TimeOFDayTick);
                // assign incoming clock time to SSN Global Clock (Pseudo Clock because we don't have an RTCC)
                set_ssn_time(TimeOFDayTick);
                return 1;
                break;

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
        // See if there is another message in the buffer so we can do this all over again
        Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_Socket);            
    }
    return 0;
}

