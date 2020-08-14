

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#pragma config FWDTEN   = OFF           // Turn off watchdog timer
#pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (Disabled)
#pragma config FNOSC    = FRCPLL        // Select 8MHz internal Fast RC (FRC) oscillator with PLL
#pragma config FPLLIDIV = DIV_2         // Divide PLL input (FRC) -> 4MHz
#pragma config FPLLMUL  = MUL_15        // PLL Multiplier -> 60MHz
#pragma config FPLLODIV = DIV_1         // PLL Output Divider -> 60MHz
#pragma config FPBDIV   = DIV_2         // Peripheral Clock divisor -> 30MHz
#pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select (Communicate on PGEC2/PGED2)
#pragma config JTAGEN   = OFF           // JTAG Enable (Disabled)


#include "src/SSN_API/Drivers/CURRENT_SENSOR/current_sensor.h"
#include "src/SSN_API/SSN_API.h"

/* Our main udp communication socket */
SOCKET SSN_UDP_SOCKET;
/* SSN Server Address and Port */
uint8_t SSN_SERVER_IP[] = {192, 168, 1, 100};
uint16_t SSN_SERVER_PORT = 9999;
/* A counter to maintain how many messages have been sent from SSN to Server since wakeup */
uint32_t SSN_SENT_MESSAGES_COUNTER = 0;
/* Size, Status and the actual message to be sent from SSN to Server */
uint8_t ssn_message_to_send_size, send_message_status, message_to_send[max_send_message_size];
/* Counter variables for interrupts */
uint8_t interrupts_per_second = 2, half_second_counter = 0, report_counter = 0;
/* Current State of the SSN in the SSN State Machine */
uint8_t SSN_CURRENT_STATE;
/* Report Interval of SSN set according to the configurations passed to the SSN */
uint8_t SSN_REPORT_INTERVAL = 1;

/* Half-Second interrupt that controls our send message routine of the SSN
 * Half-second and not one second is because we can not set an interrupt of up to 1 second with the current clock of the SSN
 * We only start this interrupt service once we have Ethernet configured and all self-tests are successful
 * The message to be sent is constructed every half a second in the main function and only reported to the server after every "SSN_REPORT_INTERVAL" seconds */
void __ISR(_TIMER_1_VECTOR, IPL4SOFT) Timer1IntHandler_SSN_Hearbeat(void){

    // clear timer 1 interrupt flag, IFS0<4>
    IFS0CLR = 0x0010;       

    // Indicate the status of SSN from the SSN LED after every half second
    SSN_LED_INDICATE(SSN_CURRENT_STATE);

    // check of we have reached one second interval (because two half-seconds make one second)
    half_second_counter++;
    if (half_second_counter >= interrupts_per_second) {

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /* Normal routines that should execute in any case, whether we are messaging or not */
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        
        // reset half second counter
        half_second_counter = 0;
        
        // add a second to report counter
        report_counter++;

        // increment global clock
        increment_pseudo_clock_time(&ssn_clock);
        
        // increment global uptime in seconds
        ssn_uptime_in_seconds++;
        
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        printf("Interrupt\n");
        // Is it time to report?
        if (report_counter >= SSN_REPORT_INTERVAL) {
            
            // Reset the reporting counter
            report_counter = 0;

            // Check Ethernet Physical Link Status before sending message
            // Send message to the server only when we have a stable physical link
            if(Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
                SSN_CURRENT_STATE = NO_ETHERNET_STATE;
                printf("LOG: Ethernet Physical Link BAD. Can't Send Message...\n");
            }
            else {
                send_message_status = Send_Message_Over_UDP(SSN_UDP_SOCKET, message_to_send, ssn_message_to_send_size, SSN_SERVER_IP, SSN_SERVER_PORT);
                if (SSN_CURRENT_STATE == ACK_CONFIG_STATE) {
                    // set SSN_CURRENT_STATE to normal status update state
                    SSN_CURRENT_STATE = NO_TIMEOFDAY_STATE;
                }
                if (send_message_status > 0)
                    printf("-> %d-Byte Message# %d to IP: %d:%d:%d:%d @ PORT:%d\n", send_message_status, SSN_SENT_MESSAGES_COUNTER++, SSN_SERVER_IP[0], SSN_SERVER_IP[1], SSN_SERVER_IP[2], 
                            SSN_SERVER_IP[3], SSN_SERVER_PORT);
                else
                    printf("LOG: No data sent. Error Code: %d\n", send_message_status);
            }
        }
    }
}


