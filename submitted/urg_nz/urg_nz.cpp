/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2008 Geoffrey Biggs
 *
 * urg_nz Hokuyo URG laser scanner driver.
 *
 * This distribution is licensed to you under the terms described in the LICENSE file included in
 * this distribution.
 *
 * This work is a product of the National Institute of Advanced Industrial Science and Technology,
 * Japan. Registration number: ___
 *
 * This file is part of urg_nz.
 *
 * urg_nz is free software: you can redistribute it and/or modify it under the terms of the GNU
 * Lesser General Public License as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * urg_nz is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with urg_nz.  If
 * not, see <http://www.gnu.org/licenses/>.
 */

#include "urg_nz.h"
using namespace urg_nz;

#include <flexiport/flexiport.h>
#include <flexiport/port.h>
#include <flexiport/serialport.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <sstream>
#include <iostream>
using namespace std;
using namespace flexiport;

#if defined (WIN32)
	#define __func__    __FUNCTION__
#endif

namespace urg_nz
{

#ifndef M_PI
    const double M_PI                       = 3.14159265358979323846;
#endif
// Convert radians to degrees
#ifndef RTOD
    inline double RTOD (double rad)
    {
    	return rad * 180.0 / M_PI;
    }
#endif
// Convert degrees to radians
#ifndef DTOR
    inline double DTOR (double deg)
    {
    	return deg * M_PI / 180.0;
    }
#endif

// SCIP1: 66 bytes (64 bytes of data + line feed + NULL)
const unsigned int SCIP1_LINE_LENGTH        = 66;
// SCIP2: 67 bytes (64 bytes of data + checksum byte + line feed + NULL)
const unsigned int SCIP2_LINE_LENGTH        = 67;

////////////////////////////////////////////////////////////////////////////////////////////////////
// SCIP protocol version 1 notes
////////////////////////////////////////////////////////////////////////////////////////////////////

/* | = byte boundary, ... indicates variable byte block (max 64 bytes), (x) = x byte block
 - No checksum
 - Host to sensor: Command | Parameters... | LF
 - Sensor to host: Command | Parameters... | LF | Status | LF | Data... | LF | LF
 - Where a block of data would take more than 64 bytes, a line feed is inserted every 64 bytes.
 - Status 0 is OK, anything else is an error.

L  Power
   L|Control code|LF
   L|Control code|LF|Status|LF|LF
   3 byte command block
G  Get data
   G|Start(3)|End(3)|Cluster(2)|LF
   G|Start(3)|End(3)|Cluster(2)|LF|Status|LF|Data...|LF|LF
   10 byte command block
S  Set baud rate
   S|Baud rate(6)|Reserved(7)|LF
   S|Baud rate(6)|Reserved(7)|Status|LF|LF|
   16 byte command block
V  Version info
   V|LF
   V|LF|Status...|LF|Vendor...|LF|Product...|LF|Firmware...|LF|Protocol...|LF|Serial...|LF|LF
   2 byte command block
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// SCIP protocol version 2 notes
////////////////////////////////////////////////////////////////////////////////////////////////////

/* | = byte boundary, ... indicates variable byte block (max 64 bytes), (x) = x byte block
 - We don't use the string block (which can be up to 16 bytes) so it's marked as size 0 and ignored
   in the command definitions below.
 - Host to sensor: Command(2) | Parameters... | String(0) | LF
 - Sensor to host: Command(2) | Parameters... | String(0) | LF | Status(2) | Sum | LF
 - Each data row: Data (max 64) | Sum | LF
 - Data rows are broken after a maximum of 64 bytes, each one having a checksum and a line feed.
 - Status codes 00 and 99 are OK, anything else is an error.
 - Checksum is calculated by... well, see the code.

VV    Version info
      V|V|LF
      V|V|LF|Status(2)|Sum|LF|Vendor...|;|Sum|LF|Product...|;|Sum|LF|Firmware...|;|Sum|LF|
          Protocol...|;|Sum|LF|Serial...|;|Sum|LF|LF
      3 byte command block
PP    Specification info
      P|P|LF
      P|P|LF|Status(2)|Sum|LF|Model...|;|Sum|LF|MaxRange...|;|Sum|LF|MinRange...|;|Sum|LF|
          TotalSteps...|;|Sum|LF|FirstStep...|;|Sum|LF|LastStep...|;|Sum|LF|FrontStep...|;|Sum|LF|
          MotorSpeed...|;|Sum|LF|LF
      3 byte command block
II    Status info
      I|I|LF
      I|I|LF|Status(2)|Sum|LF|Model...|;|Sum|LF|Power...|;|Sum|LF|MotorSpeed...|;|Sum|LF|
          Mode...|;|Sum|LF|Baud...|;|Sum|LF|Time...|;|Sum|LF|Diagnostic...|;|Sum|LF|LF
      3 byte command block
BM    Power on
      B|M|LF
      B|M|LF|Status(2)|Sum|LF|LF
      3 byte command block
QT    Power off
      Q|T|LF
      Q|T|LF|Status(2)|Sum|LF|LF
      3 byte command block
SS    Set baud rate
      S|S|Baud(6)|LF
      S|S|Baud(6)|LF|Status(2)|Sum|LF|LF
      9 byte command block
MDMS  Get new data
      M|D/S|Start(4)|End(4)|Cluster(2)|Interval(1)|Number(2)|LF
      M|D/S|Start(4)|End(4)|Cluster(2)|Interval(1)|Number(2)|LF|Status(2)|Sum|LF|Data...|LF|LF
      16 byte command block
GDGS  Get latest data
      G|D/S|Start(4)|End(4)|Cluster(2)|LF
      G|D/S|Start(4)|End(4)|Cluster(2)|LF|Status(2)|Sum|LF|Data...|LF|LF
      12 byte command block
CR    Set motor speed
      C|R|Speed(2)|LF
      C|R|Speed(2)|LF|Status(2)|Sum|LF|LF
      5 byte command block
TM    Get sensor time
      T|M|Code|LF
      T|M|Code|LF|Status(2)|Sum|LF[|Time(4)|Sum|LF|LF]
      4 byte command block
      Optional part only comes back for control code 1.
RS    Reset
      R|S|LF
      R|S|LF|Status(2)|Sum|LF|LF
      3 byte command block
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////////////////////////////////////////////

string SCIP1ErrorToString (char error, char cmd)
{
	return string ("No error descriptions available");
}

// error must be null-terminated
string SCIP2ErrorToString (char *error, char *cmd)
{
	stringstream ss;

	// Check for universal errors first
	if (error[1] == 'A')
		return "Unable to create transmission data or reply command internally";
	else if (error[1] == 'B')
		return "Buffer shortage or command repeated that is already processed";
	else if (error[1] == 'C')
		return "Command with insufficient parameters 1";
	else if (error[1] == 'D')
		return "Undefined command 1";
	else if (error[1] == 'E')
		return "Undefined command 2";
	else if (error[1] == 'F')
		return "Command with insufficient parameters 2";
	else if (error[1] == 'G')
		return "String character in command exceeds 16 letters";
	else if (error[1] == 'H')
		return "String character has invalid letters";
	else if (error[0] == '0' && error[1] == 'I')
		return "Sensor is now in firmware update mode";

	int errorCode = atoi (error);

	if (cmd[0] == 'B' && cmd[1] == 'M')
	{
		switch (errorCode)
		{
			case 1:
				return "Unable to control due to laser malfunction";
			case 2:
				return "Laser is already on";
		}
	}
// No info in the manual for this.
//	else if (cmd[0] == 'Q' && cmd[1] == 'T')
//	{
//		switch (errorCode)
//		{
//			default:
//				stringstream ss;
//				ss << "Unknown error code " << errorCode << " for command " << cmd[0] << cmd[1];
//				return ss.str ();
//		}
//	}
	else if ((cmd[0] == 'G' && cmd[1] == 'D') ||
			 (cmd[0] == 'G' && cmd[1] == 'S'))
	{
		switch (errorCode)
		{
			case 1:
				return "Starting step has non-numeric value";
			case 2:
				return "Ending step has non-numeric value";
			case 3:
				return "Cluster count has non-numeric value";
			case 4:
				return "Ending step is out of range";
			case 5:
				return "Ending step is smaller than start step";
			case 6:
				return "Scan interval is greater than 9";
			case 7:
				return "Number of scans is greater than 99";
			default:
				if (errorCode >= 50)
					ss << "Hardware error: " << errorCode;
				else
					ss << "Unknown error code " << errorCode << " for command " << cmd[0] << cmd[1];

				return ss.str ();
		}
	}
	else if ((cmd[0] == 'M' && cmd[1] == 'D') ||
			 (cmd[0] == 'M' && cmd[1] == 'S'))
	{
		switch (errorCode)
		{
			case 1:
				return "Starting step has non-numeric value";
			case 2:
				return "Ending step has non-numeric value";
			case 3:
				return "Cluster count has non-numeric value";
			case 4:
				return "Ending step is out of range";
			case 5:
				return "Ending step is smaller than start step";
			case 6:
				return "Scan interval is greater than 9";
			case 7:
				return "Number of scans is greater than 99";
			default:
				if (errorCode >= 50)
					ss << "Hardware error: " << errorCode;
				else
					ss << "Unknown error code " << errorCode << " for command " << cmd[0] << cmd[1];

				return ss.str ();
		}
	}
	else if (cmd[0] == 'T' && cmd[1] == 'M')
	{
		switch (errorCode)
		{
			case 1:
				return "Invalid control code";
			case 2:
				return "Adjust mode on command received when sensor's adjust mode is already on";
			case 3:
				return "Adjust mode off command received when sensor's adjust mode is already off";
			case 4:
				return "Adjust mode is off when requested time";
		}
	}
	else if (cmd[0] == 'S' && cmd[1] == 'S')
	{
		switch (errorCode)
		{
			case 1:
				return "Baud rate has non-numeric value";
			case 2:
				return "Invalid baud rate";
			case 3:
				return "Sensor is already running at that baud rate";
		}
	}
	else if (cmd[0] == 'C' && cmd[1] == 'R')
	{
		switch (errorCode)
		{
			case 1:
				return "Invalid speed";
			case 2:
				return "Speed is out of range";
			case 3:
				return "Motor is already running at that speed";
		}
	}
// No info in the manual for this.
//	else if (cmd[0] == 'R' && cmd[1] == 'S')
//	{
//		switch (errorCode)
//		{
//			case :
//				return "";
//			default:
//				stringstream ss;
//				ss << "Unknown error code " << errorCode << " for command " << cmd[0] << cmd[1];
//				return ss.str ();
//		}
//	}
// No info in the manual for this.
//	else if (cmd[0] == 'V' && cmd[1] == 'V')
//	{
//		switch (errorCode)
//		{
//			case :
//				return "";
//		}
//	}
// No info in the manual for this.
//	else if (cmd[0] == 'P' && cmd[1] == 'P')
//	{
//		switch (errorCode)
//		{
//			case :
//				return "";
//		}
//	}
// No info in the manual for this.
//	else if (cmd[0] == 'I' && cmd[1] == 'I')
//	{
//		switch (errorCode)
//		{
//			case :
//				return "";
//		}
//	}
	else
	{
		ss << "Unknown command: " << cmd[0] << cmd[1];
		return ss.str ();
	}

	// Known commands with unknown error codes fall through to here
	ss << "Unknown error code " << errorCode << " for command " << cmd[0] << cmd[1];
	return ss.str ();
}

unsigned int Decode2ByteValue (char *data)
{
	unsigned int byte1, byte2;

	byte1 = data[0] - 0x30;
	byte2 = data[1] - 0x30;

	return (byte1 << 6) + (byte2);
}

unsigned int Decode3ByteValue (char *data)
{
	unsigned int byte1, byte2, byte3;

	byte1 = data[0] - 0x30;
	byte2 = data[1] - 0x30;
	byte3 = data[2] - 0x30;

	return (byte1 << 12) + (byte2 << 6) + (byte3);
}

unsigned int Decode4ByteValue (char *data)
{
	unsigned int byte1, byte2, byte3, byte4;

	byte1 = data[0] - 0x30;
	byte2 = data[1] - 0x30;
	byte3 = data[2] - 0x30;
	byte4 = data[3] - 0x30;

	return (byte1 << 18) + (byte2 << 12) + (byte3 << 6) + (byte4);
}

void NumberToString (unsigned int num, char *dest, int length)
{
#if defined (WIN32)
	_snprintf (dest, length + 1, "%*d", length, num);
#else
	snprintf (dest, length + 1, "%*d", length, num);
#endif
	// Replace all leading spaces with '0'
	for (int ii = 0; ii < length && dest[ii] == ' '; ii++)
		dest[ii] = '0';
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// URGError class
////////////////////////////////////////////////////////////////////////////////////////////////////

string URGError::AsString (void) const throw ()
{
	switch (_errorCode)
	{
		case URG_ERR_READ:
			return "URG_ERR_READ";
		case URG_ERR_WRITE:
			return "URG_ERR_WRITE";
		case URG_ERR_PROTOCOL:
			return "URG_ERR_PROTOCOL";
		case URG_ERR_CHANGEBAUD:
			return "URG_ERR_CHANGEBAUD";
		case URG_ERR_CONNECT_FAILED:
			return "URG_ERR_CONNECT_FAILED";
		case URG_ERR_CLOSE_FAILED:
			return "URG_ERR_CLOSE_FAILED";
		case URG_ERR_NODESTINATION:
			return "URG_ERR_NODESTINATION";
		case URG_ERR_BADFIRMWARE:
			return "URG_ERR_BADFIRMWARE";
		case URG_ERR_SCIPVERSION:
			return "URG_ERR_SCIPVERSION";
		case URG_ERR_MEMORY:
			return "URG_ERR_MEMORY";
		case URG_ERR_UNSUPPORTED:
			return "URG_ERR_UNSUPPORTED";
		case URG_ERR_BADARG:
			return "URG_ERR_BADARG";
		case URG_ERR_NODATA:
			return "URG_ERR_NODATA";
		case URG_ERR_NOTSERIAL:
			return "URG_ERR_NOTSERIAL";
	}
	return "UNKNOWN_ERROR";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// URGSensorInfo class
////////////////////////////////////////////////////////////////////////////////////////////////////

URGSensorInfo::URGSensorInfo (void)
	: minRange (0), maxRange (0), steps (0), firstStep (0), lastStep (0), frontStep (0),
	standardSpeed (0), power (false), speed (0), baud (0), time (0), minAngle (0.0), maxAngle (0.0),
	resolution (0.0), scanableSteps (0)
{
}

// Set various known values based on what the manual says
void URGSensorInfo::SetDefaults (void)
{
	minRange = 20;
	maxRange = 4095;
	steps = 1024;
	firstStep = 44;
	lastStep = 725;
	frontStep = 384;
}

void URGSensorInfo::CalculateValues (void)
{
	resolution = DTOR (360.0) / steps;
	// If any of the steps are beyond INT_MAX, we have problems.
	// We also have an incredibly high-resolution sensor.
	minAngle = (static_cast<int> (firstStep) - static_cast<int> (frontStep)) * resolution;
	maxAngle = (static_cast<int> (lastStep) - static_cast<int> (frontStep)) * resolution;
	scanableSteps = lastStep - firstStep + 1;
}

string URGSensorInfo::AsString (void)
{
	stringstream ss;

	ss << "Vendor: " << vendor << endl;
	ss << "Product: " << product << endl;
	ss << "Firmware: " << firmware << endl;
	ss << "Protocol: " << protocol << endl;
	ss << "Serial: " << serial << endl;
	ss << "Model: " << model << endl;

	ss << "Mininum range: " << minRange << "mm\tMaximum range: " << maxRange << "mm" << endl;
	ss << "Steps in 360 degrees: " << steps << "\tScanable steps: " << scanableSteps << endl;
	ss << "First step: " << firstStep << "\tFront step: " << frontStep << "\tLast step: " <<
		lastStep << endl;
	ss << "Resolution: " << resolution << " radians/step" << endl;
	ss << "Minimum angle: " << minAngle << " radians\tMaximum angle: " << maxAngle <<
		" radians" << endl;
	ss << "Standard motor speed: " << standardSpeed << "rpm" << endl;

	ss << "Power status: " << (power ? "On" : "Off") << "\tMeasurement state: " <<
			measureState << endl;
	ss << "Motor speed: " << speed << "rpm\tBaud rate: " << baud << "bps" << endl;
	ss << "Time stamp: " << time << "ms" << endl;
	ss << "Sensor diagnostic: " << sensorDiagnostic << endl;

	return ss.str ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// URGData class
////////////////////////////////////////////////////////////////////////////////////////////////////

URGData::URGData (void)
	: _data (NULL), _length (0), _error (-1), _time (0)
{
}

URGData::URGData (unsigned short *data, unsigned int length, short error, unsigned int time)
	: _error (error), _time (time)
{
	_length = length;
	if (_length == 0)
		_data = NULL;
	else
	{
		if ((_data = new unsigned short[_length]) == NULL)
		{
			_length = 0;
			throw URGError (URG_ERR_MEMORY, "Failed to allocate space to copy data.");
		}
		memcpy (_data, data, sizeof (unsigned short) * _length);
	}
}

URGData::URGData (const URGData &rhs)
{
	_length = rhs.Length ();
	if (_length == 0)
		_data = NULL;
	else
	{
		if ((_data = new unsigned short[_length]) == NULL)
		{
			_length = 0;
			throw URGError (URG_ERR_MEMORY, "Failed to allocate space to copy data.");
		}
		memcpy (_data, rhs.Ranges (), sizeof (unsigned short) * _length);
	}
	_error = rhs.GetErrorCode ();
	_time = rhs.TimeStamp ();
}

URGData::~URGData (void)
{
	if (_data != NULL)
		delete[] _data;
}

string URGData::ErrorCodeToString (void)
{
	switch (_error)
	{
		case -1:
			return "No error.";
		case 0:
			return "Possibility of detected object is at 22m.";
		case 1:
			return "Reflected light has low intensity.";
		case 2:
			return "Reflected light has low intensity.";
		case 3:
			return "Reflected light has low intensity.";
		case 4:
			return "Reflected light has low intensity.";
		case 5:
			return "Reflected light has low intensity.";
		case 6:
			return "Possibility of detected object is at 5.7m.";
		case 7:
			return "Distance data on the preceding and succeeding steps have errors.";
		case 8:
			return "Others.";
		case 9:
			return "The same step had error in the last two scan.";
		case 10:
			return "Others.";
		case 11:
			return "Others.";
		case 12:
			return "Others.";
		case 13:
			return "Others.";
		case 14:
			return "Others.";
		case 15:
			return "Others.";
		case 16:
			return "Possibility of detected object is in the range 4096mm.";
		case 17:
			return "Others.";
		case 18:
			return "Unspecified.";
		case 19:
			return "Non-measurable distance.";
		default:
			stringstream ss;
			ss << "Unknown error code: " << _error;
			return ss.str ();
	}
}

URGData& URGData::operator= (const URGData &rhs)
{
	if (rhs.Length () == 0)
	{
		_length = 0;
		if (_data != NULL)
			delete[] _data;
		_data = NULL;
		_error = rhs.GetErrorCode ();
		_time = rhs.TimeStamp ();
	}
	else
	{
		unsigned int rhsLength = rhs.Length ();
		unsigned short *newData;
		if (rhsLength != _length)
		{
			// Copy the data into a temporary variable pointing to new space (prevents dangling
			// pointers on allocation error and prevents self-assignment making a mess).
			if ((newData = new unsigned short[rhsLength]) == NULL)
				throw URGError (URG_ERR_MEMORY, "Failed to allocate space to copy data.");
			memcpy (newData, rhs.Ranges (), sizeof (unsigned short) * rhsLength);
			if (_data != NULL)
				delete[] _data;
			_data = newData;
			_length = rhs.Length ();
		}
		else
		{
			// If lengths are the same, no need to reallocate
			memcpy (_data, rhs.Ranges (), sizeof (unsigned short) * _length);
		}

		_error = rhs.GetErrorCode ();
		_time = rhs.TimeStamp ();
	}

	return *this;
}

unsigned short URGData::operator[] (unsigned int index)
{
	if (index >= _length)
		throw URGError (URG_ERR_BADARG, "Invalid data index.");
	return _data[index];
}

string URGData::AsString (void)
{
	stringstream ss;

	ss << _length << " readings:" << endl;
	for (unsigned int ii = 0; ii < _length; ii++)
		ss << _data[ii] << "\t";
	ss << endl << "Data error: (" << _error << ") " << ErrorCodeToString () << endl;
	ss << "Time stamp: " << _time << endl;

	return ss.str ();
}

void URGData::CleanUp (void)
{
	if (_data != NULL)
		delete[] _data;
	_data = NULL;
	_length = 0;
	_error = 0;
	_time = 0;
}

void URGData::AllocateData (unsigned int length)
{
	// If no data yet, allocate new
	if (_data == NULL)
	{
		if ((_data = new unsigned short[length]) == NULL)
		{
			_length = 0;
			throw URGError (URG_ERR_MEMORY, "Failed to allocate space to copy data.");
		}
		_length = length;
	}
	// If there is data, reallocate only if the length is different
	else if (length != _length)
	{
		delete[] _data;
		if ((_data = new unsigned short[length]) == NULL)
		{
			_length = 0;
			throw URGError (URG_ERR_MEMORY, "Failed to allocate space to copy data.");
		}
		_length = length;
	}
	// Else data is already allocated to the right length, so do nothing
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// URGLaser class
////////////////////////////////////////////////////////////////////////////////////////////////////

// Public API
////////////////////////////////////////////////////////////////////////////////////////////////////

URGLaser::URGLaser (void)
	: _port (NULL), _scipVersion (1), _verbose (false), _minAngle (0.0), _maxAngle (0.0),
	_resolution (0.0), _firstStep (0), _lastStep (0), _frontStep (0)
{
}

URGLaser::~URGLaser (void)
{
	if (_port != NULL)
		delete _port;
}

void URGLaser::Open (string portOptions)
{
	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Creating and opening port using options: " <<
			portOptions << endl;
	}
	_port = flexiport::CreatePort (portOptions);
	_port->Open ();

	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Connected using " << _port->GetPortType () <<
			" connection." << endl;
		cerr << _port->GetStatus ();
	}
	_port->Flush ();

	// Figure out the SCIP version currently in use and switch to a higher one if possible
	GetAndSetSCIPVersion ();
	// Get some values we need for providing default ranges
	GetDefaults ();
}

void URGLaser::Close (void)
{
	if (!_port)
		throw URGError (URG_ERR_CLOSE_FAILED, "Port is not open.");
	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Closing connection." << endl;
	delete _port;
	_port = NULL;
}

bool URGLaser::IsOpen (void) const
{
	if (_port != NULL)
		return _port->IsOpen ();
	return false;
}

void URGLaser::SetPower (bool on)
{
	if (_scipVersion == 1)
	{
		if (on)
		{
			if (_verbose)
				cerr << "URGLaser::" << __func__ << "() Turning laser on." << endl;
			SendCommand ("L", "1", 1, NULL);
		}
		else
		{
			if (_verbose)
				cerr << "URGLaser::" << __func__ << "() Turning laser off." << endl;
			SendCommand ("L", "0", 1, NULL);
		}
		SkipLines (1);
	}
	else if (_scipVersion == 2)
	{
		if (on)
		{
			if (_verbose)
				cerr << "URGLaser::" << __func__ << "() Turning laser on." << endl;
			SendCommand ("BM", NULL, 0, "02");
		}
		else
		{
			if (_verbose)
				cerr << "URGLaser::" << __func__ << "() Turning laser off." << endl;
			SendCommand ("QT", NULL, 0, "02");
		}
		SkipLines (1);
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");
}

// This function assumes that both the port and the laser scanner are already set to the same baud.
void URGLaser::SetBaud (unsigned int baud)
{
	if (_port->GetPortType () != "serial")
		throw URGError (URG_ERR_NOTSERIAL, "Cannot change baud rate of non-serial connection.");

	char newBaud[13];
	memset (newBaud, 0, sizeof (char) * 13);

	if (baud != 19200 && baud != 57600 && baud != 115200 &&
			baud != 250000 && baud != 500000 && baud != 750000)
	{
		stringstream ss;
		ss << "Bad baud rate: " << baud << endl;
		throw URGError (URG_ERR_BADARG, ss.str ());
	}
	NumberToString (baud, newBaud, 6);

	if (_scipVersion == 1)
	{
		// Send the command to change baud rate
		SendCommand ("S", newBaud, 13, NULL);
		SkipLines (1);
		// Change the port's baud rate
		reinterpret_cast<SerialPort*> (_port)->SetBaudRate (baud);
	}
	else if (_scipVersion == 2)
	{
		// Send the command to change baud rate
		SendCommand ("SS", newBaud, 6, "03");
		SkipLines (1);
		// Change the port's baud rate
		reinterpret_cast<SerialPort*> (_port)->SetBaudRate (baud);
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");
}

void URGLaser::Reset (void)
{
	if (_scipVersion == 1)
		throw URGError (URG_ERR_UNSUPPORTED, "SCIP version 1 does not support the reset command.");
	else if (_scipVersion == 2)
	{
		if (_verbose)
			cerr << "URGLaser::" << __func__ << "() Resetting laser." << endl;
		SendCommand ("RS", NULL, 0, NULL);
		SkipLines (1);
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");
}

void URGLaser::SetMotorSpeed (unsigned int speed)
{
	if (_scipVersion == 1)
	{
		throw URGError (URG_ERR_UNSUPPORTED,
				"SCIP version 1 does not support the set motor speed command.");
	}
	else if (_scipVersion == 2)
	{
		// Sanity check the value
		if ((speed > 600 || speed < 540 || (speed % 6) != 0) && speed != 0)
			throw URGError (URG_ERR_BADARG, "Invalid motor speed.");
		char buffer[3];
		buffer[2] = '\0';
		if (speed == 0)
		{
			if (_verbose)
				cerr << "URGLaser::" << __func__ << "() Reseting motor speed to default." << endl;
			buffer[0] = '9';
			buffer[1] = '9';
		}
		else
		{
			if (_verbose)
			{
				cerr << "URGLaser::" << __func__ << "() Setting motor speed to " <<
					speed << "rpm." << endl;
			}
			if (speed == 540)
			{
				buffer[0] = '1';
				buffer[1] = '0';
			}
			else
			{
				buffer[0] = '0';
				buffer[1] = 100 - (speed / 6) + 0x30;
			}
		}
		SendCommand ("CR", buffer, 2, "03");
		SkipLines (1);
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");
}

void URGLaser::GetSensorInfo (URGSensorInfo *info)
{
	if (info == NULL)
		throw URGError (URG_ERR_NODESTINATION, "No info object provided.");

	if (_scipVersion == 1)
	{
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ <<
				"() Getting sensor information using SCIP version 1." << endl;
		}

		info->SetDefaults ();

		char buffer[SCIP1_LINE_LENGTH];
		memset (buffer, 0, sizeof (char) * SCIP1_LINE_LENGTH);

		SendCommand ("V", NULL, 0, NULL);
		// Get the vendor info line
		ReadLine (buffer);
		info->vendor = &buffer[5]; // Chop off the "VEND:" tag
		// Get the product info line
		ReadLine (buffer);
		info->product = &buffer[5];
		// Get the firmware line
		ReadLine (buffer);
		info->firmware = &buffer[5];
		// Get the protocol version line
		ReadLine (buffer);
		info->protocol = &buffer[5];
		// Get the serial number
		ReadLine (buffer);
		info->serial = &buffer[5];
		// Get either the status line or the end of message
		ReadLine (buffer);
		if (buffer[0] != '\n')
		{
			// Got a status line
			info->sensorDiagnostic = &buffer[5];
			SkipLines (1);
		}

		// Check the firmware version major number. If it's >=3 there is probably some extra info
		// in the firmware line.
		// e.g.: FIRM:3.1.04,07/08/02(20-4095[mm],240[deg],44-725[step],600[rpm])
		// Note that this example is right up against the maximum SCIP v1 line length of 64 bytes.
		if (atoi (info->firmware.c_str ()) >= 3)
		{
			if (_verbose)
				cerr << "SCIP1 Firmware line for parsing: " << info->firmware << endl;
			// Now the fun part: parsing the line. It would be nice if we could use the POSIX regex
			// functions, but since MS doesn't believe in POSIX we get to do it the hard way.
			// Start by finding the first (
			char *valueStart;
			if ((valueStart = strchr (info->firmware.c_str (), '(')) == NULL)
			{
				// No bracket? Crud. Fail and use the hard-coded values from the manual.
				info->CalculateValues ();
			}
			// Now put it through sscanf and hope...
			int aperture;
			int numFound = sscanf (valueStart, "%d-%d[mm],%d[deg],%d-%d[step],%d[rpm]",
									&info->minRange, &info->maxRange, &aperture,
									&info->firstStep, &info->lastStep, &info->speed);
			if (numFound != 6)
			{
				// Didn't get enough values out, assume unknown format and fall back on the defaults
				info->SetDefaults ();
				info->CalculateValues ();
				if (_verbose)
				{
					cerr << "Retrieved sensor info (hard-coded):" << endl;
					cerr << info->AsString ();
				}
			}

			// Need to calculate stuff differently since it gave us an aperture value
			info->resolution = static_cast<double> (aperture) /
									static_cast<double> (info->lastStep - info->firstStep);
			// Assume that the range is evenly spread
			info->scanableSteps = info->lastStep - info->firstStep + 1;
			info->frontStep = info->scanableSteps / 2;
			info->minAngle = (info->firstStep - info->frontStep) * info->resolution;
			info->maxAngle = (info->lastStep - info->frontStep) * info->resolution;
		}
		else
		{
			// We're stuck with hard-coded defaults from the manual (already set earlier).
			info->CalculateValues ();
			if (_verbose)
			{
				cerr << "Retrieved sensor info (hard-coded):" << endl;
				cerr << info->AsString ();
			}
		}
	}
	else if (_scipVersion == 2)
	{
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ <<
				"() Getting sensor information using SCIP version 2." << endl;
		}

		char buffer[SCIP2_LINE_LENGTH];
		memset (buffer, 0, sizeof (char) * SCIP2_LINE_LENGTH);

		// We need to send three commands to get all the info we want: VV, PP and II
		SendCommand ("VV", NULL, 0, NULL);
		// Get the vendor info line
		ReadLineWithCheck (buffer, -1, true);
		info->vendor = &buffer[5]; // Chop off the "VEND:" tag
		// Get the product info line
		ReadLineWithCheck (buffer, -1, true);
		info->product = &buffer[5];
		// Get the firmware line
		ReadLineWithCheck (buffer, -1, true);
		info->firmware = &buffer[5];
		// Get the protocol version line
		ReadLineWithCheck (buffer, -1, true);
		info->protocol = &buffer[5];
		// Get the serial number
		ReadLineWithCheck (buffer, -1, true);
		info->serial = &buffer[5];
		// Skip the end-of-message
		SkipLines (1);

		// Next up, PP
		SendCommand ("PP", NULL, 0, NULL);
		// Get the model line
		ReadLineWithCheck (buffer, -1, true);
		info->model = &buffer[5];
		// On to the fun ones that require parsing
		ReadLineWithCheck (buffer, -1, true);
		info->minRange = atoi (&buffer[5]);
		ReadLineWithCheck (buffer, -1, true);
		info->maxRange = atoi (&buffer[5]);
		ReadLineWithCheck (buffer, -1, true);
		info->steps = atoi (&buffer[5]);
		ReadLineWithCheck (buffer, -1, true);
		info->firstStep = atoi (&buffer[5]);
		ReadLineWithCheck (buffer, -1, true);
		info->lastStep = atoi (&buffer[5]);
		ReadLineWithCheck (buffer, -1, true);
		info->frontStep = atoi (&buffer[5]);
		ReadLineWithCheck (buffer, -1, true);
		info->standardSpeed = atoi (&buffer[5]);
		// Skip the end-of-message
		SkipLines (1);

		// Command II: Revenge of the Commands.
		SendCommand ("II", NULL, 0, NULL);
		// Skip the model line (we already have it from PP)
		SkipLines (1);
		// Get and parse the power state
		ReadLineWithCheck (buffer, -1, true);
		if (strncmp (&buffer[5], "OFF", 3) == 0)
			info->power = false;
		else
			info->power = true;
		// Motor speed
		ReadLineWithCheck (buffer, -1, true);
		// TODO: check if the format of this line changes if the motor speed is changed
		if (sscanf (buffer, "SCSP:%*7s(%d[rpm]", &info->speed) != 1)
			throw URGError (URG_ERR_PROTOCOL, "Motor speed line parse failed.");
		// Measuring state
		ReadLineWithCheck (buffer, -1, true);
		info->measureState = &buffer[5];
		// Baud rate
		ReadLineWithCheck (buffer, -1, true);
		if (sscanf (buffer, "SBPS:%d[bps]", &info->baud) != 1)
			throw URGError (URG_ERR_PROTOCOL, "Baud rate line parse failed.");
		// Time stamp
		ReadLineWithCheck (buffer, -1, true);
		if (sscanf (buffer, "TIME:%x", &info->time) != 1)
			throw URGError (URG_ERR_PROTOCOL, "Timestamp line parse failed.");
		// Diagnostic
		ReadLineWithCheck (buffer, -1, true);
		info->sensorDiagnostic = &buffer[5];
		// Skip the end-of-message
		SkipLines (1);

		info->CalculateValues ();
		if (_verbose)
		{
			cerr << "Retrieved sensor info:" << endl;
			cerr << info->AsString ();
		}
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");
}

unsigned int URGLaser::GetTime (void)
{
	if (_scipVersion == 1)
	{
		throw URGError (URG_ERR_UNSUPPORTED,
				"SCIP version 1 does not support the get time command.");
	}
	else if (_scipVersion == 2)
	{
		if (_verbose)
			cerr << "URGLaser::" << __func__ << "() Retrieving time from laser." << endl;
		SendCommand ("TM", "0", 1, NULL);
		SendCommand ("TM", "1", 1, NULL);
		char buffer[7];
		ReadLineWithCheck (buffer, 6);
		SendCommand ("TM", "2", 1, NULL);
		SkipLines (1);
		// We need to decode the time value that's in the buffer
		return Decode4ByteValue (buffer);
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");

	return 0;
}

unsigned int URGLaser::GetRanges (URGData *data, int startStep, int endStep,
		 							unsigned int clusterCount)
{
	if (data == NULL)
		throw URGError (URG_ERR_NODESTINATION, "No data destination provided.");

	char buffer[11];
	memset (buffer, 0, sizeof (char) * 11);

	if (startStep < 0)
		startStep = _firstStep;
	if (endStep < 0)
		endStep = _lastStep;

	unsigned int numSteps = (endStep - startStep + 1) / clusterCount;
	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Reading " << numSteps << " ranges between " <<
			startStep << " and " << endStep << " with a cluster count of " << clusterCount << endl;
	}

	if (_scipVersion == 1)
	{
		// Send the command to ask for the most recent range data from startStep to endStep
		NumberToString (startStep, buffer, 3);
		NumberToString (endStep, &buffer[3], 3);
		NumberToString (clusterCount, &buffer[6], 2);
		data->_error = SendCommand ("G", buffer, 8, NULL);
		// In SCIP1 mode we're going to get back 2-byte data
		Read2ByteRangeData (data, numSteps);
	}
	else if (_scipVersion == 2)
	{
		// Send the command to ask for the most recent range data from startStep to endStep
		NumberToString (startStep, buffer, 4);
		NumberToString (endStep, &buffer[4], 4);
		NumberToString (clusterCount, &buffer[8], 2);
		data->_error = SendCommand ("GD", buffer, 10, NULL);
		// There will be a timestamp before the data (if there is data)
		// Normally we would send 6 for the expected length, but we may get no timestamp back if
		// there was no data.
		if (ReadLineWithCheck (buffer) == 0)
			throw URGError (URG_ERR_NODATA, "No data received. Check data error code.");
		data->_time = Decode4ByteValue (buffer);
		// In SCIP2 mode we're going to get back 3-byte data because we're sending the GD command
		Read3ByteRangeData (data, numSteps);
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");

	return data->_length;
}

unsigned int URGLaser::GetRanges (URGData *data, double startAngle,
									double endAngle, unsigned int clusterCount)
{
	if (data == NULL)
		throw URGError (URG_ERR_NODESTINATION, "No data destination provided.");

	// Calculate the given angles in steps, rounding towards _frontStep
	int startStep, endStep;
	startStep = AngleToStep (startAngle);
	endStep = AngleToStep (endAngle);

	// Check the steps are within the allowable range
	if (startStep < _firstStep || startStep > _lastStep)
		throw URGError (URG_ERR_BADARG, "Start step is out of range.");
	if (endStep < _firstStep || endStep > _lastStep)
		throw URGError (URG_ERR_BADARG, "End step is out of range.");

	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Start angle " << startAngle << " is step " <<
			startStep << ", end angle " << endAngle << " is step " << endStep << endl;
	}

	// Get the data
	return GetRanges (data, startStep, endStep, clusterCount);
}

unsigned int URGLaser::GetNewRanges (URGData *data, int startStep, int endStep,
									unsigned int clusterCount)
{
	if (data == NULL)
		throw URGError (URG_ERR_NODESTINATION, "No data destination provided.");

	if (_scipVersion == 1)
	{
		throw URGError (URG_ERR_UNSUPPORTED,
				"SCIP version 1 does not support the get new ranges command.");
	}
	else if (_scipVersion == 2)
	{
		char buffer[14];
		memset (buffer, 0, sizeof (char) * 14);

		if (startStep < 0)
			startStep = _firstStep;
		if (endStep < 0)
			endStep = _lastStep;

		unsigned int numSteps = (endStep - startStep + 1) / clusterCount;
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ << "() Reading " << numSteps <<
				" new ranges between " << startStep << " and " << endStep <<
				" with a cluster count of " << clusterCount << endl;
		}

		// Send the command to ask for the most recent range data from startStep to endStep
		NumberToString (startStep, buffer, 4);
		NumberToString (endStep, &buffer[4], 4);
		NumberToString (clusterCount, &buffer[8], 2);
		NumberToString (1, &buffer[10], 1);
		NumberToString (1, &buffer[11], 2);
		data->_error = SendCommand ("MD", buffer, 13, NULL);
		// There will be a timestamp before the data (if there is data)
		// Normally we would send 6 for the expected length, but we may get no timestamp back if
		// there was no data.
		if (ReadLineWithCheck (buffer) == 0)
			throw URGError (URG_ERR_NODATA, "No data received. Check data error code.");
		data->_time = Decode4ByteValue (buffer);
		// In SCIP2 mode we're going to get back 3-byte data because we're sending the MD command
		Read3ByteRangeData (data, numSteps);
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");

	return data->_length;
}

unsigned int URGLaser::GetNewRanges (URGData *data, double startAngle, double endAngle,
									unsigned int clusterCount)
{
	if (data == NULL)
		throw URGError (URG_ERR_NODESTINATION, "No data destination provided.");
	if (_scipVersion == 1)
	{
		throw URGError (URG_ERR_UNSUPPORTED,
				"SCIP version 1 does not support the get new ranges command.");
	}

	// Calculate the given angles in steps, rounding towards _frontStep
	int startStep, endStep;
	startStep = AngleToStep (startAngle);
	endStep = AngleToStep (endAngle);

	// Check the steps are within the allowable range
	if (startStep < _firstStep || startStep > _lastStep)
		throw URGError (URG_ERR_BADARG, "Start step is out of range.");
	if (endStep < _firstStep || endStep > _lastStep)
		throw URGError (URG_ERR_BADARG, "End step is out of range.");

	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Start angle " << startAngle << " is step " <<
			startStep << ", end angle " << endAngle << " is step " << endStep << endl;
	}

	// Get the data
	return GetNewRanges (data, startStep, endStep, clusterCount);
}

double URGLaser::StepToAngle (unsigned int step)
{
	return (static_cast<int> (step) - static_cast<int> (_frontStep)) * _resolution;
}

unsigned int URGLaser::AngleToStep (double angle)
{
	unsigned int result;
	double resultF;
	resultF = _frontStep +
		(static_cast<double> (angle) / static_cast<double> (_resolution));
	// Round towards _frontStep so that the step values are always inside the angles given
	if (resultF < _frontStep)
		result = static_cast<int> (ceil (resultF));
	else
		result = static_cast<int> (floor (resultF));

	return result;
}

// Private functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// If expectedLength is not -1, it should include the terminating line feed but not the NULL
// (although the buffer still has to include this).
// If expectedLenght is -1, this function expects buffer to be a certain length to allow up to the
// maximum line length to be read. See SCIP1_LINE_LENGTH and SCIP2_LINE_LENGTH.
// The line feed that terminates a line will be replaced with a NULL.
// The return value is the number of bytes received, not including the NULL byte or the line feed.
int URGLaser::ReadLine (char *buffer, int expectedLength)
{
	int lineLength = 0;

	if (expectedLength == -1)
	{
		int maxLength = (_scipVersion == 1) ? SCIP1_LINE_LENGTH : SCIP2_LINE_LENGTH;
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ << "() Reading up to " << maxLength << " bytes." <<
				endl;
		}
		// We need to get at least 1 byte in a line: the line feed.
		if ((lineLength = _port->ReadLine (buffer, maxLength)) < 0)
			throw URGError (URG_ERR_READ, "Timed out trying to read a line.");
		else if (lineLength == 0)
			throw URGError (URG_ERR_READ, "No data received when trying to read a line.");
		// Replace the line feed with a NULL
		buffer[lineLength - 1] = '\0';
	}
	else
	{
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ << "() Reading exactly " << expectedLength <<
				" bytes." << endl;
		}
		if ((lineLength = _port->ReadLine (buffer, expectedLength + 1)) < 0) // +1 for the NULL
			throw URGError (URG_ERR_READ, "Timed out trying to read a line.");
		else if (lineLength == 0)
			throw URGError (URG_ERR_READ, "No data received when trying to read a line.");
		else if (lineLength < expectedLength)
		{
			stringstream ss;
			ss << "URGLaser::" << __func__ << "() Got an incorrect line length: " << lineLength <<
				" != " << expectedLength;
			throw URGError (URG_ERR_PROTOCOL, ss.str ());
		}
		// Replace the line feed with a NULL
		buffer[lineLength - 1] = '\0';
	}

	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Read " << lineLength << " bytes." << endl;
		cerr << "URGLaser::" << __func__ << "() Line is " << buffer << endl;
	}
	return lineLength - 1; // Line feed not included
}

// This function will read a line and then calculate its checksum, comparing it with the checksum
// at the end of the line. The checksum will be removed (along with the semi-colon, if present).
// buffer and expectedLength args are as for ReadLine().
// If hasSemicolon is true, the byte before the checksum is assumed to be the semi-colon separator
// and so not a part of the checksum. If it's not a semi-colon, an exception is thrown.
// Empty lines (i.e. a line that is just the line feed, as at the end of the message) will result in
// a return value of zero and no checksum check will be performed. Otherwise the number of actual
// data bytes (i.e. excluding the checksum and semicolon) will be returned.
int URGLaser::ReadLineWithCheck (char *buffer, int expectedLength, bool hasSemicolon)
{
	int lineLength = ReadLine (buffer, expectedLength);
	if (_scipVersion == 1)
	{
		// No checksums in SCIP version 1
		return lineLength;
	}

	// If the line is empty, assume it was a line-feed message terminator, in which case there is no
	// checksum to check.
	if (lineLength == 0)
		return 0;

	// Ignore the checksum itself, and possibly a semicolon (ReadLine has already chopped off the
	// line feed for us).
	int bytesToConsider = lineLength - 1 - (hasSemicolon ? 1 : 0);
	int checksumIndex = bytesToConsider + (hasSemicolon ? 1 : 0);
	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Considering " << bytesToConsider <<
			" bytes for checksum from a line length of " << lineLength << " bytes." << endl;
	}
	if (bytesToConsider < 1)
	{
		stringstream ss;
		ss << "Not enough bytes to calculate checksum with: " << bytesToConsider <<
			" bytes (line length is " << lineLength << " bytes).";
		throw URGError (URG_ERR_PROTOCOL, ss.str ());
	}

	int checkSum = 0;
	// Start by adding the byte values
	for (int ii = 0; ii < bytesToConsider; ii++)
		checkSum += buffer[ii];
	// Take the lowest 6 bits
	checkSum &= 0x3F;
	// Add 0x30
	checkSum += 0x30;

	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ << "() Calculated checksum = " << checkSum << " (" <<
			static_cast<char> (checkSum) << "), given checksum = " <<
			static_cast<int> (buffer[checksumIndex]) << " (" << buffer[checksumIndex] <<
			")" << endl;
	}
	if (checkSum != static_cast<int> (buffer[checksumIndex]))
	{
		stringstream ss;
		ss << "Invalid checksum -  given: " << static_cast<int> (buffer[checksumIndex]) <<
			", calculated: " << checkSum;
		throw URGError (URG_ERR_PROTOCOL, ss.str ());
	}

	// Null out the semi-colon (if there) and checksum
	buffer[bytesToConsider] = '\0';

	return bytesToConsider;
}

// Reads lines until the number specified has passed.
void URGLaser::SkipLines (int count)
{
	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Skipping " << count << " lines." << endl;
	if (_port->SkipUntil (0x0A, count) < 0)
		throw URGError (URG_ERR_READ, "Timed out while skipping.");
}

// Sends a command with optional parameters and checks that the echo of the command and parameters
// sent are correct, and that the returned status code is 0 or the first byte of extraOK (for
// SCIP1), or 00, 99 or the first two bytes of extraOK (for SCIP2).
// cmd must be a 1 byte string for SCIP1 and a 2-byte NULL-terminated string for SCIP2.
// If paramLength is 0, no parameters will be sent or expected in the reply.
// extraOK must be a 1-byte string for SCIP1 and a 2-byte string for SCIP2.
// Return value is the status code returned for the command.
int URGLaser::SendCommand (char *cmd, char *param, int paramLength, char *extraOK)
{
	int statusCode = -1;
	char response[16];
	if (_scipVersion == 1)
	{
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ << "() Writing in SCIP1 mode. Command is " <<
				cmd[0] << ", parameters length is " << paramLength << endl;
		}
		// Write the command
		if (_port->Write (cmd, 1) < 1)
			throw URGError (URG_ERR_WRITE, "Failed to write command byte.");
		if (paramLength > 0)
		{
			if (_port->Write (param, paramLength) < paramLength)
				throw URGError (URG_ERR_WRITE, "Failed to write command parameters.");
		}
		if (_port->Write ("\n", 1) < 1)
			throw URGError (URG_ERR_WRITE, "Failed to write termination character.");

		// Read back the response (should get at least 4 bytes , possibly up to 16 including \n's
		// depending on the parameters): cmd[0] params \n status \n
		int statusIndex = 2 + paramLength;
		ReadLine (response, 2 + paramLength);
		ReadLine (&response[statusIndex], 2);
		// First make sure that the echoed command matches
		if (response[0] != cmd[0])
		{
			throw URGError (URG_ERR_PROTOCOL, string ("Incorrect command echo: ") + cmd[0] +
					string (" != ") + response[0]);
		}
		// Then compare the parameters
		if (paramLength > 0)
		{
			if (memcmp (&response[1], param, paramLength) != 0)
				throw URGError (URG_ERR_PROTOCOL, string ("Incorrect paramaters echo for command ")
						+ cmd[0]);
		}
		// Next up, check the status byte
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ << "() Command response status: " <<
				response[statusIndex] << endl;
		}
		if (response[statusIndex] != '0')
		{
			if (extraOK != NULL)
			{
				if (response[statusIndex] != extraOK[0])
				{
					stringstream ss;
					ss << "Bad response to " << cmd[0] << " command: " << " " <<
						SCIP1ErrorToString (response[statusIndex], cmd[0]);
					throw URGError (URG_ERR_PROTOCOL, ss.str ());
				}
			}
			else
			{
				stringstream ss;
				ss << "Bad response to " << cmd[0] << " command: " << response[statusIndex] <<
					" " << SCIP1ErrorToString (response[statusIndex], cmd[0]);
				throw URGError (URG_ERR_PROTOCOL, ss.str ());
			}
		}
		statusCode = atoi (&response[statusIndex]);
		// All OK, data starts at beginning of port's buffer
	}
	else if (_scipVersion == 2)
	{
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ << "() Writing in SCIP2 mode. Command is " <<
				cmd << ", parameters length is " << paramLength << endl;
		}
		// Write the command
		if (_port->Write (cmd, 2) < 2)
			throw URGError (URG_ERR_WRITE, "Failed to write command byte.");
		if (paramLength > 0)
		{
			if (_port->Write (param, paramLength) < paramLength)
				throw URGError (URG_ERR_WRITE, "Failed to write command parameters.");
		}
		if (_port->Write ("\n", 1) < 1)
			throw URGError (URG_ERR_WRITE, "Failed to write termination character.");

		// Read back the command echo (minimum of 3 bytes, maximum of 16 bytes)
		ReadLine (response, 3 + paramLength);
		// Check the echo is correct
		if (response[0] != cmd[0] || response[1] != cmd[1])
		{
			stringstream ss;
			ss << "Incorrect command echo: " << cmd << " != " << response[0] << response[1];
			throw URGError (URG_ERR_PROTOCOL, ss.str ());
		}
		// Then compare the parameters
		if (paramLength > 0)
		{
			if (memcmp (&response[2], param, paramLength) != 0)
				throw URGError (URG_ERR_PROTOCOL, string ("Incorrect paramaters echo for command ")
						+ cmd);
		}

		// The next line should be the status line
		ReadLineWithCheck (response, 4);
		if (_verbose)
		{
			cerr << "URGLaser::" << __func__ << "() Command response status: " << response[0] <<
				response[1] << endl;
		}
		// Check the status code is OK
		response[2] = '\0';
		if (!(response[0] == '0' && response[1] == '0') &&
			!(response[0] == '9' && response[1] == '9'))
		{
			if (extraOK != NULL)
			{
				if (response[0] != extraOK[0] || response[1] != extraOK[1])
				{
					stringstream ss;
					ss << "Bad response to " << cmd << " command: " << response[0] << response[1] <<
						" " << SCIP2ErrorToString (response, cmd);
					throw URGError (URG_ERR_PROTOCOL, ss.str ());
				}
			}
			else
			{
				stringstream ss;
				ss << "Bad response to " << cmd << " command: " << response[0] << response[1] <<
					" " << SCIP2ErrorToString (response, cmd);
				throw URGError (URG_ERR_PROTOCOL, ss.str ());
			}
		}
		statusCode = atoi (response);
		// All OK, data starts at beginning of port's buffer
	}
	else
		throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");

	return statusCode;
}

void URGLaser::GetAndSetSCIPVersion (void)
{
	bool scip1Failed = false;

	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Testing SCIP protocol version." << endl;
	// Try SCIP version 1 first by sending an info command
	try
	{
		SendCommand ("V", NULL, 0, NULL);
	}
	catch (URGError &e)
	{
		// That didn't work too well...
		if (_verbose)
			cerr << "URGLaser::" << __func__ << "() Initial SCIP version 1 test failed." << endl;
		scip1Failed = true;
	}

	if (scip1Failed)
	{
		// Currently using SCIP version 2
		_scipVersion = 2;

		_port->Flush ();
		// Make sure by sending a VV command
		try
		{
			SendCommand ("VV", NULL, 0, NULL);
		}
		catch (URGError &e)
		{
			throw URGError (URG_ERR_SCIPVERSION, "SCIP versions 1 and 2 failed.");
		}

		// Otherwise all OK, dump the rest of the result
		SkipLines (6);
		if (_verbose)
			cerr << "URGLaser::" << __func__ << "() Using SCIP version 2." << endl;
		return;
	}
	else
	{
		// Currently using SCIP version 1
		// Get the firmware version and check if we can move to SCIP version 2
		_scipVersion = 1;

		// Skip the vendor and product info
		SkipLines (2);
		// Get the firmware line
		char buffer[SCIP1_LINE_LENGTH];
		memset (buffer, 0, sizeof (char) * SCIP1_LINE_LENGTH);
		try
		{
			// If the laser is already in SCIP2 mode this has a tendency to time out (rather than
			// the laser doing what the manual says and giving us an error code to the command
			// that we sent to get the info).
			ReadLine (buffer);
		}
		catch (URGError e)
		{
			if (e.Code () != URG_ERR_READ) // We're only interested in timeouts
				throw;
			if (_verbose)
			{
				cerr << "URGLaser::" << __func__ <<
					"() Timed out trying SCIP version 1, trying SCIP version 2." << endl;
			}
			// Already in SCIP version 2 mode.
			_scipVersion = 2;
			_port->Flush ();
			// Make sure by sending a VV command
			try
			{
				SendCommand ("VV", NULL, 0, NULL);
			}
			catch (URGError &e)
			{
				cout << "error: " << e.Code() << " " << e.what() << endl;
				throw URGError (URG_ERR_SCIPVERSION, "SCIP versions 1 and 2 failed.");
			}
			// Otherwise all OK, dump the rest of the result
			SkipLines (6);
			if (_verbose)
				cerr << "URGLaser::" << __func__ << "() Using SCIP version 2." << endl;
			return;
		}

		if (strncmp (buffer, "FIRM:", 5) != 0)
		{
			throw URGError (URG_ERR_PROTOCOL,
				"'FIRM:' was not found when checking firmware version.");
		}
		// Pull out the major version number
		int majorVer = strtol (&buffer[5], NULL, 10);
		if (errno == ERANGE)
			throw URGError (URG_ERR_BADFIRMWARE, "Out-of-range firmware version.");
		if (_verbose)
			cerr << "URGLaser::" << __func__ << "() Firmware major version is " << majorVer << endl;
		// Dump the rest of the V command result (one of these will be the empty last line)
		SkipLines (3);

		// If the firmware version is less than 3, we're stuck with SCIP version 1.
		if (majorVer < 3)
		{
			if (_verbose)
				cerr << "URGLaser::" << __func__ <<
					"() Firmware does not support SCIP version 2; using SCIP version 1." << endl;
			return;
		}
		// Otherwise we can try SCIP version 2
		else
		{
			_port->Flush ();
			// We'll hijack the SendCommand function a bit here. Normally it takes 1-byte commands,
			// (we're currently using SCIP version 1, remember), but the command to change to SCIP
			// version 2 is 7 bytes long (why did they have to do it that way?). So send the first
			// byte as the command and the other 6 as parameters.
			try
			{
				SendCommand ("S", "CIP2.0", 6, NULL);
			}
			catch (URGError &e)
			{
				if (_verbose)
					cerr << "URGLaser::" << __func__ <<
						"() Could not change to SCIP version 2; using SCIP version 1." << endl;
				return;
			}
			// There'll be a trailing line on the end
			SkipLines (1);

			// Changed to SCIP version 2
			if (_verbose)
				cerr << "URGLaser::" << __func__ << "() Using SCIP version 2." << endl;
			_scipVersion = 2;
			return;
		}
	}

	// Fallback case if didn't find a good SCIP version and return above
	throw URGError (URG_ERR_SCIPVERSION, "Unknown SCIP version.");
}

void URGLaser::GetDefaults (void)
{
	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Getting default values." << endl;

	// Get the laser's info
	URGSensorInfo info;
	GetSensorInfo (&info);

	_minAngle = info.minAngle;
	_maxAngle = info.maxAngle;
	_resolution = info.resolution;
	_firstStep = info.firstStep;
	_lastStep = info.lastStep;
	_frontStep = info.frontStep;
	_maxRange = info.maxRange;
	if (_verbose)
	{
		cerr << "URGLaser::" << __func__ <<
			"() Got default values: " << _minAngle << " " << _maxAngle << " " << _resolution <<
			" " << _firstStep << " " << _lastStep << " " << _frontStep << " " << _maxRange << endl;
	}
}

void URGLaser::Read2ByteRangeData (URGData *data, unsigned int numSteps)
{
	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Reading " << numSteps << " ranges." << endl;

	// This will automatically take care of whether it actually needs to (re)allocate or not.
	data->AllocateData (numSteps);

	// 2 byte data is easy since it fits neatly in a 64-byte block
	char buffer[SCIP2_LINE_LENGTH];
	unsigned int currentStep = 0;
	int numBytesInLine = 0;
	while (true)
	{
		// Read a line of data
		numBytesInLine = ReadLineWithCheck (buffer);
		// Check if we've reached the end of the data
		if (numBytesInLine == 0)
			break;
		// Process pairs of bytes until we encounter the end of the line
		for (int ii = 0; ii < numBytesInLine; ii += 2, currentStep++)
		{
			if (buffer[ii] == '\n' || buffer[ii + 1] == '\n')
			{
				// Line feed in the middle of a line? Why?
				throw URGError (URG_ERR_PROTOCOL, "Found line feed in a data block.");
			}
			data->_data[currentStep] = Decode2ByteValue (&buffer[ii]);
		}
		// End of this line. Go around again.
	}

	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Read " << currentStep << " ranges." << endl;
	if (currentStep != numSteps)
		throw URGError (URG_ERR_PROTOCOL, "Read less range readings than were asked for.");
}

void URGLaser::Read3ByteRangeData (URGData *data, unsigned int numSteps)
{
	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Reading " << numSteps << " ranges." << endl;

	// This will automatically take care of whether it actually needs to (re)allocate or not.
	data->AllocateData (numSteps);

	// 3 byte data is a pain because it crosses the line boundary, it may overlap by 0, 1 or 2 bytes
	char buffer[SCIP2_LINE_LENGTH];
	unsigned int currentStep = 0;
	int numBytesInLine = 0, splitCount = 0;
	char splitValue[3];
	while (true)
	{
		// Read a line of data
		numBytesInLine = ReadLineWithCheck (buffer);
		// Check if we've reached the end of the data
		if (numBytesInLine == 0)
			break;
		// Process triplets of bytes until we encounter or overrun the end of the line
		for (int ii = 0; ii < numBytesInLine;)
		{
			if (buffer[ii] == '\n' || buffer[ii + 1] == '\n')
			{
				// Line feed in the middle of a line? Why?
				throw URGError (URG_ERR_PROTOCOL, "Found line feed in a data block.");
			}
			if (ii == numBytesInLine - 2)       // Short 1 byte
			{
				splitValue[0] = buffer[ii];
				splitValue[1] = buffer[ii + 1];
				splitCount = 1;     // Will be reset on the next iteration, after it's used
				ii += 2;
			}
			else if (ii == numBytesInLine - 1)  // Short 2 bytes
			{
				splitValue[0] = buffer[ii];
				splitCount = 2;     // Will be reset on the next iteration, after it's used
				ii += 1;
			}
			else
			{
				if (splitCount == 1)
				{
					splitValue[2] = buffer[ii++];
					data->_data[currentStep] = Decode3ByteValue (splitValue);
				}
				else if (splitCount == 2)
				{
					splitValue[1] = buffer[ii++];
					splitValue[2] = buffer[ii++];
					data->_data[currentStep] = Decode3ByteValue (splitValue);
				}
				else
				{
					data->_data[currentStep] = Decode3ByteValue (&buffer[ii]);
					ii += 3;
				}
				if (data->_data[currentStep] > _maxRange)
				{
					cerr << "WARNING: URGLaser::" << __func__ <<
						"() Value at step " << currentStep << " beyond maximum range: " <<
						data->_data[currentStep] << " (raw bytes: ";
					if (splitCount != 0)
						cerr << splitValue[0] << splitValue[1] << splitValue[2] << ")" << endl;
					else
						cerr << buffer[0] << buffer[1] << buffer[2] << ")" << endl;
				}
				currentStep++;
				splitCount = 0;     // Reset this here now that it's been used
			}
		}
		// End of this line. Go around again.
	}

	if (_verbose)
		cerr << "URGLaser::" << __func__ << "() Read " << currentStep << " ranges." << endl;
	if (currentStep != numSteps)
	{
		throw URGError (URG_ERR_PROTOCOL,
			"Read a  different number of range readings than were asked for.");
	}
}

} // namespace urg_nz
