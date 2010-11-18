import json
import os, glob
from decimal import *

import Gnuplot, Gnuplot.funcutils, re
#from scipy import *


class bar_instance:
	def __init__(self, speed, geo, geo_sec, geo_spread, num, num_sec, num_spread):
		self.speed, self.geo, self.geo_sec, self.geo_spread, self.num, self.num_sec, self.num_spread = speed, geo, geo_sec, geo_spread, num, num_sec, num_spread

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
	print >>f, "plot '%s' using 1:2:3 with errorbars title '%s' lc rgb 'red', '%s' using 1:4:5 with errorbars title '%s' lc rgb 'green', '%s' using 1:2:3 with lines title '%s' lc rgb 'red', '%s' using 1:4:5 with lines title '%s' lc rgb 'green'" % (input_filename, geo_name + " Error bar", input_filename, num_name + " Error bar", input_filename, geo_name, input_filename, num_name)
	print >>f, "unset multiplot"
	print >>f, "reset"
	f.flush()

def make_line(info_i, num_head):
	sec_to_micro = 1000000

	geo_spread = ""
	num_spread = ""

	if num_head > 3:
		geo_spread = " & " + str(info.geo_spread)
		num_spread = " & " + str(info.num_spread)

	geo_time = "0"
	num_time = "0"
	if info_i.geo > 0:
		geo_time = str(info_i.geo)
	else:
		geo_time = str(info_i.geo_sec * sec_to_micro)
	
	if info_i.num > 0:
		num_time = str(info_i.num)
	else:
		num_time = str(info_i.num_sec * sec_to_micro)

	return str(info_i.speed) + " & " + geo_time + geo_spread + " & " + num_time + num_spread

def print_tables(info_list, filename):
	filename = filename.replace("_", "")
	filename = tables + os.path.basename(filename).split('.')[0] + ".tex"
	newline = "\\\\\n"
	header = "Speed & Geometrical & Numerical\n & ($\mu$s) & ($\mu$s)\n"
	number_headlines = len(header.split("&")) / 2
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
	sec_to_micro = 1000000
	
	
	

	for bar in info_list:
		num_speed = bar.num
		if num_speed < 0:
			num_speed = bar.num_sec * sec_to_micro

		geo_speed = bar.geo
		if geo_speed < 0:
			geo_speed = bar.geo_sec * sec_to_micro

		if bar.speed > max_speed_to_test:
			break;
		strAccum += str(bar.speed) + "\t" + str(geo_speed) + "\t" + str(bar.geo_spread) + "\t" + str(num_speed) + "\t" + str(bar.num_spread) +"\n"
		maxGeoValue = max(maxGeoValue, geo_speed + bar.geo_spread)
		maxNumValue = max(maxNumValue, num_speed + bar.num_spread)
		
		saveFile(filename + ".dat", strAccum)
	doGnuPlot(filename, "Geo", maxGeoValue, max_speed_to_test)
	doGnuPlot(filename, "Num", maxNumValue, max_speed_to_test)

class spread:
	def __init__(self, geo_spread, geo_sec, num_spread, num_sec):
		self.geo_spread, self.geo_sec, self.num_spread, self.num_sec = geo_spread, geo_sec, num_spread, num_sec

class speed:
	def __init__(self, geo_speed, geo_sec, num_speed, num_sec):
		self.geo_speed, self.geo_sec, self.num_speed, self.num_sec = geo_speed, geo_sec, num_speed, num_sec

def find_spread(spread_list, max_second):
	sec_to_micro = 1000000
	
	spread_val = 0
	
	spread_val_max = 0
	spread_max = []

	for spread_instance in spread_list:
		val = spread_instance[0]
		val_sec = spread_instance[1]
		if (val > 0):
			spread_val += val
		else:
			spread_val += val_sec * sec_to_micro
		if val_sec < max_second:
			spread_max.append((val, val_sec))
		
	
	for max_val in spread_max:
		if max_val[0] > 0:
			spread_val_max += max_val[0]
		else:
			spread_val_max += max_val[1] * sec_to_micro

	print (spread_val, spread_val_max)

	return (spread_val, spread_val_max)


def print_special_tables(all_info, s_type):
	geo_spread = []
	num_spread = []
	max_second = 2
	micro_to_sec = 1000000

	for spread_instance in all_info:
		if isinstance(spread_instance, spread):
			geo_spread.append((spread_instance.geo_spread, spread_instance.geo_sec))
			num_spread.append((spread_instance.num_spread, spread_instance.num_sec))
		else:
			geo_spread.append((spread_instance.geo_speed, spread_instance.geo_sec))
			num_spread.append((spread_instance.num_speed, spread_instance.num_sec))

	(geo_spread_val, geo_spread_val_max) = find_spread(geo_spread, max_second)
	(num_spread_val, num_spread_val_max) = find_spread(num_spread, max_second)
	
	strAcum = "\\begin{tabular}[3]{c|c|c}\n"
	strAcum += " & Geometrical %s & Numerical %s\n\hline\n" %(s_type, s_type)
	strAcum += "All %ss ($\mu$s) & %s & %s\n\\hline\n" % (s_type, geo_spread_val, num_spread_val)
	strAcum += "Only %ss from runs ($\mu$s) & %s & %s\n" % (s_type, geo_spread_val_max, num_spread_val_max) 
	strAcum += "that took less than %s sec & & \n" % max_second
	strAcum += "\\hline"
	strAcum += "\\end{tabular}\n"

	getcontext().prec = 6
	
	all_val = (geo_spread_val - num_spread_val)
	all_val_sec = Decimal(all_val) / Decimal(micro_to_sec * 60 * 60) 

	no_max = (geo_spread_val_max - num_spread_val_max)
	no_max_sec = Decimal(no_max) / Decimal(micro_to_sec * 60 * 60)

	all_val_str = ""
	all_val_other_str =""

	no_max_str = ""
	no_max_other_str = ""
	
	if all_val < 0:
		all_val_str = "Geometrical"
		all_val_other_str = "Numerical"
	else:
		all_val_str = "Numerical"
		all_val_other_str = "Geometrical"

	if no_max_str < 0:
		no_max_str = "Geometrical"
		no_max_other_str = "Numerical"
	else:
		no_max_str = "Numerical"
		no_max_other_str = "Geometrical"
	
	adjecture = ""
	
	type_str = ""
	if s_type == "speed":
		type_str = "is faster by"
	else:
		type_str = "has a lesser spread by"

	fast  = "The %s method %s %s mu (or %s s) compared to the %s method" % (all_val_str, type_str, abs(all_val),  abs(all_val_sec), all_val_other_str)
	fast_no_max = "The %s method %s %s mu (or %s s) if we disregard every calculation that took more than %s seconds, compared to the %s method" % (no_max_str, type_str, abs(no_max), abs(no_max_sec), max_second, no_max_other_str)
	print fast
	print fast_no_max
	
	strAcum += fast
	strAcum += fast_no_max


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
			bar_list.append(bar_instance(speeds[index], geometrical[index], geometrical_second[index], geometrical_spread[index],            					                numerical[index]  , numerical_second[index]  , numerical_spread[index]  ))
			
			spread_data.append(spread(geometrical_spread[index], geometrical_second[index], 
						  numerical_spread[index], numerical_second[index]))
			speed_data.append( speed(geometrical[index], geometrical_second[index], 
						 numerical[index], numerical_second[index]))			

	#	print_plot(bar_list, infile)
	#	print_tables(bar_list, infile)		
	print_special_tables(spread_data, "spread")
	print_special_tables(speed_data, "speed")
		
processData()
