// For arduino only, not for ESP
#define DEBUG_EEPROM

#include <EEManager.h>
#include <EEPROM.h>


struct datatype1 {
    char text[7];
};

struct datatype2 {
    uint8_t a = 1;
    uint8_t b = 100;
};

MemPart memPart(2);

void print_var(EEPROMVar var);

int ok_cnt = 0, err_cnt = 0;
bool assert(bool val);

void setup() {
    datatype1 data1, data2;
    datatype2 data3, data4;
    strcpy(data1.text, "Hello!");

    Serial.begin(115200);

    uint16_t load_num = 1;
    EEPROMVar load_num_var = memPart.getVar("load_num", &load_num);
    Serial.print("Load #");
    Serial.println(load_num);
    load_num++;
    load_num_var.updateNow();
    print_var(load_num_var);


    Serial.println("[Test started]");
    
    Serial.println("\nTest 1");
    EEPROMVar var = memPart.getVar("1", &data1);
    EEPROMVar var2 = memPart.getVar("1", &data2);
    assert(var == var2);

    Serial.println("\nTest 2");
    EEPROMVar var3 = memPart.getVar("1", &data3);
    assert(var3 != var && var3.getStartAddr() == var.getStartAddr());

    Serial.println("\nTest 3");
    EEPROMVar var4 = memPart.getVar("2", &data3);
    var = memPart.getVar("1", &data1); // update var metainfo
    assert(var4.getStartAddr() == var.getNextAddr() && var4.getStartAddr() == var.getNextVarAddr());

    Serial.println("\nTest 4");
    EEPROMVar var5 = memPart.getVar("2", &data4);
    assert(var5 == var4);

    Serial.print("data3.a = ");
    Serial.println(data3.a);
    data3.a++;
    var4.updateNow();

    Serial.println("[Test ended]");
    Serial.print(ok_cnt);
    Serial.print('/');
    Serial.println(ok_cnt + err_cnt);
}

void loop() {

}

void print_var(EEPROMVar var) {
    Serial.println("Var info:");
    Serial.println(var.getHameHash());
    Serial.println(var.getStartAddr());
    Serial.println(var.getDataSize());
    Serial.println(var.getNextVarAddr());

    Serial.println("Raw data: ");
    for (int i = var.getStartAddr(); i <= var.getEndAddr(); i++) {
        Serial.print(EEPROM.read(i));
        Serial.print(' ');
    }
    Serial.println();
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
