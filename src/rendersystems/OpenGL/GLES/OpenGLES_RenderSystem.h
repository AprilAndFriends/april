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
/// Defines a generic OpenGL render system.

#ifdef _OPENGLES
#ifndef APRIL_OPENGLES_RENDER_SYSTEM_H
#define APRIL_OPENGLES_RENDER_SYSTEM_H

#include "OpenGL_RenderSystem.h"

namespace april
{
	class OpenGLES_PixelShader;
	class OpenGLES_Texture;
	class OpenGLES_VertexShader;

	class OpenGLES_RenderSystem : public OpenGL_RenderSystem
	{
	public:
		class ShaderProgram
		{
		public:
			friend class OpenGLES_RenderSystem;

			ShaderProgram();
			~ShaderProgram();

			bool load(unsigned int pixelShaderId, unsigned int vertexShaderId);

		protected:
			unsigned int glShaderProgram;

		};

		OpenGLES_RenderSystem();
		~OpenGLES_RenderSystem();

	protected:
		OpenGLES_VertexShader* vertexShaderPlain;
		OpenGLES_VertexShader* vertexShaderTextured;
		OpenGLES_VertexShader* vertexShaderColored;
		OpenGLES_VertexShader* vertexShaderColoredTextured;
		OpenGLES_PixelShader* pixelShaderMultiply;
		OpenGLES_PixelShader* pixelShaderAlphaMap;
		OpenGLES_PixelShader* pixelShaderLerp;
		OpenGLES_PixelShader* pixelShaderTexturedMultiply;
		OpenGLES_PixelShader* pixelShaderTexturedAlphaMap;
		OpenGLES_PixelShader* pixelShaderTexturedLerp;
		OpenGLES_PixelShader* pixelShaderColoredMultiply;
		OpenGLES_PixelShader* pixelShaderColoredAlphaMap;
		OpenGLES_PixelShader* pixelShaderColoredLerp;
		OpenGLES_PixelShader* pixelShaderColoredTexturedMultiply;
		OpenGLES_PixelShader* pixelShaderColoredTexturedAlphaMap;
		OpenGLES_PixelShader* pixelShaderColoredTexturedLerp;
		ShaderProgram* shaderMultiply;
		ShaderProgram* shaderAlphaMap;
		ShaderProgram* shaderLerp;
		ShaderProgram* shaderTexturedMultiply;
		ShaderProgram* shaderTexturedAlphaMap;
		ShaderProgram* shaderTexturedLerp;
		ShaderProgram* shaderColoredMultiply;
		ShaderProgram* shaderColoredAlphaMap;
		ShaderProgram* shaderColoredLerp;
		ShaderProgram* shaderColoredTexturedMultiply;
		ShaderProgram* shaderColoredTexturedAlphaMap;
		ShaderProgram* shaderColoredTexturedLerp;

		bool deviceState_matrixChanged;
		bool deviceState_systemColorChanged;
		bool deviceState_colorModeFactorChanged;
		ShaderProgram* deviceState_shader;

		void _deviceInit();
		bool _deviceCreate(Options options);
		bool _deviceDestroy();
		void _deviceAssignWindow(Window* window);
		void _deviceSetupCaps();
		void _deviceSetup();

		void _updateDeviceState(bool forceUpdate = false);

		void _setDeviceModelviewMatrix(const gmat4& matrix);
		void _setDeviceProjectionMatrix(const gmat4& matrix);
		void _setDeviceBlendMode(BlendMode mode);
		void _setDeviceColorMode(ColorMode colorMode, float colorModeFactor, bool useTexture, bool useColor, const Color& systemColor);
		void _updateShader(bool forceUpdate);

		void _setGlTextureEnabled(bool enabled);
		void _setGlColorEnabled(bool enabled);
		void _setGlVertexPointer(int stride, const void* pointer);
		void _setGlTexturePointer(int stride, const void* pointer);
		void _setGlColorPointer(int stride, const void* pointer);

	};
	
}
#endif
#endif
