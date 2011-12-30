/// @file
/// @author  Kresimir Spes
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php
/// 
/// @section DESCRIPTION
/// 
/// Defines a manager for blurring.

#ifndef APRILUTIL_BLUR_MANAGER_H
#define APRILUTIL_BLUR_MANAGER_H

#include "aprilutilExport.h"

namespace april
{
	class Texture;
	
	class aprilutilExport BlurManager
	{
	public:
		BlurManager();
		~BlurManager();

		void begin(int nLevels, float inc);
		void end();
		void draw(int w, int h);
		
	protected:
		Texture* mTex1;
		Texture* mTex2;
		int mNumLevels;
		bool mEnable;
		float mInc;

	};

}

#endif
