#!/usr/bin/env python

# public modules -----------------------------------------------------------------------------------------------------------------------
from tkinter import *
from tkinter import ttk
from tkinter import messagebox as msg
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import serial

# default values -----------------------------------------------------------------------------------------------------------------------
defaultTriggerType = "AUTO"
defaultTriggerLevel = 128
defaultTriggerPosition = 128
defaultTimeInterval = 0.256		# defaultTimeInterval = 256 * defaultSamplingPeriod = 256 / 1000 Hz

# initialization -----------------------------------------------------------------------------------------------------------------------
TP = defaultTriggerPosition
TL = defaultTriggerLevel
TT = defaultTriggerType
TI = defaultTimeInterval
lastTimeUnit = "s"
xCH0 = []
yCH0 = []
xCH1 = []
yCH1 = []
for i in range(0, 256):
	xCH0.append(i)
	yCH0.append(0)
	xCH1.append(i)
	yCH1.append(0)

# FUNCTIONS ---------------------------------------------------------------------------------------------------------------------------
# -------------------------------------------------------------------------------------------------------------------------------------
def drawGraph():
	global yCH0
	global yCH1
	# Eliminazione grafico precedente:
	plottingFigure.clear()
	# Creazione di un grafico all'interno della finestra di disegno:
	graph = plottingFigure.add_subplot(autoscaley_on = False, ybound = 256)
	# Tracciamento del grafico:
	graph.plot(xCH0, yCH0)
	graph.plot(xCH1, yCH1)
	plottingFigure_canvas.draw()
# -------------------------------------------------------------------------------------------------------------------------------------
def updateTL():
	global TL
	global serialCom
	error = False
	try:
		loc_TL = int(triggerLevel_val.get())
		if((loc_TL < 0) or (loc_TL > 255)):
			error = True
	except ValueError:
		error = True
	if(error == True):
		# Apparizione messaggio di errore:
		msg.showerror(title = "ERROR", message = "Invalid input")
		# Riassegnamo il valore precedente:
		triggerLevel.set(TL)
	else:
		# Aggiornamento al nuovo valore:
		TL = loc_TL
		# Invio parametro su porta seriale:
		TL_opcode = b''.join([b'*TL', TL.to_bytes(1, byteorder = "big") , b'#'])	# Creazione vettore di bytes
		serialCom.write(TL_opcode)
# -------------------------------------------------------------------------------------------------------------------------------------
def updateTP():
	global TP
	error = False
	try:
		loc_TP = int(triggerPosition_val.get())
		if((loc_TP < 0) or (loc_TP > 255)):
			error = True
	except ValueError:
		error = True
	if(error == True):
		# Apparizione messaggio di errore:
		msg.showerror(title = "ERROR", message = "Invalid input")
		# Riassegnamo il valore precedente:
		triggerPosition.set(TP)
	else:
		# Aggiornamento al nuovo valore:
		TP = loc_TP
# -------------------------------------------------------------------------------------------------------------------------------------
def updateTI():
	global TI
	global lastTimeUnit
	error = False
	try:
		loc_TI = float(timeInterval_val.get())
		samplingPeriod = loc_TI / 256.0
		newTimeUnit = timeUnit.get()
		if(newTimeUnit == "s"):
			samplingPeriod = samplingPeriod * 1000000000
		elif(newTimeUnit == "ms"):
			samplingPeriod = samplingPeriod * 1000000
		elif(newTimeUnit == "us"):
			samplingPeriod = samplingPeriod * 1000
		samplingPeriod = int(samplingPeriod)
		if((samplingPeriod > 100000000) or (samplingPeriod < 10000)):
			error = True
	except ValueError:
		error = True
	if(error == True):
		# Apparizione messaggio di errore:
		msg.showerror(title = "ERROR", message = "Invalid input")
		# Riassegnamo i valori precedenti:
		timeInterval.set(TI)
		if(lastTimeUnit == "s"):
			timeUnit_rdbtn_s.state(["selected"])
			timeUnit_rdbtn_ms.state(["!selected"])
			timeUnit_rdbtn_us.state(["!selected"])
			timeUnit_rdbtn_ns.state(["!selected"])
		elif(lastTimeUnit == "ms"):
			timeUnit_rdbtn_s.state(["!selected"])
			timeUnit_rdbtn_ms.state(["selected"])
			timeUnit_rdbtn_us.state(["!selected"])
			timeUnit_rdbtn_ns.state(["!selected"])
		elif(lastTimeUnit == "us"):
			timeUnit_rdbtn_s.state(["!selected"])
			timeUnit_rdbtn_ms.state(["!selected"])
			timeUnit_rdbtn_us.state(["selected"])
			timeUnit_rdbtn_ns.state(["!selected"])
		elif(lastTimeUnit == "ns"):
			timeUnit_rdbtn_s.state(["!selected"])
			timeUnit_rdbtn_ms.state(["!selected"])
			timeUnit_rdbtn_us.state(["!selected"])
			timeUnit_rdbtn_ns.state(["selected"])
		else:
			mainWindow.quit()	# qualcosa è andato storto, fermiamo il programma
	else:
		# Aggiornamento al nuovo valore:
		TI = loc_TI
		lastTimeUnit = newTimeUnit
		# Invio parametro su porta seriale:
		SP_opcode = b''.join([b'*SP', samplingPeriod.to_bytes(4, byteorder = "big") , b'#'])	# Creazione vettore di bytes
		serialCom.write(SP_opcode)
