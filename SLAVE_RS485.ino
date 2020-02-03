/*************Bibliotecas necessárias****************/
#include <SoftwareSerial.h>
#include "RS485_protocol.h"
#include "Arduino.h"
#include <IRremote.h>
#include <DHT.h>

/***********Variáveis***********/
byte estadoLUZ;
int valueLDR_qt;
int valueLDR_lamp = 0;
byte cnt_noite = 0;
unsigned long tempo_lig;
unsigned long tempo_temp;
byte flag_stop = 0;
byte temp;
byte estadoIR;
byte tempControl = 0;
byte setPointTemp;
byte valueLUM;
byte end_Slave = 1;

/******Atribuição de nomes aos pinos************/
#define rele 6
#define IR 5
#define LDR_qt A0
#define LDR_lamp A1
#define PIR 8

/*******Inicialização Infravermelho**********/

IRrecv irrecv(IR);
decode_results results;

/*******Inicialização Temperatura************/
#define DHTPIN 11
#define DHTTYPE 22
DHT dht(DHTPIN, DHTTYPE);

/*******RS-485**********/
SoftwareSerial rs485 (2, 3);  //RX, TX <-> RO, DI
const byte ENABLE_PIN = 4;      //Pino conectado ao RE e DE

void fWrite (const byte what){  //Função responsável pelo envio dos dados
	rs485.write (what);
}

int fAvailable (){              //Função responsável pela verificação de disponibilidade do RS-485
  	return rs485.available ();
}

int fRead (){                   //Função responsável pela receção dos dados
  	return rs485.read ();
}

