#include <iostream>
#include <gbxutilacfr/trivialtracer.h>
#include <gbxsmartbatteryacfr/gbxsmartbatteryacfr.h>

using namespace std;

int main( int argc, char **argv )
{
    int opt;
    std::string port = "/dev/ttyS0";
    bool debug = false;
    
    // Get some options from the command line
    while ((opt = getopt(argc, argv, "p:")) != -1)
    {
        switch ( opt )
        {
            case 'p':
                port = optarg;
                break;
            case 'v':
                debug = true;
                break;
            default:
                cout << "Usage: " << argv[0] << " [-p port] [-v(erbose)]" << endl
                     << "-p port\tPort the oceanserver battery system is connected to. E.g. /dev/ttyS0" << endl;
                return 1;
        }
    }
    
    const unsigned int numRecords = 5;
    cout << "INFO(simple_test): The plan is to read " << numRecords << " records from the oceanserver system and display the results." << endl << endl;

    // instantiate reader
    gbxutilacfr::TrivialTracer tracer( debug );
    
    gbxsmartbatteryacfr::OceanServerReader *reader;
    
    try 
    {
        reader = new gbxsmartbatteryacfr::OceanServerReader( port, tracer );
    } 
    catch ( gbxsmartbatteryacfr::HardwareReadingException &e )
    {
        cout << "ERROR(simple_test): Caught a hardware reading exception when initialising reader: " 
                << e.what() << endl;
        return 1;
    }
    
    // data storage
    gbxsmartbatteryacfr::OceanServerSystem data;
    
    for (unsigned int i=0; i<=numRecords; i++)
    {
        try 
        {
            reader->read( data );
            cout << "TRACE(simple_test): Reading record " << i << ": " << endl
                    << "=====================================" << endl << endl
                    << gbxsmartbatteryacfr::toString( data ) << endl;
        }
        catch ( gbxsmartbatteryacfr::ParsingException &e )
        {
            cout << "INFO(simple_test): Caught a parsing exception: " 
                    << e.what() << endl
                    << "This can happen sometimes. Not a problem of the driver." << endl
                    << "The higher level program which uses the driver has to deal with this situation." << endl;
            continue;
        }
        catch ( gbxsmartbatteryacfr::HardwareReadingException &e )
        {
            cout << "ERROR(simple_test): Caught a hardware reading exception: " 
                    << e.what() << endl 
                    << "This shouldn't happen!" << endl;
            return 1;
        }
        catch(std::exception &e)
        {
            cout << "ERROR(simple_test): Caught an unknown exception: " 
                    << e.what() << endl
                    << "This shouldn't happen!" << endl;
            return 1;
        }
    } //end of for loop


    cout << "INFO(simple_test): Successfully read " << numRecords << " records." << endl;
    
    delete reader;
    return 0;
}
