// For arduino only, not for ESP
#define DEBUG_EEPROM

#include <EEManager.h>
#include <EEPROM.h>


struct {
    char text[6] = "Hello";
} data, data2;

void print_var(Variable var);

int ok_cnt = 0, err_cnt = 0;
bool assert(bool val);

void setup() {
    Variable var("MyVar");
    var.init(0, &data, true);
    Variable var2("MyVar");
    Serial.begin(115200);
    var2.init(0, &data2);
    
    assert(var == var2);

    data.text[0]++;
    var.updateNow();
    var2.init(0, &data2);

    assert(var == var2);
}

void loop() {

}

void print_var(Variable var) {
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


