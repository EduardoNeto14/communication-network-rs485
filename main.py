'''Bibliotecas necessárias'''
import serial
import time
import smtplib
import string
from random import *
import configData as md

from pip._vendor.distlib.compat import raw_input

print("\nCONTROLO DA HABITAÇÃO \n\n")
decision = raw_input("SIM - Continuar\nQualquer argumento - Sair\nIndique o que pretende: ")

if(decision != "SIM"):                                                                                                                    #Se o valor introduzido pelo utilizador for diferente de "SIM"...
    exit(0)                                                                                                                               #O programa é terminado

try:
    arduino = serial.Serial("/dev/ttyACM0", 9600)                                                                                         #Tenta iniciar comunicação série com Arduino
    time.sleep(2)
except:                                                                                                                                   #Se não for possível, o programa termina
    print("\nO controlo da sua habitação não se encontra disponível! \n\n  Verifique as ligacoes entre Arduino e Raspberry \n " )
    exit(0)

stateDict = {"ON" : 0, "OFF" : 1, "Indisponível" : 2}                                                                                      #Dicionário com key e valor correspondente
sectionArray = ["Quarto", "Cozinha", "Casa de Banho", "Sala"]                                                                              #Vetor com nome das secções
sectionNumber = [1, 2, 3, 4]                                                                                                               #Vetor com as secções possíveis para visualização e controlo da iluminação
relayData = [None, None, None, None]                                                                                                       #Vetores com valores inicializados a nulo
lumData = [None, None, None, None]
controlLightArray =[[b'A', b'a'], [b'B', b'b'], [b'C', b'c'], [b'D', b'd']]                                                                #Códigos referentes ao controlo da iluminação
reqLightArray = [b'w', b'x', b'y', b'z']                                                                                                   #Códigos referentes ao request da iluminação
controlTempArray = [b'S', b's']                                                                                                            #Códigos referentes ao controlo da Temperatura
reqTempArray = [b'T', b't']                                                                                                                #Códigos referentes ao request da temperatura
sectionTempArray = [1]                                                                                                                     #Vetor com as secções possíveis para visualização e controlo da temperatura

'''Função responsável por formar password aleatória e enviar para mail'''
def mailPassword():

    '''Numero minimo e máximo de carateres na password'''
    min_char = 4
    max_char = 7
    '''Número de tentativas para acertar a passowrd iniciado a 0'''
    nr_pass = 0

    elements = string.ascii_lowercase + string.ascii_uppercase + string.digits              #Elementos que podem fazer parte da password (letras minusculas, maiusculas e numeros)
    password = "".join(choice(elements) for x in range(randint(min_char, max_char)))        #Para um tamanho aleatorio escolhido, um caracter é aleatoriamente escolhido para a passowrd

    server_ssl = smtplib.SMTP_SSL('smtp.gmail.com', 465)                                    #Iniciar conexão com servidor de email
    server_ssl.ehlo()                                                                       #Indentificação ao servidor
    server_ssl.login(md.EMAIL, md.PASSWORD)                                                 #Login
    msg = password                                                                          #Mensagem será a password gerada
    server_ssl.sendmail(md.EMAIL, md.EMAIL, msg)                                            #Envio de email
    server_ssl.quit()                                                                       #Fechar conexão

    pass_mail = raw_input("Indroduza o código: ")                                           #Indroduzir password

    while (pass_mail != password):                                                          #Ciclo de 3 tentativas
        nr_pass += 1
        if nr_pass == 3:
            print("Número de tentativas excedido!")
            exit(0)
        pass_mail = raw_input("Introduza o código: ")

'''Função main que apresentará as ações de controlo de possíveis. Dependendo do que for introduzido
pelo utilizador, haverá um reencaminhamento para outra função.
Se for introduzido 0, o programa terminará.
Existe uma verificação de erro para a introdução de dados'''

def Main():
    print("\nMENU PRINCIPAL! \n")
    print("1 - Informação da Iluminação \n")
    print("2 - Informação da Temperatura \n")
    print("3 - Controlo da Iluminação \n")
    print("4 - Controlo da Temperatura \n")
    print("0 - Sair\n")

    mode = None
    while mode is None:
        try:
            mode = int(raw_input('\nAção desejada:'))
        except:
                print("Comando inválido! Tente novamente, por favor...\n")
                pass

    if (mode == 1):
        comm_req_Light()

    elif (mode == 2):
        comm_req_Temp()

    elif (mode == 3):
        menuControLight()

    elif (mode == 4):
        menuControlTemp()

    elif (mode == 0):
        exit(0)

    else:
        Main()

'''Esta função apresenta o menu para o controlo de temperatura.
Primeiro é pedida a secção.
Posteriormente é pedido o valor da temperatura desejada.
Existe verificação de erro na introdução de dados.
Validação dos dados introduzidos (se é possível controlar determinada secção).
O valor da temperatura, será sempre o valor absoluto.'''

