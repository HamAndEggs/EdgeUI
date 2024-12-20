/*
   Copyright (C) 2021, Richard e Collins.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
   
   Original code base is at https://github.com/HamAndEggs/TinyPNG
   
   Resources used,
   https://en.wikipedia.org/wiki/Portable_Network_Graphics
   https://www.w3.org/TR/PNG/#5Chunk-layout

   */

#ifndef TINY_TGA_H
#define TINY_TGA_H

#include <vector>
#include <string>
#include <assert.h>

namespace tinytga{ // Using a namespace to try to prevent name clashes as my class names are kind of obvious :)
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef VERBOSE_MESSAGE
	#ifdef VERBOSE_BUILD
		#define VERBOSE_MESSAGE(THE_MESSAGE__)	{std::clog << __LINE__ << ":" << THE_MESSAGE__ << "\n";}
	#else
		#define VERBOSE_MESSAGE(THE_MESSAGE__)
	#endif
#endif

#pragma pack(push,1)
typedef struct TGAHeader
{
	uint8_t  id_length   ;	// number of bytes reserved for image ID (directly after this header)
	uint8_t  map_type    ;	// 0 - no colour map. 1 - includes colour map.
	uint8_t  type        ;	// type of image stored
	uint16_t map_origin  ;	// first index supplied by thtarget_pixel_datae colour map
	uint16_t map_length  ;	// number of entries in the colour map
	uint8_t  map_width   ;	// bits per colour map entry
	uint16_t x_origin    ;	// lower left corner of the image
	uint16_t y_origin    ;	//
	uint16_t width       ;	// width of the image
	uint16_t height      ;	// height of the image
	uint8_t  depth       ;	// # bits per pixel
	uint8_t  alpha_bits  : 4;
	bool  mirrored    : 1;
	bool  flipped     : 1;
	uint8_t  interleaving: 2; // 0 => no interleaving, 1 => way (even/odd) interleaving, 2 => way interleaving (eg: AT&T PC)

	TGAHeader(uint16_t pWidth, uint16_t pHeight)
	{
		id_length = 0;
		map_type = 0;
		type = 2;//TGA_TYPE_TRUECOLOUR;
		map_origin = 0;
		map_length = 0;
		map_width = 0;
		x_origin = 0;
		y_origin = 0;
		width = pWidth;
		height = pHeight;
		depth = 24;
		alpha_bits = 0;
		mirrored = false;
		flipped = true;
		interleaving = 0;
	}

}TGAHeader;
#pragma pack(pop)

typedef enum
{
	TGA_A8,		  // 8 bit
	TGA_R4G4B4A4, // 16 bit
	TGA_R5G6B5,	  // 16 bit
	TGA_R5G5B5A1, // 16 bit
	TGA_R8G8B8,	  // 24 bit
	TGA_R8G8B8A8, // 32 bit
	TGA_INVALID = 0x7fffffff,
} TGA_TextureFormat;


/**
 * @brief 
 * 
 */
class Loader
{
public:

