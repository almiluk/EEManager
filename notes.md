# almilukEEManager

## Differences from original library
1. The first write key mechanism has been removed.
2. Named memory partition and named variables have been added.

## Use cases
1. Manager gives a memory partition by its name.
   - Manager creates new partintion if partition with such name doesn't extist. And write it to EEPROM.
2. Partition gives a variable by its name.
   1. Partition gets a name of a variable and pointer to some RAM variable for linking
   2. Partition try to find the variable with specified name in EEPROM.
      1. If variable is found, create variable describing object and link it to local variable and EEPROM address.
      2. Else, Partition asks Manager to register new variable and save its value.
         1. Get next empty EEPROM address.
         2. Create variable describing object and link it to local variable and EEPROM address.
         3. Write the value of the local variable to EEPROM using got address.
         4. Write information about new EEPROM variable to EEPROM.
         5. Partition add new variable to itself.

## Data structures

1. Stutus code
``` C++
enum MemStatusCode {
    found,
    created,
    failed
};
```

2. Variable
Most of the logic from the original EEManager class is here.
``` C++
class Variable {
    uint32_t    nameHash;
    uint16_t    dataSize;       // size from EEManager class
    uint16_t    nextVarAddr;

    uint8_t*    data;
    uint16_t    addr;
    bool        needUpdate;     // _update from EEManager class   
    uint32_t    lastWriteTime;  // _tmr from EEManager class
    uint16_t    updTimeout;     // _tout from EEManager class
};
```

3. Memory partition
``` C++
class MemPart {
    uint16_t    firstVarAddr;

    Variable    lastVar;

    template <typename T> 
    Variable getVar(char* name, T* data);
};
```

4. EEManager
``` C++
class EEMemManager {
    uint16_t lastAddr;
    static const uint16_t startAddr = 0;
    
    MemPart GetMemPart(char* name);
};
```

## TODO
- [ ] Move unsafe methods from MemPart and Variable to some classes inherited from them inside EEMemManager.
- [X] Save only nessary part of Variable (name, addres, nextVarAddess). Collapse on try to get variable using local variable of wrong size + unsecure => add guard bytes?!
- [ ] Add MU for EEPROM data for many instanses of Variable with same addr.
- [ ] Store all used variables in RAM, return pointer to one instance for same names and udapte all by MemPart.update() => ~MU, user don't have to use Variable (only local variables linked to EEPROM and MemPart.update()). Problem: update can be applied after local variable left its vision area.
