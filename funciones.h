// sube cuando esta listo para leer 
extern volatile int readFlag;
// Almacena el valor leido
extern volatile int analogVal;
uint8_t volatile banderaADC=0x00;

int tam = 40;

//FILTROS PROFE

const int BL = 8;
const int B[BL] = {
      0,   16,   48,   64,   64,   48,   16,    0
};

//const int BL = 12;
//const int B[BL] = {
//     -8,   -8,    8,   32,   48,   56,   56,   48,   32,    8,   -8,   -8
//};

//const int BL = 18;
//const int B[BL] = {
//      0,   -8,  -16,   -8,    0,   16,   32,   48,   64,   64,   48,   32,
//     16,    0,   -8,  -16,   -8,    0
//};
/*
const int BL = 31;
const int B[BL] = {
      0,    0,    0,    8,    8,    0,    0,   -8,   -8,   -8,    0,    8,
     24,   40,   48,   56,   48,   40,   24,    8,    0,   -8,   -8,   -8,
      0,    0,    8,    8,    0,    0,    0
};*/

//FILTROS GRUPO


/*
const int BL = 8;
const long B[8] = {
     -2,   10,   24,   32,   32,   24,   10,   -2
};
*/

/*
const int BL = 12;
const long B[12] = {
     -6,   -2,    6,   14,   24,   28,   28,   24,   14,    6,   -2,   -6
};
*/
/*
const int BL = 18;
const int16_T B[18] = {
     -512,  -1280,  -1664,  -1216,     64,   2176,   4544,   6592,   7808,
     7808,   6592,   4544,   2176,     64,  -1216,  -1664,  -1280,   -512
};
*/
/*
const int BL = 31;
const int8_T B[31] = {
     -2,    0,    2,    3,    3,    2,   -1,   -4,   -6,   -5,   -2,    4,
     12,   19,   25,   27,   25,   19,   12,    4,   -2,   -5,   -6,   -4,
     -1,    2,    3,    3,    2,    0,   -2
};
*/

const long seno5[40]= {
 255,   253,   249,   241,   231,   218,   202,   185,   167,   147,   128,    
 108,    88,   70,    53,    37,    24,    14 ,    6,     2, 0 ,    2 ,    6,    14 ,   24 ,   37 ,   
 53 ,   70,    88,   108,   127,   147,   167 ,  185,   202,   218,   231,   241,   249 ,  253}; //seno 5 hz   
const long  seno10[20]= {
  255,   249 ,  231,   202,   167,   128,    88,    53,    24,     6,     0,     6,    24,    53,    88,   127,   167,   202,   231,   249
 };  //seno 10 hz
const long  seno20[10]= {
  255,   231,   167,    88,    24,     0,    24,    88,   167,   231
 };  //seno 20 hz

/*Inicio de variables volatiles para pasar datos entre funciones*/
volatile int x[BL];
volatile int k;
volatile int analogVal;
volatile int readFlag;

/*Variables especificas del ejemplo:*/
volatile int walk;
volatile int cont;
uint32_t ts1,ts2;
unsigned long previousMillis=0;

/*Definiciones del ejemplo*/
#define RUIDO 1
#define PASO 2
#define IMPULSO 3
#define PASOANALOGO 4
#define SIN 5
#define PERIODOTEST 100

#define FIR1 1
#define FIR2 2
#define FIR3 3
#define IIR 4

#define cle(reg,bit) (reg &= ~(1 << bit)) 
#define sbi(reg,bit) (reg |= (1 << bit))
#define eq(reg,var) (reg |= var);
#define excep 0x08

void  Inicializar_ADC_PWM_Serial(){
  cont=0;
  walk=0;
  k=0;
  previousMillis=0;
  pinMode(LED_BUILTIN,OUTPUT);//LED_BUILTIN

  Serial.begin(1000000);/*velocidad maxima para que no interfiera en tiempos*/
  while (!Serial) {
    ; // Esperar a que el puerto inicie
  }
  Serial.println("Setup");
}


void enviardato(){
  Serial.print(analogVal);
  Serial.print(" ");
  Serial.print(cont);
  Serial.println("");
}
 
/*INICIAN FUNCIONES DE EJEMPLO*/