/**********Função de interpretação de dados recebidos (RS-485)************/
void processInfo() {
  	byte buf [10];      //Vetor que armazena os dados da mensagem

  	byte received = recvMsg (fAvailable, fRead, buf, sizeof (buf));   //Chamada da função que tratará dos dados recebidos, formando a mensagem enviada

  	if (received) {                          //Se recebeu ...
    		if (buf [0] != end_Slave)            //Verifica se o byte correspondente ao endereço de destino corresponde ao seu
        		return;                          //Se não for sai da função       
                                             //Se for...
    		if (buf[1] == 1) {                   //Ação de controlo corresponde ao pedido do estado da iluminação?

        		estadoLUZ = digitalRead(rele);      //Lê o estado do relé
           
        		/*Serial.print(buf[1]);          //Apresentaria esse valor no "Monitor Série" para verificação
        		Serial.print("\n");
        		Serial.print(estadoLUZ);
        		Serial.print("\n");*/

            /*****Composição da mensagem******/
            
        		byte msg[] = {                  
            			0,               //Primeiro byte corresponde ao endereço do master
            			3,               //Byte referente ao ACK (espécie de confirmação)
            			estadoLUZ,       //Byte com dados sobre o estado da Luz
                  valueLUM         //Byte com valor da luminosidade
        		};

            /********Envio de dados********/
        		delay (1);                          //Tempo de espera para o master se preparar (ativar o modo de receção)
        		digitalWrite (ENABLE_PIN, HIGH);    //Ativar modo de envio
        		sendMsg (fWrite, msg, sizeof msg);  //Chamada de função responsável pelo tratamento de dados antes do envio (adiciona STX e ETX e inverte os bytes a enviar de forma a prevenir corrupção de dados)
        		delayMicroseconds (660);            //Tempo de espera para envio                     
        		digitalWrite (ENABLE_PIN, LOW);     //Ativa modo de receção
    		}

    		else if (buf [1] == 2) {                //Ação de controlo corresponde ao controlo da iluminação?

            /*****Composição da mensagem******/
        		byte msg [] = {
            			0,          //Primeiro byte corresponde ao endereço do master
            			3           //Byte referente ao ACK (espécie de confirmação)
        		};

            /********Envio de dados********/
        		delay (1);                           //Tempo de espera para o master se preparar (ativar o modo de receção)
        		digitalWrite (ENABLE_PIN, HIGH);     //Ativar modo de envio
        		sendMsg (fWrite, msg, sizeof msg);   //Chamada de função responsável pelo tratamento de dados antes do envio (adiciona STX e ETX e inverte os bytes a enviar de forma a prevenir corrupção de dados)
        		delayMicroseconds (660);             //Tempo de espera para envio   
        		digitalWrite (ENABLE_PIN, LOW);      //Ativa modo de receção

        		digitalWrite (rele, buf [2]);  //Coloca o relé com o estado que foi recebido -> 3º byte da mensagem recebida -> buf[2]
        		/*Serial.println(buf[2]);*/    //Mostra no "Monitor Série" o que foi recebido        

			      /*byte infoLUZ = buf[2];                      //Este ciclo iria ser usado no caso de ser necessário a comutação de escada
          		  if(infoLUZ != estadoLUZ){                 //O 3º byte iria ser atribuído à variável infoLUZ, sendo este o valor pretendido para a iluminação
            			  if(digitalRead(rele) == 0){           //Se o valor pretendido fosse diferente do valor atual (que seria lido através da luminosidade da lâmpada e não do estado do relé)
              				  digitalWrite(rele, 1);            //O estado do relé seria invertido de forma a mudar o estado da luz 
            			  }
                   
            		    else if(digitalRead(rele) == 1){
              				digitalWrite(rele, 0); 
            			  } 
          		 }*/
    		}

    	  else if (buf[1] == 3){              //Ação de controlo corresponde ao pedido da temperatura?

			      /*****Composição da mensagem******/
			      byte msg [] = {
  				      0,               //Primeiro byte corresponde ao endereço do master
                3,               //Byte referente ao ACK (espécie de confirmação)
  				      temp             //Byte com valor da temperatura
			      };

            /********Envio de dados********/
            delay (1);                           //Tempo de espera para o master se preparar (ativar o modo de receção)
            digitalWrite (ENABLE_PIN, HIGH);     //Ativar modo de envio
            sendMsg (fWrite, msg, sizeof msg);   //Chamada de função responsável pelo tratamento de dados antes do envio (adiciona STX e ETX e inverte os bytes a enviar de forma a prevenir corrupção de dados)
            delayMicroseconds (660);             //Tempo de espera para envio   
            digitalWrite (ENABLE_PIN, LOW);      //Ativa modo de receção
		    }

        else if (buf[1] == 4){              //Ação de controlo corresponde ao controlo da temperatura?

            byte msg [] = {
                  0,          //Primeiro byte corresponde ao endereço do master
                  3           //Byte referente ao ACK (espécie de confirmação)
            };

            /********Envio de dados********/
            delay (1);                           //Tempo de espera para o master se preparar (ativar o modo de receção)
            digitalWrite (ENABLE_PIN, HIGH);     //Ativar modo de envio
            sendMsg (fWrite, msg, sizeof msg);   //Chamada de função responsável pelo tratamento de dados antes do envio (adiciona STX e ETX e inverte os bytes a enviar de forma a prevenir corrupção de dados)
            delayMicroseconds (660);             //Tempo de espera para envio   
            digitalWrite (ENABLE_PIN, LOW);      //Ativa modo de receção

            setPointTemp = buf[2];               //Coloca a variável setPointTemp (valor do set point da temperatura desejada) com o valor recebida
            tempControl = 1;                     //Ativa a flag para o controlo
        }

    		else                  //Se receber outro comando, sai...
        		return;
  	}
}

/****Leitura da luminosidade da lâmpada*******/
void leituraLAMP(){
  	valueLDR_lamp = analogRead(LDR_lamp);   //Leitura do valor
  
  	if (valueLDR_lamp > 800)                //Se este valor for maior que 800, é porque a lâmpada está ligada (ON - 1)
      		estadoLUZ = 1;
  	else                                    //Se este valor for menor ou igual a 800, é porque a lâmpada está desligada (OFF - 0)
      		estadoLUZ = 0;
}

