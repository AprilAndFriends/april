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

		bool create(RenderSystem::Options options);
		bool destroy();
		void assignWindow(Window* window);

		void setPixelShader(PixelShader* pixelShader);
		void setVertexShader(VertexShader* vertexShader);

		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices);
		void render(RenderOperation renderOperation, PlainVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices);
		void render(RenderOperation renderOperation, TexturedVertex* v, int nVertices, Color color);
		void render(RenderOperation renderOperation, ColoredVertex* v, int nVertices);
		void render(RenderOperation renderOperation, ColoredTexturedVertex* v, int nVertices);

	protected:
		ColorMode activeTextureColorMode;
		unsigned char activeTextureColorModeAlpha;
		ShaderProgram* activeShader;

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

		void _setupCaps();
		
		void _setupDefaultParameters();
		void _applyStateChanges();
		void _setGlClientState(unsigned int type, bool enabled);

		void _setTextureBlendMode(BlendMode mode);
		void _setTextureColorMode(ColorMode textureColorMode, float factor);
		void _loadIdentityMatrix();
		void _setMatrixMode(unsigned int mode);
		void _setGlVertexPointer(int stride, const void* pointer, bool forceUpdate = false);
		void _setGlTexturePointer(int stride, const void *pointer);
		void _setGlColorPointer(int stride, const void *pointer);

		void _updateShader();

		void _setModelviewMatrix(const gmat4& matrix);
		void _setProjectionMatrix(const gmat4& matrix);

	private:
		ShaderProgram* _currentShader;
		OpenGLES_Texture* _currentTexture;
		float _currentLerpAlpha;
		float _currentSystemColor[4];

		bool _matrixDirty;

	};
	
}
#endif
#endif
