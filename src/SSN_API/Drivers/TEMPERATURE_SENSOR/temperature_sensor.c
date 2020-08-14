
#include "temperature_sensor.h"

void open_I2C2() {
    // setup the peripheral at 100KHz
    I2C2CON = 0x00;             // turn off the I2C2 module
    I2C2CONbits.DISSLW = 1;     // Disable slew rate for 100kHz
    I2C2BRG = 0x091;            // 100KHz operation
    I2C2CONbits.ON = 1;       // turn on the I2C2 module
    // setup the required pins for this peripheral
    PORTSetPinsDigitalOut(IOPORT_A, BIT_2);
    PORTSetPinsDigitalOut(IOPORT_B, BIT_2);
    PORTSetPinsDigitalOut(IOPORT_B, BIT_3);
    PORTSetBits(IOPORT_B, BIT_2);
    PORTSetBits(IOPORT_B, BIT_3);
}

void setup_Temperature_Humidity_Sensor() {
    open_I2C2();
}

void I2C2_wait_while_busy() {
    while(I2C2CON & 0x1F || I2C2STATbits.TRSTAT); // idle
}

void I2C2_transmit_start_bit() {
    I2C2CONbits.SEN = 1;
    while (I2C2CONbits.SEN == 1);    
}

void I2C2_transmit_stop_bit() {
	I2C2CONbits.PEN = 1;
    while (I2C2CONbits.PEN == 1);  
}

void I2C2_transmit_restart_bit() {
    I2C2CONbits.RSEN = 1;
    while (I2C2CONbits.RSEN == 1);    
}

void I2C2_transmit_byte(uint8_t byte) {
    I2C2TRN = byte;
}

uint8_t I2C2_receive_byte() {
    I2C2CONbits.RCEN = 1;                           // Receive enable
    while (I2C2CONbits.RCEN || !I2C2STATbits.RBF);  // Wait until RCEN is cleared (automatic)  
    return I2C2RCV;
}

void I2C2_ack(void) {
    I2C2_wait_while_busy();
    I2C2CONbits.ACKDT = 0; // Set hardware to send ACK bit
    I2C2CONbits.ACKEN = 1; // Send ACK bit, will be automatically cleared by hardware when sent  
    while(I2C2CONbits.ACKEN); // Wait until ACKEN bit is cleared, meaning ACK bit has been sent
}

void AM2320_I2C2_Read_Temp_and_Humidity(){
    // startup sequence, wake up the AM2320
    // initiate start condition
    I2C2_transmit_start_bit();
    I2C2_wait_while_busy();
    // Address the temperature sensor
    I2C2_transmit_byte((AM2320_I2C_Address | 0));
    I2C2_wait_while_busy();
    // stop bit
    I2C2_transmit_stop_bit();
    I2C2_wait_while_busy();
    sleep_for_microseconds(10000);
    
    // read register sequence
    // initiate start condition
    I2C2_transmit_start_bit();
    I2C2_wait_while_busy();
    // Address the temperature sensor
    I2C2_transmit_byte((AM2320_I2C_Address | 0));
    I2C2_wait_while_busy();

    // functional code for reading registers
    I2C2_transmit_byte(AM2320_Read_Function_Code);
    I2C2_wait_while_busy();

    // starting address to read from
    I2C2_transmit_byte(AM2320_Starting_Address);
    I2C2_wait_while_busy();

    // number of bytes to be read
    I2C2_transmit_byte(AM2320_Num_Bytes_Requested);
    I2C2_wait_while_busy();

    // stop bit
    I2C2_transmit_stop_bit();
    I2C2_wait_while_busy();
    sleep_for_microseconds(2000);
    
    // receive the output of the sensor in its specific format
    // initiate start condition
    I2C2_transmit_start_bit();
    I2C2_wait_while_busy();
    // Address the temperature sensor
    I2C2_transmit_byte((AM2320_I2C_Address | 1));
    I2C2_wait_while_busy();

    // returns function code 
    recv_data[0] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();
    I2C2_ack();
    I2C2_wait_while_busy();
    sleep_for_microseconds(3);
    if (recv_data[0] == AM2320_Read_Function_Code)
#ifdef _TEMPSENSOR_DEBUG_
        printf(">> Control byte received\n");
#endif
    
    // returns number of bytes requested
    recv_data[1] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();    
    I2C2_ack();
    I2C2_wait_while_busy();    
    sleep_for_microseconds(3);
    if (recv_data[1] == AM2320_Num_Bytes_Requested)
#ifdef _TEMPSENSOR_DEBUG_
    printf(">> Number byte received\n");
#endif
    
    // returns humidity high byte
    recv_data[2] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();
    I2C2_ack();
    I2C2_wait_while_busy();    
    sleep_for_microseconds(3);
    
    // returns humidity low byte
    recv_data[3] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();
    I2C2_ack();
    I2C2_wait_while_busy();    
    sleep_for_microseconds(3);
    
    // returns temperature high byte
    recv_data[4] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();
    I2C2_ack();
    I2C2_wait_while_busy();    
    sleep_for_microseconds(3);

    // returns temperature low byte
    recv_data[5] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();
    I2C2_ack();
    I2C2_wait_while_busy();    
    sleep_for_microseconds(3);
    
    // returns CRC check code high byte
    recv_data[6] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();    
    I2C2_ack();
    I2C2_wait_while_busy();    
    sleep_for_microseconds(3);

    // returns CRC check code low byte
    recv_data[7] = 0xFF & I2C2_receive_byte();
    I2C2_wait_while_busy();    

    // transmit final stop bit
    I2C2_transmit_stop_bit();
    I2C2_wait_while_busy();
}

