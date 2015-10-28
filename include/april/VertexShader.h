/// @file
/// @version 4.0
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

#include <hltypes/hstream.h>
#include <hltypes/hstring.h>

#include "aprilExport.h"

namespace april
{
	class RenderSystem;

	/// @brief Defines a generic vertex shader.
	class aprilExport VertexShader
	{
	public:
		friend class RenderSystem;

		/// @brief Checks if the shader is loaded.
		virtual bool isLoaded() = 0;

		/// @brief Loads a precompiled shader from a file.
		/// @param[in] stream Stream with shader data.
		/// @return True if successful.
		/// @note Some systems use precompiled shaders while others use raw shader code.
		bool load(const hstream& stream);
		/// @brief Loads a shader from a file.
		/// @param[in] filename Filename of the file.
		/// @return True if successful.
		/// @note Some systems use precompiled shaders while others use raw shader code.
		bool loadFile(chstr filename);
		/// @brief Loads a shader from a resource.
		/// @param[in] filename Filename of the resource.
		/// @return True if successful.
		/// @note Some systems use precompiled shaders while others use raw shader code.
		bool loadResource(chstr filename);
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
		/// @brief Destructor.
		virtual ~VertexShader();

		/// @brief Creates the actual shader in the system
		/// @param[in] filename The filename.
		/// @param[out] stream The loaded data stream.
		/// @return True if successful.
		virtual bool _createShader(chstr filename, const hstream& stream) = 0;

	};

}

#endif
