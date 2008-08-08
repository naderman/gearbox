#!/usr/bin/env python

import hokuyo_aist
from optparse import OptionParser

def main ():
	parser = OptionParser ()
	parser.add_option ('-b', '--baud', dest = 'baudRate', type = 'int', default = '19200',
						help = 'Baud rate to set the laser to *after* connecting [default: %default]')
	parser.add_option ('-c', '--clustercount', dest = 'clusterCount', type = 'int', default = '1',
						help = 'Cluster count [default: %default]')
	parser.add_option ('-e', '--endangle', dest = 'endAngle', type = 'float', default = '0',
						help = 'End angle to get ranges to [default: %default]')
	parser.add_option ('-f', '--firststep', dest = 'firstStep', type = 'int', default = '-1',
						help = 'First step to get ranges from [default: %default]')
	parser.add_option ('-l', '--laststep', dest = 'lastStep', type = 'int', default = '-1',
						help = 'Last step to get ranges to [default: %default]')
	parser.add_option ('-m', '--motorspeed', dest = 'motorSpeed', type = 'int', default = '0',
						help = 'Motor speed stepping (higher is slower) [default: %default]')
	parser.add_option ('-n', '--new', dest = 'getNew', action = 'store_true', default = 'False',
						help = 'Get new ranges instead of latest ranges [default: %default]')
	parser.add_option ('-o', '--portoptions', dest = 'portOptions', type = 'string', default = 'type=serial,device=/dev/ttyACM0,timeout=1',
						help = 'Port options (see flexiport library) [default: %default]')
	parser.add_option ('-s', '--startangle', dest = 'startAngle', type = 'float', default = '0',
						help = 'Start angle to get ranges from [default: %default]')
	parser.add_option ('-v', '--verbose', dest = 'verbose', action = 'store_true', default = 'False',
						help = 'Put the hokuyo_aist library into verbose mode [default: %default]')

	# Scan command line arguments
	options, args = parser.parse_args ()

	try:
		# Create an instance of a laser scanner object
		laser = hokuyo_aist.HokuyoLaser ()
		if options.verbose == True:
			# Set verbose mode so we see more information in stderr
			laser.SetVerbose (True)

		# Open the laser
		laser.Open (options.portOptions)
		# Turn the laser on
		laser.SetPower (True)
		# Set the baud rate
		try:
			laser.SetBaud (options.baudRate)
		except hokuyo_aist.HokuyoError, e:
			print 'Failed to change baud rate: (' + str (e.Code ()) + ') ' + e.what ()
		# Set the motor speed
		laser.SetMotorSpeed (options.motorSpeed)

		# Get some laser info
		#print 'Laser sensor information:'
		#info = hokuyo_aist.HokuyoSensorInfo info ()
		#laser.GetSensorInfo (info)
		#print info.AsString ()

		# Get range data
		data = hokuyo_aist.HokuyoData ()
		if (options.firstStep == -1 and options.lastStep == -1) and \
				(options.startAngle == 0 and options.endAngle == 0):
			# Get all ranges
			if options.getNew == True:
				laser.GetNewRanges (data, -1, -1, options.clusterCount)
			else:
				laser.GetRanges (data, -1, -1, options.clusterCount)
		elif options.firstStep != -1 or options.lastStep != -1:
			# Get by step
			if options.getNew == True:
				laser.GetNewRanges (data, options.firstStep, options.lastStep, options.clusterCount)
			else:
				laser.GetRanges (data, options.firstStep, options.lastStep, options.clusterCount)
		else:
			# Get by angle
			if options.getNew == True:
				laser.GetNewRangesByAngle (data, options.startAngle, options.endAngle, options.clusterCount)
			else:
				laser.GetRangesByAngle (data, options.startAngle, options.endAngle, options.clusterCount)

		print 'Laser range data:'
		print data.AsString ()

		# Close the laser
		laser.Close ()

	except hokuyo_aist.HokuyoError, e:
		print 'Caught exception: (' + str (e.Code ()) + ') ' + e.what ()
		sys.exit (1)


if __name__ == '__main__':
	main ()
