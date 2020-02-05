/***** Carrega as bibliotecas *****/
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "RS485_protocol.h"
#include <Wire.h>

/***** Variáveis *****/
byte porta;
byte estadoPorta;
byte envio;
byte estadoRele1;
byte estadoMestre;
byte statusReqLight;
byte statusReqLUM;
byte statusReqTemp;

/******Atribuição de nomes aos pinos************/
#define rele 5

/***** SoftwareSerial(RX, TX) *****/
SoftwareSerial RS485(2, 3);

/***** Pinos *****/
const byte ENABLE_PIN = 4;
const byte LED_PIN = 13;


/***** Funções RS485 *****/
void fWrite (const byte what){
	RS485.write (what);  
}
  
int fAvailable (){
	return RS485.available ();  
}

int fRead (){
	return RS485.read ();  
}

/*********Função responsável por pedido de dados************/
int requestData(const byte device, const byte pedido){    //Dois argumentos: Endereço da slave e request a fazer
  
	RS485.begin (28800);              //Inicialização para transmissão de dados RS-485 

/***** Mensagem a enviar *****/
  	byte msg [] = { 
        	device,        //Endereço da Slave
        	pedido         //Pedido (Iluminação ou Temperatura)
  	};

/***** Envio da mensagem *****/

    	digitalWrite (ENABLE_PIN, HIGH);     //Ativar modo de envio
      sendMsg (fWrite, msg, sizeof msg);   //Chamada de função responsável pelo tratamento de dados antes do envio (adiciona STX e ETX e inverte os bytes a enviar de forma a prevenir corrupção de dados)
      delayMicroseconds (660);             //Tempo de espera para envio   
      digitalWrite (ENABLE_PIN, LOW);      //Ativa modo de receção

/***** Resposta ao envio *****/ 

    	byte buf [10];                                                      //Vetor que armazena os dados da mensagem
    	byte received = recvMsg (fAvailable, fRead, buf, sizeof buf);       //Chamada da função que tratará dos dados recebidos, formando a mensagem enviada

    if(pedido == 1){                            //Se o pedido é referente à iluminação...
  	    if(received == 0){                      //E não recebeu nada...
        		statusReqLight = 2;                 //Coloca o estado da iluminação com valor impossível
            statusReqLUM = 255;                 //Coloca o valor da luminosidade com valor impossível
  	    }
  	    else{                                   //Se recebeu
        		statusReqLight = buf[2];            //Coloca o valor da iluminação com o valor do 3º byte
            statusReqLUM = buf[3];              //Coloca o valor da luminosidade com o valor do 4º byte
        		received = 0;                       //Reset à variável received 
  	    }
    }
    
    if(pedido == 3){                            //Se o pedido é referente à temperatura
        if(received == 0){                      //E não recebeu nada...
            statusReqTemp = 255;                //Coloca o valor da temperatura com valor impossível
        }
        else{                                   //Se recebeu...
            statusReqTemp = buf[2];             //Coloca o valor da temperatura com o valor do 3º byte
            received = 0;                       //Reset à variável received 
        }        
    }
   
   RS485.end();                                 //Termina comunicação para com o barramento
}

/*********Função responsável por controlo************/
int transmitData(const byte device, const byte type, const byte state){       //Três argumentos: Endereço da slave, controlo a fazer e estado pretendido
  
  	RS485.begin (28800);          //Inicialização para transmissão de dados RS-485
  
/***** Mensagem a enviar *****/
  	byte msg [] = { 
        	device,      //Endereço da slave
        	type,        //Tipo de Controlo
        	state        //Estado pretendido
    };

/***** Envio da mensagem *****/
      digitalWrite (ENABLE_PIN, HIGH);     //Ativar modo de envio
      sendMsg (fWrite, msg, sizeof msg);   //Chamada de função responsável pelo tratamento de dados antes do envio (adiciona STX e ETX e inverte os bytes a enviar de forma a prevenir corrupção de dados)
      delayMicroseconds (660);             //Tempo de espera para envio   
      digitalWrite (ENABLE_PIN, LOW);      //Ativa modo de receção

/***** Resposta ao envio *****/
    	byte buf [10];                                                      //Vetor que armazena os dados da mensagem
    	byte received = recvMsg (fAvailable, fRead, buf, sizeof buf);       //Chamada da função que tratará dos dados recebidos, formando a mensagem enviada
    	digitalWrite (LED_PIN, received == 0);                              //Liga o LED em caso de erro   
    	RS485.end();                                                        //Termina comunicação para com o barramento
}

/**********Inicializações***********/
void setup() {
/***** Inicia a comunicação série com o Raspberry *****/
    	Serial.begin(9600);
  
/***** Estados Pinos *****/
    	pinMode(ENABLE_PIN, OUTPUT);
    	pinMode(LED_PIN, OUTPUT);
    	pinMode(rele, OUTPUT);
} 


