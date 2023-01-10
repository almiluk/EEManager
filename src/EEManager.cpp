#include "EEManager.h"

#include <CRC32.h>


// EEMemManager
//
uint16_t EEMemManager::lastAddr;

//
bool EEMemManager::init() {
    lastAddr = 0;

    // TODO: save just last address (with check val), but not MemPart

    partitionsPart = new MemPart();
    //Variable testPartVar(startAddr, &partitionsPart, "\0");

    /*if (partitionsPart.getNameHash() != CRC32::calculate("\0", 1)) {
        // very first run
        partitionsPart.setName("\0");
        partitionsPart.setFirstVarAddr(0);
        testPartVar.updateNow();

        lastAddr = sizeof(MemPart) - 1;
    }*/
}

//
MemPart EEMemManager::GetMemPart(char* name) {
    uint32_t nameHash = CRC32::calculate(name, strlen(name));
    
    MemPart mem_part = *partitionsPart;
    /*while (mem_part.getNextMemPartAddr() != 0) {
        if (mem_part.getNameHash() == nameHash) {
            return mem_part;
        } else {
            Variable testPartVar(mem_part.getNextMemPartAddr(), &mem_part, "\0");
        }
    }*/

    // TODO: create new
}

// MemPart
//
/*template <typename T>
Variable MemPart::getVar(char* name, T* data) {
    uint32_t name_hash = CRC32::calculate(name, strlen(name));
    Variable var(firstVarAddr, data, name);
    if (firstVarAddr != 0) {
        do {
            if (var.nameHash == name_hash) {
                Serial.print("found: ");
                Serial.println(var.getStartAddr());
                return var;
            }
            var = Variable(var.getNextVarAddr(), data, name);
        } while (var.getNextVarAddr() != 0);   
    }
    
    var = EEMemManager::writeNewVar(name, data);
    if (firstVarAddr != 0) {
        lastVar.setNextVarAddr(var.getStartAddr());
        lastVar.updateMetaInfo();
    } else {
        firstVarAddr = var.getStartAddr();
    }
    lastVar = var;
    return var;
}*/

// Variable
//
/*MemStatusCode Variable::init(bool write){
    if (getDataAddr() + dataSize > (uint16_t)EEPROM.length()) return MemStatusCode::failed;  // не хватит места

    if (write) {
        updateMetaInfo();
        updateNow();
        return MemStatusCode::created;
    }

    VariableInfo var;
    EEPROM.get(addr, var);
    for (uint16_t i = 0; i < dataSize; i++) data[i] = EEPROM.read(dataAddr() + i);
    return MemStatusCode::ok;
}*/

// set update timeout
void Variable::setTimeout(uint16_t timeout) {
    upd_timeout = timeout;
};

// write data from RAM to EEPROM
void Variable::updateNow() {
    writeBytes(getDataAddr(), data, dataSize);
}

//
void Variable::updateMetaInfo(){
    uint8_t *meta_data = (uint8_t*)(VariableInfo*)this;
    writeBytes(addr, meta_data, sizeof(VariableInfo));
}

// schedule an update
void Variable::update() {
    last_write_time = millis();
    need_update = true;
}

bool Variable::tick() {
    if (need_update && millis() - last_write_time >= upd_timeout) {
        updateNow();
        need_update = false;
        return true;
    } 
    return false;
}

bool Variable::operator==(const Variable& other) {
    return dataSize == other.dataSize
            && !memcmp(data, other.data, dataSize)
            && nameHash == other.nameHash
            && nextVarAddr == other.nextVarAddr
            && addr == other.addr;
}

bool Variable::operator!=(const Variable& other) {
    return !(*this == other);
}

// get start EEPROM address of stored data
uint16_t Variable::getStartAddr() {
    return addr;
}

// get last EEPROM address of stored data
uint16_t Variable::getEndAddr() {
    return getDataAddr() + dataSize - 1;
}

// get the next EEPROM address after this variable data
uint16_t Variable::getNextAddr() {
    return getDataAddr() + dataSize;
}

void Variable::writeBytes(uint16_t addr, uint8_t* data, uint16_t size) {
    #if defined(ESP8266) || defined(ESP32)
        for(uint16_t i = 0; i < size; i++) {
            EEPROM.write(addr + i, data[i]);
        }
        EEPROM.commit();
    #else
        for(uint16_t i = 0; i < size; i++) {
            EEPROM.update(addr + i, data[i]);
        }
    #endif
}
