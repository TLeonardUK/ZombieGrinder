// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Renderer/Textures/PNG/PNGPixelmapFactory.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

#include "Engine/Renderer/Renderer.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/IO/StreamFactory.h"

#include <string>

void PNGPixelmapFactory::libpng_read_function(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_voidp ptr = png_get_io_ptr(png_ptr);
	((Stream*)ptr)->ReadBuffer((char*)data, 0, length);
}

void PNGPixelmapFactory::libpng_write_function(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_voidp ptr = png_get_io_ptr(png_ptr);
	((Stream*)ptr)->WriteBuffer((char*)data, 0, length);
}

bool PNGPixelmapFactory::Try_Save(Stream* stream, Pixelmap* texture)
{
	// Calculate format specifications for saving.
	int color_type				= 0;
	int width					= texture->Get_Width();
	int pitch					= texture->Get_Pitch();
	int height					= texture->Get_Height();
	int bpp						= 0;
	const unsigned char* buffer	= texture->Get_Data();

	switch (texture->Get_Format())
	{
		// Greyscale
	case PixelmapFormat::R8:
		color_type = PNG_COLOR_TYPE_GRAY;
		bpp = 1;
		break;

		// RGB
	case PixelmapFormat::R8G8B8:
		color_type = PNG_COLOR_TYPE_RGB;
		bpp = 3;
		break;

		// RGBA
	case PixelmapFormat::R8G8B8A8:
		color_type = PNG_COLOR_TYPE_RGB_ALPHA;
		bpp = 4;
		break;

	default:
		{
			return false;
		}
	}

	// Create textures required for saving.	
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) 
	{
		return false;
	}
	png_infop info_ptr = png_create_info_struct (png_ptr);
	if (info_ptr == NULL) 
	{
		return false;
	}

	// Setup error handler.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	// Set image attributes.
	png_set_IHDR (png_ptr,
		info_ptr,
		width,
		height,
		8,
		color_type,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);

	// Create memory
	int bytes_per_row = width * bpp;
	png_bytep* image_data = (png_bytep*)Get_Allocator()->Alloc(height * sizeof(png_byte*));

	for (int y = 0; y < height; ++y) 
	{
		image_data[height - (y + 1)] = (png_byte*)buffer + (y * pitch);
	}

	// Fast PNG encoding, large resulting file size.
	png_set_compression_level(png_ptr, 0);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_strategy(png_ptr, 2); //Z_HUFFMAN_ONLY

	// Setup custom writing.
	png_set_write_fn(png_ptr, (png_voidp)stream, libpng_write_function, NULL);
	png_set_rows(png_ptr, info_ptr, image_data);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	// Cleanup.	
	Get_Allocator()->Free(image_data);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return true;
}

bool PNGPixelmapFactory::Try_Save(const char* url, Pixelmap* texture)
{
	// Can we open this path as a file?
	Stream* stream = NULL;	
//	if (ResourceFactory::Try_Get() != NULL)
//	{
//		stream = ResourceFactory::Get()->Open(url, (StreamMode::Type)(StreamMode::Write|StreamMode::Truncate));
//	}
//	else
//	{
		stream = StreamFactory::Open(url, (StreamMode::Type)(StreamMode::Write|StreamMode::Truncate));
//	}
	if (stream == NULL)
	{
		return false;
	}

	Try_Save(stream, texture);

	SAFE_DELETE(stream);

	// Return texture.
	return true;
}

Pixelmap* PNGPixelmapFactory::Try_Load(const char* url)
{
	// Can we open this path as a file?
	Stream* stream = NULL;
	if (ResourceFactory::Try_Get() != NULL)
	{
		stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	}
	if (stream == NULL)
	{
		stream = StreamFactory::Open(url, StreamMode::Read);
	}

	if (stream == NULL)
	{
		DBG_LOG("Could not load Pixelmap '%s', could not open stream.", url);
		return NULL;
	}

	Pixelmap* pix = Try_Load(stream);

	SAFE_DELETE(stream);
	return pix;
}

Pixelmap* PNGPixelmapFactory::Try_Load(Stream* stream)
{
	// Big enough to be a PNG?
	if (stream->Length() < 8)
	{
		DBG_LOG("Could not load Pixelmap, stream not big enough.");
		return NULL; 
	}

	// Read in header.
	png_byte header[8];
	stream->ReadBuffer((char*)header, 0, 8);

	// Is header valid for a PNG?
	if (png_sig_cmp(header, 0, 8))
	{	
		DBG_LOG("Could not load Pixelmap, invalid header.");
		return NULL;
	}

	// Create initial structures required for reading.
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
	{
		return NULL;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return NULL;
    }
	
	png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
		return NULL;
    }

	// Setup error handler.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}

	// Setup custom reading.
	png_set_read_fn(png_ptr, (png_voidp)stream, libpng_read_function);
	
	// init png reading
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

	// Get general information about image.
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);
	
	// Expand to ensure we use 24-bit for RGB and 32-bit for RGBA.
	if (color_type == PNG_COLOR_TYPE_PALETTE || (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8))
	{
		png_set_expand(png_ptr);
	}
	
	// Expand greyscale to rgb.
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);
	}

	// Do we have alpha channel?
	bool has_alpha = ((color_type & PNG_COLOR_MASK_ALPHA) != 0);

	// Calculate pitch.	
	PixelmapFormat::Type format = PixelmapFormat::R8G8B8A8;
	int bytes_per_pixel = 4;
	if (has_alpha == false)
	{
		png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
	}

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

	int pitch_bytes = png_get_rowbytes(png_ptr, info_ptr);
	//pitch_bytes += 3 - ((pitch_bytes - 1) % 4); // Align to 4-byte

	// Allocate texture memory.
	png_byte* image_data;
	int data_size = pitch_bytes * temp_height * sizeof(png_byte);// + 15;
	image_data = (png_byte*)new char[data_size];//Get_Allocator()->Alloc(data_size);
	DBG_ASSERT(image_data != NULL);

	// Read in each row.
	png_bytep* row_pointers = (png_bytep*)Get_Allocator()->Alloc(temp_height * sizeof(png_bytep));
	DBG_ASSERT(row_pointers != NULL);

	for (int i = 0; i < (int)temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * pitch_bytes;
    }

	// Read in all the image data.
	png_read_image(png_ptr, row_pointers);
	
	// Create Pixelmap.
	Pixelmap* pixelmap = new Pixelmap((unsigned char*)image_data, temp_width, temp_height, temp_width * bytes_per_pixel, format);

	// Cleanup.	
	Get_Allocator()->Free(row_pointers);
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

	// Return texture.
	return pixelmap;
}
