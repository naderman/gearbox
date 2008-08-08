#include <hokuyo_aist.h>
using namespace hokuyo_aist;

#include <boost/python.hpp>
#include <boost/python/def.hpp>

#include <iostream>

class HokuyoErrorWrap : public HokuyoError, public boost::python::wrapper<HokuyoError>
{
	public:
		HokuyoErrorWrap (unsigned int code, std::string desc)
		: HokuyoError (code, desc)
		{}

		unsigned int Code (void) const throw ()
		{
			if (boost::python::override f = get_override ("Code"))
				return f ();
			return HokuyoError::Code ();
		}
		unsigned int DefaultCode (void) const throw ()
			{ return HokuyoError::Code (); }

		const char* what (void) const throw ()
		{
			if (boost::python::override f = get_override ("what"))
				return f ();
			return HokuyoError::what ();
		}
		const char* Defaultwhat (void) const throw ()
			{ return HokuyoError::what (); }

		std::string AsString (void) const throw ()
		{
			if (boost::python::override f = get_override ("AsString"))
				return f ();
			return HokuyoError::AsString ();
		}
		std::string DefaultAsString (void) const throw ()
			{ return HokuyoError::AsString (); }
};

class HokuyoDataWrap : public HokuyoData, public boost::python::wrapper<HokuyoData>
{
	public:
		HokuyoDataWrap (void)
		: HokuyoData ()
		{}
		HokuyoDataWrap (uint32_t *ranges, unsigned int length, bool error, unsigned int time)
		: HokuyoData (ranges, length, error, time)
		{}
		HokuyoDataWrap (uint32_t *ranges, uint32_t *intensities, unsigned int length, bool error,
				unsigned int time)
		: HokuyoData (ranges, intensities, length, error, time)
		{}

