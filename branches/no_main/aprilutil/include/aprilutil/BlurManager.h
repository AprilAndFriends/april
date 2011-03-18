/************************************************************************************\
This source file is part of the APRIL Utility library                                *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes                                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef BLUR_MANAGER_H
#define BLUR_MANAGER_H

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
