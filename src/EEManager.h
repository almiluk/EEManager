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

class EEMemManager;
class MemPart;
class Variable;


class VariableInfo {
public: 
    uint32_t getHameHash() { return nameHash; };
    uint16_t getDataSize() { return dataSize; };
    uint16_t getNextVarAddr() { return nextVarAddr; };
protected:
    uint32_t        nameHash = 0;
    uint16_t        dataSize = 0;
    uint16_t        nextVarAddr = 0;
};


// TODO: add checks data != nullptr (before init() call or if init failed)
class Variable : public VariableInfo {
public:
    Variable() {};

    Variable(const char* name, uint16_t timeout = 5000){
        setTimeout(timeout);
        nameHash = CRC32::calculate(name, strlen(name));
    }

    template <typename T>
    MemStatusCode init(uint16_t addr, T* data, bool write = false) {
        this->data = (uint8_t*)data;
        dataSize = sizeof(T);
        this->addr = addr;

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
        if (var.getDataSize() != sizeof(T)) {
            return MemStatusCode::failed;
        }
        dataSize = var.getDataSize();
        for (uint16_t i = 0; i < dataSize; i++) {
            this->data[i] = EEPROM.read(getDataAddr() + i);
        }
        return MemStatusCode::ok;
    }

    void updateNow();
    void updateMetaInfo();
    void update();
    bool tick();
    void reset();
    uint16_t getStartAddr();
    uint16_t getEndAddr();
    uint16_t getNextAddr();
    void setTimeout(uint16_t timeout = 5000);
    uint16_t getDataAddr() { return addr + offset; };

    bool operator==(const Variable& other);
    bool operator!=(const Variable& other);

private:
    void        writeBytes(uint16_t addr, uint8_t* data, uint16_t size);

    uint16_t        addr = 0;
    uint8_t*        data = nullptr;
    static const uint16_t  offset = sizeof(VariableInfo);
    bool            need_update = 0;        // _update from EEManager class   
    uint32_t        last_write_time = 0;    // _tmr from EEManager class
    uint16_t        upd_timeout = 5000;     // _tout from EEManager class
};

class EEMemManager {
private:
    static uint16_t lastAddr;
    static const uint16_t startAddr = 0;

    static MemPart* partitionsPart;

public:
    static bool init();
    static MemPart GetMemPart(char* name);
    template <typename T>
    static Variable writeNewVar(char* name, T* data) {
        Variable var(name);
        // TODO: check init result code
        var.init(lastAddr + 1, data, true);
        lastAddr = var.getEndAddr();
        return var;
    };

    static bool tick();
};


class MemPartInfo {
protected:
    uint16_t firstVarAddr = 0;
};

class MemPart : public MemPartInfo {
public:
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
    //template <typename T> MemStatusCode getVal(char* name, T* data);

    bool tick();

private:
    class EditableVariable : public Variable {
    public:
        EditableVariable(const Variable& var) : Variable(var) {};
        EditableVariable() {};
        void setNextVarAddr(uint16_t next_var_addr) { nextVarAddr = next_var_addr; } ;
    };

    EditableVariable lastVar;
};

#endif
