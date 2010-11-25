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

#	if int(minNum) >= max_speed_to_test:
#		return

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
	print >>f, "plot '%s' using 1:2:3 with errorbars title '%s' lc rgb 'red', '%s' using 1:4:5 with errorbars title '%s' lc rgb 'blue', '%s' using 1:2:3 with lines title '%s' lc rgb 'red', '%s' using 1:4:5 with lines title '%s' lc rgb 'blue'" % (input_filename, geo_name + " Error bar", input_filename, num_name + " Error bar", input_filename, geo_name, input_filename, num_name)
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
	
	second_max = 5000

	sec_to_micro = 1000000
	max_speed_to_test = -1
	for bar in info_list:
		num_speed = bar.num
		if num_speed < 0:
			num_speed = bar.num_sec * sec_to_micro

		max_speed_to_test = max(bar.speed, max_speed_to_test)

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
	
	print filename + "\n"
	doGnuPlot(filename, "Geo_" + str(second_max), maxGeoValue, second_max)
	doGnuPlot(filename, "Num_" + str(second_max), maxNumValue, second_max)

class spread:
	def __init__(self, speed, geo_spread, geo_sec, num_spread, num_sec):
		self.speed, self.geo_spread, self.geo_sec, self.num_spread, self.num_sec = speed, geo_spread, geo_sec, num_spread, num_sec

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
	
		val_query = val
		val_sec_query = val_sec

		#if len(spread_instance) == 3:
						
	#		val_query = spread_instance[2][0]
	#		val_sec_query = spread_instance[2][1]
			
		if (val > 0):
			spread_val += val
		else:
			spread_val += val_sec * sec_to_micro
		
		if (val_sec_query < max_second) and (val_query < max_second * sec_to_micro):
			spread_max.append((val, val_sec))
	
	
	for max_val in spread_max:
		if max_val[0] > 0:
			spread_val_max += max_val[0]
		else:
			spread_val_max += max_val[1] * sec_to_micro
	
	spread_val_avg = spread_val / len(spread_list)
	spread_max_avg = spread_val_max / len(spread_max)

	num_over_max = len(spread_list) - len(spread_max)

	return (spread_val, spread_val_avg, spread_val_max, spread_max_avg, num_over_max)