    /**
     * @brief Decodes the PNG that is held in memory.
     * 
     * @param pMemory The PNG as loaded from a file in memory.
     * @return true  If the PNG was loaded ok.
     * @return false If PNG was corrupt / invalid.
     */
    bool LoadFromMemory(const std::vector<uint8_t>& pMemory)
    {
        // read in the TGA header
        const TGAHeader *header = (const TGAHeader*)pMemory.data();

        // set up some local flags to determine the image properties
        const uint8_t tga_type = header->type;
        bool rle_compressed = (tga_type & 0x08) != 0;	// RLE compressed image data
        bool huf_compressed = (tga_type & 0x20) != 0;	// Huffman, Delta & RLE compressed
        bool has_alpha = (header->depth == 32) | (header->alpha_bits != 0);

        // not supporting compressed images (yet)
        if(rle_compressed || huf_compressed)
        {
            VERBOSE_MESSAGE("not supporting compressed images");
            return false;
        }

        // determine the target format to load the image as
        mTextureFormat = TGA_INVALID;
        mWidth = header->width;
        mHeight = header->height;

    	VERBOSE_MESSAGE("Image " << mWidth << "x" << mHeight);

        switch(tga_type&0x07)
        {
        case 0x01:	// indexed image
            VERBOSE_MESSAGE("indexed image not supported");
            return false;	// unsupported (currently)
            break;

        case 0x02:	// RGB image
            switch(header->depth)
            {
            case 16:	mTextureFormat = has_alpha ? TGA_R5G5B5A1 : TGA_R5G6B5;	break;
            case 24:	mTextureFormat = TGA_R8G8B8;	break;
            case 32:	mTextureFormat = has_alpha ? TGA_R8G8B8A8 : TGA_R8G8B8;	break;
            default:	return false;	// unsupported bit depth (currently)
            }
            break;

        case 0x03:	// grey scale image
            switch(header->depth)
            {
            case 8:	mTextureFormat = TGA_A8;	break;
            default:
                return false;	// unsupported bit depth (currently)
            }
            break;

        case 0x00:	// no image data in file
        default:	// unsupported
            VERBOSE_MESSAGE("no image data in file, not supported");
            return false;
        }

        // fill in the target with converted file data
        int source_byte_size = (header->depth>>3);
        int y = -1;
        int y_step = 1;
        int h = header->height;
        if(header->flipped == 0)
        {
            y_step = -1;
            y = header->height;
        }

//        int target_byte_size = has_alpha?4:3;
 //       VERBOSE_MESSAGE("target_byte_size " << target_byte_size);

        // initialise the target image buffer
        const size_t bufferSize = header->width * header->height;

        // This should reset internal index to 0 and ensure the memory it uses is at least the size we need.
        // It should not reallocate memory if the buffer is already the correct size, this is a good speed up.
        mRed.resize(0); // https://en.cppreference.com/w/cpp/container/vector/resize
        mGreen.resize(0);
        mBlue.resize(0);
        mAlpha.resize(0);
        mRed.reserve(bufferSize); // https://en.cppreference.com/w/cpp/container/vector/reserve
        mGreen.reserve(bufferSize);
        mBlue.reserve(bufferSize);
        mAlpha.reserve(bufferSize);

        //Get a pointer to the image data (stored in BGRA byte sequence)
        const uint8_t* file_pixel_data = pMemory.data() + header->id_length + header->map_length;

        while( h--)
        {
            y += y_step;
            const uint8_t* source = file_pixel_data;

            //Read next line.
            file_pixel_data += header->width * source_byte_size;

            for( int x = 0 ; x < header->width ; x++ )
            {
                uint8_t component_value[4];

// This is just wrong for 16 bit formats!!! Todo: Fix!
                for(int c = 0 ; c < source_byte_size ; c++ )
                {
                    component_value[c] = *source++;
                }

                if(source_byte_size == 1)
                {
                    component_value[1] = component_value[0];
                    component_value[2] = component_value[0];
                    component_value[3] = component_value[0];
                }

                switch(mTextureFormat)
                {
                case TGA_A8:
                    mRed.push_back(255);
                    mGreen.push_back(255);
                    mBlue.push_back(255);
                    mAlpha.push_back(component_value[3]);
                    break;

                case TGA_R5G6B5:
                    assert(!"LoadTGA failed, TGA_R5G6B5 unsupported");
//                    *dest++ = ((component_value[1]&0xfc)<<3)|(component_value[0]>>3);
//                    *dest++ = (component_value[2]&0xf8)|(component_value[1]>>5);
                    break;

                case TGA_R5G5B5A1:
                    assert(!"LoadTGA failed, TGA_R5G5B5A1 unsupported");
//                    *dest++ = ((component_value[1]&0xf8)<<3)|((component_value[0]&0xfe)>>2)|(component_value[3]&0x01);
//                    *dest++ = (component_value[2]&0xf8)|(component_value[1]>>5);
                    break;

                case TGA_R8G8B8:
                    mRed.push_back(component_value[2]);
                    mGreen.push_back(component_value[1]);
                    mBlue.push_back(component_value[0]);
                    mAlpha.push_back(255);                    
                    break;

                case TGA_R8G8B8A8:
                    mRed.push_back(component_value[2]);
                    mGreen.push_back(component_value[1]);
                    mBlue.push_back(component_value[0]);
                    mAlpha.push_back(component_value[3]);
                    break;

                default:
                    assert(!"LoadTGA failed, unknown destination format");
                }
            }
        }
        return true;
    }

    uint32_t GetWidth()const{return mWidth;}
    uint32_t GetHeight()const{return mHeight;}

    /**
     * @brief Will return a 24bit image in RGB order. Will convert the original data to correct bit depth. Alpha is ignored.
     * 
     * @param rRGB 
     * @return true 
     * @return false 
     */
    bool GetRGB(std::vector<uint8_t>& rRGB)const
    {
        rRGB.resize(mWidth*mHeight*3);

        uint8_t* dest = rRGB.data();

        for( uint32_t n = 0 ; n < (mWidth*mHeight) ; n++, dest += 3 )
        {
            dest[0] = mRed[n];
            dest[1] = mGreen[n];
            dest[2] = mBlue[n];
        }

        return true;
    }

    /**
     * @brief Will return a 32bit image in RGBA order. Will convert the original data to correct bit depth.
     * Alpha is set to 255 if was not already in source image.
     * 
     * @param rRGBA 
     * @return true 
     * @return false 
     */
    bool GetRGBA(std::vector<uint8_t>& rRGBA)const
    {
        rRGBA.resize(mWidth*mHeight*4);

        uint8_t* dest = rRGBA.data();

        for( uint32_t n = 0 ; n < (mWidth*mHeight) ; n++, dest += 4 )
        {
            dest[0] = mRed[n];
            dest[1] = mGreen[n];
            dest[2] = mBlue[n];
            dest[3] = mAlpha[n];
        }

        return true;
    }

    /**
     * @brief Gets the alpha status of the PNG file.
     * 
     * @return true 
     * @return false 
     */
    bool GetHasAlpha()const{return mHasAlpha;}

private:
    bool mHasAlpha = false;
    uint32_t mWidth = 0;
    uint32_t mHeight = 0;
    TGA_TextureFormat mTextureFormat = TGA_INVALID;
    // When I load I split out the channels like this to help avoid endian issues.
    // There will be supporting functions to return the data in the most popular arrangements.
    std::vector<uint8_t> mRed,mGreen,mBlue,mAlpha;

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
}//namespace tinytga

#endif //TINY_TGA_H