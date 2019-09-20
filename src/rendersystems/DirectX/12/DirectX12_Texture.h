/// @file
/// @version 5.2
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
/// 
/// @section DESCRIPTION
/// 
/// Defines a DirectX12 specific texture.

#ifdef _DIRECTX12
#ifndef APRIL_DirectX12_TEXTURE_H
#define APRIL_DirectX12_TEXTURE_H

#include <hltypes/hplatform.h>
#include <hltypes/hstring.h>

#include "RenderSystem.h"
#include "DirectX_Texture.h"

using namespace Microsoft::WRL;

namespace april
{
	class DirectX12_RenderSystem;

	class DirectX12_Texture : public DirectX_Texture
	{
	public:
		friend class DirectX12_RenderSystem;
		
		void* getBackendId() const;

	protected:
		DirectX12_Texture(bool fromResource);
		
		ComPtr<ID3D12Resource> d3dTexture;
		ComPtr<ID3D12Resource> uploadHeap;
		DXGI_FORMAT dxgiFormat;

		bool _deviceCreateTexture(unsigned char* data, int size);
		bool _deviceDestroyTexture();
		void _assignFormat();

		Lock _tryLockSystem(int x, int y, int w, int h);
		bool _unlockSystem(Lock& lock, bool update);

	};

}
#endif
#endif