def print_special_tables(all_info, s_type, number_type_name, number):
	geo_spread = []
	num_spread = []
	max_second = 1
	micro_to_sec = 1000000

	sec_str = ""
	if max_second <= 1:
		sec_str = "second"
	else:
		sec_str = "seconds"

	exclusion = "took less than"
	if s_type == "spread":
		exclusion = "had less spread than"
	

	for spread_instance in all_info:
		
		if isinstance(spread_instance, spread):
			speed = spread_instance.speed
			geo_spread.append((spread_instance.geo_spread, spread_instance.geo_sec, (speed.geo_speed, speed.geo_sec)))
			num_spread.append((spread_instance.num_spread, spread_instance.num_sec, (speed.num_speed, speed.num_sec)))
		else:
			geo_spread.append((spread_instance.geo_speed, spread_instance.geo_sec))
			num_spread.append((spread_instance.num_speed, spread_instance.num_sec))

	(geo_spread_val, geo_spread_avg, geo_spread_val_max, geo_spread_max_avg, geo_over_max) = find_spread(geo_spread, max_second)
	(num_spread_val, num_spread_avg, num_spread_val_max, num_spread_max_avg, num_over_max) = find_spread(num_spread, max_second)

	text_name_type = ""
	text_name_order = ""
	if "sequential"  in  number_type_name.lower():
		text_name_type = "Sequential"
	elif "prime" in number_type_name.lower():
		text_name_type = "Prime"
	elif "random-" in number_type_name.lower():
		text_name_type = "Random"

	if "-normal" in number_type_name.lower():
		text_name_order = "Sorted"
	elif "-random" in number_type_name.lower():
		text_name_order = "Random"
	else:
		text_name_order = "Complete"

	strAcum = "\\begin{table}[bth!]\\footnotesize\n "
        strAcum += "\\begin{tabular}[3]{c|r|r}\n"
	strAcum += " & Geometrical ($\mu$s) %s & Numerical ($\mu$s) %s\\\\\n\hline\n" %(s_type, s_type)
	strAcum += "All %ss & %s & %s \\\\ \n" % (s_type, geo_spread_val, num_spread_val)
	if (geo_over_max > 0) or (num_over_max > 0):
		strAcum += "\\hline \n"
		strAcum += "Only %ss from calculations & %s & %s \\\\ \n" % (s_type, geo_spread_val_max, num_spread_val_max) 
		strAcum += "that took less than %s %s & & \\\\ \n" % (max_second, sec_str)
	strAcum += "\\hline\n"

	strAcum += "Average %s & %s & %s \\\\\n" % (s_type, geo_spread_avg, num_spread_avg)
	strAcum += "\hline\n"
	if (geo_over_max > 0) or (num_over_max > 0):
		strAcum += "Average %s from calculations that & %s & %s \\\\ \n" % (s_type, geo_spread_max_avg, num_spread_max_avg)
		strAcum += "%s %s %s & & \\\\ \n" % (exclusion, max_second, sec_str)
	
	strAcum += "\\end{tabular}\\\\ \\\\\n\caption{" + text_name_order + " " + text_name_type + " Results\\\\\n"

	getcontext().prec = 3

	time_mes1 = "hours"
	time_mes2 = "hours"
	
	all_val = (geo_spread_val - num_spread_val)
	all_val_sec = Decimal(all_val) / Decimal(micro_to_sec * 60 * 60) 
	
	if abs(all_val_sec) < 1:
		all_val_sec *= 60
		time_mes1 = "minutes"
	
	if abs(all_val_sec) < 1:
		all_val_sec *= 60
		time_mes1 = "seconds"

	no_max = (geo_spread_val_max - num_spread_val_max)
	no_max_sec = Decimal(no_max) / Decimal(micro_to_sec * 60 * 60)
	
	if abs(no_max_sec) < 1:
		no_max_sec *= 60
		time_mes2 = "minutes"
	
	if abs(no_max_sec) < 1:
		no_max_sec *= 60
		time_mes2 = "seconds"

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

	if no_max < 0:
		no_max_str = "Geometrical"
		no_max_other_str = "Numerical"
	else:
		no_max_str = "Numerical"
		no_max_other_str = "Geometrical"
	
	adjecture = ""
	
	type_str = ""
	if s_type == "speed":
		type_str = "is faster"
		exclusion = "calculation that took more"
	else:
		type_str = "has a lesser spread"
		exclusion = "spread that is larger"

	fast  = "The %s algorithm %s than the %s algorithm by %s $\mu$s (or %s %s).\\\\\n" % (all_val_str, type_str, all_val_other_str, abs(all_val), abs(all_val_sec), time_mes1)
	
	if (geo_over_max > 0) or (num_over_max > 0):
		fast_no_max = "The %s algorithm %s than the %s algorithm by %s $\mu$s (or %s %s), if we disregard every %s than %s %s.\\\\\n" % (no_max_str, type_str, no_max_other_str, abs(no_max), abs(no_max_sec), time_mes2, exclusion, max_second, sec_str)
	else:
		fast_no_max = ""

	print fast
	print fast_no_max

	strAcum += fast
	strAcum += fast_no_max

	cal = ""
	spre = ""
	if (geo_over_max > 1) or (geo_over_max == 0 and num_over_max > 1):
		cal = "calculations"
		spre = "spreads"
	else:
		cal = "calculation"
		spre = "spread"
	
	if s_type == "speed":
		type_str = "is faster by"
		exclusion = "%s that took more" % cal
	else:
		type_str = "has a lesser spread by"
		exclusion = "%s that is larger" % spre

	number_over_max_str = ""
	if (geo_over_max > 0) and (num_over_max > 0):
		number_over_max_str = "The Geometrical algorithm produced %s %s than %s %s, while the Numerical algorithm produced %s, using %s data sets (with a total of %s data points)\\\\\n" % (geo_over_max, exclusion, max_second, sec_str, num_over_max, str(number), str(len(geo_spread)))
	else:
		num_to_put = -1
		algo_type_str = ""
		if num_over_max > 0:
			num_to_put = num_over_max
			algo_type_str = "Numerical"
		elif geo_over_max > 0:
			num_to_put = geo_over_max
			algo_type_str = "Geometrical"
		
		if num_to_put > 0:
			number_over_max_str = "The %s algorithm produced %s %s than %s %s using %s data sets (with a total of %s data points)" % (algo_type_str, num_to_put, exclusion, max_second, sec_str, str(number), str(len(num_spread)))
			
	
	strAcum += number_over_max_str
	strAcum += "}\label{" + number_type_name + "_" + s_type + "table}"
	strAcum += "\end{table}"


	saveFile("../report/data/tables/" + number_type_name + "_" + s_type + "_table.tex", strAcum)