# -------------------------------------------------------------------------------------------------------------------------------------
def updateTT():
	global TT
	TT = triggerType_cbox.get()
	if(TT == "AUTO"):
		mode = 0
	elif(TT == "NORMAL"):
		mode = 1
	elif(TT == "SINGLE"):
		mode = 2
	elif(TT == "STOP"):
		mode = 3
	# Invio parametro su porta seriale:
	TT_opcode = b''.join([b'*TT', mode.to_bytes(1, byteorder = "big") , b'#'])	# Creazione vettore di bytes
	serialCom.write(TT_opcode)
# -------------------------------------------------------------------------------------------------------------------------------------
def serialLoop():
	bufferCH0 = []
	bufferCH1 = []
	global yCH0
	global yCH1
	flag = 0
	# Ricezione del carattere '#'
	startByte = serialCom.read(1)
	if(str(startByte) == "b''"):	# Nessun byte è stato ricevuto entro il timeout
		flag = 1
		mainWindow.after(10, serialLoop)
	elif(str(startByte) == "b'*'"):
		flag = 1
		# Ricezione dei dati:
		dataBuffer = serialCom.read(1022)
		stopByte = serialCom.read(1)
		if(str(stopByte) == "b'#'"):
			# Separazione buffer:
			bufferCH0.clear()
			bufferCH0.clear()
			for i in range(0, 1022, 2):
				bufferCH0.append(dataBuffer[i])
				bufferCH1.append(dataBuffer[i + 1])
			# Aggiornamento grafico:
			yCH0.clear()
			yCH1.clear()
			for j in range(255 - TP, 511 - TP):
				yCH0.append(bufferCH0[j])
				yCH1.append(bufferCH1[j])
			drawGraph()
			# Lanciamo subito una nuova ricezione:
			mainWindow.after(0, serialLoop)
	# Se vengono ricevuti byte non racchiusi tra *# dobbiamo ignorarli, ma bisogna comunque iniziare una nuova ricezione
	if(flag == 0):
		mainWindow.after(0, serialLoop)
# -------------------------------------------------------------------------------------------------------------------------------------

# MAIN --------------------------------------------------------------------------------------------------------------------------------

# Apertura porta seriale (TIMEOUT è espresso in secondi e deve essere abbastanza grande da permettere la ricezione di 1022 bytes)
# serialCom = serial.Serial(port = "/dev/ttyUSB0", baudrate = 115200, bytesize = 8, timeout = 0.2)
serialCom = serial.Serial(port = "/dev/ttyACM0", baudrate = 115200, bytesize = 8, timeout = 0.1)	# ((1022+1+1)*8) / 115200 = 0.072

# Creazione della finestra principale:
mainWindow = Tk()
mainWindow.title("DSO")

# Creazione del frame principale all'interno della finestra:
mainFrame = ttk.Frame(mainWindow, padding = "0 0 20 0")
mainFrame.grid(column = 1, row = 0)

# Creazione delle stringhe necessarie:
triggerLevel = StringVar(value = defaultTriggerLevel)
triggerPosition = StringVar(value = defaultTriggerPosition)
timeInterval = StringVar(value = defaultTimeInterval)
timeUnit = StringVar()
triggerType = StringVar()

# Creazione oggetti di selezione per triggerLevel:
triggerLevel_val = ttk.Entry(mainFrame, width = 10, textvariable = triggerLevel)							# Casella di ingresso
triggerLevel_val.grid(column = 3, row = 0, sticky = (N, W, E, S), pady = 10)
ttk.Label(mainFrame, text = " Trigger Level ").grid(column = 2, row = 0, sticky = E, pady = 10)				# Etichetta
TL_updateButton = ttk.Button(mainFrame, text = "Update TL", command = updateTL)								# Bottone di update
TL_updateButton.grid(column = 4, row = 0, sticky = (N, W, E, S), pady = 10)

