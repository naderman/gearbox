/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Mathew Ridley
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef GBXSERIALACFR_SERIAL_H
#define GBXSERIALACFR_SERIAL_H

#include <string>
#include <gbxserialacfr/uncopyable.h>
#include <gbxserialacfr/lockfile/lockfile.h>

namespace gbxserialacfr {

//!
//! @brief Exception thrown by Serial.
//!
class SerialException : public std::exception
{ 
    std::string  message_;
public:
    SerialException(const char *message)
        : message_(message) {}
    SerialException(const std::string &message)
        : message_(message) {}
    ~SerialException()throw(){}
    virtual const char* what() const throw() { return message_.c_str(); }
};

//!
//! @brief Encapsulates a serial port.
//!
//! This class hard-codes some options, such as:
//!  - 8 data bits
//!  - no handshaking
//!
//! Warning: this thing is _NOT_ thread-safe.
//!
//! @author Matthew Ridley, Alex Brooks
//!
class Serial : public Uncopyable
{
public:

    //! Opens a device @ref dev.
    //! Throws exceptions on error.
    //! If timeouts are enabled, they default to 1sec.
    //! If useLockFile is set to true, Serial will use the file-system to 
    //! prevent concurrent access to a serial device by multiple instances of Serial.
    Serial( const std::string &dev,
            int  baudRate,
            bool enableTimeouts,
            int  debuglevel = 0,
            bool useLockFile = true );

    //! Destructor closes serial port
    ~Serial();

    //! Debug messages are printed to stdout.  debugLevel should be in the range [0,3].
    void setDebugLevel( int debugLevel ) { debugLevel_ = debugLevel; }

    //! Sets the baud rate. Flushes any data.
    void setBaudRate(int baud);

    //! Sets timeout (timeouts must be enabled)
    void setTimeout(int sec, int usec);

    //! Reads up to @ref count bytes into buffer @ref buf.
    //! Returns the number of bytes read, or '-1' on timeout (if timeouts are enabled).
    //! If timeouts are not enabled, blocks till it gets something.
    int read(void *buf, int count);

    //! Tries to read exactly @ref count bytes into @ref buf.  
    //! Returns the number of bytes read, or throws an exception.
    //!
    //! If timeouts are not enabled we might block forever, waiting for the number of bytes we want or an error.
    //!
    //! If timeouts are enabled we won't block more than the timeout specified.
    //! Returns -1 if it timed out.
    //! NOTE: The timeout applies for each individual read() call.  We might have to make lots of them,
    //!       so the total time for which this function blocks might be longer than the specified timeout.
    //!
    int readFull(void *buf, int count);

    //! Reads a line of data up to @ref count bytes-1 (including @ref termchar), terminated by @ref termchar.
    //! Returns the number of bytes read.
    //! After reading the line then the string will be NULL terminated.
    //!
    //! Example: if you expect to read the string "1234\n", you need something like:
    //!                char buf[6];
    //!                serial.readLine( buf, 6 );
    //!
    //!          where the two extra characters are for the "\n" and the terminating "\0".
    //!
    //! If timeouts are not enabled we might block forever, waiting for the number of bytes we want or an error.
    //!
    //! If timeouts are enabled we won't block more than the timeout specified.
    //! Returns -1 if it timed out.
    //! NOTE: The timeout applies for each individual read() call.  We might have to make lots of them,
    //!       so the total time for which this function blocks might be longer than the specified timeout.
    //!
    int readLine(void *buf, int count, char termchar='\n');

    //! Returns the number of bytes available for reading (non-blocking).
    int bytesAvailable();

    //! Returns the number of bytes available for reading.  Waits according to the timeout.
    //! Returns:
    //! - <0 :  timed out
    //! - >0 :  data ready
    int bytesAvailableWait();

    //! Writes some data.  Returns the number of bytes written.
    int write(const void *buf, int count);

    //! Writes a ("\0"-terminated) string. 
    //! Returns the number of bytes written.
    int writeString(const char *buf);
    inline int writeString(const std::string &s) {
        return writeString(s.c_str());
    }

    //! Flushs both input and output buffers.
    //! This discards all data in buffers.
    void flush();

    //! Finishes transmission from output buffers and drains input buffers.
    void drain();

    //! This gives direct access to the file descriptor: be careful with this...
    int fileDescriptor() { return portFd_; }

    //! Print some diagnostic information about the current status of the port to cout.
    std::string getStatusString();


private:

    // Utility function to wait up to the timeout for data to appear.
    // Returns:
    //  TIMED_OUT: timed out
    //  GOT_DATA : data available
    int waitForDataOrTimeout(void);

    // Opens a device @ref dev.
    void open(int flags=0);

    // Won't throw exceptions.
    void close();

    const std::string dev_;
    int portFd_;
    int timeoutSec_;
    int timeoutUSec_;
    bool timeoutsEnabled_;
    
    int debugLevel_;

    lockfile::LockFile *lockFile_;
};

}

#endif
