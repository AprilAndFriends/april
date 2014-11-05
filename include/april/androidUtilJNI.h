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
/// Defines some methods and special macros used with April's AndroidJNI system.
/// This is mainly meant to be used as a utility to simplify JNI calls when a static native interface class is present in a sublibrary.

#ifdef _ANDROID
#ifndef APRIL_ANDROID_UTIL_JNI_H
#define APRIL_ANDROID_UTIL_JNI_H

#ifndef JNI_H_
#include "jni.h"
#endif

#include <hltypes/hlog.h>
#include <hltypes/hstring.h>

namespace april
{
	/// @brief Gets the JNI environment.
	/// @return The JNI environment.
	JNIEnv* getJNIEnv();
	/// @brief Gets the main activity.
	/// @return The main activity.
	/// @note This Java object does not necessarily have to be a "com.april.Activity", but is never NULL.
	jobject getActivity();
	/// @brief Gets the main APRIL activity.
	/// @return The main APRIL activity.
	/// @note This Java object is always a "com.april.Activity", but it can be NULL if "com.april.Activity" is not used.
	jobject getAprilActivity();
	/// @brief Finds a JNI class.
	/// @param[in] env JNI environment.
	/// @param[in] classPath Full package path of the class.
	/// @return The associated JNI class.
	/// @note The result will be NULL if the class was not found.
	jclass findJNIClass(JNIEnv* env, chstr classPath);
	/// @brief Creates a hstr from a Java string.
	/// @param[in] env JNI environment.
	/// @param[in] string The Java string.
	/// @return A hstr created from the Java string.
	/// @see _JSTR_TO_HSTR()
	hstr _jstringToHstr(JNIEnv* env, jstring string);
}

/// @brief Utility macro to get a definition for a JNI function formatted how Java needs it.
/// @param[in] returnType The return type of the JNI function.
/// @param[in] arguments A list of arguments for the JNI function.
/// @note The arguments are not separated by commas but merged into a full string. This is simply how Java defines it.
#define _JARGS(returnType, arguments) "(" arguments ")" returnType
/// @brief Utility macro to define array arguments for a JNI function.
/// @param[in] type Name of the type that is used as array type.
#define _JARR(type) "[" type
/// @brief Utility macro to define a Java object argument for a JNI function.
#define _JOBJ "Ljava/lang/Object;"
/// @brief Utility macro to define a Java string argument for a JNI function.
#define _JSTR "Ljava/lang/String;"
/// @brief Utility macro to define arguments of a JNI function.
/// @param[in] class String name class.
#define _JCLASS(classe) "L" classe ";"
/// @brief Utility macro to define an int argument for a JNI function.
#define _JINT "I"
/// @brief Utility macro to define a bool argument for a JNI function.
#define _JBOOL "Z"
/// @brief Utility macro to define a float argument for a JNI function.
#define _JFLOAT "F"
/// @brief Utility macro to define a void argument for a JNI function.
#define _JVOID "V"

/// @brief Creates a hstr from a Java string.
/// @param[in] string The Java string.
/// @return A hstr created from the Java string.
/// @note Make sure to get the JNI environment first.
/// @see getJNIEnv()
/// @see _jstringToHstr()
#define _JSTR_TO_HSTR(string) april::_jstringToHstr(env, string)

// If this macro is defined before header inclusion, it will automatically generate macros for easier access to a general purpose JNI class.
#ifdef __NATIVE_INTERFACE_CLASS
/// @brief Gets the defined native interface class' Java object.
/// @param[in] className Name of the variable to contain the JNI class object.
#define APRIL_GET_NATIVE_INTERFACE_CLASS(className) \
	JNIEnv* env = april::getJNIEnv(); \
	jclass className = april::findJNIClass(env, __NATIVE_INTERFACE_CLASS); \
	if (className == NULL) \
	{ \
		hlog::error("JNI", "Could not find native interface class: " + hstr(__NATIVE_INTERFACE_CLASS)); \
	}
/// @brief Gets the defined native interface class' Java object.
/// @param[in] className Name of the variable to contain the JNI class object.
/// @param[in] methodName Name of the variable to contain the JNI method object.
/// @param[in] methodString String name of the method.
/// @param[in] args Arguments of the method.
/// @see _JARGS
#define APRIL_GET_NATIVE_INTERFACE_METHOD(className, methodName, methodString, args) \
	APRIL_GET_NATIVE_INTERFACE_CLASS(className); \
	jmethodID methodName = env->GetStaticMethodID(className, methodString, args); \
	if (methodName == NULL) \
	{ \
		hlog::error("JNI", "Could not find method, check definition: " + hstr(methodString)); \
	}
/// @brief Gets the defined native interface class' Java object.
/// @param[in] className Name of the variable to contain the JNI class object.
/// @param[in] fieldName Name of the variable to contain the JNI field object.
/// @param[in] fieldString String name of the field.
/// @param[in] type Type of the field.
/// @see _JARGS
#define APRIL_GET_NATIVE_INTERFACE_FIELD(className, fieldName, fieldString, type) \
	APRIL_GET_NATIVE_INTERFACE_CLASS(className); \
	jfieldID fieldName = env->GetStaticFieldID(className, fieldString, type); \
	if (fieldName == NULL) \
	{ \
		hlog::error("JNI", "Could not find field, check definition: " + hstr(fieldString)); \
	}
#endif

#endif
#endif
