/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Key definition converter.

#ifndef APRIL_MAC_KEYS_H
#define APRIL_MAC_KEYS_H

#include "Keys.h"

namespace april
{
	Key getAprilMacKeyCode(unsigned int macKeyCode);
	void initMacKeyMap();
}

#endif
