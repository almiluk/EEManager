// For arduino only, not for ESP
#define DEBUG_EEPROM

#include <EEManager.h>
#include <EEPROM.h>


struct {
    char text[6] = "Hello";
} data, data2;

void print_var(EEPROMVar var);

int ok_cnt = 0, err_cnt = 0;
bool assert(bool val);

void setup() {
    Serial.begin(115200);

    EEPROMVar var("MyVar");
    var.init(0, &data);
    Serial.println("Try to reboot the board once if the next test is failed");
    assert(data.text[0] == 'b');
    MemStatusCode code = var.init(0, &data2, true);
    assert(data2.text[0] == 'H');
    EEPROMVar var2("MyVar");
    var2.init(0, &data);
    
    assert(var == var2);

    data2.text[0] = 'b';
    var.updateNow();
    data.text[0] = 'c';
    var2.init(0, &data);

    assert(var == var2);
    print_var(var);
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