		uint32_t Range (unsigned int index)
			{ return _ranges[index]; }
		uint32_t Intensity (unsigned int index)
			{ return _intensities[index]; }
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS (HokuyoLaserOverloads1, GetRanges, 1, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS (HokuyoLaserOverloads2, GetRangesByAngle, 3, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS (HokuyoLaserOverloads3, GetNewRanges, 1, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS (HokuyoLaserOverloads4, GetNewRangesByAngle, 3, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS (HokuyoLaserOverloads5, GetNewRangesAndIntensities, 1, 4)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS (HokuyoLaserOverloads6, GetNewRangesAndIntensitiesByAngle, 3, 4)

BOOST_PYTHON_MODULE (hokuyo_aist)
{
	using namespace boost::python;

	class_<HokuyoErrorWrap, boost::noncopyable> ("HokuyoError", init<unsigned int, std::string> ())
		.def ("Code", &HokuyoError::Code, &HokuyoErrorWrap::DefaultCode)
		.def ("what", &HokuyoError::what, &HokuyoErrorWrap::Defaultwhat)
		.def ("AsString", &HokuyoError::AsString, &HokuyoErrorWrap::DefaultAsString)
		;

	scope ().attr ("HOKUYO_ERR_READ") = HOKUYO_ERR_READ;
	scope ().attr ("HOKUYO_ERR_WRITE") = HOKUYO_ERR_WRITE;
	scope ().attr ("HOKUYO_ERR_PROTOCOL") = HOKUYO_ERR_PROTOCOL;
	scope ().attr ("HOKUYO_ERR_CHANGEBAUD") = HOKUYO_ERR_CHANGEBAUD;
	scope ().attr ("HOKUYO_ERR_CONNECT_FAILED") = HOKUYO_ERR_CONNECT_FAILED;
	scope ().attr ("HOKUYO_ERR_CLOSE_FAILED") = HOKUYO_ERR_CLOSE_FAILED;
	scope ().attr ("HOKUYO_ERR_NODESTINATION") = HOKUYO_ERR_NODESTINATION;
	scope ().attr ("HOKUYO_ERR_BADFIRMWARE") = HOKUYO_ERR_BADFIRMWARE;
	scope ().attr ("HOKUYO_ERR_SCIPVERSION") = HOKUYO_ERR_SCIPVERSION;
	scope ().attr ("HOKUYO_ERR_MEMORY") = HOKUYO_ERR_MEMORY;
	scope ().attr ("HOKUYO_ERR_UNSUPPORTED") = HOKUYO_ERR_UNSUPPORTED;
	scope ().attr ("HOKUYO_ERR_BADARG") = HOKUYO_ERR_BADARG;
	scope ().attr ("HOKUYO_ERR_NODATA") = HOKUYO_ERR_NODATA;
	scope ().attr ("HOKUYO_ERR_NOTSERIAL") = HOKUYO_ERR_NOTSERIAL;

	// TODO: this causes an undefined symbol error when importing into Python
//	class_<HokuyoSensorInfo> ("HokuyoSensorInfo")
//		.def ("AsString", &HokuyoSensorInfo::AsString)
//		.def_readonly ("vendor", &HokuyoSensorInfo::vendor)
//		.def_readonly ("product", &HokuyoSensorInfo::product)
//		.def_readonly ("firmware", &HokuyoSensorInfo::firmware)
//		.def_readonly ("protocol", &HokuyoSensorInfo::protocol)
//		.def_readonly ("serial", &HokuyoSensorInfo::serial)
//		.def_readonly ("model", &HokuyoSensorInfo::model)
//		.def_readonly ("minRange", &HokuyoSensorInfo::minRange)
//		.def_readonly ("maxRange", &HokuyoSensorInfo::maxRange)
//		.def_readonly ("steps", &HokuyoSensorInfo::steps)
//		.def_readonly ("firstStep", &HokuyoSensorInfo::firstStep)
//		.def_readonly ("lastStep", &HokuyoSensorInfo::lastStep)
//		.def_readonly ("frontStep", &HokuyoSensorInfo::frontStep)
//		.def_readonly ("standardSpeed", &HokuyoSensorInfo::standardSpeed)
//		.def_readonly ("power", &HokuyoSensorInfo::power)
//		.def_readonly ("speed", &HokuyoSensorInfo::speed)
//		.def_readonly ("speedLevel", &HokuyoSensorInfo::speedLevel)
//		.def_readonly ("measureState", &HokuyoSensorInfo::measureState)
//		.def_readonly ("baud", &HokuyoSensorInfo::baud)
//		.def_readonly ("time", &HokuyoSensorInfo::time)
//		.def_readonly ("sensorDiagnostic", &HokuyoSensorInfo::sensorDiagnostic)
//		.def_readonly ("minAngle", &HokuyoSensorInfo::minAngle)
//		.def_readonly ("maxAngle", &HokuyoSensorInfo::maxAngle)
//		.def_readonly ("resolution", &HokuyoSensorInfo::resolution)
//		.def_readonly ("scanableSteps", &HokuyoSensorInfo::scanableSteps)
//		;

	class_ <HokuyoDataWrap, boost::noncopyable> ("HokuyoData")
		.def (init<uint32_t*, unsigned int, bool, unsigned int> ())
		.def (init<uint32_t*, uint32_t*, unsigned int, bool, unsigned int> ())
		// TODO: write a wrapper function to copy the data into a python array, because this doesn't work
//		.def ("Ranges", &HokuyoData::Ranges, return_value_policy<reference_existing_object> (), with_custodian_and_ward_postcall<1, 0> ())
		.def ("Range", &HokuyoDataWrap::Range)
		.def ("Intensity", &HokuyoDataWrap::Intensity)
		.def ("Length", &HokuyoData::Length)
		.def ("GetErrorStatus", &HokuyoData::GetErrorStatus)
		.def ("ErrorCodeToString", &HokuyoData::ErrorCodeToString)
		.def ("TimeStamp", &HokuyoData::TimeStamp)
		.def ("AsString", &HokuyoData::AsString)
		.def ("CleanUp", &HokuyoData::CleanUp)
		;

	class_ <HokuyoLaser> ("HokuyoLaser")
		.def ("Open", &HokuyoLaser::Open)
		.def ("Close", &HokuyoLaser::Close)
		.def ("IsOpen", &HokuyoLaser::IsOpen)
		.def ("SetPower", &HokuyoLaser::SetPower)
		.def ("SetBaud", &HokuyoLaser::SetBaud)
		.def ("Reset", &HokuyoLaser::Reset)
		.def ("SetMotorSpeed", &HokuyoLaser::SetMotorSpeed)
		.def ("SetHighSensitivity", &HokuyoLaser::SetHighSensitivity)
		.def ("GetSensorInfo", &HokuyoLaser::GetSensorInfo)
		.def ("GetTime", &HokuyoLaser::GetTime)
		.def ("GetRanges", &HokuyoLaser::GetRanges, HokuyoLaserOverloads1 ())
		.def ("GetRangesByAngle", &HokuyoLaser::GetRangesByAngle, HokuyoLaserOverloads2 ())
		.def ("GetNewRanges", &HokuyoLaser::GetNewRanges, HokuyoLaserOverloads3 ())
		.def ("GetNewRangesByAngle", &HokuyoLaser::GetNewRangesByAngle, HokuyoLaserOverloads4 ())
		.def ("GetNewRangesAndIntensities", &HokuyoLaser::GetNewRangesAndIntensities, HokuyoLaserOverloads5 ())
		.def ("GetNewRangesAndIntensitiesByAngle", &HokuyoLaser::GetNewRangesAndIntensitiesByAngle, HokuyoLaserOverloads6 ())
		.def ("SCIPVersion", &HokuyoLaser::SCIPVersion)
		.def ("SetVerbose", &HokuyoLaser::SetVerbose)
		.def ("StepToAngle", &HokuyoLaser::StepToAngle)
		.def ("AngleToStep", &HokuyoLaser::AngleToStep)
		;
}
