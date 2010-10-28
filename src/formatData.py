import json
import os, glob

import Gnuplot, Gnuplot.funcutils, re

class bar_instance:
	def __init__(self, speed, geo, num):
		self.speed, self.geo, self.num = speed, geo, num

	def __str__(self):
		return "Speed: " + str(self.speed) + ", Geometrical time: " + str(self.geo) + ", Numerical time: " + str(self.num)

inData = "data/"
outData = "../report/data/"
graphs = outData + "graphs/"
tables = outData + "tables/"

def loadJsonFile(filename):
	file = open(filename, "r")
	return json.load(file)	

def saveFile(filename, data):
	file = open(filename, "w")
	file.write(data)
	file.close()	

def doGnuPlot(filename, maxYValue):
	
	Gnu = Gnuplot.Gnuplot()
	Gnu('set data style linespoints') # Set style of the graph
	
	title = os.path.basename(filename)	
	Gnu.title(title)

	m = re.search('[a-zA-Z_]+([0-9]+)[a-zA-Z_]+', title)
	minNum = m.group(1)

	Gnu.xlabel("Maximum speed")
	Gnu.ylabel("Number of microseconds")
	Gnu.set_range("yrange", (0, maxYValue))
	Gnu.set_range("xrange", (minNum, 4900))

	first = Gnuplot.File(filename + ".dat", using=(1,2), title='Geometrical algorithm')
	second = Gnuplot.File(filename + ".dat", using=(1,3), title='Numerical algorithm')	
	
	Gnu.plot(first, second)	
	Gnu.replot()
	Gnu.hardcopy(filename + '.ps', enhanced=1, color=1)
	print filename
		

def print_tables(info_list, filename):
	filename = tables + os.path.basename(filename).split('.')[0] + ".tex"

	newline = "\\\\\n"
	header = "Speed & Geometrical & Numerical"
	strAccum = 	"\\begin{tabular}[6]{c|c|c||c|c|c}\n" + header + " & " + header + newline + "\hline\n"
	
	halflength = len(info_list) / 2
	
	for num in range(0, halflength):
		half_num = num + halflength
		info = info_list[num]
		first_half = str(info.speed) + " & " + str(info.geo) + " & " + str(info.num)		
		if len(info_list) < half_num:		
			strAccum += first_half + " &  & & " + newline
		else:
			info2 = info_list[half_num]
			second_half = str(info2.speed) + " & " + str(info2.geo) + " & " + str(info2.num)			
			strAccum += first_half + " & " +  second_half + newline
	strAccum += "\end{tabular}"
	saveFile(filename, strAccum)

def print_plot(info_list, filename):
	filename = graphs + os.path.basename(filename).split('.')[0]
	
	strAccum = "#Speed\tGeo\tNum\n"
	maxValue = -1	
	for bar in info_list:
		strAccum += str(bar.speed) + "\t" + str(bar.geo) + "\t" + str(bar.num) + "\n"
		maxValue = max(maxValue, bar.geo)
	
	saveFile(filename + ".dat", strAccum)
	doGnuPlot(filename, maxValue)

def processData():
	for infile in glob.glob(os.path.join(inData, '*.json')):
		json = loadJsonFile(infile)
		numerical = json.get('Numerical results')	
		geometrical = json.get('Geometrical results')
		speeds = json.get('Speeds used')

		bar_list = []
		for index in range(0, len(numerical)):
			bar_list.append(bar_instance(speeds[index], geometrical[index], numerical[index]))
		
		print_plot(bar_list, infile)
		print_tables(bar_list, infile)		
		
processData()

