#!/usr/bin/python3
#
# run.py
# written by Yist Lin
# 2016.01.14
#

import os
import subprocess

thread_amount = [100, 25, 10, 5, 2, 1]
filename = ["big_data", "large_data"]
inputsize = [1000000, 10000000]

for i in range(2):
	print("")
	print("filename = " + filename[i] + ", input size = " + str(inputsize[i]))
	print("-------------------------------------")

	for j in range(6):
		segment_size = inputsize[i] / thread_amount[j]
		print ("segment_size = %d" % (segment_size) )
		cmd = "/usr/bin/time -f \"%e,%U,%S\" ./merger "+ str(int(segment_size)) +" < "+ filename[i] +" > /dev/null"
		
		real_time = 0.0
		user_time = 0.0
		sys_time = 0.0

		times = 20
		for k in range(times):
			process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
			output, error = process.communicate()
			line = error.decode("utf-8")
			result = line.split(',')
			real_time += float(result[0])
			user_time += float(result[1])
			sys_time += float(result[2])

		real_time /= float(times)
		user_time /= float(times)
		sys_time /= float(times)
		print("real %.3f" % round(real_time,2) )
		print("user %.3f" % round(user_time,2) )
		print("sys  %.3f" % round(sys_time,2) )