/*******Função para verificação de movimento durante a noite*********/
void funcNoite(){
    	if (digitalRead(PIR) == HIGH){                                //Se detetou movimento..
          /*Serial.println("Movimento detetado!");*/                //Escreve no "Monitor Série" uma mensagem
          
      		if(cnt_noite == 0 && digitalRead(rele) == 1){             //Dupla cofirmação... Servirá para alterar o estado de relé e apenas permitir que essa alteração seja feita uma vez
              /*Serial.println("Alterar estado do rele!");*/        //Escreve no "Monitor Série" uma mensagem
          		digitalWrite(rele, 0);                                //Em caso do relé estar a 1, mete-o a 0
      		}
      		else if (cnt_noite == 0 && digitalRead(rele) == 0){       //Dupla cofirmação... Servirá para alterar o estado de relé e apenas permitir que essa alteração seja feita uma vez
              /*Serial.println("Alterar estado do rele!");*/        //Escreve no "Monitor Série" uma mensagem
          		digitalWrite(rele, 1);                                //Em caso do relé estar a 0, mete-o a 1
      		}
         
          /*Serial.println("Inicializar contagem!");*/
      		flag_stop = 0;                                            //Flag que impedirá que o código a executar quando não exista movimento, só o faça se sentir movimento anteriormente
      		cnt_noite = 1;                                            //Flag que indica que o estado do relé já foi alterado uma vez
      		tempo_lig = millis();                                     //Sempre que sentir movimento, a contagem é reinicializada
          /*Serial.println(tempo_lig);*/                            //Escreve no "Monitor Série" o tempo de início da contagem
    	}
    
    	else if(digitalRead(PIR) == LOW && flag_stop == 0){                           //Se não detetou movimento
          /*Serial.println("Movimento não detetado!");*/                            //Escreve no "Monitor Série" uma mensagem
      		if((millis() - tempo_lig) >= 20000){                                      //Se passaram 20 segundos...
              /*Serial.println("Passaram 20 segundos. Alterar estado do relé!");*/  //Escreve no "Monitor Série" uma mensagem
          		cnt_noite = 0;                                                        //Altera a flag de forma a permitir que o estado seja alterado novamente se sentir movimento (ciclo terminado)
          		flag_stop = 1;                                                        //Flag colocada de forma a não permitir que este ciclo seja executado sem que movimento tenha sido sentido
          
          		if (digitalRead(rele) == 1)                                           //Altera o estado do relé
              			digitalWrite(rele, 0);
          		else if (digitalRead(rele) == 0)
              			digitalWrite(rele, 1);
       		}
    	}
}

/*******Função para verificação de movimento durante o dia*********/
void funcDia(){
    if (digitalRead(PIR) == HIGH){                          //Se sentiu movimento...
        tempo_lig = millis();                               //Reinicializa contagem
    }
  
    else if(digitalRead(PIR) == LOW){                       //Se não sentir movimento...
        if((millis() - tempo_lig) >= 30000){                //E se passaram 30 segundos...
            if (digitalRead(rele) == 1 )                    //Altera o estado do relé
                digitalWrite(rele, 0);
            else if (digitalRead(rele) == 0 )
                digitalWrite(rele, 1);
        }
    }
}

/******Função para leitura e interpretação de sinais infravermelhos******/
void leituraIR(){
    if (irrecv.decode(&results)){                               //Verifica a receção de algum sinal
      
      /*Serial.print(results.value, HEX);*/                     //Mostra o valor recebido, em hexadecimal, no "Monitor Série"
      
        if (results.value == 0xA90){                            //Se o sinal recebido corresponde a 0xA90
            /*Serial.print(" - Alterar estado da luz \n");*/    //Escreve a função no "Monitor Série"
    	      if(estadoIR == 0)                                   //Ativa flag para que o sensor de movimento não interfira
    		        estadoIR == 1;
    	      else if (estadoIR == 1)                             //Desativa flag para que o sensor de movimento seja responsável pelo controlo da iluminação
    		        estadoIR=0;
         
            if (digitalRead(rele) == 1)                         //Altera o estado do relé
                digitalWrite(rele, 0);
            else if (digitalRead(rele) == 0)
                digitalWrite(rele,1 );
        }

        else if (results.value == 0x90){                        //Se o sinal recebido corresponde a 0x90
            /*Serial.print(" - Incrementar Set Point \n");*/    //Escreve a função no "Monitor Série"
            tempControl = 1;                                    //Ativa a flag para controlo de temperatura
            setPointTemp += 1;                                  //Incrementa o set point
        }
      
        else if (results.value == 0x0890){                      //Se o sinal recebido corresponde a 0x890
            /*Serial.print(" - Decrementar Set Point \n");*/    //Escreve a função no "Monitor Série"  
            tempControl = 1;                                    //Ativa a flag para controlo da temperatura
            setPointTemp -= 1;                                  //Decrementa o set point
        }
      
        else if (results.value == 0xA70){                             //Se o sinal recebido corresponde a 0xA70
            /*Serial.print(" - Desligar Controlo Temperatura \n");*/  //Escreve a função no "Monitor Série"
            tempControl = 0;                                          //Desativa a flag para controlo da temperatura
        }
    
        irrecv.resume();                                        //Continua com a leitura de sinais
    }
}