# Creazione oggetti di selezione per triggerType:
triggerType_cbox = ttk.Combobox(mainFrame, width = 10, textvariable = triggerType, state = "readonly")		# Menu a tendina senza entry generica
triggerType_cbox["values"] = ("AUTO", "NORMAL", "SINGLE", "STOP")											# Scelte possibili
triggerType_cbox.set(defaultTriggerType)																	# Impostazione default value
triggerType_cbox.grid(column = 3, row = 3, sticky = (N, W, E, S), pady = 10)
ttk.Label(mainFrame, text = " Trigger Type ").grid(column = 2, row = 3, sticky = E, pady = 10)				# Etichetta
TT_updateButton = ttk.Button(mainFrame, text = "Update TT", command = updateTT)								# Bottone di update
TT_updateButton.grid(column = 4, row = 3, sticky = (N, W, E, S), pady = 10)

# Creazione oggetti di selezione per triggerPosition:
triggerPosition_val = ttk.Entry(mainFrame, width = 10, textvariable = triggerPosition)						# Casella di ingresso
triggerPosition_val.grid(column = 3, row = 4, sticky = (N, W, E, S), pady = 10)
ttk.Label(mainFrame, text = " Trigger Position ").grid(column = 2, row = 4, sticky = E, pady = 10)			# Etichetta
TP_updateButton = ttk.Button(mainFrame, text = "Update TP", command = updateTP)								# Bottone di update
TP_updateButton.grid(column = 4, row = 4, sticky = (N, W, E, S), pady = 10)

# Creazione oggetti di selezione per timeInterval:
timeInterval_val = ttk.Entry(mainFrame, width = 10, textvariable = timeInterval)							# Casella di ingresso
timeInterval_val.grid(column = 3, row = 5, sticky = (N, W, E, S), pady = 10)
ttk.Label(mainFrame, text = " Time Interval ").grid(column = 2, row = 5, sticky = E, pady = 10)				# Etichetta
timeUnit_rdbtn_s = ttk.Radiobutton(mainFrame, text = "s", variable = timeUnit, value = "s")					# Radiobutton per esprimere il tempo in s
timeUnit_rdbtn_s.grid(column = 3, row = 6, sticky = W)
timeUnit_rdbtn_ms = ttk.Radiobutton(mainFrame, text = "ms", variable = timeUnit, value = "ms")				# Radiobutton per esprimere il tempo in ms
timeUnit_rdbtn_ms.grid(column = 3, row = 7, sticky = W)
timeUnit_rdbtn_us = ttk.Radiobutton(mainFrame, text = "us", variable = timeUnit, value = "us")				# Radiobutton per esprimere il tempo in us
timeUnit_rdbtn_us.grid(column = 3, row = 8, sticky = W)
timeUnit_rdbtn_ns = ttk.Radiobutton(mainFrame, text = "ns", variable = timeUnit, value = "ns")				# Radiobutton per esprimere il tempo in ns
timeUnit_rdbtn_ns.grid(column = 3, row = 9, sticky = W)
timeUnit_rdbtn_s.state(["selected"])																		# Impostiamo s come unità di default
ttk.Label(mainFrame, text = " Time Unit ").grid(column = 2, row = 6, sticky = E)							# Etichetta radiobutton
TI_updateButton = ttk.Button(mainFrame, text = "Update TI", command = updateTI)								# Bottone di update
TI_updateButton.grid(column = 4, row = 5, sticky = (N, W, E, S), pady = 10)

# Creazione della finestra di disegno, che conterrà tutti i grafici (in questo caso uno solo):
plottingFigure = Figure(figsize = (5,5), dpi = 100)

# Creazione di un canvas (compatibile con tkinter) contenente la finestra di disegno (matplotlib):
plottingFigure_canvas = FigureCanvasTkAgg(plottingFigure, master = mainWindow)

# Aggiunta del canvas alla finestra principale (tkinter):
plottingFigure_canvas.get_tk_widget().grid(column = 0, row = 0, sticky = (N, W, E, S))

# Configuriamo il grafico in modo che si ingradisca proporzionalmente espandendo mainWindow
mainWindow.columnconfigure(0, weight = 1)
mainWindow.rowconfigure(0, weight = 1)

# Tracciamo il grafico effettivo nella finestra di disegno:
drawGraph()

# Avvio del loop di ricezione dei dati:
mainWindow.after(0, serialLoop)

# Apertura all'utente della finestra principale:

mainWindow.mainloop()
