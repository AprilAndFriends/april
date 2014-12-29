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
/// Defines a generic vertex shader.

#ifndef APRIL_VERTEX_SHADER_H
#define APRIL_VERTEX_SHADER_H

#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	/// @brief Defines a generic vertex shader.
	class aprilExport VertexShader
	{
	public:
		/// @brief Destructor.
		virtual ~VertexShader();

		/// @brief Loads a precompiled shader from a file.
		/// @param[in] filename Filename of the file.
		/// @return True if successful.
		virtual bool loadFile(chstr filename) = 0;
		/// @brief Loads a precompiled shader from a resource.
		/// @param[in] filename Filename of the resource.
		/// @return True if successful.
		virtual bool loadResource(chstr filename) = 0;
		/// @brief Sets the B-constant of the shader.
		/// @param[in] quads The quads.
		/// @param[in] quadCount How many quads there are.
		virtual void setConstantsB(const int* quads, unsigned int quadCount) = 0;
		/// @brief Sets the I-constant of the shader.
		/// @param[in] quads The quads.
		/// @param[in] quadCount How many quads there are.
		virtual void setConstantsI(const int* quads, unsigned int quadCount) = 0;
		/// @brief Sets the F-constant of the shader.
		/// @param[in] quads The quads.
		/// @param[in] quadCount How many quads there are.
		virtual void setConstantsF(const float* quads, unsigned int quadCount) = 0;

	protected:
		/// @brief Basic constructor.
		VertexShader();

		// TODOa - use hstream instead of data+size

		/// @brief Loads a precompiled shader from a file.
		/// @param[in] filename Filename of the file.
		/// @param[out] data The loaded data.
		/// @param[out] size The size of the loaded data.
		/// @return True if successful.
		bool _loadFileData(chstr filename, unsigned char** data, int* size);
		/// @brief Loads a precompiled shader from a resource.
		/// @param[in] filename Filename of the resource.
		/// @param[out] data The loaded data.
		/// @param[out] size The size of the loaded data.
		/// @return True if successful.
		bool _loadResourceData(chstr filename, unsigned char** data, int* size);

	};

}

#endif
