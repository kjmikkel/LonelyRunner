import json
import os, glob

import Gnuplot, Gnuplot.funcutils, re
#from scipy import *


class bar_instance:
	def __init__(self, speed, geo, geo_spread, num, num_spread):
		self.speed, self.geo, self.geo_spread, self.num, self.num_spread = speed, geo, geo_spread, num, num_spread

	def __str__(self):
		return "Speed: " + str(self.speed) + ", Geometrical time: (" + str(self.geo) + ", " + str(self.geo_spread) + ", Numerical time:(" + str(self.num) + ", " + str(self.num_spread) + ")"

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

def doGnuPlot(filename, extra_name, maxYValue, max_speed_to_test):

	DATAFILE='xrddata.dat'
	PLOTFILE='xrddata.ps'

	Gnu = Gnuplot.Gnuplot()
	Gnu('set data style linespoints') # Set style of the graph
	
	title = os.path.basename(filename)	
	
	m = re.search('[a-zA-Z_]+([0-9]+)[a-zA-Z_]+', title)
	title = title.replace("_", " ")
	title = title.replace("test", "")
	title = title.strip()
	minNum = m.group(1)

	if int(minNum) >= max_speed_to_test:
		return

	time = "Time (mu)"
	speed_name = ""
	
	num_name = "Numerical algorithm"
	geo_name = "Geometrical algorithm"
	
	if title.lower().find("random") < 0: 
		speed_name = "Maximum Speed"
	else:
		speed_name = "Index"
	
	input_filename = filename + ".dat"
	
	f=os.popen('gnuplot' ,'w')
	print >>f, "set xrange [%f:%f]" % (int(minNum), max_speed_to_test)
	print >>f, "set yrange [%f:%f]" % (0, maxYValue)
	print >>f, "set title '%s'" % title
	print >>f, "set term postscript"
	print >>f, "set out '%s'" % (filename + "_" + extra_name + ".ps")

	print >>f, "set noborder"
	print >>f, "set multiplot"	
	print >>f, "set xlabel '%s'; set ylabel '%s'" % (speed_name, time) 	
	print >>f, "plot '%s' using 1:2:3 with errorbars title '%s' lc rgb 'red', '%s' using 1:4:5 with errorbars title '%s' lc rgb 'green', '%s' using 1:2:3 with lines title '%s' lc rgb 'red', '%s' using 1:4:5 with lines title '%s' lc rgb 'green'" % (input_filename, geo_name, input_filename, num_name, input_filename, geo_name, input_filename, num_name)
	print >>f, "unset multiplot"
	print >>f, "reset"
	f.flush()

def make_line(info_i, num_head):
	geo_spread = ""
	num_spread = ""
	if num_head > 3:
		geo_spread = " & " + str(info.geo_spread)
		num_spread = " & " + str(info.num_spread)

	return str(info_i.speed) + " & " + str(info_i.geo) + geo_spread + " & " + str(info_i.num) + num_spread

def print_tables(info_list, filename):
	filename = filename.replace("_", "")
	filename = tables + os.path.basename(filename).split('.')[0] + ".tex"
	newline = "\\\\\n"
	header = "Speed & Geometrical & Numerical"
	number_headlines = len(header.split("&"))
	strAccum = "\\begin{tabular}[6]{"+ ("c|" * number_headlines) + "|" + ("c|" * number_headlines) + "}\n" + header + " & " + header + newline + "\hline\n"
	halflength = len(info_list) / 2
	
	for num in range(0, halflength):
		half_num = num + halflength
		info = info_list[num]
		first_half = make_line(info, number_headlines)		
		second_half = "& & "
		if not len(info_list) < half_num:
			info2 = info_list[half_num]
			second_half = make_line(info2, number_headlines)	
			
		strAccum += first_half + " & " +  second_half + newline
	
	strAccum += "\end{tabular}"
	saveFile(filename, strAccum)

