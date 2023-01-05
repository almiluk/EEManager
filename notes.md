# almilukEEManager

## Differences from original library
1. The first write key mechanism has been removed.
2. Named memory partition and named variables have been added.

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
    uint8_t*    data = nullptr;
    uint16_t    size, addr;
    bool        need_update = 0;      // _update from EEManager class   
    uint32_t    last_write_time = 0;  // _tmr from EEManager class
    uint16_t    upd_timeout;          // _tout from EEManager class
};
```

3. Memory partition
``` C++
class MemPart {
    uint32_t    nameHash;
    Variable    vars[8];
    uint8_t     vars_n = 0;
    uint16_t    nextPart;

    Variable getVar(char* name);
    template <typename T> MemStatusCode getVal(char* name, T* data);
};
```

4. EEManager
``` C++
class EEMemManager {
    MemPart     parts[8];
    uint8_t     parts_n = 0;
    uint16_t    nextManager;

    bool init();
    MemPart GetMemPart(char* name);
    uint16_t addMemPart(char* name);
};
```
