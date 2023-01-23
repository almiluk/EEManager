/*
    EEPROM manager - The library for easier work with EEPROM 
        and reducing its wear.
    GitHub: https://github.com/almiluk/EEManager
    Aleksandr Lukianov, almiluk@gmial.com
    MIT License

    Original library:
    GitHub: https://github.com/GyverLibs/EEManager
    https://alexgyver.ru/
    AlexGyver, alex@alexgyver.ru
*/

#ifndef _EEManager_h
#define _EEManager_h

#ifdef DEBUG_EEPROM
    #define DEBUG_PRINT(value) Serial.print(value)
    #define DEBUG_PRINTLN(value) Serial.println(value)
#else
    #define DEBUG_PRINT(value)
    #define DEBUG_PRINTLN(value)
#endif

#include <Arduino.h>
#include <EEPROM.h>
#include <CRC32.h>

#include "MemPart.h"

/** EEPROM managing class.*/
class EEMemManager {
public:
    /** Initialize manager.
     *  Read information about stored data from EEPROM 
     *  or write that information if it's the first run.
    */
    static bool init();
    
    /** Find or create an EEPROM partition with given name.
     *  In case a new partition was created the information about that
     *  partition will be written to EEPROM automatically.
     * 
     *  @param name The name of the searching partition.
     *  @return Found or created EEPROM partition.
    */
    static MemPart GetMemPart(char* name);

    /** A SYSTEM METHOD.
     *  Get the address for storing new data and increase the lastAddr value by the size of new data.
     * 
     *  @param new_data_size The size of the new data.
     *  @return start The EEPROM address for storing new data.
    */
    static uint16_t getAddrForNewData(uint16_t new_data_size);

    //static bool tick();

private:
    static uint16_t lastAddr;       /**< The biggest used EEPROM address*/
    static EEPROMVar lastAddrVar;   /**< The EEPROM variable storing lastAddr value*/
    static MemPart metaMemPart;     /**< The EEPROM partition storing system data*/

    static const uint16_t startAddr = 0;    /**< The initial EEPROM address for managing data*/
};

#endif
