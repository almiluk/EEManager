#define CHECK_VALUE 0xABCD

#include "EEManager.h"
#include <CRC32.h>


// EEMemManager
// sizeof(uint16_t) - size of check value
uint16_t EEMemManager::lastAddr = startAddr + sizeof(uint16_t) - 1;
Variable EEMemManager::lastAddrVar = Variable("last_addr");

//
bool EEMemManager::init() {
    // FIXME
    uint16_t check_val;
    uint16_t last_addr_var_addr = lastAddr; // default value of lasrAddr => always the same 
    // first run
    if(EEPROM.get(startAddr, check_val) != CHECK_VALUE) {
        lastAddrVar.init(last_addr_var_addr, &lastAddr, true);
        EEPROM.put(startAddr, (uint16_t)CHECK_VALUE);
    } else {
        lastAddrVar.init(last_addr_var_addr, &lastAddr);
    }

    metaMemPart = MemPart(last_addr_var_addr);
}

//
MemPart EEMemManager::GetMemPart(char* name) {
    MemPart mem_part;
    Variable mem_part_var = metaMemPart.getVar(name, &mem_part);

    // add fictitious variable to determine firstVarAddr and write it to EEPROM
    uint8_t fictitious_system_value;
    mem_part.getVar("fictitious_system_value", &fictitious_system_value);
    mem_part_var.updateNow();
    return mem_part;
}

// MemPart

// Variable
//
Variable::Variable(const char* name, uint16_t timeout) {
    setTimeout(timeout);
    nameHash = CRC32::calculate(name, strlen(name));
}

// set update timeout
void Variable::setTimeout(uint16_t timeout) {
    updTimeout = timeout;
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
    lastWriteTime = millis();
    needUpdate = true;
}

bool Variable::tick() {
    if (needUpdate && millis() - lastWriteTime >= updTimeout) {
        updateNow();
        needUpdate = false;
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

MemStatusCode Variable::linkToEeprom(bool write) {
    if (getDataAddr() + dataSize > (uint16_t)EEPROM.length()) {
        return MemStatusCode::failed;  // not enough space in EEPROM
    }

    if (write) {
        updateMetaInfo();
        updateNow();
        return MemStatusCode::created;
    }

    VariableInfo var;
    EEPROM.get(addr, var);
    nameHash = var.getHameHash();
    nextVarAddr = var.getNextVarAddr();
    // TODO: check if it == sizeof(T)
    if (var.getDataSize() != dataSize) {
        return MemStatusCode::failed;
    }
    dataSize = var.getDataSize();
    for (uint16_t i = 0; i < dataSize; i++) {
        this->data[i] = EEPROM.read(getDataAddr() + i);
    }
    return MemStatusCode::ok;
}
