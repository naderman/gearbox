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
 * Japan. Registration number: H22PRO-1086.
 *
 * This file is part of hokuyo_aist.
 *
 * This software is licensed under the Eclipse Public License -v 1.0 (EPL). See
 * http://www.opensource.org/licenses/eclipse-1.0.txt
 */

#ifndef __HOKUYO_AIST_H
#define __HOKUYO_AIST_H

#include <flexiport/port.h>
#include <string>

#if defined (WIN32)
	typedef unsigned char           uint8_t;
	typedef unsigned int            uint32_t;
	#if defined (HOKUYO_AIST_STATIC)
		#define HOKUYO_AIST_EXPORT
	#elif defined (HOKUYO_AIST_EXPORTS)
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
		virtual ~HokuyoError () throw () {};

		virtual unsigned int Code () const throw ()
		{
			return _errorCode;
		}

		virtual const char* what () const throw ()
		{
			return _errorDesc.c_str ();
		}

		virtual std::string AsString () const throw();

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

		HokuyoSensorInfo ();
		HokuyoSensorInfo (const HokuyoSensorInfo &rhs);

		/// @brief Assignment operator.
		HokuyoSensorInfo& operator= (const HokuyoSensorInfo &rhs);

		/// @brief Format the entire object into a string.
		std::string AsString ();

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
		/// Speed level (0 for default)
		unsigned short speedLevel;
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
		void SetDefaults ();
		void CalculateValues ();
};

/** @brief Structure to store data returned from the laser scanner. */
class HOKUYO_AIST_EXPORT HokuyoData
{
	public:
		friend class HokuyoLaser;

		/// This constructor creates an empty HokuyoData with no data currently allocated.
		HokuyoData ();
		/// This constructor performs a deep copy of existing range data.
		HokuyoData (uint32_t *ranges, unsigned int length, bool error, unsigned int time);
		/// This constructor performs a deep copy of existing range and intensity data.
		HokuyoData (uint32_t *ranges, uint32_t *intensities, unsigned int length,
					bool error, unsigned int time);
		/// This copy constructor performs a deep copy of present data.
		HokuyoData (const HokuyoData &rhs);
		~HokuyoData ();

		/** @brief Return a pointer to an array of range readings in millimetres.

		Values less than 20mm indicate an error. Check the error value for the data to see a
		probable cause for the error. Most of the time, it will just be an out-of-range reading. */
		const uint32_t* Ranges () const                 { return _ranges; }
		/// @brief Return a pointer to an array of intensity readings.
		const uint32_t* Intensities () const            { return _intensities; }
		/// @brief Get the number of samples in the data.
		unsigned int Length () const                    { return _length; }
		/** @brief Indicates if one or more steps had an error.

		A step's value will be less than 20 if it had an error. Use @ref ErrorCodeToString to get
		a textual representation of the error. */
		bool GetErrorStatus () const                    { return _error; }
		/// @brief Return a string representing the error for the given error code.
		std::string ErrorCodeToString (uint32_t errorCode);
		/** @brief Get the time stamp of the data in milliseconds (only available using SCIP
		version 2). */
		unsigned int TimeStamp () const                 { return _time; }

		/// @brief Assignment operator.
		HokuyoData& operator= (const HokuyoData &rhs);
		/// @brief Subscript operator. Provides direct access to an element of the range data.
		uint32_t operator[] (unsigned int index);

		/// @brief Format the entire object into a string.
		std::string AsString ();

		/// @brief Force the data to clean up.
		void CleanUp ();

	protected:
		uint32_t *_ranges;
		uint32_t *_intensities;
		unsigned int _length;
		bool _error;
		unsigned int _time;
		bool _sensorIsUTM30LX;

		void AllocateData (unsigned int length, bool includeIntensities = false);
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
		HokuyoLaser ();
		~HokuyoLaser ();

		/// @brief Open the laser scanner and begin scanning.
		void Open (std::string portOptions);

