import matplotlib.pyplot as plt

#Abrimos el archivo con los datos
f = open("salidaProc.txt", "r")

numHilos = 2


# Hilos afines
tamP = []
tamC = []

R_C = [[] for _ in range(numHilos)]
R_P = [[] for _ in range(numHilos)]

W_C = [[] for _ in range(numHilos)]
W_P = [[] for _ in range(numHilos)]

RW_C = [[] for _ in range(numHilos)]
RW_P = [[] for _ in range(numHilos)]

line = f.readline()
while line:

    #Obtenemos el tamaño de los datos privados
    tamP.append(int(line.split(":")[1]))
    #Obtenemos el tamaño de los datos compartidos
    line = f.readline()
    tamC.append(int(line.split(":")[1]))
    
    if tamP[-1] == 40000000:
        break
    
    for i in range(0, numHilos):
        line = f.readline()
        tiempos = line.split(" ")
        
        R_P[i].append(float(tiempos[0]))
        R_C[i].append(float(tiempos[1]))

        W_P[i].append(float(tiempos[2]))
        W_C[i].append(float(tiempos[3]))

        RW_P[i].append(float(tiempos[4]))
        RW_C[i].append(float(tiempos[5]))

    
    line = f.readline()

#Graficamos los datos
for i in (0, numHilos):
    plt.plot(tamP, R_P[i], label='Hilo ' + str(i))
plt.savefig('afines/R_P.png')

for i in (0, numHilos):
    plt.plot(tamP, R_C[i], label='Hilo ' + str(i))
plt.savefig('afines/R_C.png')

for i in (0, numHilos):
    plt.plot(tamP, W_P[i], label='Hilo ' + str(i))
plt.savefig('afines/W_P.png')

for i in (0, numHilos):
    plt.plot(tamP, W_C[i], label='Hilo ' + str(i))
plt.savefig('afines/W_C.png')

for i in (0, numHilos):
    plt.plot(tamP, RW_P[i], label='Hilo ' + str(i))
plt.savefig('afines/RW_P.png')

for i in (0, numHilos):
    plt.plot(tamP, RW_C[i], label='Hilo ' + str(i))
plt.savefig('afines/RW_C.png')

# Hilos no afines

# # Add labels and title
# plt.xlabel('X axis')
# plt.ylabel('Y axis')
# plt.title('Multiple Y Data Plot')

# # Add a legend
# plt.legend()

# # Display the plot
# plt.show()




# tamP = []
# tamC = []

# R_C = [[] for _ in range(numHilos)]
# R_P = [[] for _ in range(numHilos)]

# W_C = [[] for _ in range(numHilos)]
# W_P = [[] for _ in range(numHilos)]

# RW_C = [[] for _ in range(numHilos)]
# RW_P = [[] for _ in range(numHilos)]
# #Graficamos los datos

# f.close()