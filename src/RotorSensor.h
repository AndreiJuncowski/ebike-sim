#ifndef ROTOR_SENSOR_H
#define ROTOR_SENSOR_H
#include <Arduino.h>

class RotorSensor {
    private:
        boolean anterior = false;
        boolean atual = false;
        uint8_t pin;
        uint32_t diametermm;
        unsigned long turns = 0;
        unsigned long microsAnterior = 0;
        unsigned long microsAtual = 0;
        unsigned long tempoPercorrido = 0;
        unsigned long microsPulsoAnterior = 0;
        float cadence = 0;
        float velocity = 0;

    public:
        RotorSensor(uint8_t _pin, uint32_t _diametermm): pin(_pin), diametermm(_diametermm) {
            
        }

        void init() {
            pinMode (pin, INPUT_PULLUP);
        }

        void loop() {
            atual = digitalRead(pin) == false;
            microsAtual = micros();
            tempoPercorrido = microsAtual - microsAnterior;
            
            if (atual != anterior && tempoPercorrido >= 1000){
                // Serial.println("VOLTA");
                anterior = atual;
                microsAnterior = microsAtual;
                if (atual){
                    turns++;
                    unsigned long  tempoVoltaMicrosegundos = (microsAtual - microsPulsoAnterior);
                    cadence  = 60000000 / tempoVoltaMicrosegundos;
                    microsPulsoAnterior = microsAtual;
                    velocity =  float(diametermm * 1000) / float(tempoVoltaMicrosegundos);
                }
            }
            if(tempoPercorrido > 3000000 && velocity > 0) {
                Serial.println("RESET VELOCIDADE");
                velocity = 0;
                cadence = 0;
            }
        }

        float getCadence() {
            return cadence;
        }

        float getVelocityMs() {
            return velocity;
        }

        float getVelocityKmh() {
            return velocity * 3.6;
        }

        float getDistance() {
            return turns * diametermm * 1000 * 0.0000001;
        }

        void print() {
            Serial.print(" VELOCIDADE[m/s]=");Serial.print(getVelocityMs());
            Serial.print(" VELOCIDADE[km/h]=");Serial.print(getVelocityKmh());
            Serial.print(" VOLTAS=");Serial.print(turns);
            Serial.print(" DISTANCIA=");Serial.print(getDistance());
            Serial.print("\n");
        }
};

#endif
