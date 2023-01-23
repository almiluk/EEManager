#ifndef MEM_PART_H
#define MEM_PART_H

#include <Arduino.h>
#include <CRC32.h>
#include "EEManager.h"

#define DEFAULT_TIMEOUT 5000

/**Result codes for operations with EEPROM*/
enum MemStatusCode {
    ok,
    created,
    failed
};

/**The class with the information about an EEPROM partition for storing in EEPROM.*/
class MemPartInfo {
public:
    MemPartInfo(uint16_t addr) : firstVarAddr(addr) {};
    MemPartInfo() {};

protected:
    uint16_t firstVarAddr = 0;
};

/**EEPROM partition. A named part of EEPROM consisting variables*/
class MemPart : public MemPartInfo {
private:
    // TODO: add checks data != nullptr (before init() call or if init failed)
    /**EEPROM variable. Consists of user's data and system metadata.*/
    class Variable {
    private:
        /**Metainformation about an EEPROM variable for storing in EEPROM.*/
        struct VariableInfo {
            uint32_t nameHash = 0;      /**< The hash (basically CRC32) of variable name.*/
            uint16_t dataSize = 0;      /**< The size of stored user's data.*/
            /** The EEPROM address of the next EEPROM variable in the same partition 
             *  or 0 if it's the last variable in its partition.
            */
            uint16_t nextVarAddr = 0;   
        } varInfo;

    public:
        Variable(const char* name);

        /** Initialize variable.
         *  Link the variable with some local RAM data and read(write) that data 
         *  and variable metainformation from(to) EEPROM.
         * 
         * @param addr The start EEPROM address for a variable data.
         * @param data The pointer to the user data to be stored. The variable will 
         *          always stay linked to that pointer (until another init() call).
         * @param write The bool value indicating if a variable data must be written to EEPROM, but not read.
         * @return A result code of initialization.
        */
        template <typename T>
        MemStatusCode init(uint16_t addr, T* data, bool write = false) {
            this->data = (uint8_t*)data;
            varInfo.dataSize = sizeof(T);
            this->addr = addr;        
            varInfo.nextVarAddr = 0;
            
            return linkToEeprom(write);
        }

        /** Update metadata of this variable in EEPROM immediately.*/
        void updateMetaInfo();

        /** Update user data of this variable in EEPROM immediately.*/
        void updateNow();

        bool operator==(const Variable& other) const;
        bool operator!=(const Variable& other) const;

        uint32_t getHameHash() const;
        uint16_t getDataSize() const;
        uint16_t getNextVarAddr() const;
        void setNextVarAddr(uint16_t next_var_addr);
        uint16_t getStartAddr() const;
        uint16_t getEndAddr() const;

        /** Get next EEPROM address after this variable data.*/
        uint16_t getNextAddr() const;
        
        /** The size of the metadata of the variable*/
        const static uint16_t metaInfoSize = sizeof(VariableInfo);

    private:
        /** Get the start EEPROM address of the user data of this variable*/
        uint16_t        getDataAddr() const { return addr + metaInfoSize; };

        /** Write a byte array to EEPROM.
         *  (Only edited data will be really written)
         * 
         *  @param addr The start EEPROM address for writing.
         *  @param data The pointer to the data to be written.
         *  @param size The number of bytes to be written.
        */
        void            writeBytes(uint16_t addr, uint8_t* data, uint16_t size);
        
        /** 
         *  Link the variable with data and read(write) and metainformation from(to) EEPROM.
         * 
         *  @return A status code of linking.
        */
        MemStatusCode   linkToEeprom(bool write);
        
        uint16_t    addr = 0;       /**< The start EEPROM address of all variable data.*/
        uint8_t*    data = nullptr; /**< The pointer to the user data linked to this variable.*/
    };

    /** Just call EEMemManager::getAddrForNewData(). 
     *  Moved to separated method to avoid cross usage in definitions.
    */
    uint16_t getAddrForNewVar(uint16_t data_size);

public:

