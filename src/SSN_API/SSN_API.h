
#ifndef __SSN_API_h__    /* Guard against multiple inclusion */
#define __SSN_API_h__

#include <plib.h>
#include "SystemTests/SystemTests.h"
#include "FlashMemory/FlashMemory.h"
#include "Connection/Connection.h"
#include "Communication/Communication.h"

/** 
 * \mainpage 
 * <A NAME="Contents"></A>
 * @section contents_sec Table of Contents
 * <A HREF="#Introduction">Introduction</A><br>
 * <A HREF="#API">API</A><br>
 * <A HREF="#CodingStandard">Coding Standard</A><br>
 * <A HREF="#VersionLog">Version Log</A><br>
 * <A HREF="#Acronyms">Acronyms</A><br>
 * <A HREF="#Documentation">Documentation</A><br>
 * <A HREF="#Requirements">Requirements</A><br>
 * <A HREF="#Tools">Tools</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Introduction"></A>
 * @section intro_sec Introduction
 * This document summarizes the core functions, variables and APIs used for the development of SSN firmware.
 *
 * \image html SSNv1.2.jpeg "Smart Sense Node v1.2"
 *
 * Based on the design problem for an IoT-based Smart Sense Node Network, our design is built around the following hardware/components.
 *  - PIC32MX170F256B Microcontroller with the following on-chip resources
 *      -# 256KB Program Memory (Flash) and 64KB Data Memory (SRAM)
 *      -# Multiple ADC channels with upto a Million Samples Per Second and 10-bit resolution
 *      -# Support for I2C and SPI peripherals
 *  - 24LC08 1KB EEPROM with I2C interface
 *  - W5500 Ethernet Offload Chip with SPI interface and integrated MAC and PHY
 *  - AM2320 Temperature and humidity sensor with I2C interface
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="API"></A>
 * @section api_sec API
 * The SSN API is a High-level API that deals with devices such as Flash Memory and Ethernet at an abstract level hiding the peripheral level 
 * details of individual devices such as the protocols being used to communicate and single byte transactions between the MCU and peripherals. 
 * This API is itself dependant on a Driver API for each peripheral/device in use.
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Coding Standard"></A>
 * @section CodingStandard_sec CodingStandard
 * Most of the API functions are designed on the basis of dependancy injection. For example, consider the following function definition
 * @code
 * void Calculate_RMS_Current_On_All_Channels(uint8_t* SENSOR_RATINGS, uint16_t num_samples, unsigned char* single_byte_RMS_CURRENTS) {
 *	    uint32_t count = 0, ADC_raw_samples[NO_OF_MACHINES] = {0}, max_ADC_raw_sample[NO_OF_MACHINES] = {0}, ADC_raw_non_zero_sum[NO_OF_MACHINES] = {0}, ADC_raw_non_zero_count[NO_OF_MACHINES] = {0};
 *	    uint32_t MAX_SAMPLE_BASED_CURRENT_RMS_value[NO_OF_MACHINES] = {0}, AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[NO_OF_MACHINES] = {0}, CURRENT_RMS_VALUE[NO_OF_MACHINES] = {0};
 *	    float SENSOR_TYPE_SCALAR;
 *	    uint8_t i;
 *	    
 *	    while(count < num_samples) {
 *	        
 *	        for (i = 0; i < NO_OF_MACHINES; i++) {
 *	            // Sample one value from ith channel
 *	            ADC_raw_samples[i] = sample_Current_Sensor_channel(i);
 *	            // record the maximum value in this sample space for MAX Sample based RMS calculation for ith channel
 *	            if (ADC_raw_samples[i] > max_ADC_raw_sample[i])
 *	                max_ADC_raw_sample[i] = ADC_raw_samples[i];
 *	            // record every non-zero value in this sample space for AVERAGE Sample based RMS calculation for ith channel
 *	            if (ADC_raw_samples[i] > 0) {
 *	                ADC_raw_non_zero_sum[i] += ADC_raw_samples[i];
 *	                ADC_raw_non_zero_count[i]++;
 *	            }
 *	        }
 *	        count++;
 *	        // pick 200 samples per wave cycle of AC Sine Wave @ 50Hz => 100us sampling period
 *	        sleep_for_microseconds(100); 
 *	    }
 *
 *	    // Calculate the RMS Current Values using two methods and average them
 *	    for (i = 0; i < NO_OF_MACHINES; i++) {
 *	        SENSOR_TYPE_SCALAR = VOLTAGE_OUTPUT_CURRENT_SENSOR_SCALAR;
 *	        if (SENSOR_RATINGS[i] == 100)
 *	            SENSOR_TYPE_SCALAR = CURRENT_OUTPUT_CURRENT_SENSOR_SCALAR;
 *	        MAX_SAMPLE_BASED_CURRENT_RMS_value[i] = (SENSOR_RATINGS[i] / 724.07) * SENSOR_TYPE_SCALAR * (0.707 * (float)max_ADC_raw_sample[i]);
 *	        AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[i] = (SENSOR_RATINGS[i] / 718.89) * SENSOR_TYPE_SCALAR * (1.1 * (float)ADC_raw_non_zero_sum[i]/ADC_raw_non_zero_count[i]);
 *	        CURRENT_RMS_VALUE[i] = (float)(MAX_SAMPLE_BASED_CURRENT_RMS_value[i] + AVERAGE_SAMPLE_BASED_CURRENT_RMS_value[i]) / 2;
 *	        single_byte_RMS_CURRENTS[i] = (unsigned char)CURRENT_RMS_VALUE[i];
 *	    }
 *	}
 * @endcode
 *
 * We use this function to calculate RMS value of currents for all current transformers by sampling all ADC channels but this function expects to 
 * receive the current ratings of these sensors from where ever this routine is invoked. It also requires the number of samples to take before making 
 * this calculation. Therefore the dependancies for this function are passed as parameters of this function call. Most of the functions in the SSN API
 * are written in a similar way.
 *
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Version Log"></A>
 * @section VersionLog_sec VersionLog
 * 
 * The current state of SSN firmware is at <b>Version 1.0</b>
 *
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Acronyms"></A>
 * @section Acronyms_sec Acronyms
 * 
 * <b>SSN -> Smart Sense Node</b> 
 *
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Documentation"></A>
 * @section Documentation_sec Documentation
 * 
 * The documentation is mostly presented in the Files tab. For viewing individual code files with documentation for functions and variables, 
 * go to <b>Files->File List</b> and click on the file symbols next to file names. Clicking on the names directly will show the source code 
 * inside those files.
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Requirements"></A>
 * @section Requirements_sec Requirements
 * 
 * This firmware code relies on two dependencies
 * 	- Legacy Peripheral library for PIC32MX series MCUs available <a href="https://www.microchip.com/SWLibraryWeb/product.aspx?product=PIC32%20Peripheral%20Library">here</a> 
 * 	- Wiznet W5500 ioLibrary driver available <a href="http://wizwiki.net/wiki/doku.php/products:w5500:driver">here</a> 
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 *
 * <HR> 
 *
 *  <A NAME="Tools"></A>
 * @section Tools_sec Tools
 * 
 * The entire firmware for this code has been written in <b>C</b> using <b>MPLABX IDE</b> and <b>XC32 compiler v1.40</b> 
 * available <a href="https://www.microchip.com/development-tools/pic-and-dspic-downloads-archive">here</a>. The Peripheral library must be installed 
 * inside the XC32 compiler folder for correct inclusion in the source code. 
 * 
 * <br><A HREF="#Contents">Table of Contents</A><br>
 */

/** 
 *  Includes are needed peripherals and APIs for SSN functionality
 */
static inline void SSN_Setup() {
    // Setup calls for all our peripherals/devices
    setup_printf(19200);
    setup_EEPROM();
    setup_Ethernet(5000000);
    setup_Current_Sensors();
    setup_Temperature_Humidity_Sensor();
    setup_LED_Indicator();
    setup_Interrupts();
}


#endif /* _EXAMPLE_FILE_NAME_H */

