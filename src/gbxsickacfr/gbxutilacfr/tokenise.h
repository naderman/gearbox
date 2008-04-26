/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2007-2008 Alex Brooks
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXUTILACFR_TOKENISE_H
#define GBXUTILACFR_TOKENISE_H

#include <string>
#include <vector>

namespace gbxsickacfr {
namespace gbxutilacfr {

//! Takes a string containing tokens separated by a delimiter
//! Returns the vector of tokens
std::vector<std::string> tokenise( const std::string &str, 
                                   const std::string &delimiter );

}
}

#endif