int main() {

    // SSN current sensor configurations
    uint8_t SSN_CONFIG[EEPROM_CONFIG_SIZE];
    uint8_t SSN_CURRENT_SENSOR_RATINGS[4];
    uint8_t SSN_CURRENT_SENSOR_THRESHOLDS[4];
    uint8_t SSN_CURRENT_SENSOR_MAXLOADS[4];

    // Variables for our SSN Machine Statistics
    uint8_t Machine_load_currents[NO_OF_MACHINES] = {0};
    uint8_t Machine_load_percentages[NO_OF_MACHINES] = {0};
    uint8_t Machine_status[NO_OF_MACHINES] = {MACHINE_RESET_SENTINEL_STATE, MACHINE_RESET_SENTINEL_STATE, MACHINE_RESET_SENTINEL_STATE, MACHINE_RESET_SENTINEL_STATE};
    pseudo_clock Machine_status_timestamp[NO_OF_MACHINES];
    uint32_t Machine_status_duration[NO_OF_MACHINES] = {0};

    // SSN Previous state variables
    bool SSN_PREVIOUS_STATE_SAVED = false;
    uint8_t SSN_PREVIOUS_STATE;
    
    // SSN network related variables
    uint8_t SSN_UDP_SOCKET_NUM = 1;
    uint8_t SSN_DEFAULT_MAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint8_t SSN_MAC_ADDRESS[6] = {0}; // the last two bytes of our SSN_MAC_ADDRESS will be our node id bytes
    
    /* Variables needed for our received message
     * The params variable is an array of whatever parameters we receive in a message from the Server. 
     * This can contain our new MAC address, time of day, sensor configurations, etc. */
    uint32_t Received_Message_Bytes_in_Buffer;
    uint8_t message_to_recv[max_recv_message_size];
    uint8_t received_message_id, received_message_status;
    uint8_t params[max_recv_message_size];
    
    // Variables for our temperature and humidity sensor
    uint8_t temp_humidity_recv_status;
    uint8_t temperature_bytes[2], relative_humidity_bytes[2];
    uint8_t i, abnormal_activity = 0;
    
    // Setup Smart Sense Node
    SSN_Setup();
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////* System Tests *//////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
//    // 1. Check EEPROM
//    printf("\n\n\nLOG: SSN AWAKE...\n");
//    if (EEPROM_Check() == EEPROM_TEST_PASSED) {
//        printf("LOG: EEPROM TEST SUCCESSFUL.\n");
//    } 
//    else {
//        printf("LOG: EEPROM TEST FAILED. SSN Halted...\n");
//        SSN_CURRENT_STATE = SELF_TEST_FAILED_STATE;
//        while(SSN_IS_ALIVE) {
//            // Forever, indicate the status of SSN i.e. SELF_TEST_FAILED_STATE from the SSN LED
//            SSN_LED_INDICATE(SSN_CURRENT_STATE);
//            sleep_for_microseconds(500000);
//        }
//    }
//    
//    // 2. Check Temperature Sensor (called off)
//    temp_humidity_recv_status = sample_Temperature_Humidity_bytes(temperature_bytes, relative_humidity_bytes);
//    // TODO: Add gradient based temperature and humidity change check
//    if (ambient_condition_status() == ABNORMAL_AMBIENT_CONDITION) {
//        printf("LOG: TEMPERATURE SENSOR TEST FAILED. SSN Halted...\n");
//        SSN_CURRENT_STATE = SELF_TEST_FAILED_STATE;
////        while(SSN_IS_ALIVE) {
////            // Forever, indicate the status of SSN i.e. SELF_TEST_FAILED_STATE from the SSN LED
////            SSN_LED_INDICATE(SSN_CURRENT_STATE);
////            sleep_for_microseconds(500000);
////        }
//    } 
//    else {
//        printf("LOG: TEMPERATURE SENSOR TEST SUCCESSFUL.\n");
//    }
//    
//    // 3. Check Ethernet Physical Link to SSN, i.e., is the Ethernet cable plugged into RJ45 of our SSN
//    if (Ethernet_get_physical_link_status() != PHY_LINK_ON) {
//        printf("LOG: Ethernet Physical Link BAD. Waiting for link...\n");
//        SSN_CURRENT_STATE = NO_ETHERNET_STATE;
//        while (Ethernet_get_physical_link_status() != PHY_LINK_ON) {
//            // Indicate the status of SSN i.e. NO_ETHERNET_STATE from the SSN LED as long as we don't get a stable physical link
//            SSN_LED_INDICATE(SSN_CURRENT_STATE);
//            sleep_for_microseconds(500000);
//        }
//        /* At this point, we have tested our EEPROM, our Temperature sensor and our Ethernet Connection
//         * Now we can move into our regular SSN state machine */ 
//        printf("LOG: Ethernet Physical Link OK\n");
//        SSN_CURRENT_STATE = NO_MAC_STATE;
//    } 
//    else {
//        printf("LOG: ETHERNET PHYSICAL LINK TEST SUCCESSFUL\n");
//        /* At this point, we have tested our EEPROM, our Temperature sensor and our Ethernet Connection.
//         * Now we can move into our regular SSN state machine */
//        SSN_CURRENT_STATE = NO_MAC_STATE;
//    }
    
    RunSystemTests();

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////* Find in EEPROM *////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
//    // 1. Check EEPROM for MAC address
//    EEPROM_Read_Array(EEPROM_BLOCK_0, EEPROM_MAC_LOC, SSN_MAC_ADDRESS, EEPROM_MAC_SIZE);
//    uint8_t valid_MAC_in_EEPROM = is_Valid_MAC(SSN_MAC_ADDRESS);
//    if (!valid_MAC_in_EEPROM) {
//        // we don't have a valid MAC address, use default MAC instead
//        printf("LOG: Invalid MAC in EEPROM. Using Default MAC %X:%X:%X:%X:%X:%X\n", SSN_DEFAULT_MAC[0], SSN_DEFAULT_MAC[1], SSN_DEFAULT_MAC[2], SSN_DEFAULT_MAC[3], SSN_DEFAULT_MAC[4], 
//                SSN_DEFAULT_MAC[5]);
//        for (i = 0; i < 6; i++)
//            SSN_MAC_ADDRESS[i] = SSN_DEFAULT_MAC[i];
//        // Since we have no custom MAC address with us, set state to ask for MAC address with Get_MAC message
//        SSN_CURRENT_STATE = NO_MAC_STATE;
//    } 
//    else {
//        printf("LOG: Found MAC in EEPROM -> %X:%X:%X:%X:%X:%X\n", SSN_MAC_ADDRESS[0], SSN_MAC_ADDRESS[1], SSN_MAC_ADDRESS[2], SSN_MAC_ADDRESS[3], SSN_MAC_ADDRESS[4], SSN_MAC_ADDRESS[5]);
//        SSN_CURRENT_STATE = NO_CONFIG_STATE;
//    }
//
//    // 2. Check CONFIG in EEPROM only if MAC was valid
//    if (SSN_CURRENT_STATE == NO_CONFIG_STATE) {
//        // Check EEPROM for current sensor configurations
//        EEPROM_Read_Array(EEPROM_BLOCK_0, EEPROM_CONFIG_LOC, SSN_CONFIG, EEPROM_CONFIG_SIZE);
//        uint8_t valid_CONFIG_in_EEPROM = is_Valid_CONFIG(SSN_CONFIG);
//        if (!valid_CONFIG_in_EEPROM) {
//            // we don't have a valid config, need to send Get_Config message to SSN Server
//            printf("LOG: Invalid CONFIG in EEPROM. Getting CONFIG from SSN_SERVER now...\n");
//        } 
//        else {
//            // Copy from the configurations, the sensor ratings, thresholds and maximum load values into our variables
//            for (i = 0; i < NO_OF_MACHINES; i++) {
//                // Get the parameters from the Configurations
//                SSN_CURRENT_SENSOR_RATINGS[i]    = SSN_CONFIG[3*i+0];
//                SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[3*i+1];
//                SSN_CURRENT_SENSOR_MAXLOADS[i]   = SSN_CONFIG[3*i+2];
//            }
//            SSN_REPORT_INTERVAL = SSN_CONFIG[12];
//            printf("LOG: Received New Current Sensor CONFIG: \n"
//                    "     >> S1-Rating: %03d A | M1-Threshold: %03d A | M1-Maxload: %03d A |\n"
//                    "     >> S2-Rating: %03d A | M2-Threshold: %03d A | M2-Maxload: %03d A |\n"
//                    "     >> S3-Rating: %03d A | M3-Threshold: %03d A | M3-Maxload: %03d A |\n"
//                    "     >> S4-Rating: %03d A | M4-Threshold: %03d A | M4-Maxload: %03d A |\n"
//                    "     >> Reporting Interval: %d sec\n", 
//                    SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
//                    SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
//                    SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
//                    SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], SSN_REPORT_INTERVAL);
//            // We have our MAC address and the sensor configurations, only thing remaining is the time of day
//            SSN_CURRENT_STATE = NO_TIMEOFDAY_STATE;
//        }
//    }
    
    SSN_CURRENT_STATE = FindConfigurationsInFlashMemory(SSN_MAC_ADDRESS, SSN_DEFAULT_MAC, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, 
            SSN_CURRENT_SENSOR_MAXLOADS);
    
//    // Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
//    Ethernet_Assign_MAC(SSN_MAC_ADDRESS);
//    
//    // Get IP from DHCP, will only return once we have an IP
//    Ethernet_get_IP_from_DHCP();
//    
//    // Our main UDP socket is defined now
//    SSN_UDP_SOCKET = socket(SSN_UDP_SOCKET_num, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
    SSN_UDP_SOCKET = SetupConnection(SSN_MAC_ADDRESS, SSN_UDP_SOCKET_NUM);
    
    // Start the global clock that will trigger a response each half of a second through our half-second interrupt defined above Main function
    setup_Global_Clock_And_SSN_Half_Second_Heartbeat(PERIPH_CLK);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////* Main Loop */////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    /* Main loop will do the following:
     * 1. Receive messages from SSN Server
     * 2. Decipher received messages to extract incoming information
     * 3. Based on SSN_CURRENT_STATE, it will construct the right messages to be sent to the SSN Server */
    
    while(SSN_IS_ALIVE) {
//        printf("SSN Current State: %d\n", SSN_CURRENT_STATE);
        // Ethernet physical link status check before receiving a new message
        if(Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
            // bad Ethernet link, so only once save whatever state we were in, for example, Get_MAC, Get_Config, Update_Status, etc.
            // and don't receive any messages since we are not connected to the Ethernet
            // also set new state to NO_ETHERNET_STATE to give proper LED_indication 
            if (!SSN_PREVIOUS_STATE_SAVED) {
                // this will run only once after we shift from a running state to NO_ETHERNET_STATE, so only last state will be saved and not overwritten
                SSN_PREVIOUS_STATE_SAVED = true;
                SSN_PREVIOUS_STATE = SSN_CURRENT_STATE; 
            }
            SSN_CURRENT_STATE = NO_ETHERNET_STATE;
            printf("LOG: Ethernet Physical Link BAD. Can't Receive Incoming Message...\n");
        }
        else {
            // first check if we are returning from a NO_ETHERNET_STATE
            // if yes, then we should recover our previous state
            if (SSN_PREVIOUS_STATE_SAVED) {
                SSN_PREVIOUS_STATE_SAVED = false;
                SSN_CURRENT_STATE = SSN_PREVIOUS_STATE;
            }
        
            // check how many bytes in RX buffer of Ethernet, if it is not empty (non-zero number returned), we should read it
            Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_UDP_SOCKET);

            // if there are more than one messages in buffer, we want to receive all of them
            while (Received_Message_Bytes_in_Buffer) {

                // read the message from buffer
                received_message_status = Recv_Message_Over_UDP(SSN_UDP_SOCKET, message_to_recv, max_recv_message_size, SSN_SERVER_IP, SSN_SERVER_PORT);

                // Parse and make sense of the message
                // 'params' array stores and organizes whatever data we have received in the message
                // this might be a new MAC address, or new Sensor Configurations, or Time of Day, etc.
                received_message_id = decipher_received_message(message_to_recv, params);

                // based on which message was received (received_message_id), we extract and save the data
                switch (received_message_id) {

                    // Server wants to assign new MAC address
                    // we reset after assignment because we want to wake up and get IP with this new MAC address
                    case SET_MAC_MESSAGE_ID:
                        printf("<- SET_MAC MESSAGE RECEIVED: %X:%X:%X:%X:%X:%X\n", params[0], params[1], params[2], params[3], params[4], params[5]);
                        printf("Reseting Controller Now...\n");
                        // write the new MAC addresses to designated location in EEPROM
                        EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_MAC_LOC, params, EEPROM_MAC_SIZE);
                        // reset the SSN from software
                        SoftReset();
                        break;

                    // Server wants to assign new current sensor configurations
                    case SET_CONFIG_MESSAGE_ID:
                        // write the new config to designated location in EEPROM
                        EEPROM_Write_Array(EEPROM_BLOCK_0, EEPROM_CONFIG_LOC, params, EEPROM_CONFIG_SIZE);
                        // Copy received configurations to the SSN_CONFIG array
                        for (i = 0; i < EEPROM_CONFIG_SIZE; i++)
                            SSN_CONFIG[i] = params[i];
                        // Copy from the configurations, the sensor ratings, thresholds and maximum load values to our variables
                        for (i = 0; i < NO_OF_MACHINES; i++) {
                            /* Get the parameters from the Configurations */
                            SSN_CURRENT_SENSOR_RATINGS[i]    = SSN_CONFIG[3*i+0];
                            SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[3*i+1];
                            SSN_CURRENT_SENSOR_MAXLOADS[i]   = SSN_CONFIG[3*i+2];
                        }
                        // save new reporting interval
                        SSN_REPORT_INTERVAL = SSN_CONFIG[EEPROM_CONFIG_SIZE-1];
                        printf("LOG: Found Valid Current Sensor CONFIG in EEPROM: \n"
                            "     >> S1-Rating: %03d A | M1-Threshold: %03d A | M1-Maxload: %03d A |\n"
                            "     >> S2-Rating: %03d A | M2-Threshold: %03d A | M2-Maxload: %03d A |\n"
                            "     >> S3-Rating: %03d A | M3-Threshold: %03d A | M3-Maxload: %03d A |\n"
                            "     >> S4-Rating: %03d A | M4-Threshold: %03d A | M4-Maxload: %03d A |\n"
                            "     >> Reporting Interval: %d sec\n", 
                            SSN_CURRENT_SENSOR_RATINGS[0], SSN_CURRENT_SENSOR_THRESHOLDS[0], SSN_CURRENT_SENSOR_MAXLOADS[0],
                            SSN_CURRENT_SENSOR_RATINGS[1], SSN_CURRENT_SENSOR_THRESHOLDS[1], SSN_CURRENT_SENSOR_MAXLOADS[1],
                            SSN_CURRENT_SENSOR_RATINGS[2], SSN_CURRENT_SENSOR_THRESHOLDS[2], SSN_CURRENT_SENSOR_MAXLOADS[2],
                            SSN_CURRENT_SENSOR_RATINGS[3], SSN_CURRENT_SENSOR_THRESHOLDS[3], SSN_CURRENT_SENSOR_MAXLOADS[3], SSN_REPORT_INTERVAL);
                        // Reset Machine States 
                        for (i = 0; i < NO_OF_MACHINES; i++)
                            Machine_status[i] = MACHINE_RESET_SENTINEL_STATE;
                        // change SSN_CURRENT_STATE to acknowledge configurations
                        SSN_CURRENT_STATE = ACK_CONFIG_STATE;
                        break;

                    // Server wants to assign new time of day
                    case SET_TIMEOFDAY_MESSAGE_ID:
                        printf("<- SET_TIMEOFDAY MESSAGE RECEIVED: %d:%d:%d (%d/%d/%d)\n", params[0], params[1], params[2], params[3], params[4], params[5]);
                        // assign incoming clock time to SSN Global Clock (Pseudo Clock because we don't have an RTCC)
                        set_ssn_time(params);
                        // set SSN_CURRENT_STATE to normal status update state
                        SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
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
                        printf("(DEBUG): Reseting Controller Now...\n");
                        SoftReset();
                        break;

                    // Only for debugging, will be removed
                    // This message will reset our SSN
                    case DEBUG_RESET_SSN_MESSAGE_ID:
                        // stop the global timer
                        stop_Global_Clock();
                        // reset the SSN
                        printf("(DEBUG): Reseting Controller Now...\n");
                        sleep_for_microseconds(1000000);
                        SoftReset();
                        break;

                    default:
                        break;
                }
                // See if there is another message in the buffer so we can do this all over again
                Received_Message_Bytes_in_Buffer = is_Message_Received_Over_UDP(SSN_UDP_SOCKET);
            }
        }
        
        // based on SSN_CURRENT_STATE, construct the correct message
        switch(SSN_CURRENT_STATE) {

            // No mac address?
            case NO_MAC_STATE:
                ssn_message_to_send_size = construct_get_mac_message(message_to_send, &SSN_MAC_ADDRESS[4]);
                break;

            // No current sensor configurations?
            case NO_CONFIG_STATE:
                ssn_message_to_send_size = construct_get_configuration_message(message_to_send, &SSN_MAC_ADDRESS[4]);
                break;

            // No time of day?
            case NO_TIMEOFDAY_STATE:
                ssn_message_to_send_size = construct_get_timeofday_message(message_to_send, &SSN_MAC_ADDRESS[4]);
                break;

            // current sensor configurations received so acknowledge configurations?
            case ACK_CONFIG_STATE:
                ssn_message_to_send_size = construct_ack_configuration_message(message_to_send, &SSN_MAC_ADDRESS[4], SSN_CONFIG);
                break;

            // status update message? It is the same for both Normal and Abnormal Activity States            
            case NORMAL_ACTIVITY_STATE:
            case ABNORMAL_ACTIVITY_STATE:
                // sample temperature and humidity as byte values to report over the network
                temp_humidity_recv_status = sample_Temperature_Humidity_bytes(temperature_bytes, relative_humidity_bytes);
                // check if we are operating in normal ambient conditions, i.e. (temperature and humidity)
                abnormal_activity = ambient_condition_status();
                if (abnormal_activity == ABNORMAL_AMBIENT_CONDITION)
                    SSN_CURRENT_STATE = ABNORMAL_ACTIVITY_STATE;
                else
                    SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
                /* The following function call calculates all of our current sensor based analytics
                 * i.e. our load currents, machine state (on/idle/off), load percentage, time duration since in that state, etc. 
                 * this function takes a little more than 40 milliseconds to execute because it samples the current sensors for 40ms */ 
                Get_Machines_Status_Update(SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, Machine_load_currents, Machine_load_percentages, 
                        Machine_status, Machine_status_duration, Machine_status_timestamp);
                // Finally, construct the full status update message structure
                ssn_message_to_send_size = construct_status_update_message(message_to_send, &SSN_MAC_ADDRESS[4], temperature_bytes, relative_humidity_bytes, Machine_load_currents, 
                        Machine_load_percentages, Machine_status, Machine_status_duration, Machine_status_timestamp, ssn_uptime_in_seconds, abnormal_activity);
                break;

            default:
                break;
        }
        // sleep for a quarter of a second
        sleep_for_microseconds(250000);
    }
    
    // We should never get to this point!!!
    return 1;
}


