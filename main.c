#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#pragma config FWDTEN   = OFF           // Turn off watchdog timer
#pragma config WDTPS    = PS8192        // 4 second Watchdog Timer with 80MHz clock
#pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable (Disabled)
#pragma config FNOSC    = FRCPLL        // Select 8MHz internal Fast RC (FRC) oscillator with PLL
#pragma config FPLLIDIV = DIV_2         // Divide PLL input (FRC) -> 4MHz
#pragma config FPLLMUL  = MUL_15        // PLL Multiplier -> 60MHz
#pragma config FPLLODIV = DIV_1         // PLL Output Divider -> 60MHz
#pragma config FPBDIV   = DIV_2         // Peripheral Clock divisor -> 30MHz
#pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select (Communicate on PGEC2/PGED2)
#pragma config JTAGEN   = OFF           // JTAG Enable (Disabled)

#include "src/SSN_API/SSN_API.h"

/** Our SSN UDP communication socket */
SOCKET SSN_UDP_SOCKET;
/** SSN Server Address */
uint8_t SSN_SERVER_IP[] = {192, 168, 0, 100};
/** SSN Server PORT */
uint16_t SSN_SERVER_PORT = 9999;

/** Static IP Assignment */
uint8_t SSN_STATIC_IP[4]        = {192, 168, 0, 103};
uint8_t SSN_SUBNET_MASK[4]      = {255, 255, 255, 0};
uint8_t SSN_GATWAY_ADDRESS[4]   = {192, 168, 0, 1};