		/** @brief Open the laser scanner and begin scanning, probing the baud rate as necessary.

		If the port is a serial connection and communication with the laser fails at the given
		baud rate, the alternative baud rates supported by the device are tried (see @ref SetBaud
		for these) in order from fastest to slowest.

		@return The baud rate at which connection with the laser succeeded, or 0 for non-serial
		connections. */
		unsigned int OpenWithProbing (std::string portOptions);

		/// @brief Close the connection to the laser scanner.
		void Close ();

		/// @brief Checks if the connection to the laser scanner is open.
		bool IsOpen () const;

		/// @brief Switch the laser scanner on or off.
		void SetPower (bool on);

		/** @brief Change the baud rate when using a serial connection.

		Valid rates are 19.2Kbps, 38.4Kbps, 57.6Kbps, 115.2Kbps, 250.0Kbps, 500.0Kbps, 750.0Kbps
		(dependent on those available in FlexiPort). */
		void SetBaud (unsigned int baud);

		/** @brief Reset the laser scanner to its default settings.

		Not available with the SCIP v1 protocol. */
		void Reset ();

		/** @brief Set the speed at which the scanner's sensor spins.

		Set the speed to 0 to have it reset to the default value, and 99 to reset it to the initial
		(startup) value. Values between 1 and 10 specify a ratio of the default speed. The speeds in
		revolutions per minute that these correspond to will depend on the scanner model. For
		example, for a URG-04LX, they are (from 1 to 10) 594, 588, 576, 570, 564, 558, 552, 546, and
		540 rpm.

		Not available with the SCIP v1 protocol. */
		void SetMotorSpeed (unsigned int speed);

		/// @brief Switch the scanner between normal and high sensitivity modes.
		void SetHighSensitivity (bool on);

		/** @brief Get various information about the scanner.

		Much of the information is not available with the SCIP v1 protocol. */
		void GetSensorInfo (HokuyoSensorInfo *info);

		/** @brief Get the current value of the scanner's clock in milliseconds.

		Not available with the SCIP v1 protocol. */
		unsigned int GetTime ();

		/** @brief Get the latest scan data from the scanner.

		This function requires a pointer to a @ref HokuyoData object. It will allocate space in this
		object as necessary for storing range data. If the passed-in @ref HokuyoData object already
		has the correct quantity of space to store the range data, it will not be re-allocated. If
		it does not have any space, it will be allocated. If it has space, but it is the wrong size,
		it will be re-allocated. This means you can repeatedly send the same @ref HokuyoData object
		without having to worry about allocating its data, whether it will change or not, while also
		avoiding excessive allocations.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@param startStep The first step to get ranges from. Set to -1 for the first scannable step.
		@param endStep The last step to get ranges from. Set to -1 for the last scannable step.
		@return The number of range readings read into @ref data. */
		unsigned int GetRanges (HokuyoData *data, int startStep = -1, int endStep = -1,
								unsigned int clusterCount = 1);

		/** @brief Get the latest scan data from the scanner.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param startAngle The angle to get range readings from. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param endAngle The angle to get range readings to. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@return The number of range readings read into @ref data. */
		unsigned int GetRangesByAngle (HokuyoData *data, double startAngle, double endAngle,
								unsigned int clusterCount = 1);

