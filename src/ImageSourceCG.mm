/// @file
/// @author  Ivan Vucica
/// @version 1.31
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php

/**
  CoreGraphics based image source.
  Useful only on Mac and iPhone.
**/

#import <TargetConditionals.h>
#if TARGET_OS_MAC && !TARGET_OS_IPHONE
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#elif TARGET_OS_IPHONE
#import <UIKit/UIImage.h>
#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>

#import <OpenGLES/ES1/gl.h>
#import "PVRTexture.h"
#endif
#include "ImageSource.h"
#include "RenderSystem.h"

// fix for NSURL not working on iPad Simulator
#ifndef __GNUC__
#define __asm__ asm
#endif

__asm__(".weak_reference _OBJC_CLASS_$_NSURL");
// end iPad Simulator fix

namespace april
{
	NSURL* _getFileURL(chstr filename);
	NSURL* _getFileURLAsResource(chstr filename);
	ImageSource* _tryLoadingPVR(chstr filename);

	ImageSource::ImageSource()
	{
		imageId = 0; // unused in CG
		this->data = NULL;
		this->w = this->h = this->bpp = this->format = AF_UNDEFINED;
		this->compressedLength = 0;
	}
	
	ImageSource::~ImageSource()
	{
		free(this->data);
	}
			
	ImageSource* loadImage(chstr filename)
	{
		NSAutoreleasePool* arp = [[NSAutoreleasePool alloc] init];
#if TARGET_OS_IPHONE
		ImageSource *pvrimg=_tryLoadingPVR(filename);
		if (pvrimg)
		{
			[arp release];
			return pvrimg;
		}
#endif

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
		
		// TODO check for memory leak!
		// according to:
		// http://www.cocoabuilder.com/archive/cocoa/194282-imagekit-gc-nogo-or-better-how.html
		// here might be a memory leak and we might wanna switch to
		// CGImageSourceCreateWithData(). bug in cocoa present
		// as late as 2007!
		
		CGImageSourceRef
				imageSource = CGImageSourceCreateWithData((CFDataRef)[NSData dataWithContentsOfFile:[NSString stringWithUTF8String:filename.c_str()]], NULL);
				
				if(!imageSource)
				{
					NSLog(@"Failed to load %s", filename.c_str()); // FIXME should use logFunction!
					[arp release];
					return NULL;
				}

		CGImageRef imageRef = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
		CFRelease(imageSource);
		
#else

		UIImage* uiimg = [[UIImage alloc] initWithContentsOfFile:[_getFileURLAsResource(filename) path]];
		
		CGImageRef imageRef = [uiimg CGImage];
#endif

		// alloc img, set attributes
		ImageSource* img=new ImageSource();
		img->format = AF_RGBA; //ilGetInteger(IL_IMAGE_FORMAT); // not used
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
		
		// let's undo premultiplication
		unsigned char* p = img->data;
		unsigned char* end = p + (bytesPerRow * img->h);
		float f;
		for (;p != end; p+= 4)
		{
			f = p[3]/255.0f;
			p[0] /= f;
			p[1] /= f;
			p[2] /= f;
		}
#if TARGET_OS_MAC
		CGImageRelease(imageRef);
#else
		[uiimg release];
#endif
		[arp release];
        return img;
 /*
		
		ImageSource* img=new ImageSource();
		CGImageSourceRef myImageSourceRef = CGImageSourceCreateWithData((CFDataRef)[NSData dataWithContentsOfFile:[NSString stringWithUTF8String:filename.c_str()]], NULL);
		CGImageRef myImageRef = CGImageSourceCreateImageAtIndex (myImageSourceRef, 0, NULL);
		CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(myImageRef));
		img->w = CGImageGetWidth(myImageRef);
		img->h = CGImageGetHeight(myImageRef);
		img->bpp = CGImageGetBitsPerPixel(myImageRef)/8;
		img->format = img->bpp == 4 ? GL_RGBA : GL_RGB;
		img->data = (unsigned char*) CFDataGetBytePtr(data);
		[arp release];
		
		//CFRelease(data);
		CGImageRelease(myImageRef);
		CFRelease(myImageSourceRef);
		return img;*/
	}
}























