/************************************************************************************\
This source file is part of the Awesome Portable Rendering Interface Library         *
For latest info, see http://libapril.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Domagoj Cerjan                                                    *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <s3e.h>

#include <hltypes/util.h>

#include "ImageSource.h"
#include "RenderSystem.h"
#include <png.h>
#include <jpeglib.h>


namespace april
{
	ImageSource::ImageSource()
	{
		this->data = NULL;
		this->w = 0;
		this->h = 0;
		this->bpp = 0;
		this->format = april::AF_UNDEFINED;
	}
	
	ImageSource::~ImageSource()
	{
		if(this->data != NULL)
		{
			free(this->data);
		}
	}

	void ImageSource::copyImage(ImageSource* source, int bpp)
	{
		memcpy(this->data, source->data, bpp * source->w * source->h * sizeof(unsigned char));
	}
	
	/*
	void abort_(const char * s, ...)
	{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
	}
	*/

	ImageSource* loadImage(chstr filename)
	{
		ImageSource *img = new ImageSource();

		int width, height;
		unsigned char color_type;
		unsigned char bit_depth;
		hstr tmp;
#ifdef NO_FS_TREE
		tmp = filename.replace("/", "___");
#else
		tmp = filename;
#endif

		if(tmp.ends_with(".png"))
		{

			png_structp png_ptr;
			png_infop info_ptr;
			int number_of_passes;
			png_bytep *row_pointers;

			char header[8];    // 8 is the maximum size that can be checked

			/* open file and test for it being a png */
			FILE *fp = fopen(tmp.c_str(), "rb");
			if (!fp)
					abort();
					//abort_("[read_png_file] File %s could not be opened for reading", filename.c_str());
			fread(header, 1, 8, fp);
			if (png_sig_cmp((png_bytep)header, 0, 8))
					//abort_("[read_png_file] File %s is not recognized as a PNG file", file_name.c_str());
					abort();
                

			/* initialize stuff */
			png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

			if (!png_ptr)
					//abort_("[read_png_file] png_create_read_struct failed");
					abort();

			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr)
					//abort_("[read_png_file] png_create_info_struct failed");
					abort();

			if (setjmp(png_jmpbuf(png_ptr)))
					//abort_("[read_png_file] Error during init_io");
					abort();

			png_init_io(png_ptr, fp);
			png_set_sig_bytes(png_ptr, 8);

			png_read_info(png_ptr, info_ptr);

			width = png_get_image_width(png_ptr, info_ptr);
			height = png_get_image_height(png_ptr, info_ptr);
			color_type = png_get_color_type(png_ptr, info_ptr);
			bit_depth = png_get_bit_depth(png_ptr, info_ptr);

			number_of_passes = png_set_interlace_handling(png_ptr);
			png_read_update_info(png_ptr, info_ptr);


			/* read file */
			if (setjmp(png_jmpbuf(png_ptr)))
					//abort_("[read_png_file] Error during read_image");
					abort();

			row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);

			fprintf(stderr, "Img [%s] : \n", tmp.c_str());
			fprintf(stderr, " -> depth [%d]\n", png_ptr->bit_depth);
			fprintf(stderr, " -> channels [%d]\n", png_ptr->channels);
			fprintf(stderr, " -> width [%d]\n", png_ptr->width);
			fprintf(stderr, " -> height [%d]\n", png_ptr->height);
			fprintf(stderr, " -> bpp [%d]\n", png_ptr->pixel_depth);
			fprintf(stderr, " -> sizeof(png_bytep) [%d]\n", sizeof(png_bytep));

			unsigned char *field;
			size_t pixel_size = png_ptr->pixel_depth / png_ptr->bit_depth;
			field = (unsigned char *)malloc(height * width * pixel_size);
			/*for(int i = 0; i < width; ++i) {
				for(int j = 0; j < height * pixel_size; ++j) {
					field[i * height * pixel_size + j] = row_pointers[i][j];
				}
			}*/

			for (int i = 0; i < height; ++i)
					row_pointers[i] = field + i * width * pixel_size;

			png_read_image(png_ptr, row_pointers);
			fclose(fp);


		   png_read_destroy(png_ptr, info_ptr, (png_infop)0);
		   free(png_ptr);
		   free(info_ptr);

			img->data = field;

		}
		else if(filename.ends_with(".jpg") || filename.ends_with(".jpeg"))
		{

		}

		if(color_type == PNG_COLOR_TYPE_RGB)
			img->format = AF_RGB;
		else if(color_type	== PNG_COLOR_TYPE_RGBA)
			img->format = AF_RGBA;
		
		img->bpp = bit_depth;
		img->w = width;
		img->h = height;

		return img;

	}

}
