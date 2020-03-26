#include"funciones.h"
#include <avr/interrupt.h>
/*Instrucciones de las siguientes 3 lineas:
1. Cambie MEDIRTIEMPOS a 1 o 0 para sacar en 2 columna el tiempo de funcion en microsegundos
2. Cambie entrada a {RUIDO,PASO,IMPULSO} para simular una señal de entrada
3. Cambie salida a {FIR1,FIR2,FIR3, IIR} para comparar diferentes implementaciones de funciones
4. Abra en: ->Herramientas -> monitor serie. o ->Herramientas -> "Serial Plotter"
*/
#define MEDIRTIEMPOS 0//Cambiar a 1 y abrir el monitor serial "de la grafica no se aprecia mucho ya que la columna del centro está en microsegundos"
#define MOSTRARENTRADA 1// cambiar a 1 para mostrar la entrada
#define MUESTRASPASOIMPULSO 100// numero de muestras antes de resetear la respuesta paso e impulso, subir en sistemas lentos
int entrada=SIN;// Seleccione PASO, RUIDO, IMPULSO
int f=20;      //frecuencia de la onda seno 5, 10, 20
int salida=FIR3;// Seleccione FIR1,FIR2,FIR3, o IIR
const long interval = 10;           // Intervalo a medir periodico en milisegundos 100Hs=10ms


ISR(ADC_vect)
{
 banderaADC=0xFF;
}


void setup() {
 sei();
Inicializar_ADC_PWM_Serial();
previousMillis=0;
tam=(200/f);
Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    inicieConv();
  }
  
  if (banderaADC) {
    previousMillis = currentMillis;
    leerConv();
    long sal;// para los FIR cambiar sal a long
    float saliir;
    switch(entrada)
    { 
       case PASO: 
          if (cont>MUESTRASPASOIMPULSO)
          {
             cont=0;
            if (walk)
              walk=0;
            else
            if (salida==IIR)
              walk=1;// para el filtro IIR la respuesta impulso es de una unidad
            else
              walk=256;// para el filtro FIR la respuesta impulso es de 256 ya que esta escalizada, mire al final de las funciones FIR# que se hace un corrimiento de 8 bits 2^8-1=256
          }
          cont++;
          break; 
       case RUIDO: 
          walk+=random(-3, 4);//numero aleatorio entre -3 y (4-1)=3
          break; 
       case IMPULSO: 
          if (cont>MUESTRASPASOIMPULSO)
          {
            cont=0;
            if (salida==IIR)
              walk=1;// para el filtro IIR la respuesta paso es de una unidad
            else
              walk=256; //para el filtro FIR la respuesta paso es de 256 ya que esta escalizada, mire al final de las funciones FIR# que se hace un corrimiento de 8 bits 2^8-1=256
          }
          else{
              walk=0;
          }
          cont++;
          break; 

         case SIN:
         walk=seno5[cont];
         if(f==10)
         walk=seno10[cont];
         if(f==20)
         walk=seno20[cont];
         cont++;
         if (cont>=tam)
         {
         cont=0;
         }
         break;
          
       default: 
          Serial.println("no selecciono entrada");
          walk=-1; 
    }
    //Serial.print("Dato de entrada:");
    if (MOSTRARENTRADA)
    {
      Serial.print(walk);
      Serial.print(" ");
    }
    
    switch(salida) 
    { 
       case FIR1:
        ts1 = micros();
        sal=filtrarFIR1(walk);
        ts2 = micros();
        if (MEDIRTIEMPOS){
          Serial.print(ts2-ts1);//timepo en microsegundos
          Serial.print(" ");
        }
        Serial.print(sal);
        Serial.print(" ");
      break;
      case FIR2:
        ts1 = micros();
        sal=filtrarFIR2(walk);
        ts2 = micros();
        if (MEDIRTIEMPOS){
          Serial.print(ts2-ts1);//timepo en microsegundos
          Serial.print(" ");
        }
        Serial.print(sal);
        Serial.print(" ");
      
      break;
      case FIR3:
        ts1 = micros();
        sal=filtrarFIR3(walk);
        ts2 = micros();
        if (MEDIRTIEMPOS){
          Serial.print(ts2-ts1);//timepo en microsegundos
          Serial.print(" ");
        }
        Serial.print(sal);
        Serial.print(" ");
      break;
      case IIR:
        ts1 = micros();
        saliir=filtrarIIR((float)walk);
        ts2 = micros();
        if (MEDIRTIEMPOS){
          Serial.print(ts2-ts1);//timepo en microsegundos
          Serial.print(" ");
        }
        Serial.print(saliir,10);//pinta 10 decimales
        Serial.print(" ");
      break;
    }
    Serial.println("");
}
  delay(255);//delay de 10 ms para que haga alrededor de 100 muestras por segundo
}