def menuControlTemp():
    print("\nCONTROLO DA TEMPERATURA \n")
    print("1 - Quarto")
 #  print("2 - Section 2")

    print("\n0 - MENU INICIAL\n")

    mode = None
    while mode is None:
        try:
            mode = int(raw_input('Ação desejada:'))
        except:
            print("Comando inválido! Tente novamente, por favor...\n")
            pass

    valid = False
    comp = mode

    for x in sectionTempArray:
        if (x == comp):
            valid = True

    if (valid == True):

        mode = None
        while mode is None:
            try:
                mode = int(raw_input('Secção desejada:'))
            except:
                print("Comando inválido! Tente novamente, por favor...\n")
                pass

        value = abs(mode)
        comm_control_Temp(comp, value)

    elif (valid == False):
        if (comp == 0):
            Main()
        else:
            print("Argumento não é válido!\n")
            menuControlTemp()


'''Menu para controlo da iluminação.
Primeiro é pedida a secação, sendo então apresentados os dados recolhidos sobre a mesma.
De seguida, é pedido o estado pretendido.
Só é permitido executar o controlo se tiver sido feito um pedido de dados anteriormente.
Existe verificação de erros aquando da introdução das opções.
Validação dos dados introduzidos (se é possível...)'''

def menuControLight():
    global relayData
    print("\nCONTROLO DA ILUMINAÇÃO \n")
    print("1 - Quarto\n")
    print("2 - Cozinha\n")
    print("3 - Casa de Banho\n")
    print("4 - Sala\n")

    print("\n0 - MENU INICIAL\n")

    mode = None
    while mode is None:
        try:
            mode = int(raw_input('\nAção desejada:'))
        except:
            print("Comando inválido! Tente novamente, por favor...\n")
            pass

    comp = mode
    valid = False

    for x in sectionNumber:
        if x == comp:
            valid = True

    if (valid == True):

        if relayData[comp - 1] == None:
            print("\nTem que pedir informação da iluminação primeiro...\n")
            Main()
        elif (relayData[comp - 1] == 2):
            print("\nIndisponível para controlo...\n")
            Main()
        else:
            print("\nA luz está a: " + str(relayData[comp - 1]) + ".Para mudar, envie o inverso!\n")
            print("\nA luminosidade da secção é " + str(lumData[comp - 1]) + " %\n")

        mode = None

        while mode is None:
            try:
                mode = int(raw_input('\nEstado pretendido:'))
            except:
                print("\nComando inválido! Tente novamente, por favor...\n")
                pass

        value = mode
        comm_control_Light(comp, value)

    elif(valid == False):
        if (comp == 0):
            Main()
        else:
            print("Argumento não é válido!\n")
            menuControLight()

'''Função responsável pelo envio dos códigos e receção dos dados do Arduino (iluminação).
Realiza um ciclo for para o número de secções possíves em que envia o código correspondente e recebe a informação da mesma.
Esta informação é validada e apresentada.'''
def comm_req_Light():
    global relayData
    relayData = []
    global lumData
    lumData = []

    for porta in range(1, 5):
        print("\nAguarde...\n")

        arduino.write(reqLightArray[porta-1])

        time.sleep(0.7)
        relayStatus = arduino.readline()
        lumStatus = arduino.readline()

        if (int(lumStatus) == 255):
            lumData.append(None)
            print("A informação da luminosidade não se encontra disponível...\n")

        elif (int(lumStatus) != 255):
            print("Luminosidade: ", str(int(lumStatus)) + "%\n")
            lumData.append(int(lumStatus))

        for stateName, stateValue in stateDict.items():
            if stateValue == int(relayStatus):
                print("Secção: " + sectionArray[porta - 1] + " -> " + stateName + "\n")

        relayData.append(int(relayStatus))

    Main()

'''Função responsável pelo envio dos códigos e receção dos dados do Arduino (temperatura).
Realiza um ciclo for para o número de secções possíves em que envia o código correspondente e recebe a informação da mesma.
Esta informação é validada e apresentada.'''
def comm_req_Temp():
    tempData = []

    for porta in range(1, 2): #passar para range(1, 3)
        print("\nAguarde...\n")

        arduino.write(reqTempArray[porta-1])

        time.sleep(0.7)
        tempStatus = arduino.readline()
        if(tempStatus == 255):
            print("Indisponível!")
        else:
            print("Secção: " + sectionArray[porta-1] + " -> " + "Temperatura: " + str(int(tempStatus)) + " graus Celsius\n")

        tempData.append(int(tempStatus))

    Main()

'''Função para controlo da iluminação.
Realiza uma ultima verificação antes do envio do código correspondente.'''
def comm_control_Light(section, state):
    global relayData
    valid = False

    for x in sectionNumber:
        if x == section:
            valid = True

    if (valid == True):
        if ((state == 0) or (state == 1)):
            arduino.write(controlLightArray[section-1][state])
            relayData[section - 1] = state
            print("\nControlo enviado com sucesso!\n")
            menuControLight()
        else:
            print("\nArgumento inválido!")
            menuControLight()

    elif (valid == False):
        print("\nAlguma coisa deu errado! Verifique os argumentos\n")
        menuControLight()

'''Função para controlo da temperatura.
Envia o código correspondente e a temperatura'''
def comm_control_Temp(section, temp):

    arduino.write(controlTempArray[section-1])
    arduino.write(temp)

    print("\nControlo enviado com sucesso\n!")
    menuControlTemp()

if __name__ == '__main__':
    mailPassword()
    Main()