def print_plot(info_list, filename):
	filename = graphs + os.path.basename(filename).split('.')[0]
	
	strAccum = "#Speed\tGeo\tGeo_spread\tNum\tNum_spread\n"
	maxGeoValue = -1	
	maxNumValue = -1
	max_speed_to_test = 5000	
	for bar in info_list:
		if bar.speed > max_speed_to_test:
			break;
		strAccum += str(bar.speed) + "\t" + str(bar.geo) + "\t" + str(bar.geo_spread) + "\t" + str(bar.num) + "\t" + str(bar.num_spread) +"\n"
		maxGeoValue = max(maxGeoValue, bar.geo + bar.geo_spread)
		maxNumValue = max(maxNumValue, bar.num + bar.num_spread)
		
	saveFile(filename + ".dat", strAccum)
	doGnuPlot(filename, "Geo", maxGeoValue, max_speed_to_test)
	doGnuPlot(filename, "Num", maxNumValue, max_speed_to_test)

class spread:
	def __init__(self, geo_spread, geo_sec, num_spread, num_sec):
		self.geo_spread, self.geo_sec, self.num_spread, self.num_sec = geo_spread, geo_sec, num_spread, num_sec

class speed:
	def __init__(self, geo_speed, geo_sec, num_spreed, num_sec):
		self.geo_speed, self.geo_sec, self.num_speed, self.num_sec = geo_speed, geo_sec, num_speed, num_sec

def find_spread(spread_list, max_second):
	sec_to_micro = 1000000
	
	spread_val = 0
	
	spread_val_max = 0
	spread_max = []

	for spread in spread_list:
		index = 0
		for val in spread[0]:
			spread_val += val
			print val
			if spread[1] and spread[1][index] < max_second:
				spread_max.append(spread[0][index])
			index += 1
	
	for max_val in spread_max:
		spread_val_max += max_val

#	spread_val /= len(spread_list)
#	spread_val_max /= len(spread_max)
	return (spread_val, spread_val_max)


def print_special_tables(all_info, s_type):
	geo_spread = []
	num_spread = []
	max_second = 2
	for spread in all_info:
		geo_spread.append((spread.geo_spread, spread.geo_sec))
		num_spread.append((spread.num_spread, spread.num_sec))
	
	(geo_spread_val, geo_spread_val_max) = find_spread(geo_spread, max_second)
	(num_spread_val, num_spread_val_max) = find_spread(num_spread, max_second)
	
	strAcum = "\\begin{tabular}[3]{c|c|c|}\\n"
	strAcum += " & Geometrical %s & Numerical %s\\n\hline\\n" %(s_type, s_type)
	strAcum += "All %ss & %s & %s\\n\\hline\\n" % (s_type, geo_spread_val, num_spread_val)
	strAcum += "Only %ss from runs & %s & %s\\n" % (s_type, geo_spread_val_max, num_spread_val_max) 
    strAcum += "that took less than %s sec & & \\n" % max_second
	strAcum += "\\hline"
	strAcum += "\\end{tabular}"
	
	saveFile("../report/data/tables/" + s_type + "_table.tex", strAcum)
		

def processData():
	spread_data = []
	speed_data = []
	
	for infile in glob.glob(os.path.join(inData, '*.json')):
		json = loadJsonFile(infile)
		
		numerical = json.get('Numerical results')	
		numerical_spread = json.get('Numerical spread')
		numerical_second = json.get('Numerical seconds')

		geometrical = json.get('Geometrical results')
		geometrical_spread = json.get('Geometrical spread')
		geometrical_second = json.get('Numerical seconds')

		speeds = json.get('Speeds used')

		bar_list = []
		for index in range(0, len(numerical)):
			bar_list.append(bar_instance(speeds[index], geometrical[index], geometrical_spread[index], numerical[index], numerical_spread[index]))
			
			spread_data.append(spread(geometrical_spread[index], geometrical_second[index], numerical_spread[index], numerical_second[index]))
			speed_data.append( speed(geometrical[index], geometrical_second[index], numerical[index], numerical_second[index]))			

#		print_plot(bar_list, infile)
#		print_tables(bar_list, infile)		
	print_special_tables(spread_data, "spread")
	print_special_tables(speed_data, "speed")
		
processData()
