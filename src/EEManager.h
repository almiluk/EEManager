/*
    Менеджер EEPROM - библиотека для уменьшения износа памяти
    Документация: 
    GitHub: https://github.com/almiluk/EEManager
    Возможности:
    - Отложенная запись (обновление) по таймеру
    - Работает на базе стандартной EEPROM.h
    - Встроенный механизм ключа первой записи

    AlexGyver, alex@alexgyver.ru
    Aleksandr Lukianov, almiluk@gmial.com
    MIT License

    Версии:
    v1.0 - релиз
    v1.1 - изменены коды возврата begin
    v1.2 - добавлена nextAddr()
    v1.2.1 - поддержка esp32
    v1.2.2 - пофиксил варнинг
*/

#ifndef _EEManager_h
#define _EEManager_h
#include <Arduino.h>
#include <EEPROM.h>
#include <CRC32.h>

enum MemStatusCode {
    ok,
    created,
    failed
};

class VariableInfo {
public: 
    uint32_t getHameHash() { return nameHash; };
    uint16_t getDataSize() { return dataSize; };
    uint16_t getNextVarAddr() { return nextVarAddr; };
protected:
    uint32_t nameHash = 0;
    uint16_t dataSize = 0;
    uint16_t nextVarAddr = 0;
};


// TODO: add checks data != nullptr (before init() call or if init failed)
class Variable : public VariableInfo {
public:
    Variable(const char* name, uint16_t timeout = 5000);

    template <typename T>
    MemStatusCode init(uint16_t addr, T* data, bool write = false) {
        this->data = (uint8_t*)data;
        dataSize = sizeof(T);
        this->addr = addr;

        linkToEeprom(write);
    }

    void updateNow();
    void updateMetaInfo();
    void update();
    bool tick();
    void setTimeout(uint16_t timeout = 5000);

    bool operator==(const Variable& other);
    bool operator!=(const Variable& other);

    uint16_t getStartAddr();
    uint16_t getEndAddr();
    uint16_t getNextAddr();

private:
    uint16_t        getDataAddr() { return addr + offset; };
    void            writeBytes(uint16_t addr, uint8_t* data, uint16_t size);
    MemStatusCode   linkToEeprom(bool write);

    uint16_t    addr = 0;
    uint8_t*    data = nullptr;
    bool        needUpdate = 0;     // _update from EEManager class
    uint32_t    lastWriteTime = 0;  // _tmr from EEManager class
    uint16_t    updTimeout = 5000;  // _tout from EEManager class
    static const uint16_t  offset = sizeof(VariableInfo);
};

class MemPart;

class EEMemManager {
private:
    static uint16_t lastAddr;
    static Variable lastAddrVar;
    static const uint16_t startAddr = 0;

    static MemPart metaMemPart;

public:
    static bool init();
    static MemPart GetMemPart(char* name);
    // TODO: change to getLastAddr(uint16_t new_data_size); and move Variable creating to MemPart.getVar()
    template <typename T>
    static Variable writeNewVar(char* name, T* data) {
        Variable var(name);
        // TODO: check init result code
        var.init(lastAddr + 1, data, true);
        lastAddr = var.getEndAddr();
        // write new last address to EEPROM
        lastAddrVar.updateNow();
        return var;
    };

    static bool tick();
};


class MemPartInfo {
public:
    MemPartInfo(uint16_t addr) { firstVarAddr = addr; };
    MemPartInfo() {};

protected:
    uint16_t firstVarAddr = 0;
};

class MemPart : public MemPartInfo {
public:
    MemPart(uint16_t addr) : MemPartInfo(addr) {};
    MemPart() {};

    template <typename T> 
    Variable getVar(char* name, T* data) {
        uint32_t name_hash = CRC32::calculate(name, strlen(name));
        Variable var(name);
        uint16_t addr = firstVarAddr;
        while (addr != 0) {
            var.init(addr, data);
            if (var.getHameHash() == name_hash) {
                return var;
            }
            addr = var.getNextVarAddr();
        }
        
        var = EEMemManager::writeNewVar(name, data);
        if (firstVarAddr != 0) {
            lastVar.setNextVarAddr(var.getStartAddr());
            lastVar.updateMetaInfo();
        } else {
            firstVarAddr = var.getStartAddr();
        }
        lastVar = EditableVariable(var);
        return var;
    }

    bool tick();

private:
    class EditableVariable : public Variable {
    public:
        EditableVariable(const Variable& var) : Variable(var) {};
        void setNextVarAddr(uint16_t next_var_addr) { nextVarAddr = next_var_addr; } ;
    };

    EditableVariable lastVar = EditableVariable(Variable("fake"));
};

#endif
