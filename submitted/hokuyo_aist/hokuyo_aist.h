/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2008 Geoffrey Biggs
 *
 * hokuyo_aist Hokuyo laser scanner driver.
 *
 * This distribution is licensed to you under the terms described in the LICENSE file included in
 * this distribution.
 *
 * This work is a product of the National Institute of Advanced Industrial Science and Technology,
 * Japan. Registration number: ___
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

#ifndef __HOKUYO_AIST_H
#define __HOKUYO_AIST_H

#include <flexiport/port.h>
#include <string>

#if defined (WIN32)
	typedef unsigned char           uint8_t;
	#if defined (HOKUYO_AIST_EXPORTS)
		#define HOKUYO_AIST_EXPORT       __declspec (dllexport)
	#else
		#define HOKUYO_AIST_EXPORT       __declspec (dllimport)
	#endif
#else
	#include <stdint.h>
	#define HOKUYO_AIST_EXPORT
#endif

/** @ingroup gbx_library_hokuyo_aist
@{
*/

namespace hokuyo_aist
{

/// Hokuyo general error class.
class HOKUYO_AIST_EXPORT HokuyoError : public std::exception
{
	public:
		/** @brief Hokuyo error constructor.

    	@param code Error code of the error.
    	@param desc Description of the error. */
		HokuyoError (unsigned int code, std::string desc)
			: _errorCode (code), _errorDesc (desc)
		{}
		virtual ~HokuyoError (void) throw () {};

		virtual unsigned int Code (void) const throw ()
		{
			return _errorCode;
		}

		virtual const char* what (void) const throw ()
		{
			return _errorDesc.c_str ();
		}

		virtual std::string AsString (void) const throw();

	private:
		/** Error code. */
		unsigned int _errorCode;
		/** Error description. */
		std::string _errorDesc;
};
#if defined (WIN32)
// Exporting data members on Windows is a bloody pain (they have to be initialised somewhere else),
// so we'll use #define's on Win32 and lose the type safety.
/// Read error while reading from the laser.
#define HOKUYO_ERR_READ            1
/// Write error while writing to the laser.
#define HOKUYO_ERR_WRITE           2
/// Error in the SCIP protocol.
#define HOKUYO_ERR_PROTOCOL        3
/// Error while changing baud rate.
#define HOKUYO_ERR_CHANGEBAUD      4
/// Failed to connect to the laser.
#define HOKUYO_ERR_CONNECT_FAILED  5
/// Failed to close a port.
#define HOKUYO_ERR_CLOSE_FAILED    6
/// No destination buffer provided for range readings.
#define HOKUYO_ERR_NODESTINATION   7
/// Bad firmware version.
#define HOKUYO_ERR_BADFIRMWARE     8
/// Unknown/unsupported SCIP protocol version.
#define HOKUYO_ERR_SCIPVERSION     9
/// Memory allocation error.
#define HOKUYO_ERR_MEMORY          10
/// Unsupported function error.
#define HOKUYO_ERR_UNSUPPORTED     11
/// Argument error
#define HOKUYO_ERR_BADARG          12
/// No data received error
#define HOKUYO_ERR_NODATA          13
/// Not a serial connection error
#define HOKUYO_ERR_NOTSERIAL       14
#else
/// Read error while reading from the laser.
const unsigned int HOKUYO_ERR_READ            = 1;
/// Write error while writing to the laser.
const unsigned int HOKUYO_ERR_WRITE           = 2;
/// Error in the SCIP protocol.
const unsigned int HOKUYO_ERR_PROTOCOL        = 3;
/// Error while changing baud rate.
const unsigned int HOKUYO_ERR_CHANGEBAUD      = 4;
/// Failed to connect to the laser.
const unsigned int HOKUYO_ERR_CONNECT_FAILED  = 5;
/// Failed to close a port.
const unsigned int HOKUYO_ERR_CLOSE_FAILED    = 6;
/// No destination buffer provided for range readings.
const unsigned int HOKUYO_ERR_NODESTINATION   = 7;
/// Bad firmware version.
const unsigned int HOKUYO_ERR_BADFIRMWARE     = 8;
/// Unknown/unsupported SCIP protocol version.
const unsigned int HOKUYO_ERR_SCIPVERSION     = 9;
/// Memory allocation error.
const unsigned int HOKUYO_ERR_MEMORY          = 10;
/// Unsupported function error.
const unsigned int HOKUYO_ERR_UNSUPPORTED     = 11;
/// Argument error
const unsigned int HOKUYO_ERR_BADARG          = 12;
/// No data received error
const unsigned int HOKUYO_ERR_NODATA          = 13;
/// Not a serial connection error
const unsigned int HOKUYO_ERR_NOTSERIAL       = 14;
#endif // defined (WIN32)

/** @brief Sensor information.

Returned from a call to @GetSensorInfo. Contains various information about the laser scanner such as
firmware version and maximum possible range. */
class HOKUYO_AIST_EXPORT HokuyoSensorInfo
{
	public:
		friend class HokuyoLaser;