/*******Função para leitura da temperatura e simulação de controlo*******/
void leituraTemp(){
  	temp = dht.readTemperature();                               //Leitura da temperatura
    if (tempControl == 0)                                       //Se a flag de controlo não estiver ativa...
        setPointTemp = temp;                                    //Coloca o valor do set point igual ao da temperatura
        
    else if (tempControl == 1){                                 //Se a flag de controlo estiver ativa...
        /*Serial.println("Controlo da Temperatura Ativo!");*/   //Escreve no "Monitor Série" uma mensagem
        
        if (setPointTemp > temp)                                        //Set point maior que a temperatura?
            Serial.println("Aquecendo");                               //A temperatura tem que aquecer
        else if (setPointTemp < temp)                                   //Set point menor que a temperatura?
            Serial.println("Arrefecendo");                             //A temperatura tem que arrefecer
        else if (setPointTemp ==  temp)                                 //Set point igual à temperatura
            Serial.println("Temperatura desejada! Mantendo ... ");     //A temperatura tem que manter
    }                                                                   //Isto corresponde a uma simulação, podendo ser adaptado posteriormente
}

/************Inicializações************/
void setup(){
    Serial.begin(9600);           //Inicialização da comunicação série (UART)
    rs485.begin (28800);           //Inicialização para transmissão de dados RS-485 
    pinMode (ENABLE_PIN, OUTPUT);  //Colocar o pino responsável pelo estado do módulo MAX485 como saída
    pinMode(rele, OUTPUT);         //Colocar o pino responsável pelo estado do relé como saída
    irrecv.enableIRIn();           //Inicialização de leitura do sensor IR
    dht.begin();                   //Inicialização de leitura do sensor DHT
}

/*************LOOP***************/
void loop(){
    leituraIR();                                  //Chamada da função para leitura do sensor de infravermelhos
    
    /*estadoLAMP();*/                             //Chamada da função para leitura da luminosidade da lâmpada, se necessário
  
    valueLDR_qt = analogRead(LDR_qt);             //Leitura da luminosidade da secção
    float valueM = valueLDR_qt /float(1023);      //Transformação em percentagem
    valueLUM = valueM * byte(100);
    
    /*Serial.print("Luminosidade: ");             //Apresentar o valor percentual da percentagem no "Monitor Série"
    Serial.print(valueLUM);
    Serial.print("%\n");*/
    
    if(valueLDR_qt <=250 || cnt_noite == 1){      //Se a luminosidade da secção é menor ou igual a 250 (noite) ou a flag cnt_noite está ativa (sentiu movimento, o que levará a que a luz seja ligada e a luminosidade alterada)
	      if(estadoIR == 0)                         //Se o estado da luz não tiver sido definida pelo utilizador
        	  funcNoite();                          //Chama a função
    }

    if(valueLDR_qt >250 && digitalRead(rele) == 1){   //Se a luminosidade da secção é maior que 250 e o relé está ativo
	      if(estadoIR = 0)                              //Se o estado da luz não tiver sido definido pelo utilizador
        	  funcDia();                                //Chama a função
    }
        
    leituraTemp();                  //Chama a função para a leitura da temperatura
   	
    processInfo();                  //Chama a função para leitura e interpretação de dados recebidos no barramento

}
