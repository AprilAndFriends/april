/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Ivan Vucica (ivan@vucica.net)                                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/

/**
  CoreGraphics based image source.
  Useful only on Mac and iPhone.
**/

#include <Cocoa/Cocoa.h>
#include <OpenGL/gl.h>
#include "ImageSource.h"
#include "RenderSystem.h"

static NSURL* _getFileURL(chstr filename)
{
	
	// consider that "filename" is "../media/hello.jpg" and "appname" is 
	// installed in "/Applications/appname/bin/". then:
	
	// bundle: file:///Applications/appname/bin/appname.app/
	// file:  file:///Applications/appname/bin/appname.app/../../media/hello.jpg
	// url: file:///Applications/appname/media/hello.jpg 
	
	
#if defined(__MAC_10_6)
	
	NSString* cdp = [[[[NSFileManager alloc] init] autorelease] currentDirectoryPath];
	NSString* file = [NSString stringWithFormat:@"%@/%s", cdp, filename.c_str()];
	NSURL* url = [NSURL fileURLWithPath:file];
	url = [url absoluteURL];
	
	[cdp release];
	[file release]; 
#else
	
	NSString * bundle = [[[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/"] stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
	NSURL * bundleURL = [NSURL URLWithString:[@"file://" stringByAppendingString:bundle]];
	NSURL * file = [NSURL URLWithString:[NSString stringWithFormat:@"../%s", filename.c_str()] 
						  relativeToURL:bundleURL];
	NSURL * url = [file absoluteURL];
	
#endif
	NSLog(@"_getFileURL: %@", url);
	return url;
	
}


static NSURL* _getFileURLAsResource(chstr filename)
{
	
	// consider that "filename" is "data/media/hello.jpg" and "appname" is 
	// installed in "/Applications/". then:
	
	// resources:  /Applications/appname.app/Contents/Resources/
	// file:       /Applications/appname.app/Contents/Resources/data/media/hello.jpg
	// url: file:///Applications/appname.app/Contents/Resources/data/media/hello.jpg
	
	NSString * resources = [[NSBundle mainBundle] resourcePath];
	NSString * file = [resources stringByAppendingPathComponent:[NSString stringWithUTF8String:filename.c_str()]];
	NSURL * url = [NSURL URLWithString:[@"file://" stringByAppendingString:[file stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding] ]];
	
	NSLog(@"_getFileURLAsResource: %@", url);
	return url;
	
}




namespace April
{
	ImageSource::ImageSource()
	{
		mImageId = 0; // unused in CG
		this->data = NULL;
		this->w = this->h = this->bpp = this->format = 0;
	}
	
	ImageSource::~ImageSource()
	{
		free(this->data);
	}
	
	Color ImageSource::getPixel(int x,int y)
	{
		if (x < 0) x=0;
		if (y < 0) y=0;
		if (x > w-1) x=w-1;
		if (y > h-1) y=h-1;
		
		Color c;
		int index=(y*this->w+x);
		if (this->bpp == 3) // RGB
		{
			c.r=this->data[index*3];
			c.g=this->data[index*3+1];
			c.b=this->data[index*3+2];
			c.a=1;
		}
		else if (this->bpp == 4) // RGBA
		{
			c.r=this->data[index*4];
			c.g=this->data[index*4+1];
			c.b=this->data[index*4+2];
			c.a=this->data[index*4+3];;
		}
		
		return c;
	}
	
	Color ImageSource::getInterpolatedPixel(float x,float y)
	{
		return getPixel((int)x,(int)y);
	}
	
	void ImageSource::copyPixels(void* output,int format)
	{
		memcpy(output, this->data, this->w * this->h * this->bpp);
	}
	
	ImageSource* loadImage(chstr filename)
	{
		NSAutoreleasePool* arp = [[NSAutoreleasePool alloc] init];
		
		
		
		// TODO check for memory leak!
		// according to:
		// http://www.cocoabuilder.com/archive/cocoa/194282-imagekit-gc-nogo-or-better-how.html
		// here might be a memory leak and we might wanna switch to
		// CGImageSourceCreateFromData(). bug in cocoa present
		// as late as 2007!
		
		CGImageSourceRef imageSource = CGImageSourceCreateWithURL((CFURLRef)_getFileURLAsResource(filename), NULL);
        
        if(!imageSource){
			//CFRelease(imageSource);
			
			imageSource = CGImageSourceCreateWithURL((CFURLRef)_getFileURL(filename), NULL);
			
			if (!imageSource){
				NSLog(@"Failed to load %@", filename.c_str());
				[arp release];
				return NULL;
			}
		}
		
		CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
		CFRelease(imageSource);
		
		// alloc img, set attributes
		ImageSource* img=new ImageSource();
		img->format = GL_RGBA; //ilGetInteger(IL_IMAGE_FORMAT); // not used
		img->w = CGImageGetWidth(imageRef);
		img->h = CGImageGetHeight(imageRef);
		int bitsPerComponent = CGImageGetBitsPerComponent(imageRef); // almost always 8
		int bytesPerRow = img->w * 4; //CGImageGetBytesPerRow(textureImage); // cannot generate RGB bitmapContext, it appears: only RGBA
		img->bpp = 4; // ditto, cannot create RGB bitmap context; otherwise we would calculate from bytesPerRow
		CGColorSpaceRef space = CGImageGetColorSpace(imageRef);
		
		// alloc and clean memory for bitmap
		img->data = (unsigned char*) malloc(bytesPerRow * img->h );
		memset(img->data, 0, bytesPerRow * img->h); // clear array

		
		// draw CG image to a byte array
		CGContextRef bitmapContext = CGBitmapContextCreate(img->data,
															img->w, img->h,
															bitsPerComponent, 
															bytesPerRow,
															space,
															kCGImageAlphaPremultipliedLast);
		
		CGContextDrawImage(bitmapContext,CGRectMake(0.0, 0.0, (float)img->w, (float)img->h),imageRef);
		CGContextRelease(bitmapContext);
		CGImageRelease(imageRef);
		
		
		[arp release];

		return img;
	}
}