		HokuyoSensorInfo (void);
		HokuyoSensorInfo (const HokuyoSensorInfo &rhs);

		/// @brief Assignment operator.
		HokuyoSensorInfo& operator= (const HokuyoSensorInfo &rhs);

		/// @brief Format the entire object into a string.
		std::string AsString (void);

		// Version details.
		/// Vendor name.
		std::string vendor;
		/// Product name.
		std::string product;
		/// Firmware version.
		std::string firmware;
		/// Protocol version in use.
		std::string protocol;
		/// Serial number of this device.
		std::string serial;

		// Specification details.
		/// Sensor model number.
		std::string model;
		/// Minimum detectable range (mm).
		unsigned int minRange;
		/// Maximum detectable range (mm).
		unsigned int maxRange;
		/// Number of steps in a 360-degree scan.
		unsigned int steps;
		/// First scanable step of a full scan.
		unsigned int firstStep;
		/// Last scanable step of a full scan.
		unsigned int lastStep;
		/// Step number that points forward (typically the centre of a full scan).
		unsigned int frontStep;
		/// Standard motor speed (rpm).
		unsigned int standardSpeed;

		// Status details.
		/// Operational status - illuminated or not.
		bool power;
		/// Current motor speed (rpm).
		unsigned int speed;
		/// Measurement state.
		std::string measureState;
		/// Baud rate.
		unsigned int baud;
		/// Current sensor time (s).
		unsigned int time;
		/// Diagnostic status string.
		std::string sensorDiagnostic;

		// Calculated details
		/// Minimum possible scan angle (radians). Scans go anti-clockwise with negative angles on
		/// the right.
		double minAngle;
		/// Maximum possible scan angle (radians). Scans go anti-clockwise with negative angles on
		/// the right.
		double maxAngle;
		/// Angle between two scan points (radians).
		double resolution;
		/// Total number of steps in a full scan (lastStep - firstStep).
		unsigned int scanableSteps;

	private:
		void SetDefaults (void);
		void CalculateValues (void);
};

/** @brief Structure to store data returned from the laser scanner. */
class HOKUYO_AIST_EXPORT HokuyoData
{
	public:
		friend class HokuyoLaser;

		/// This constructor creates an empty HokuyoData with no data currently allocated.
		HokuyoData (void);
		/// This constructor performs a deep copy of data.
		HokuyoData (unsigned short *data, unsigned int length, short error, unsigned int time);
		/// This copy constructor performs a deep copy of data.
		HokuyoData (const HokuyoData &rhs);
		~HokuyoData (void);

		/** @brief Return a pointer to an array of range readings in millimetres.

		Values less than 20mm indicate an error. Check the error value for the data to see a
		probable cause for the error. Most of the time, it will just be an out-of-range reading. */
		const unsigned short* Ranges (void) const           { return _data; }
		/// @brief Get the number of samples in the data.
		unsigned int Length (void) const                    { return _length; }
		/** @brief Error code for the data (if any).

		@return -1 indicates no error. */
		short GetErrorCode (void) const                     { return _error; }
		/// @brief Return a string representing the error code for the data.
		std::string ErrorCodeToString (void);
		/** @brief Get the time stamp of the data in milliseconds (only available using SCIP
		version 2). */
		unsigned int TimeStamp (void) const                 { return _time; }

		/// @brief Assignment operator.
		HokuyoData& operator= (const HokuyoData &rhs);
		/// @brief Subscript operator.
		unsigned short operator[] (unsigned int index);

		/// @brief Format the entire object into a string.
		std::string AsString (void);

		/// @brief Force the data to clean up.
		void CleanUp (void);

	private:
		unsigned short *_data;
		unsigned int _length;
		short _error;
		unsigned int _time;

		void AllocateData (unsigned int length);
};

/** @brief Hokuyo laser scanner class.

Provides an interface for interacting with a Hokuyo laser scanner using SCIP protocol version 1
or 2. The FlexiPort library is used to implement the data communications with the scanner. See its
documentation for details on controlling the connection.

To use a serial connection, ensure that you do not also have a USB cable connected, as this will
force the scanner into USB mode, preventing the serial connection from functioning correctly.

All functions may throw instances of @ref HokuyoError or its children. Exceptions from
@ref FlexiPort may also occur. */
class HOKUYO_AIST_EXPORT HokuyoLaser
{
	public:
		HokuyoLaser (void);
		~HokuyoLaser (void);

		/// @brief Open the laser scanner and begin scanning.
		void Open (std::string portOptions);

		/// @brief Close the connection to the laser scanner.
		void Close (void);

		/// @brief Checks if the connection to the laser scanner is open.
		bool IsOpen (void) const;

