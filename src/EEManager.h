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

class EEMemManager {
public:
    bool init();
    MemPart GetMemPart(char* name);

    bool tick();
private:
    uint16_t lastAddr = 0;
    const uint16_t startAddr = 0;

    MemPart* partitionsPart;
};

class VariableInfo {
protected:
    uint32_t        nameHash;
    uint16_t        addr;
    uint16_t        dataSize;
    uint16_t        nextVarAddr = 0;
};

class Variable : VariableInfo {
public:
    Variable() {};

    template <typename T>
    Variable(uint16_t addr, T* data, const char* name, bool write = false, uint16_t timeout = 5000){
        this->data = (uint8_t*)data;
        dataSize = sizeof(T);
        setTimeout(timeout);
        nameHash = CRC32::calculate(name, strlen(name));
        init(write);
    }

    MemStatusCode init(bool write);

    void updateNow();
    void updateMetaInfo();
    void update();
    bool tick();
    void reset();
    uint32_t getHameHash() { return nameHash; };
    uint16_t getDataSize();
    uint16_t getStartAddr();
    uint16_t getEndAddr();
    uint16_t getNextAddr();
    uint16_t getNextVarAddr() { return nextVarAddr; };
    void setTimeout(uint16_t timeout = 5000);

private:
    uint16_t    dataAddr() { return addr + offset; };
    void        writeBytes(uint16_t addr, uint8_t* data, uint16_t size);

    uint8_t*        data = nullptr;
    const uint16_t  offset = sizeof(VariableInfo);
    bool            need_update = 0;      // _update from EEManager class   
    uint32_t        last_write_time = 0;  // _tmr from EEManager class
    uint16_t        upd_timeout;          // _tout from EEManager class
};

class MemPart {
public:
    template <typename T> Variable getVar(char* name, T* data);
    //template <typename T> MemStatusCode getVal(char* name, T* data);

    uint16_t getFirstVarAddr() { return firstVarAddr; } ;
    void setFirstVarAddr(uint16_t addr) { firstVarAddr = addr; };

    bool tick();

private:
    uint16_t    firstVarAddr = 0;
};

#endif
