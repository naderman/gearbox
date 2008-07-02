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
    cout << "INFO(test): The plan is to read " << numRecords << " records from the oceanserver system and display the results." << endl << endl;

    gbxutilacfr::TrivialTracer tracer( debug );    
    
    try 
    {
        gbxsmartbatteryacfr::OceanServer oceanserver( port, tracer );
        
        for (unsigned int i=0; i<=numRecords; i++)
        {            
            gbxsmartbatteryacfr::OceanServerSystem data = oceanserver.getData();
            
            cout << "TRACE(test): Reading record " << i << ": " << endl
                 << "=================================" << endl << endl
                 << gbxsmartbatteryacfr::toString( data ) << endl;
        }
    }
    catch ( gbxsmartbatteryacfr::HardwareReadingException &e )
    {
        cout << "ERROR(test): Caught a hardware reading exception: " 
                << e.what() << endl 
                << "This shouldn't happen!" << endl;
        return 1;
    }
    catch ( gbxutilacfr::Exception &e )
    {
        cout << "ERROR(test): Caught a gbxutilacfr::Exception: " 
             << e.what() << endl 
             << "This shouldn't happen!" << endl;
        return 1;
    }
    catch ( std::exception &e )
    {
        cout << "ERROR(test): Caught an unknown exception: " 
             << e.what() << endl
             << "This shouldn't happen!" << endl;
        return 1;
    }
    
    cout << "INFO(test): Successfully read " << numRecords << " records." << endl;
    
    return 0;
}

