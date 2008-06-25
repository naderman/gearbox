#include <iostream>
#include <gbxutilacfr/trivialtracer.h>
#include <gbxsmartbatteryacfr/gbxsmartbatteryacfr.h>

using namespace std;

class MyClass
{
public:
    MyClass(const std::string &port, bool debug);
    ~MyClass();
    void read();
private:
    gbxsmartbatteryacfr::OceanServerSystem data_;
    gbxsmartbatteryacfr::OceanServerReader *reader_;
};

MyClass::MyClass(const std::string &port, bool debug)
{
    gbxutilacfr::TrivialTracer tracer( debug );
    reader_ = new gbxsmartbatteryacfr::OceanServerReader( port, tracer );
}

MyClass::~MyClass()
{
    delete reader_;
}

void
MyClass::read()
{
    gbxsmartbatteryacfr::OceanServerSystem data;
    reader_->read(data);
    gbxsmartbatteryacfr::updateWithNewData( data, data_ );

    cout << "Current data:" << endl 
         << "=============" << endl
         << gbxsmartbatteryacfr::toString( data ) << endl;
    cout << "Stored data:" << endl 
         << "============" << endl
         << gbxsmartbatteryacfr::toString( data_ ) << endl;
}

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
    
    unsigned int numRecords=10;
    MyClass myClass( port, debug );
    
    for (unsigned int i=0; i<=numRecords; i++)
    {
        cout << "TRACE(fancy_test): Reading record " << i << ": " << endl
             << "====================================" << endl << endl;
        
        try
        {
            myClass.read();
        }
        catch ( gbxsmartbatteryacfr::ParsingException &e )
        {
            cout << "INFO(fancy_test): Caught a parsing exception: " 
                << e.what() << endl
                << "This can happen sometimes. Not a problem of the driver." << endl
                << "The higher level program which uses the driver has to deal with this situation." << endl;
            continue;
        }
        catch ( gbxsmartbatteryacfr::HardwareReadingException &e )
        {
            cout << "ERROR(fancy_test): Caught a hardware reading exception: " 
                << e.what() << endl 
                << "This shouldn't happen!" << endl;
            return 1;
        }
        catch(std::exception &e)
        {
            cout << "ERROR(fancy_test): Caught an unknown exception: " 
                << e.what() << endl
                << "This shouldn't happen!" << endl;
            return 1;
        }
    } // end of for loop
 
    cout << "INFO(fancy_test): Successfully read " << numRecords << " records." << endl;
    
    return 0;
}

