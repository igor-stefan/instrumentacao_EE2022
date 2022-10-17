#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "sma.hpp"

#define SERIAL_ENABLE 0   // flag se dados devem ser mostrados na serial
#define CALIBRATION 0     // flag calibracao (1 para modo de calibracao, 0 para modo de medicao)
#define OLED 0            // flag oled (1 para circuito com oled, 0 para circuito sem oled)

//definicoes display oled protocolo I2C
const int woled = 128;
const int holed = 64;
const int endereco_oled = 0x3C;
const int reset_oled = 4;

const int SMA_WINDOW_SIZE = 1 << 6; // tamanho da janela do filtro
const int LCD_ADDR = 0x27;          // endereco do lcd (necessario para implementacao do display lcd)
const int ANALOG_PIN = A0;          // pino em que é realizada a leitura
const float ADC_TO_VOLT = (float) 5. / 1023; // constante de conversao de analog para tensao

inline float adc_to_volt(int adc){return (float) adc * ADC_TO_VOLT;} // funcao que converte de valor analogico para volt
inline float volt_to_amp(float volt){ // funcao que retorna o valor correspondente em corrente da tensao medida
    #if (OLED) //curvas placa oled
      if(volt < 0.7)
        return 0.538 * volt * volt + 0.858 * volt + 0.206;
      if(volt < 1.56)
          return 0.462 * volt * volt + 1.27 * volt -0.0468;
      return 0.297 * volt * volt + 2.41* volt - 1.35;
      
      #else // curvas placa lcd
        return (volt < 2.22? .623 * volt * volt + .4 * volt + .164 : 2.68 * volt - 1.97); 
      #endif

}

inline void pisca_led(){
  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED on (HIGH is the voltage level)
  delay(300);                         // wait for a second
  digitalWrite(LED_BUILTIN, LOW);     // turn the LED off by making the voltage LOW
  delay(300);                         // wait for a second
}

SMA filter(SMA_WINDOW_SIZE); // inicializa objeto do filtro a ser implementado
Adafruit_SSD1306 oled(woled, holed, &Wire, reset_oled); // inicializa objeto do oled
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2); // inicializa objeto do lcd

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ANALOG_PIN, INPUT);
    #if (SERIAL_ENABLE)
          Serial.begin(5e5);
      #endif
    #if(OLED)
        while(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
          #if (SERIAL_ENABLE)    
            Serial.println("OLED não iniciou, falha no setup!!!\n");
          #endif
        pisca_led();
        delay(2);
      }
      oled.clearDisplay();
      oled.setTextSize(3);                      // Normal 1:1 pixel scale
      oled.setTextColor(SSD1306_WHITE);         // Draw white text
      oled.setCursor(20,20);             
    
    #else
      lcd.begin();            // inicializa o lcd
      lcd.setBacklight(HIGH); // luz de fundo do lcd
      lcd.setCursor(0, 0);    // posiciona o cursor no canto superior esquerdo
     #endif
     
      
    #if (CALIBRATION) // modo de calibracao
      #if(OLED)
        oled.println("Tensao (V)");
        oled.display(); 
      #else
        lcd.print("Tensao (V)");
      #endif
    #else // modo de medicao
      #if(OLED)
        oled.println("Corrente (A)");
        oled.display();
      #else
        lcd.print("Corrente (A)");
      #endif
    #endif
    
    #if (SERIAL_ENABLE)
      Serial.println("Setup OK");
    #endif
}

void loop() {
    float volt = adc_to_volt(analogRead(ANALOG_PIN)); // faz a leitura na porta analógica
    float current = volt_to_amp(volt); // converte de tensão para corrente
    float current_filtered = filter.updateSamples(current); // adiciona a medida às amostras do filtro

    #if(SERIAL_ENABLE)
        Serial.print(volt);
        Serial.print(" ");
        Serial.print(current);
        Serial.print(" ");
        Serial.println(current_filtered);  
    #endif

    // posiciona o cursor no display utilizado
    #if (OLED)
      oled.clearDisplay();
      oled.setCursor(10, 20); 
    #else
      lcd.setCursor(0,1);
    #endif

    #if(OLED) // caso circuito implementado com oled
      #if(CALIBRATION)
        oled.print(volt);
        oled.println(" V");
      #else 
        if(current < 0.25)       oled.println("BAIXA");
        else if(current > 5.0)   oled.println("ALTA");
        else                              {
            oled.print(current_filtered);
            oled.println(" A");
        }
      #endif
        oled.display();
    #else // caso circuito implementado com lcd
      #if(CALIBRATION)
        lcd.print(volt);
      #else 
        if(current < 0.15)       lcd.print("[Corrente baixa]");
        else if(current > 5.0)   lcd.print("[Corrente alta]");
        else                              {
            lcd.print(current_filtered);
            lcd.print("            ");
        }
      #endif
    #endif
      
    delay(10);
}
