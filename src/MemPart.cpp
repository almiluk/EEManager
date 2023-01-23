#include "EEManager.h"
#include "MemPart.h"
#include <EEPROM.h>
#include <CRC32.h>

// MemPart

uint16_t MemPart::getAddrForNewVar(uint16_t data_size) {
    return EEMemManager::getAddrForNewData(data_size);
}

// Variable

MemPart::Variable::Variable(const char* name) {
    varInfo.nameHash = CRC32::calculate(name, strlen(name));
}

void MemPart::Variable::updateMetaInfo() {
    uint8_t *meta_data = (uint8_t*)(VariableInfo*)this;
    //writeBytes(addr, meta_data, sizeof(VariableInfo));
    EEPROM.put(addr, *(VariableInfo*)this);
}

void MemPart::Variable::updateNow() {
    writeBytes(getDataAddr(), data, getDataSize());
}

bool MemPart::Variable::operator==(const Variable& other) const {
    int const x = 1;
    return getDataSize() == other.getDataSize()
            && !memcmp(data, other.data, getDataSize())
            && getHameHash() == other.getHameHash()
            && getNextVarAddr() == other.getNextVarAddr()
            && addr == other.addr;
}

bool MemPart::Variable::operator!=(const Variable& other) const {
    return !(*this == other);
}

uint32_t MemPart::Variable::getHameHash() const {
    return varInfo.nameHash;
}

uint16_t MemPart::Variable::getDataSize() const {
    return varInfo.dataSize;
}

uint16_t MemPart::Variable::getNextVarAddr() const {
    return varInfo.nextVarAddr;
}

void MemPart::Variable::setNextVarAddr(uint16_t next_var_addr) {
    varInfo.nextVarAddr = next_var_addr;
}

uint16_t MemPart::Variable::getStartAddr() const {
    return addr;
}

uint16_t MemPart::EEPROMVar::getTimeout() const {
    return updTimeout;
}

uint16_t MemPart::Variable::getEndAddr() const {
    return getDataAddr() + getDataSize() - 1;
}

uint16_t MemPart::Variable::getNextAddr() const {
    return getDataAddr() + getDataSize();
}


void MemPart::Variable::writeBytes(uint16_t addr, uint8_t* data, uint16_t size) {
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

MemStatusCode MemPart::Variable::linkToEeprom(bool write) {
    if (getDataAddr() + getDataSize() > (uint16_t)EEPROM.length()) {
        return MemStatusCode::failed;  // not enough space in EEPROM
    }

    if (write) {
        updateMetaInfo();
        updateNow();
        return MemStatusCode::created;
    }

    VariableInfo var_info;
    EEPROM.get(addr, var_info);
    varInfo.nameHash = var_info.nameHash;
    varInfo.nextVarAddr = var_info.nextVarAddr;
    // TODO: check if it == sizeof(T)
    if (var_info.dataSize != varInfo.dataSize) {
        return MemStatusCode::failed;
    }
    varInfo.dataSize = var_info.dataSize;
    for (uint16_t i = 0; i < getDataSize(); i++) {
        this->data[i] = EEPROM.read(getDataAddr() + i);
    }
    return MemStatusCode::ok;
}

void MemPart::EEPROMVar::updateNow() {
    Variable::updateNow();
}

void MemPart::EEPROMVar::update() {
    lastWriteTime = millis();
    needUpdate = true;
}

bool MemPart::EEPROMVar::tick() {
    if (needUpdate && millis() - lastWriteTime >= updTimeout) {
        updateNow();
        needUpdate = false;
        return true;
    } 
    return false;
}

void MemPart::EEPROMVar::setTimeout(uint16_t timeout) {
    updTimeout = timeout;
};