void loop(){
 
  	if(Serial.available() > 0 ){                //Comunicação série disponível?
          /********Variáveis necessárias******/
          byte portaControlo;
          byte typeCont;
          byte tiporequest;
          
        	char estadoPretendido = Serial.read();    //Leitura dos dados recebidos


          /********Interpretação dos dados recebidos e chamada das funções para ações de controlo*********/
        	if (estadoPretendido == 'a'){
            		typeCont = 2;
            		portaControlo = 1;
            		estadoRele1 = 0;
            		transmitData(portaControlo, typeCont, estadoRele1);     //Chamada da função com os devidos argumentos
        	}

        	else if (estadoPretendido == 'A'){
                typeCont = 2;
            		portaControlo = 1;
            		estadoRele1 = 1;
            		transmitData(portaControlo, typeCont, estadoRele1);     //Chamada da função com os devidos argumentos
        	}

        	else if (estadoPretendido == 'b'){
                typeCont = 2;
            		portaControlo = 2;
            		estadoRele1 = 0;
            		transmitData(portaControlo, typeCont, estadoRele1);     //Chamada da função com os devidos argumentos
        	}

        	else if (estadoPretendido == 'B'){
            		typeCont = 2;
            		portaControlo = 2;
            		estadoRele1 = 1;
            		transmitData(portaControlo, typeCont, estadoRele1);     //Chamada da função com os devidos argumentos
        	}
        
        	else if (estadoPretendido == 'c'){
            		typeCont = 2;
            		portaControlo = 3;
            		estadoRele1 = 0;
            		transmitData(portaControlo, typeCont, estadoRele1);     //Chamada da função com os devidos argumentos
        	}

        	else if (estadoPretendido == 'C'){
            		typeCont = 2;
            		portaControlo = 3;
            		estadoRele1 = 1;
            		transmitData(portaControlo, typeCont, estadoRele1);     //Chamada da função com os devidos argumentos
        	}

        	else if (estadoPretendido == 'd'){
                estadoMestre = 0;
        	}

        	else if (estadoPretendido == 'D'){
                estadoMestre = 1;
        	}

        	else if(estadoPretendido == 'T'){
            		portaControlo = 1;
            		tiporequest = 3;
            		requestData(portaControlo, tiporequest);                //Chamada da função com os devidos argumentos
            		Serial.println(statusReqTemp);                         //Envio de dados para o Raspberry
	      	}

	      	else if (estadoPretendido == 't'){
            		portaControlo = 2;
            		tiporequest = 3;
            		requestData(portaControlo, tiporequest);                //Chamada da função com os devidos argumentos
            		Serial.println(statusReqTemp);                         //Envio de dados para o Raspberry
        	}

        	else if (estadoPretendido == 'w'){
            		portaControlo = 1;
	          	  tiporequest = 1;
            		requestData(portaControlo, tiporequest);                //Chamada da função com os devidos argumentos
            		Serial.println(statusReqLight);                        //Envio de dados para o Raspberry
                Serial.println(statusReqLUM);                          //Envio de dados para o Raspberry
        	}
        
        	else if (estadoPretendido == 'x'){
            		portaControlo = 2;
	          	  tiporequest = 1;
            		requestData(portaControlo, tiporequest);                //Chamada da função com os devidos argumentos
            		Serial.println(statusReqLight);                        //Envio de dados para o Raspberry
                Serial.println(statusReqLUM);                          //Envio de dados para o Raspberry
        	}

        	else if (estadoPretendido == 'y'){
            		portaControlo = 3;
	          	  tiporequest = 1;
            		requestData(portaControlo, tiporequest);                //Chamada da função com os devidos argumentos
            		Serial.println(statusReqLight);                        //Envio de dados para o Raspberry
                Serial.println(statusReqLUM);                          //Envio de dados para o Raspberry
        	}

        	else if (estadoPretendido == 'z'){
            		Serial.println(estadoMestre);
                Serial.println(255);
        	}
         
         else if (estadoPretendido == 'S'){
                byte setPoint;
                byte setPointTemp;
                portaControlo = 1;                                //Só é feito paara a Slave 1
                typeCont = 4;
                int n = 0;
                
                while(n < 2){                                     //Receber mais dois digitos para formar o set point da temperatura
                      if (n == 0)                                 //Necessária uam conversão para byte
                          setPointTemp = (Serial.read())*10;
                      else if (n == 1)
                          setPointTemp += Serial.read();
                  
                   n++;
                 }
                setPoint = byte(setPointTemp - 16);
                transmitData(portaControlo, typeCont, setPoint);
          }
          
          /*else if (estadoPretendido == 's'){                     //Processo para outras Slaves
                Serial.println("Ready");
                 byte setPoint;
                byte setPointTemp;
                typeCont = 4;
                portaControlo = 2;
                int n = 0;
                while(n < 2){
                        if (n == 0)
                          setPointTemp = (Serial.read())*10;
                      else if (n == 1)
                          setPointTemp += Serial.read();
                  setPoint = byte(setPointTemp - 16);
                   n++;
                 }
                Serial.println(setPoint);
          }*/
        }
    digitalWrite(rele, estadoMestre);        //Rele colocado com o estado definido pela variável
  	delay(100);                              //Pequeno delay

}