def special_tables(spread, speed, name):
	if len(speed.normal) > 0:
		print_special_tables(spread.normal, "spread", name + "-normal", speed.normal_num)
		print_special_tables(speed.normal, "speed", name + "-normal", spread.normal_num)

	if len(speed.random) > 0:
		print_special_tables(spread.random, "spread", name + "-random", speed.random_num)
		print_special_tables(speed.random, "speed", name + "-random", spread.random_num)

	if len(speed.total) > 0:
		print_special_tables(spread.total, "spread", name, speed.total_num)
		print_special_tables(speed.total, "speed", name, spread.total_num)

	
class speed_spread:
	def __init__(self):
		self.random, self.normal, self.total, self.normal_num, self.random_num, self.total_num = [], [], [], 0, 0, 0
	
	def into_random(self, data_list):
		self.random_num += 1
		self.total_num += 1

		self.random.extend(data_list)
		self.total.extend(data_list)
	
	def into_normal(self, data_list):
		self.normal_num += 1
		self.total_num += 1

		self.normal.extend(data_list)
		self.total.extend(data_list)

	def add_speed_spread(self, ss_instance):
		self.normal_num += ss_instance.normal_num
		self.random_num += ss_instance.random_num
		self.total_num += ss_instance.total_num
		
		self.normal.extend(ss_instance.normal)
		self.random.extend(ss_instance.random)
		self.total.extend(ss_instance.total)

def processData():
	sequential_speed = speed_spread()
	sequential_spread = speed_spread()

	prime_speed = speed_spread()
	prime_spread = speed_spread()

	random_speed = speed_spread()
	random_spread = speed_spread()

	items = 0
	
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
		spread_data = []
		speed_data = []
		items += len(numerical)
		
		for index in range(0, len(numerical)):
			bar_list.append(bar_instance(speeds[index], geometrical[index], geometrical_second[index], geometrical_spread[index],            					                                                                numerical[index]  , numerical_second[index]  , numerical_spread[index]  ))

			speed_data.append( speed(geometrical[index], geometrical_second[index], 
						 numerical[index], numerical_second[index]))	 			
			spread_data.append(spread(speed_data[index], geometrical_spread[index], geometrical_second[index], 
						  numerical_spread[index], numerical_second[index]))

		
		if "sequential" in infile.lower():			
			if "random" in infile.lower():
				sequential_spread.into_random(spread_data)
				sequential_speed.into_random(speed_data)
			else:
				sequential_spread.into_normal(spread_data)
				sequential_speed.into_normal(speed_data)
			print "Sequential: \t" + infile
		elif "prime" in infile.lower():
			if "random" in infile.lower():
				prime_spread.into_random(spread_data)
				prime_speed.into_random(speed_data)
			else:
				prime_spread.into_normal(spread_data)
				prime_speed.into_normal(speed_data)
			print "Prime: \t\t" + infile
		elif "random" in infile.lower():
			if "sorted" in infile.lower():
				random_spread.into_normal(spread_data)
				random_speed.into_normal(speed_data)
			else:
				random_speed.into_random(spread_data)
				random_spread.into_random(speed_data)
			print "Random: \t" + infile
		else:
			print "No found: \t\t" + infile
	
		print_plot(bar_list, infile)
	#	print_tables(bar_list, infile)		
			
	return
	start = "***\n"
	print start + "Sequential:\n" + start 
	special_tables(sequential_spread, sequential_speed, "sequential")
	
	print start + "Primes:\n" + start
	special_tables(prime_spread, prime_speed, "prime")

	print start + "Random:\n" + start
	special_tables(random_spread, random_speed, "random")

	total_spread = speed_spread()
	total_spread.add_speed_spread(sequential_spread)
	total_spread.add_speed_spread(prime_spread)
	total_spread.add_speed_spread(random_spread)

	total_speed = speed_spread()
	total_speed.add_speed_spread(sequential_speed)
	total_speed.add_speed_spread(prime_speed)
	total_speed.add_speed_spread(random_speed)

	print start + "Total:\n" + start
	special_tables(total_spread, total_speed, "total")
	
	print "items: " + str(items) + ", total: " + str(len(total_spread.total))
	
processData()