/** A counter to maintain how many messages have been sent from SSN to Server since wakeup */
uint32_t SSN_SENT_MESSAGES_COUNTER = 0;
/** Boolean variable for Interrupt Enabled or not */
bool InterruptEnabled = false;
/** Counter variable for interrupts per second */
uint8_t interrupts_per_second = 2;
/** Counter variable for counting half seconds per second */
uint8_t half_second_counter = 0, delays_per_second_counter = 0; 
/** Counter variable for counting after how many intervals to send the status update */
uint8_t report_counter = 0;
/** Current State of the SSN. There is no state machine of the SSN but we still use this variable to keep track at some instances */
uint8_t SSN_CURRENT_STATE;
/** Report Interval of SSN set according to the configurations passed to the SSN */
uint8_t SSN_REPORT_INTERVAL = 1;
/** SSN current sensor configurations */
uint8_t SSN_CONFIG[EEPROM_CONFIG_SIZE];
/** SSN current sensor ratings */
uint8_t SSN_CURRENT_SENSOR_RATINGS[4];
/** SSN machine thresholds for deciding IDLE state */
uint8_t SSN_CURRENT_SENSOR_THRESHOLDS[4];
/** SSN machine maximum loads for calculating percentage loads on machines */
uint8_t SSN_CURRENT_SENSOR_MAXLOADS[4];
/** SSN machine load currents array */
float Machine_load_currents[NO_OF_MACHINES] = {0};
/** SSN machine load percentages array */
uint8_t Machine_load_percentages[NO_OF_MACHINES] = {0};
/** SSN machine status array initialized to a Sentinel or Reset state */
uint8_t Machine_status[NO_OF_MACHINES] = {MACHINE_RESET_SENTINEL_STATE, MACHINE_RESET_SENTINEL_STATE, MACHINE_RESET_SENTINEL_STATE, MACHINE_RESET_SENTINEL_STATE};
/** SSN machine timestamps for recording since when the machines have been in the current states */
uint32_t Machine_status_timestamp[NO_OF_MACHINES];
/** SSN machine status duration array for holding the number of seconds for which the machines have been in the current state */
uint32_t Machine_status_duration[NO_OF_MACHINES] = {0};
/** SSN UDP socket number */
uint8_t SSN_UDP_SOCKET_NUM = 4;
/** SSN default MAC address. This is the same for all SSNs */
uint8_t SSN_DEFAULT_MAC[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
/** SSN current MAC address. May hold the default MAC or the one received from SSN Server. The last two bytes are the SSN Identity */
uint8_t SSN_MAC_ADDRESS[6] = {0};
/** SSN temperature sensor reading bytes  */
uint8_t temperature_bytes[2];
/** SSN relative humidity reading bytes  */
uint8_t relative_humidity_bytes[2];
/** SSN temperature and humidity reading successful/unsuccessful status bit */
uint8_t temp_humidity_recv_status; 
/** SSN abnormal activity bit  */
uint8_t abnormal_activity;
/** */
uint32_t message_count = 0; 
bool all_good = true;
/** SSN loop variable  */
uint8_t i;

/** Half-Second interrupt that controls our send message routine of the SSN. Half-second and not one second is because we can not set an 
 *  interrupt of up to 1 second with the current clock of the SSN. We only start this interrupt service once we have Ethernet configured 
 *  and all self-tests are successful. The message to be sent is constructed every half a second in the main function and only reported 
 *  to the server after every "SSN_REPORT_INTERVAL" seconds. */
//void __ISR(_TIMER_1_VECTOR, IPL4SOFT) Timer1IntHandler_SSN_Hearbeat(void){
//    // clear timer 1 interrupt flag, IFS0<4>
//    IFS0CLR = 0x0010;       
//    // Indicate the status of SSN from the SSN LED after every half second
//    SSN_LED_INDICATE(SSN_CURRENT_STATE);
//    // check of we have reached one second interval (because two half-seconds make one second)
//    half_second_counter++;
//    if (half_second_counter >= interrupts_per_second) {
//        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        /* Normal routines that should execute in any case, whether we are messaging or not */
//        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        // reset half second counter
//        half_second_counter = 0;
//        // add a second to report counter
//        report_counter++;
//        // increment global uptime in seconds
//        ssn_uptime_in_seconds++;
//        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//        // Is it time to report and interrupt is enabled?
//        if (report_counter >= SSN_REPORT_INTERVAL) {
//            // Reset the reporting counter
//            report_counter = 0;
//            message_count++;
//            // open and close socket every time we must send a new message
//            SSN_UDP_SOCKET = socket(SSN_UDP_SOCKET_NUM, Sn_MR_UDP, SSN_DEFAULT_PORT, 0x00);
//            if(SSN_UDP_SOCKET==SSN_UDP_SOCKET_NUM) {
//                all_good = Send_STATUSUPDATE_Message(&SSN_MAC_ADDRESS[4], SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, temperature_bytes, relative_humidity_bytes, Machine_load_currents, 
//                        Machine_load_percentages, Machine_status, Machine_status_duration, Machine_status_timestamp, ssn_clock, abnormal_activity);
//                if(!all_good) {
//                    printf("-> Socket Error.\n");
//                }
//                close(SSN_UDP_SOCKET);
//            } else {
//                printf("-> Socket Initialization Failed.\n");
//            }
//        }
//    }
//}

/** 
 * The main loop of SSN operation. It calls the following functions in order:
 *      - Sets up all required peripherals
 *      - Runs system tests for checking:
 *          -# EEPROM Read/Write
 *          -# Temperature and Humidity Sensor
 *          -# Ethernet Physical Connection
 *      - Finds MAC address in EEPROM; if available, assigns it to SSN; if not available, assigns default MAC address to SSN
 *      - Sets up Ethernet connection using which ever MAC address was selected
 *      - If using default MAC address, SSN sends periodic GET_MAC requests to SSN Server until it successfully retrieves one and resets self
 *      - Waits for five seconds for new Current Sensor Configurations from SSN Server. These cannot be reprogrammed after this five seconds window
 *      - If new configurations received, assigns them to SSN, writes them to EEPROM and proceeds
 *      - If new configurations not received, finds Current Sensor Configurations in EEPROM; if available, assigns them to SSN; 
          if not available SSN sends periodic GET_CONFIG requests to SSN Server until it successfully retrieves one and writes them to EEPROM and proceeds
 *      - SSN sends periodic GET_TimeOfDay requests to SSN Server until it successfully retrieves it and proceeds
 *      - SSN starts global clock and half-second periodic interrupt 
 *      - SSN calculates machine status update and ambient conditions every 100 milliseconds. 
          The ISR sends the status update after every ${SSN_REPORT_INTERVAL} seconds
 */
int main() {
    // Setup Smart Sense Node
    SSN_Setup();
    RunSystemTests();
    SSN_CURRENT_STATE = FindMACInFlashMemory(SSN_MAC_ADDRESS, SSN_DEFAULT_MAC);
    // Start Ethernet Now with a MAC address (either default MAC or custom SSN MAC)
//    SSN_UDP_SOCKET = SetupConnectionWithDHCP(SSN_MAC_ADDRESS, SSN_UDP_SOCKET_NUM);
    SSN_UDP_SOCKET = SetupConnectionWithStaticIP(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
    uint16_t SendAfter = 0;
    // When we will receive a MAC address (if we didn't have it), we will reset the controller
    while (SSN_CURRENT_STATE == NO_MAC_STATE) {
        // Check Ethernet Physical Link Status before sending message
        if (Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
            printf("LOG: Ethernet Physical Link BAD. Can't Send Message...\n");
            No_Ethernet_LED_INDICATE();
        }
        // request a MAC address after every 5 seconds
        if (SendAfter % 50 == 0) {
            Send_GETMAC_Message(&SSN_MAC_ADDRESS[4], SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        }
        // Try to receive a message every 100 milliseconds
        Receive_MAC(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        // Give LED indication every second
        if (SendAfter % 10 == 0) {
            Node_Up_Not_Configured_LED_INDICATE();
        }
        SendAfter++;        
        // 100 milliseconds
        sleep_for_microseconds(100000);
    }
    // Wait for new configurations for five seconds
    printf("LOG: Waiting for updated configurations from Server...\n");
    // Notify the server you are waiting for configurations
    Send_GETCONFIG_Message(&SSN_MAC_ADDRESS[4], SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
    SendAfter = 0; 
    bool NewConfigsReceived = false;        
    while (SendAfter < 50) {
        // Try to receive a message every 100 milliseconds
        if (Receive_CONFIG(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, 
                Machine_status)) {
            // Copy from the configurations, the sensor ratings, thresholds and maximum load values into our variables
//            for (i = 0; i < NO_OF_MACHINES; i++) {
//                // Get the parameters from the Configurations
//                SSN_CURRENT_SENSOR_RATINGS[i]    = SSN_CONFIG[3*i+0];
//                SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[3*i+1];
//                SSN_CURRENT_SENSOR_MAXLOADS[i]   = SSN_CONFIG[3*i+2];
//            }
//            SSN_REPORT_INTERVAL = SSN_CONFIG[12];
            NewConfigsReceived = true;
            break;
        }
        // Give LED indication every second
        if (SendAfter % 10 == 0) {
            Node_Up_Not_Configured_LED_INDICATE();
        }
        SendAfter++;
        // 100 milliseconds
        sleep_for_microseconds(100000);
    }
    
    if (!NewConfigsReceived) {
        // Find configurations in EEPROM
        SSN_CURRENT_STATE = FindSensorConfigurationsInFlashMemory(SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS);
        SendAfter = 0;
        while (SSN_CURRENT_STATE == NO_CONFIG_STATE) {
            // Check Ethernet Physical Link Status before sending message
            if (Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
                printf("LOG: Ethernet Physical Link BAD. Can't Send Message...\n");
                No_Ethernet_LED_INDICATE();
            }
            // request a MAC address after every 5 seconds
            if (SendAfter % 50 == 0) {
                Send_GETCONFIG_Message(&SSN_MAC_ADDRESS[4], SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
            }
            // Try to receive a message every 100 milliseconds
            if (Receive_CONFIG(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, 
                    SSN_CURRENT_SENSOR_MAXLOADS, Machine_status)) {
                // Copy from the configurations, the sensor ratings, thresholds and maximum load values into our variables
//                for (i = 0; i < NO_OF_MACHINES; i++) {
//                    // Get the parameters from the Configurations
//                    SSN_CURRENT_SENSOR_RATINGS[i]    = SSN_CONFIG[3*i+0];
//                    SSN_CURRENT_SENSOR_THRESHOLDS[i] = SSN_CONFIG[3*i+1];
//                    SSN_CURRENT_SENSOR_MAXLOADS[i]   = SSN_CONFIG[3*i+2];
//                }
//                SSN_REPORT_INTERVAL = SSN_CONFIG[12];
                break;
            }
            // Give LED indication every second
            if (SendAfter % 10 == 0) {
                Node_Up_Not_Configured_LED_INDICATE();
            }
            SendAfter++;
            // 100 milliseconds
            sleep_for_microseconds(100000);
        }
    }
    // Get time of day
    SendAfter = 0;
    while (1) {
        // Check Ethernet Physical Link Status before sending message
        if (Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
            printf("LOG: Ethernet Physical Link BAD. Can't Send Message...\n");
            No_Ethernet_LED_INDICATE();
        }
        // request a MAC address after every 5 seconds
        if (SendAfter % 50 == 0) {
            Send_GETTimeOfDay_Message(&SSN_MAC_ADDRESS[4], SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        }
        // Try to receive a message every 100 milliseconds
        if (Receive_TimeOfDay(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT)) {
            // initialize SSN's uptime
            ssn_uptime_in_seconds = 0;
            break;   
        }
        // Give LED indication every second
        if (SendAfter % 10 == 0) {
            Node_Up_Not_Configured_LED_INDICATE();
        }
        SendAfter++;        
        // 100 milliseconds
        sleep_for_microseconds(100000);
    }
    // Start the global clock that will trigger a response each half of a second through our half-second interrupt defined above Main function
    // setup_Global_Clock_And_SSN_Half_Second_Heartbeat(PERIPH_CLK);
    //InterruptEnabled = true;
    while(SSN_IS_ALIVE) {
        // sample sensors and do calculations
        temp_humidity_recv_status = sample_Temperature_Humidity_bytes(temperature_bytes, relative_humidity_bytes);
        abnormal_activity = ambient_condition_status();
        if (abnormal_activity == NORMAL_AMBIENT_CONDITION) {
            SSN_CURRENT_STATE = NORMAL_ACTIVITY_STATE;
        }
        else {
            SSN_CURRENT_STATE = ABNORMAL_ACTIVITY_STATE;
        }
        // We can receive configurations and time of day on the fly
        Receive_CONFIG(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, SSN_CONFIG, &SSN_REPORT_INTERVAL, SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, 
                Machine_status);
        Receive_TimeOfDay(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT);
        //Check Ethernet Physical Link Status before sending message
        if (Ethernet_get_physical_link_status() == PHY_LINK_OFF) {
            printf("LOG: Ethernet Physical Link BAD. Can't Send Message...\n");
            No_Ethernet_LED_INDICATE();
        }
        Get_Machines_Status_Update(SSN_CURRENT_SENSOR_RATINGS, SSN_CURRENT_SENSOR_THRESHOLDS, SSN_CURRENT_SENSOR_MAXLOADS, Machine_load_currents, Machine_load_percentages, Machine_status, 
                Machine_status_duration, Machine_status_timestamp);
        // Indicate the status of SSN from the SSN LED after every half second
        SSN_LED_INDICATE(SSN_CURRENT_STATE);
        // check of we have reached one second interval (because two half-seconds make one second)
        delays_per_second_counter++;
        if (delays_per_second_counter >= 9) { // 10 because the loop delay is 100ms
            // 1 second passed?
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            /* Normal routines that should execute in any case, whether we are messaging or not */
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // reset half second counter
            delays_per_second_counter = 0;
            // add a second to report counter
            report_counter++;
            // increment global uptime in seconds
            ssn_uptime_in_seconds++;
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Is it time to report and interrupt is enabled?
            if (report_counter >= SSN_REPORT_INTERVAL) {
                // Reset the reporting counter
                report_counter = 0;
                message_count++;
                // open and close socket every time we must send a new message
                all_good = Send_STATUSUPDATE_Message(&SSN_MAC_ADDRESS[4], SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, temperature_bytes, relative_humidity_bytes, Machine_load_currents, 
                        Machine_load_percentages, Machine_status, Machine_status_duration, Machine_status_timestamp, ssn_clock, abnormal_activity);
            }
        }
        // if socket gets corrupted, we reinitialize/reset the whole thing
        if(!all_good) {
            printf("Socket Corrupted. Reinitializing..\n");
            setup_Ethernet(5000000);
//            SSN_UDP_SOCKET = SetupConnectionWithDHCP(SSN_MAC_ADDRESS, SSN_UDP_SOCKET_NUM);
            SSN_UDP_SOCKET = SetupConnectionWithStaticIP(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
            printf("Reinitialization Successful.\n");
        }
        // sleep for 100 milliseconds
        sleep_for_microseconds(100000);
    }
    // we should never reach this point
    return 1;
}

int main_current_testing() {
    SSN_Setup();
    SSN_CURRENT_SENSOR_RATINGS[0] = 100;
    SSN_CURRENT_SENSOR_RATINGS[1] = 000;
    SSN_CURRENT_SENSOR_RATINGS[2] = 030;
    SSN_CURRENT_SENSOR_RATINGS[3] = 000;
    while(true) {
        printf("In here\n");
        Calculate_RMS_Current_On_All_Channels(SSN_CURRENT_SENSOR_RATINGS, 400, Machine_load_currents);
        // sleep for a second
        sleep_for_microseconds(1000000);
    }
    // we should never get to this point
    return 1;
}

int main_network_test() {
    SSN_Setup();
    SSN_UDP_SOCKET = SetupConnectionWithStaticIP(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
    uint8_t test_message_array[100] = "I am Annus Zulfiqar and I am trying to test this network";
    uint8_t test_message_size = 56;
    while(true) {
        all_good = SendMessage(SSN_UDP_SOCKET, SSN_SERVER_IP, SSN_SERVER_PORT, test_message_array, test_message_size);
        if(!all_good) {
            printf("Socket Corrupted. Reinitializing..\n");
            setup_Ethernet(5000000);
            SSN_UDP_SOCKET = SetupConnectionWithStaticIP(SSN_UDP_SOCKET_NUM, SSN_MAC_ADDRESS, SSN_STATIC_IP, SSN_SUBNET_MASK, SSN_GATWAY_ADDRESS);
            printf("Reinitialization Successful.\n");
        }
        sleep_for_microseconds(1000000);
    }
    // we should never get to this point
    return 1;
}


