#include <EEManager.h>


int ok_cnt = 0, err_cnt = 0;
bool assert(bool val);

void setup() {
    Serial.begin(115200);
    //EEPROM.put(0, "dseesfesfesfesfefesfesfjnfjesbjebfhseeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");

    EEMemManager::init();
    MemPart part1 = EEMemManager::GetMemPart("part1");
    MemPart part3 = EEMemManager::GetMemPart("part3");
    MemPart part2 = EEMemManager::GetMemPart("part2");

    bool created;
    int loads_num = 1;
    EEPROMVar loads_num_var = part1.getVar("loads_num", &loads_num, &created);
    Serial.print("Load #");
    Serial.println(loads_num);

    int next_load_num = 2;
    EEPROMVar next_load_num_var = part1.getVar("next_load_num", &next_load_num, &created);
    Serial.print("Next load #");
    Serial.println(next_load_num); 

    assert(loads_num + 1 == next_load_num);

    int x3 = 3;
    EEPROMVar x3_var = part1.getVar("x3", &x3);
    Serial.print("x3 ");
    Serial.println(x3); 
    assert(loads_num + 2 == x3);

    int x4 = 4;
    EEPROMVar x4_var = part2.getVar("x4", &x4);
    Serial.print("x4 ");
    Serial.println(x4); 
    assert(loads_num + 3 == x4);

    int x5 = 5;
    EEPROMVar x5_var = part2.getVar("x5", &x5);
    Serial.print("x5 ");
    Serial.println(x5); 
    assert(loads_num + 4 == x5);

    int x6 = 6;
    EEPROMVar x6_var = part3.getVar("x6", &x6);
    Serial.print("x6 ");
    Serial.println(x6); 
    assert(loads_num + 5 == x6);

    loads_num++;
    next_load_num++;
    x3++;
    x4++;
    x5++;
    x6++;
    loads_num_var.updateNow();
    next_load_num_var.updateNow();
    x3_var.updateNow();
    x4_var.updateNow();
    x5_var.updateNow();
    x6_var.updateNow();
}

void loop() {

}

bool assert(bool val) {
    if (val) {
        Serial.println("[Pass]");
        ok_cnt++;
    } else {
        Serial.println("[Error]");
        err_cnt++;
    }    
}