		/// @brief Switch the laser scanner on or off.
		void SetPower (bool on);

		/** @brief Change the baud rate when using a serial connection.

		Valid baud rates are: 19.2Kbps, 57.6Kbps, 115.2Kbps, 250.0Kbps, 500.0Kbps, 750.0Kbps
		(dependent on those available in FlexiPort). */
		void SetBaud (unsigned int baud);

		/** @brief Reset the laser scanner to its default settings.

		Not available with the SCIP v1 protocol. */
		void Reset (void);

		/** @brief Set the speed at which the scanner's sensor spins in revolutions per minute.

		Valid speeds are: 540, 546, 552, 558, 564, 570, 576, 582, 588, 594, 600. Set the speed to 0
		to have it reset to the default value.

		Not available with the SCIP v1 protocol. */
		void SetMotorSpeed (unsigned int speed);

		/** @brief Get various information about the scanner.

		Much of the information is not available with the SCIP v1 protocol. */
		void GetSensorInfo (HokuyoSensorInfo *info);

		/** @brief Get the current value of the scanner's clock in milliseconds.

		Not available with the SCIP v1 protocol. */
		unsigned int GetTime (void);

		/** @brief Get the latest scan data from the scanner.

		This function requires a pointer to a @ref HokuyoData object. It will allocate space in this
		object as necessary for storing range data. If the passed-in @ref HokuyoData object already
		has the correct quantity of space to store the range data, it will not be re-allocated. If
		it does not have any space, it will be allocated. If it has space, but it is the wrong size,
		it will be re-allocated. This means you can repeatedly send the same @ref HokuyoData object
		without having to worry about allocating its data, whether it will change or not, while also
		avoiding excessive allocations.

		Not available with the SCIP v1 protocol.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@param startStep The first step to get ranges from. Set to -1 for the first scannable step.
		@param endStep The last step to get ranges from. Set to -1 for the last scannable step.
		@return The number of range readings read into @ref data. */
		unsigned int GetRanges (HokuyoData *data, int startStep = -1, int endStep = -1,
								unsigned int clusterCount = 1);

		/** @brief Get the latest scan data from the scanner.

		Not available with the SCIP v1 protocol.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param startAngle The angle to get range readings from. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param endAngle The angle to get range readings to. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@return The number of range readings read into @ref data. */
		unsigned int GetRanges (HokuyoData *data, double startAngle, double endAngle,
								unsigned int clusterCount = 1);

		/** @brief Get a new scan from the scanner.

		Unlike @ref GetRanges, which returns the most recent scan the scanner took, this function
		will request a new scan. This means it will wait while the scanner performs the scan.
		Otherwise behaves identicallty to @ref GetRanges.

		Not available with the SCIP v1 protocol.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@param startStep The first step to get ranges from. Set to -1 for the first scannable step.
		@param endStep The last step to get ranges from. Set to -1 for the last scannable step.
		@return The number of range readings read into @ref data. */
		unsigned int GetNewRanges (HokuyoData *data, int startStep = -1, int endStep = -1,
								unsigned int clusterCount = 1);

		/** @brief Get a new scan from the scanner.

		Not available with the SCIP v1 protocol.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param startAngle The angle to get range readings from. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param endAngle The angle to get range readings to. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@return The number of range readings read into @ref data. */
		unsigned int GetNewRanges (HokuyoData *data, double startAngle, double endAngle,
								unsigned int clusterCount = 1);

		/// @brief Return the major version of the SCIP protocol in use.
		uint8_t SCIPVersion (void) const            { return _scipVersion; }

		/** @brief Turns on and off printing of verbose operating information to stderr. Default is
		off. */
		void SetVerbose (bool verbose)              { _verbose = verbose; }

		/// @brief A convenience function to convert a step index to an angle.
		double StepToAngle (unsigned int step);
		/// @brief A convenience function to convert an angle to a step (rounded towards the front).
		unsigned int AngleToStep (double angle);

	private:
		flexiport::Port *_port;

		uint8_t _scipVersion;
		bool _verbose;
		double _minAngle, _maxAngle, _resolution;
		int _firstStep, _lastStep, _frontStep;
		unsigned int _maxRange;

		int ReadLine (char *buffer, int expectedLength = -1);
		int ReadLineWithCheck (char *buffer, int expectedLength = -1, bool hasSemicolon = false);
		void SkipLines (int count);
		int SendCommand (const char *cmd, const char *param, int paramLength, const char *extraOK);

		void GetAndSetSCIPVersion (void);
		void GetDefaults (void);
		void Read2ByteRangeData (HokuyoData *data, unsigned int numSteps);
		void Read3ByteRangeData (HokuyoData *data, unsigned int numSteps);
};

} // namespace hokuyo_aist

/** @} */

#endif // __HOKUYO_AIST_H
