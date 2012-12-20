#ifndef __DAVAENGINE_DXT_HELPER_H__
#define __DAVAENGINE_DXT_HELPER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"
#include "Render/Image.h"

namespace nvtt
{
	enum Format;
	struct Decompressor;
};

namespace DAVA 
{

class Texture;
class Sprite;
class Image;


class LibDxtHelper
{
public:
	
	static bool IsDxtFile(const char *fileName);

	static bool IsDxtFile(FILE * file);

	//input data only in RGBA8888
	static bool WriteDxtFile(const char* fileName, int32 width, int32 height, uint8 * data, PixelFormat compressionFormat, uint32 mipmupNumber);

	static bool ReadDxtFile(const char *fileName, Vector<DAVA::Image*> &imageSet, bool forseSoftwareConvertation = false);
	
	static bool ReadDxtFile(FILE * file, Vector<DAVA::Image*> &imageSet, bool forseSoftwareConvertation = false);

	static bool DecompressImageToRGBA(const DAVA::Image & image, Vector<DAVA::Image*> &imageSet, bool forseSoftwareConvertation = false);

	static PixelFormat GetPixelFormat(const char* fileName);
	
	static PixelFormat GetPixelFormat(FILE * file);
	
	static bool GetTextureSize(const char *fileName, uint32 & width, uint32 & height);

	static bool GetTextureSize(FILE * file, uint32 & width, uint32 & height);

	static uint32 GetMipMapLevelsCount(const char *fileName);

	static uint32 GetMipMapLevelsCount(FILE * file);

	static uint32 GetDataSize(const char *fileName);
	
	static uint32 GetDataSize(FILE * file);

	//static void Test();

private:
	
	struct DDSInfo
	{
		uint32 width;
		uint32 height;
		uint32 dataSize;
		uint32 mipMupsNumber;
		uint32 headerSize;

		DDSInfo()
		{
			width		  = 0;
			height		  = 0;
			dataSize	  = 0;
			mipMupsNumber = 0;
			headerSize	  = 0;
		}
	};

	static bool InitDecompressor(nvtt::Decompressor & dec, const char *fileName);
	
	static bool InitDecompressor(nvtt::Decompressor & dec, FILE * file);

	static bool InitDecompressor(nvtt::Decompressor & dec, const uint8 * mem, uint32 size);

	static bool ReadDxtFile(nvtt::Decompressor & dec, Vector<DAVA::Image*> &imageSet, bool forseSoftwareConvertation);

	static PixelFormat GetPixelFormat(nvtt::Decompressor & dec);

	static bool GetTextureSize(nvtt::Decompressor & dec, uint32 & width, uint32 & height);

	static uint32 GetMipMapLevelsCount(nvtt::Decompressor & dec);

	static uint32 GetDataSize(nvtt::Decompressor & dec);
	
	static bool GetInfo(nvtt::Decompressor & dec, DDSInfo &info);

	static void ConvertFromBGRAtoRGBA(uint8* data, uint32 size);

	static uint32 GetGlCompressionFormatByDDSInfo(nvtt::Format format);

};

};

#endif // __DXT_HELPER_H__