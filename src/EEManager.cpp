#include "EEManager.h"
#include "MemPart.h"
#include <CRC32.h>

#define CHECK_VALUE 0xABCD

// EEMemManager
// sizeof(uint16_t) - size of the check value
uint16_t EEMemManager::lastAddr = startAddr + sizeof(uint16_t) - 1; /**< The biggest used EEPROM address*/
EEPROMVar EEMemManager::lastAddrVar; /**< EEPROM variable storing lastAddr value*/
MemPart EEMemManager::metaMemPart; /**< EEPROM partition storing system data*/

bool EEMemManager::init() {
    // FIXME
    uint16_t check_val;
    uint16_t last_addr_var_addr = lastAddr + 1; // default value of lastAddr => always the same
    if(EEPROM.get(startAddr, check_val) != CHECK_VALUE) {
        // the very first run
        DEBUG_PRINTLN("first run");
        metaMemPart = MemPart();
        // TODO: clear EEPROM ?!
        //lastAddrVar = metaMemPart.addFirstVar("last_addr", &lastAddr);
        lastAddrVar = metaMemPart.getVar("last_addr", &lastAddr);
        EEPROM.put(startAddr, (uint16_t)CHECK_VALUE);
    } else {
        // not first run
        DEBUG_PRINTLN("not first run");
        metaMemPart = MemPart(last_addr_var_addr);
        lastAddrVar = metaMemPart.getVar("last_addr", &lastAddr);
    }
    DEBUG_PRINTLN(lastAddr);
}

MemPart EEMemManager::GetMemPart(char* name) {
    MemPartInfo mem_part_info;
    bool is_new_mem_part;
    EEPROMVar mem_part_var = metaMemPart.getVar(name, &mem_part_info, &is_new_mem_part);
    MemPart mem_part;


    if (is_new_mem_part) {
        mem_part = MemPart(0);
        // add fictitious variable to determine firstVarAddr and write it to EEPROM
        uint8_t fictitious_system_value = 0;
        mem_part.getVar("fictitious_system_value", &fictitious_system_value);
        mem_part_info = mem_part;
        mem_part_var.updateNow();
    } else {
        mem_part = MemPart(mem_part_info);
    }

    return mem_part;
}

uint16_t EEMemManager::getAddrForNewData(uint16_t new_data_size) {
    // TODO: move checking of EEPROM length here
    uint16_t addr_to_write = lastAddr + 1;
    lastAddr += new_data_size;
    lastAddrVar.updateNow();
    return addr_to_write;
}
