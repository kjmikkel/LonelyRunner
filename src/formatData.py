import json
import os, glob

import Gnuplot, Gnuplot.funcutils, re
#from scipy import *


class bar_instance:
	def __init__(self, speed, geo, geo_spread, num, num_spread):
		self.speed, self.geo, self.geo_spread, self.num, self.num_spread = speed, geo, geo_spread, num, num_spread

	def __str__(self):
		return "Speed: " + str(self.speed) + ", Geometrical time: (" + str(self.geo) + ", " + str(self.geo_spread) + ", Numerical time:(" + str(self.num) + ", " + str(self.num_spread) + ")"

inData = "data Backup/"
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

def doGnuPlot(filename, extra_name, maxYValue):
	
	DATAFILE='xrddata.dat'
	PLOTFILE='xrddata.ps'

	Gnu = Gnuplot.Gnuplot()
	Gnu('set data style linespoints') # Set style of the graph
	
	title = os.path.basename(filename)	
	
	m = re.search('[a-zA-Z_]+([0-9]+)[a-zA-Z_]+', title)
	title = title.replace("_", " ")
	
	minNum = m.group(1)

	input_filename = filename + ".dat"

	f=os.popen('gnuplot' ,'w')
	print >>f, "set xrange [%f:%f]" % (int(minNum), 5000)
	print >>f, "set yrange [%f:%f]" % (0, maxYValue)
	print >>f, "set title %s" % title
	print >>f, "set term postscript"
	print >>f, "set out '%s'" % (filename + "_" + extra_name + ".ps")
	print >>f, "set noborder"
	print >>f, "set multiplot"
	
	print >>f, "set xlabel 'Maximum speed'; set ylabel 'Number of microseconds'"	
	print >>f, "plot '%s' using 1:2:3 with errorbars title 'Geometrical algorithm' lc rgb 'red', '%s' using 1:4:5 with errorbars title 'Numerical algorithm' lc rgb 'green'" % (input_filename, input_filename)
	print >>f, "unset multiplot"
	print >>f, "reset"
	f.flush()

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
	
	strAccum = "#Speed\tGeo\tGeo_spread\tNum\tNum_spread\n"
	maxGeoValue = -1	
	maxNumValue = -1
	for bar in info_list:
		strAccum += str(bar.speed) + "\t" + str(bar.geo) + "\t" + str(bar.geo_spread) + "\t" + str(bar.num) + "\t" + str(bar.num_spread) +"\n"
		maxGeoValue = max(maxGeoValue, bar.geo)
		maxNumValue = max(maxNumValue, bar.num)
		
	saveFile(filename + ".dat", strAccum)
	doGnuPlot(filename, "Geo", maxGeoValue)
	doGnuPlot(filename, "Num", maxNumValue)
	

def processData():
	for infile in glob.glob(os.path.join(inData, '*.json')):
		json = loadJsonFile(infile)
		numerical = json.get('Numerical results')	
		numerical_spread = json.get('Numerical spread')
		
		geometrical = json.get('Geometrical results')
		geometrical_spread = json.get('Geometrical spread')



		speeds = json.get('Speeds used')

		bar_list = []
		for index in range(0, len(numerical)):
			bar_list.append(bar_instance(speeds[index], geometrical[index], geometrical_spread[index], numerical[index], numerical_spread[index]))
		
		print_plot(bar_list, infile)
		print_tables(bar_list, infile)		
		
processData()

