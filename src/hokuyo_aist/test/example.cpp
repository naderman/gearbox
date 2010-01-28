/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2008 Geoffrey Biggs
 *
 * hokuyo_aist Hokuyo URG laser scanner driver.
 *
 * This distribution is licensed to you under the terms described in the LICENSE file included in
 * this distribution.
 *
 * This work is a product of the National Institute of Advanced Industrial Science and Technology,
 * Japan. Registration number: H22PRO-1086.
 *
 * This file is part of hokuyo_aist.
 *
 * hokuyo_aist is free software: you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * hokuyo_aist is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with hokuyo_aist.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <math.h>
#include <iostream>
using namespace std;

#include <hokuyo_aist/hokuyo_aist.h>

int main(int argc, char **argv)
{
	string portOptions = "type=serial,device=/dev/ttyACM0,timeout=1";
	double startAngle = 0.0, endAngle = 0.0;
	int firstStep = -1, lastStep = -1;
	unsigned int baud = 19200, speed = 0, clusterCount = 1;
	bool getIntensities = false, getNew = false, verbose = false;

#if defined (WIN32)
	portOptions = "type=serial,device=COM3,timeout=1";
#else
	int opt;
	// Get some options from the command line
	while ((opt = getopt(argc, argv, "b:c:e:f:il:m:no:s:vh")) != -1)
	{
		switch (opt)
		{
			case 'b':
				sscanf (optarg, "%d", &baud);
				break;
			case 'c':
				sscanf (optarg, "%d", &clusterCount);
				break;
			case 'e':
				sscanf (optarg, "%lf", &endAngle);
				break;
			case 'f':
				sscanf (optarg, "%d", &firstStep);
				break;
			case 'i':
				getIntensities = true;
				break;
			case 'l':
				sscanf (optarg, "%d", &lastStep);
				break;
			case 'm':
				sscanf (optarg, "%d", &speed);
				break;
			case 'n':
				getNew = true;
				break;
			case 'o':
				portOptions = optarg;
				break;
			case 's':
				sscanf (optarg, "%lf", &startAngle);
				break;
			case 'v':
				verbose = true;
				break;
			case '?':
			case 'h':
			default:
				cout << "Usage: " << argv[0] << " [options]" << endl << endl;
				cout << "-b baud\t\tBaud rate to set the laser to *after* connecting." << endl;
				cout << "-c count\tCluster count." << endl;
				cout << "-e angle\tEnd angle to get ranges to." << endl;
				cout << "-f step\t\tFirst step to get ranges from." << endl;
				cout << "-i\t\tGet intensity data along with ranges." << endl;
				cout << "-l step\t\tLast step to get ranges to." << endl;
				cout << "-m speed\tMotor speed." << endl;
				cout << "-n\t\tGet new ranges instead of latest ranges." << endl;
				cout << "-o options\tPort options (see flexiport library)." << endl;
				cout << "-s angle\tStart angle to get ranges from." << endl;
				cout << "-v\t\tPut the hokuyo_aist library into verbose mode." << endl;
				return 1;
		}
	}
#endif // defined (WIN32)

	try
	{
		hokuyo_aist::HokuyoLaser laser; // Laser scanner object
		// Set the laser to verbose mode (so we see more information in the console)
		if (verbose)
			laser.SetVerbose (true);

		// Open the laser
		laser.Open (portOptions);
		// Turn the laser on
		laser.SetPower (true);
		// Set the baud rate
		try
		{
			laser.SetBaud (baud);
		}
		catch (hokuyo_aist::HokuyoError e)
		{
			cerr << "Failed to change baud rate: (" << e.Code () << ") " << e.what () << endl;
		}
		// Set the motor speed
		try
		{
			laser.SetMotorSpeed (speed);
		}
		catch (hokuyo_aist::HokuyoError e)
		{
			cerr << "Failed to set motor speed: (" << e.Code () << ") " << e.what () << endl;
		}

		// Get some laser info
		cout << "Laser sensor information:" << endl;
		hokuyo_aist::HokuyoSensorInfo info;
		laser.GetSensorInfo (&info);
		cout << info.AsString ();

		// Get range data
		hokuyo_aist::HokuyoData data;
		if ((firstStep == -1 && lastStep == -1) &&
			(startAngle == 0.0 && endAngle == 0.0))
		{
			// Get all ranges
			if (getNew)
				laser.GetNewRanges (&data, -1, -1, clusterCount);
			else if (getIntensities)
				laser.GetNewRangesAndIntensities (&data, -1, -1, clusterCount);
			else
				laser.GetRanges (&data, -1, -1, clusterCount);
		}
		else if (firstStep != -1 || lastStep != -1)
		{
			// Get by step
			if (getNew)
				laser.GetNewRanges (&data, firstStep, lastStep, clusterCount);
			else if (getIntensities)
				laser.GetNewRangesAndIntensities (&data, firstStep, lastStep, clusterCount);
			else
				laser.GetRanges (&data, firstStep, lastStep, clusterCount);
		}
		else
		{
			// Get by angle
			if (getNew)
				laser.GetNewRangesByAngle (&data, startAngle, endAngle, clusterCount);
			else if (getIntensities)
				laser.GetNewRangesAndIntensitiesByAngle (&data, startAngle, endAngle, clusterCount);
			else
				laser.GetRangesByAngle (&data, startAngle, endAngle, clusterCount);
		}

		cout << "Laser range data:" << endl;
		cout << data.AsString ();

		// Close the laser
		laser.Close ();
	}
	catch (hokuyo_aist::HokuyoError e)
	{
		cerr << "Caught exception: (" << e.Code () << ") " << e.what () << endl;
		return 1;
	}

	return 0;
}
