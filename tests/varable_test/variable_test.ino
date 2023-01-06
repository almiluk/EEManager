// For arduino only, not for ESP

#include <EEManager.h>
#include <EEPROM.h>


struct {
    char text[6] = "Hello";
} data;

Variable var(0, &data, "MyVar", false);

void setup() {
    Serial.begin(9600);

    Serial.println("Raw data: ");
    for (int i = 0; i < sizeof(VariableInfo) + sizeof(data); i++){
        Serial.print(EEPROM.read(i));
        Serial.print(' ');
    }

    Serial.println("\nVar info:");
    Serial.println(var.getHameHash());
    Serial.println(var.getStartAddr());
    Serial.println(var.getDataSize());
    Serial.println(var.getNextVarAddr());

    data.text[0]++;
    //var.updateNow();
}

void loop() {

}