uint16_t convert_bytes_to_word(int8_t high_byte, int8_t low_byte) {
    return ((high_byte & 0xFF) << 8) | (low_byte & 0xFF);
}

unsigned short crc16(unsigned char *ptr, unsigned char len) {
    unsigned short crc =0xFFFF;
    unsigned char i;
    while(len--) {
        crc ^= *ptr++;
        for (i = 0; i < 8; i++) {
            if(crc & 0x01) {
                crc >>= 1;
                crc ^= 0xA001;
            } 
            else crc>>=1;
        }
    }
    return crc;
}

uint8_t CRC_check() {
    unsigned short CRC_calculated = crc16(recv_data, 6);
    unsigned short CRC_received   = ((recv_data[7] & 0xFF) << 8) | (recv_data[6] & 0xFF);
    if (CRC_calculated == CRC_received)
        return 1;
    return 0;
}

uint8_t sample_Temperature_Humidity(uint16_t *temperature, uint16_t* relative_humidity) {
    uint16_t temp_t, humid_t; // temporary variables
    AM2320_I2C2_Read_Temp_and_Humidity();
    temp_t = convert_bytes_to_word(recv_data[4], recv_data[5]);
    humid_t = convert_bytes_to_word(recv_data[2], recv_data[3]);
    if (CRC_check()) {
        // CRC check confirms correct readings
        *temperature = temp_t;
        *relative_humidity = humid_t;
        return 1;
    }
    return 0;
}

uint8_t sample_Temperature_Humidity_bytes(uint8_t* temperature_bytes, uint8_t* relative_humidity_bytes) {
    AM2320_I2C2_Read_Temp_and_Humidity();
    if (CRC_check()) {
        temperature_bytes[0] = recv_data[4];
        temperature_bytes[1] = recv_data[5];
        relative_humidity_bytes[0] = recv_data[2];
        relative_humidity_bytes[1] = recv_data[3];
        return 1;
    }
    return 0;
}

uint8_t ambient_condition_status() {
    float temperature = (float)((recv_data[4] << 8) | recv_data[5]) / 10.0f;
    float relative_humidity = (float)((recv_data[2] << 8) | recv_data[3]) / 10.0f;
    // printf("Temperature: %.2f, Humidity: %.2f", temperature, relative_humidity);
    // Normal condition, return 0
    if (temperature < MAX_NORMAL_TEMPERATURE && temperature > MIN_NORMAL_TEMPERATURE)
        if (relative_humidity < MAX_NORMAL_RELATIVE_HUMIDITY && relative_humidity > MIN_NORMAL_RELATIVE_HUMIDITY)
            return NORMAL_AMBIENT_CONDITION;
    // This shouldn't be returned, 1
    return ABNORMAL_AMBIENT_CONDITION;
}

// CRC check confirms correct readings
//        printf("Temp inside (before writing): %x %x\n", temperature_bytes[0], temperature_bytes[1]);
//        temperature_bytes[0] = recv_data[4];
//        temperature_bytes[1] = recv_data[5];
//        relative_humidity_bytes[0] = recv_data[2];
//        relative_humidity_bytes[1] = recv_data[3];
//        printf("Temp inside: %x %x\n", temperature_bytes[0], temperature_bytes[1]);
//        printf("Humid inside: %x %x\n", relative_humidity_bytes[0], relative_humidity_bytes[1]);
//        printf("---- %x %x %x %x\n", recv_data[4], recv_data[5], recv_data[2], recv_data[3]);
//        printf("Addresses inside: %x %x %x %x\n", &temperature_bytes[0], &temperature_bytes[1], &relative_humidity_bytes[0], &relative_humidity_bytes[1]);
