/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_TEXTURE_DESCRIPTOR_H__
#define __DAVAENGINE_TEXTURE_DESCRIPTOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Utils/MD5.h"
#include "FileSystem/FilePath.h"
#include "Render/Texture.h"

namespace DAVA
{

class File;
class TextureDescriptor 
{
    static const int32 DATE_BUFFER_SIZE = 20;
    static const int32 LINE_SIZE = 256;
    static const int8 CURRENT_VERSION = 7;
    
	enum eSignatures
	{
		COMPRESSED_FILE = 0x00EEEE00,
		NOTCOMPRESSED_FILE = 0x00EE00EE
	};

public:

	struct TextureDrawSettings: public InspBase
	{
	public:
		TextureDrawSettings() { SetDefaultValues(); }
		void SetDefaultValues();

		int8 wrapModeS;
		int8 wrapModeT;

		int8 minFilter;
		int8 magFilter;

		INTROSPECTION(TextureDrawSettings,
			MEMBER(wrapModeS, InspDesc("wrapModeS", GlobalEnumMap<Texture::TextureWrap>::Instance()), I_VIEW | I_EDIT | I_SAVE)
			MEMBER(wrapModeT, InspDesc("wrapModeT", GlobalEnumMap<Texture::TextureWrap>::Instance()), I_VIEW | I_EDIT)
			MEMBER(minFilter, InspDesc("minFilter", GlobalEnumMap<Texture::TextureFilter>::Instance()), I_VIEW | I_EDIT | I_SAVE)
			MEMBER(magFilter, InspDesc("magFilter", GlobalEnumMap<Texture::TextureFilter>::Instance()), I_VIEW | I_EDIT | I_SAVE)
		)
	};
    
	struct TextureDataSettings: public InspBase
	{
	private:
		void EnableFlag(bool enable, int8 flag);
		bool IsFlagEnabled(int8 flag) const;
	public:
		enum eOptionsFlag
		{
			FLAG_GENERATE_MIPMAPS = 1 << 0,
			FLAG_INVALID = 1 << 7
		};

		TextureDataSettings() { SetDefaultValues(); }
		void SetDefaultValues();

		void SetGenerateMipmaps(const bool & generateMipmaps);
		bool GetGenerateMipMaps() const;

		int8 textureFlags;
		uint8 faceDescription;

		INTROSPECTION(TextureDataSettings,
			PROPERTY("generateMipMaps", "generateMipMaps", GetGenerateMipMaps, SetGenerateMipmaps, I_VIEW | I_EDIT | I_SAVE)
			MEMBER(faceDescription, "faceDescription", I_SAVE)
		)
	};

    struct Compression: public InspBase
    {
        int32 format;
        mutable uint32 sourceFileCrc;
        int32 compressToWidth;
        int32 compressToHeight;
        mutable uint32 convertedFileCrc;
        
		Compression() { Clear(); } 
        void Clear();

		INTROSPECTION(Compression,
			MEMBER(format, InspDesc("format", GlobalEnumMap<PixelFormat>::Instance()), I_VIEW | I_EDIT | I_SAVE)
			MEMBER(sourceFileCrc, "Source File CRC", I_SAVE)
			MEMBER(compressToWidth, "compressToWidth", I_SAVE)
			MEMBER(compressToHeight, "compressToHeight", I_SAVE)
            MEMBER(convertedFileCrc, "Converted File CRC", I_SAVE)
		)
    };
    

public:
    TextureDescriptor(bool needCompressionSettings = true);
	virtual ~TextureDescriptor();

	static TextureDescriptor * CreateFromFile(const FilePath &filePathname);
	static TextureDescriptor * CreateDescriptor(Texture::TextureWrap wrap, bool generateMipmaps);

	void Initialize(Texture::TextureWrap wrap, bool generateMipmaps);
	void Initialize(const TextureDescriptor *descriptor);
	bool Initialize(const FilePath &filePathname);

	void SetDefaultValues(bool needCompressionSettings = true);

    void SetQualityGroup(const FastName &group);
    FastName GetQualityGroup() const;
    
    bool Load(const FilePath &filePathname); //may be protected?

    void Save() const;
    void Save(const FilePath &filePathname) const;
    void Export(const FilePath &filePathname);

    bool IsCompressedTextureActual(eGPUFamily forGPU) const;
    bool UpdateCrcForFormat(eGPUFamily forGPU) const;
    
    
    bool IsCompressedFile() const;
	void SetGenerateMipmaps(bool generateMipmaps);
    bool GetGenerateMipMaps() const;
	bool IsCubeMap() const;

    FilePath GetSourceTexturePathname() const; 

    static String GetSourceTextureExtension();
    static String GetSupportedTextureExtensions();

    static FilePath GetDescriptorPathname(const FilePath &texturePathname);
    static String GetDescriptorExtension();
    
    PixelFormat GetPixelFormatForCompression(eGPUFamily forGPU);
    
protected:


	void AllocateCompressionData();
	void ReleaseCompressionData();


protected:
    
    const Compression * GetCompressionParams(eGPUFamily forGPU) const;
    
    void LoadNotCompressed(File *file);
    void LoadCompressed(File *file);
    
    void ReadGeneralSettings(File *file);
    void WriteGeneralSettings(File *file) const;
    
    void ReadCompression(File *file, Compression *compression);
	void ReadCompressionWithDateOld(File *file, Compression *compression);
	void ReadCompressionWith16CRCOld(File *file, Compression *compression);

	void WriteCompression(File *file, const Compression *compression) const;
    
    
    void ConvertToCurrentVersion(int8 version, int32 signature, File *file);

	void LoadVersion5(int32 signature, File *file);
	void LoadVersion6(int32 signature, File *file);
    
	uint32 ReadSourceCRC() const;
	uint32 ReadConvertedCRC(eGPUFamily forGPU) const;
    
public:
    
    FilePath pathname;

	//moved from Texture
	PixelFormat format:8;			// texture format
	FastName qualityGroup;
    
	TextureDrawSettings drawSettings;
	TextureDataSettings dataSettings;
	Compression **compression;
    
    //Binary only
    int8 exportedAsGpuFamily;
    int8 exportedAsPixelFormat;
	
    bool isCompressedFile:1;
};
    
};
#endif // __DAVAENGINE_TEXTURE_DESCRIPTOR_H__