    // TODO: public -> private
    /** The class for user interaction with an EEPROM variable.*/
#ifdef DEBUG_EEPROM
    class EEPROMVar : public Variable {
#else
    class EEPROMVar : private Variable {
#endif
    public:
        EEPROMVar(Variable const& var) : Variable(var) {};
        EEPROMVar() : Variable("\0") {};

        /** Update user data of this variable in EEPROM immediately.*/
        void updateNow();

        /** Schedule an update of user data of this variable in EEPROM.
         *  Data will be updated on the next tick() call in updTimeout 
         *  milliseconds after the last update() call.
        */
        void update();

        /** Call it regularly (in loop()).
         *  Update user data of this variable in EEPROM if update() was called 
         *  and the last call was early than in updTimeout milliseconds.
         * 
         *  @return true if data in EEPROM was update otherwise false.
        */
        bool tick();

        /** Get update timeout in miliseconds.*/
        uint16_t getTimeout() const;
        /** Set update timeout in miliseconds.*/
        void setTimeout(uint16_t timeout);

    private:
        bool        needUpdate = 0;     // _update from EEManager class
        uint32_t    lastWriteTime = 0;  // _tmr from EEManager class
        uint16_t    updTimeout = DEFAULT_TIMEOUT;  // _tout from EEManager class
    };

    MemPart(uint16_t addr) : MemPartInfo(addr) {};
    MemPart(const MemPartInfo& mem_part_info) : MemPartInfo(mem_part_info) {};
    MemPart() {};

    /** Find or create an EEPROM variable with given name.
     *  In case a new variable was created the information about that
     *  variable will be written to EEPROM automatically. 
     * 
     *  @param name The name of the searching variable.
     *  @param data The pointer to the object to link the searching variable to.  
     *  @param created_new_var Optional. The output parameter. It will bee set to true if
     *  new variable was created and to false otherwise.
     *  @return The found or created EEPROM variable.
    */
    template <typename T>
    EEPROMVar getVar(char* name, T* data, bool* created_new_var = nullptr) {
        if (created_new_var) *created_new_var = false;
        uint32_t name_hash = CRC32::calculate(name, strlen(name));
        Variable var(name);
        uint16_t addr = firstVarAddr;
        // TODO: separate reading of metadata and data
        T data_copy = *data;
        DEBUG_PRINT("For ");
        DEBUG_PRINTLN(name);
        //DEBUG_PRINTLN(name_hash);
        while (addr != 0) {
            DEBUG_PRINT("Checked ");
            DEBUG_PRINTLN(addr);
            var.init(addr, &data_copy);
            //DEBUG_PRINTLN(var.getHameHash());
            if (var.getHameHash() == name_hash) {
                DEBUG_PRINT("Found ");
                DEBUG_PRINTLN(addr);
                var.init(addr, data);
                return var;
            }
            addr = var.getNextVarAddr();
        }
        
        Variable last_var = var;
        if (created_new_var) *created_new_var = true;
        var = Variable(name);
        addr = getAddrForNewVar(sizeof(T) + Variable::metaInfoSize);
        // TODO: check init() returned code
        var.init(addr, data, true);
        if (firstVarAddr != 0) {
            last_var.setNextVarAddr(var.getStartAddr());
            last_var.updateMetaInfo();
        } else {
            firstVarAddr = var.getStartAddr();
        }
        return var;
    }

    
    /* SYSTEM METHOD.
     *  Add the first variable to the partition without variables. 
     * 
     *  @param name The name of the variable to create.
     *  @param data The pointer to the data to link the new variable to.
     *  @return Created variable.
    */
    /*
    template <typename T>
    EEPROMVar addFirstVar(char* name, T* data) {
        if (firstVarAddr != 0) {
            // FIXME
            return;
        }

        uint16_t addr = getAddrForNewVar(sizeof(T) + Variable::metaInfoSize);
        Variable var(name);
        // TODO: check init() returned code
        var.init(addr, data, true);
        firstVarAddr = addr;
        return var;
    };
    */

    //bool tick();
};

typedef MemPart::EEPROMVar EEPROMVar;

#endif