long filtrarFIR1(int in)
{
  int i=1;
  x[k] = (int)in;
  long y=0; 
  for (i=1; i<=BL; i++)// NOTA, DEBE INICIAR EN 1. EJERCICIO: haga una prueba de escritorio con una respuesta impulso y compruebe...
  {
      y += B[i-1] * x[(i + k) % BL];// verifique que para su filtro no exista overflow. 
  }
  
    k = (k+1) % BL;
    return y>>8; //si no es multiplo de 2^n divida por el factor de normalización adecuado a su filtro. 
}

long filtrarFIR2(int in)
{
  int i=0;
  x[k] = in;
  long y=0;
  int inx=k;
  for(i = 0; i < BL; ++i) {
    y += x[inx] * B[i];// verifique que para su filtro no exista overflow. 
    inx = inx != 0 ? inx-1 : BL-1;
  }
  k++;
  k = (k>=BL) ? 0:k;
  return y>>8; //si no es multiplo de 2^n divida por el factor de normalización adecuado a su filtro. 
  
}

long filtrarFIR3(int in)
{
  int i=0;
  x[k] = in;
  int inx=k;
  const int *apuntadorcoef=&B[0];
  volatile int *apuntadorarrc=&x[inx];
  long y=0;
  for(i = 0; i < BL; ++i) {
    y += (long)(*apuntadorarrc) * (long)(*apuntadorcoef);// verifique que para su filtro no exista overflow. 
    apuntadorcoef++;
    if (inx != 0){
      apuntadorarrc--;
      inx--;
    }
    else{
      apuntadorarrc=&x[BL-1];
      inx=BL-1;
    }
  }
  k++;
  k = (k>=BL) ? 0:k; 
  return y>>8; //si no es multiplo de 2^n divida por el factor de normalización adecuado a su filtro. 
}

/*Parametros del filtro IIR*/
const int NL=3;
//const float NUM[3]={3.844633284e-06,7.689266567e-06,3.844633284e-06};
//const float NUM[3]={0.036574835843928, 0.073149671687856, 0.036574835843928};
//const float NUM[3]={0.10598800495854646, -0.1580936232020776, 0.06284028813992219};
//const float NUM[3]={0.39133577250659896, -0.7826715450131979, 0.39133577250659896};
float NUM[3]={1.0,-2.0,1.0};
const int DL=3;
//const float DEN[3]={1,   -1.994446397,   0.9944617748};
//const float DEN[3]={1,   1.3908952814253899, -0.5371946248011019};
//const float DEN[3]={1,   0.8939931253174443, 0.2209};
//const float DEN[3]={1,   -0.36952737735124147, 0.1958157126558331};
const float DEN[3]={1,   0, -1};



float w[NL]={0,0,0};


float filtrarIIR(float in){
float y;
w[0]=(DEN[0]*in)-(DEN[1]*w[1])-(DEN[2]*w[2]);// OJO QUE EL MENOS YA ESTA EN LA ECUACION
y=((NUM[0]*w[0])+(NUM[1]*w[1])+(NUM[2]*w[2]));
w[2]=w[1];
w[1]=w[0];
return y;
}

void inicieConv()
{
  /*configuro pre-escalizador suponiendo clk efectivo de 8 Mhz
  para que quede el clkADC 125k
  */
  sbi(ADCSRA,ADPS2);
  sbi(ADCSRA,ADPS1);
  sbi(ADCSRA,ADPS0);
  
  /*configuro MUX para entrada ADC0 P23 A0
  */
  cle(ADMUX,MUX0);
  cle(ADMUX,MUX1);
  cle(ADMUX,MUX2);
  cle(ADMUX,MUX3);
  //Respuesta orientada a L
  cle(ADMUX,ADLAR);
  /*configuro referencia de V en Vcc
  */
  sbi(ADMUX,REFS0);
  sbi(ADMUX,REFS1);
  /*Configurar modo single y encender el ADC*/
  sbi(ADCSRA,ADEN);
  cle(ADCSRA,ADATE);

  /*deshabilitar interrupción*/
  cle(ADCSRA,ADIF); 
  //finalizado por interrupcion
  eq(ADCSRA,excep);
  
  /*INICIE
  */
  sbi(ADCSRA,6);
  
  //eq(ADCSRA,0xC7);
  //eq(ADMUX,0xC8); 
}


double  leerConv()
{
  uint8_t l=ADCL;  //lectura de bits ADC
  uint8_t h=ADCH;
  
  double aux=(h << 8) | (l);
  return aux;
}