		/** @brief Get a new scan from the scanner.

		Unlike @ref GetRanges, which returns the most recent scan the scanner took, this function
		will request a new scan. This means it will wait while the scanner performs the scan, which
		means the rate at which scans can be retrieved using this function is less than with @ref
		GetRanges. Otherwise behaves identicallty to @ref GetRanges.

		Not available with the SCIP v1 protocol.

		@note The command used to retrieve a fresh scan is also used for the continuous scanning
		mode (not yet supported by this library). After completing a scan, it will turn the laser
		off (in anticipation of another continuous scan command being sent, which will automatically
		turn the laser back on again). If you want to mix @ref GetNewRanges and @ref GetRanges, you
		will need to turn the laser on after each call to @ref GetNewRanges.

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
		unsigned int GetNewRangesByAngle (HokuyoData *data, double startAngle, double endAngle,
								unsigned int clusterCount = 1);

		/** @brief Get a new scan from the scanner with intensity data.

		Unlike @ref GetRanges, which returns the most recent scan the scanner took, this function
		will request a new scan. This means it will wait while the scanner performs the scan.
		Otherwise behaves identicallty to @ref GetRanges.

		Not available with the SCIP v1 protocol.

		@note The command used to retrieve a fresh scan is also used for the continuous scanning
		mode (not yet supported by this library). After completing a scan, it will turn the laser
		off (in anticipation of another continuous scan command being sent, which will automatically
		turn the laser back on again). If you want to mix @ref GetNewRanges and @ref GetRanges, you
		will need to turn the laser on after each call to @ref GetNewRanges.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@param startStep The first step to get ranges from. Set to -1 for the first scannable step.
		@param endStep The last step to get ranges from. Set to -1 for the last scannable step.
		@return The number of range readings read into @ref data. */
		unsigned int GetNewRangesAndIntensities (HokuyoData *data, int startStep = -1,
												int endStep = -1, unsigned int clusterCount = 1);

		/** @brief Get a new scan from the scanner with intensity data.

		Not available with the SCIP v1 protocol.

		@param data Pointer to a @ref HokuyoData object to store the range readings in.
		@param startAngle The angle to get range readings from. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param endAngle The angle to get range readings to. Exclusive; if this falls between two
		steps the step inside the angle will be returned, but the step outside won't.
		@param clusterCount The number of readings to cluster together into a single reading. The
		minimum value from a cluster is returned as the range for that cluster.
		@return The number of range readings read into @ref data. */
		unsigned int GetNewRangesAndIntensitiesByAngle (HokuyoData *data, double startAngle,
												double endAngle, unsigned int clusterCount = 1);

		/// @brief Return the major version of the SCIP protocol in use.
		uint8_t SCIPVersion () const            { return _scipVersion; }

		/** @brief Turns on and off printing of verbose operating information to stderr. Default is
		off. */
		void SetVerbose (bool verbose)          { _verbose = verbose; }

		/** @brief Enables/disables ignoring unknown lines in sensor info messages. Default is
		off. */
		void IgnoreUnknowns (bool ignore)       { _ignoreUnknowns = ignore; }

		/// @brief A convenience function to convert a step index to an angle.
		double StepToAngle (unsigned int step);
		/// @brief A convenience function to convert an angle to a step (rounded towards the front).
		unsigned int AngleToStep (double angle);

	private:
		flexiport::Port *_port;

		uint8_t _scipVersion;
		bool _verbose, _sensorIsUTM30LX, _enableCheckSumWorkaround, _ignoreUnknowns;
		double _minAngle, _maxAngle, _resolution;
		int _firstStep, _lastStep, _frontStep;
		unsigned int _maxRange;

		void ClearReadBuffer ();
		int ReadLine (char *buffer, int expectedLength = -1);
		int ReadLineWithCheck (char *buffer, int expectedLength = -1, bool hasSemicolon = false);
		void SkipLines (int count);
		int SendCommand (const char *cmd, const char *param, int paramLength, const char *extraOK);

		void GetAndSetSCIPVersion ();
		void GetDefaults ();
		void ProcessVVLine (const char *buffer, HokuyoSensorInfo *info);
		void ProcessPPLine (const char *buffer, HokuyoSensorInfo *info);
		void ProcessIILine (const char *buffer, HokuyoSensorInfo *info);
		void Read2ByteRangeData (HokuyoData *data, unsigned int numSteps);
		void Read3ByteRangeData (HokuyoData *data, unsigned int numSteps);
		void Read3ByteRangeAndIntensityData (HokuyoData *data, unsigned int numSteps);

		int ConfirmCheckSum (const char *buffer, int length, int expectedSum);
};

} // namespace hokuyo_aist

/** @} */

#endif // __HOKUYO_AIST_H
