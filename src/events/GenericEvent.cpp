/// @file
/// @version 5.1
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#include "GenericEvent.h"

namespace april
{
	HL_ENUM_CLASS_DEFINE(GenericEvent::Type,
	(
		HL_ENUM_DEFINE(GenericEvent::Type, QuitRequest);
		HL_ENUM_DEFINE(GenericEvent::Type, FocusChange);
		HL_ENUM_DEFINE(GenericEvent::Type, ActivityChange);
		HL_ENUM_DEFINE(GenericEvent::Type, SizeChange);
		HL_ENUM_DEFINE(GenericEvent::Type, InputModeChange);
		HL_ENUM_DEFINE(GenericEvent::Type, VirtualKeyboardChange);
		HL_ENUM_DEFINE(GenericEvent::Type, LowMemoryWarning);
	));

	GenericEvent::GenericEvent(Type type) :
		intValue(0),
		intValueOther(0),
		floatValue(0.0f),
		boolValue(false)
	{
		this->type = type;
	}

	GenericEvent::GenericEvent(Type type, int intValue) :
		intValue(0),
		intValueOther(0),
		floatValue(0.0f),
		boolValue(false)
	{
		this->type = type;
		this->intValue = intValue;
	}

	GenericEvent::GenericEvent(Type type, int intValue, int intValueOther, bool boolValue) :
		intValue(0),
		intValueOther(0),
		floatValue(0.0f),
		boolValue(false)
	{
		this->type = type;
		this->intValue = intValue;
		this->intValueOther = intValueOther;
		this->boolValue = boolValue;
	}

	GenericEvent::GenericEvent(Type type, bool boolValue) :
		intValue(0),
		intValueOther(0),
		floatValue(0.0f),
		boolValue(false)
	{
		this->type = type;
		this->boolValue = boolValue;
	}

	GenericEvent::GenericEvent(Type type, bool boolValue, float floatValue) :
		intValue(0),
		intValueOther(0),
		floatValue(0.0f),
		boolValue(false)
	{
		this->type = type;
		this->boolValue = boolValue;
		this->floatValue = floatValue;
	}

	GenericEvent::GenericEvent(Type type, cgvec2f gvec2fValue) :
		intValue(0),
		intValueOther(0),
		floatValue(0.0f),
		boolValue(false)
	{
		this->type = type;
		this->gvec2fValue = gvec2fValue;
	}

}
