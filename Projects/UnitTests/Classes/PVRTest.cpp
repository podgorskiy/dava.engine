/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTR ACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "PVRTest.h"

static const PixelFormat formats[] =
{
    FORMAT_PVR2,
    FORMAT_PVR4,
    FORMAT_RGBA8888,
    FORMAT_RGBA4444,
    FORMAT_RGBA5551,
    FORMAT_RGB888,
    FORMAT_RGB565,
    FORMAT_A8,
    FORMAT_RGBA16161616,
    FORMAT_RGBA32323232
};


PVRTest::PVRTest()
: TestTemplate<PVRTest>("PVRTest")
{
    pngSprite = NULL;
    pvrSprite = NULL;
    decompressedPNGSprite = NULL;

    currentTest = FIRST_TEST;
    for(int32 i = 0; i < TESTS_COUNT; ++i)
    {
        PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(formats[i]);
        RegisterFunction(this, &PVRTest::TestFunction, Format("PVRTest of %s", formatDescriptor.name.c_str()), NULL);
    }
}

void PVRTest::LoadResources()
{
    GetBackground()->SetColor(Color(0.0f, 1.0f, 0.0f, 1.0f));

    Font *font = FTFont::Create("~res:/Fonts/korinna.ttf");
    font->SetSize(20);
    font->SetColor(Color::White());

    compareResultText = new UIStaticText(Rect(0, 256, 512, 200));
    compareResultText->SetAlign(ALIGN_LEFT | ALIGN_VCENTER);
    compareResultText->SetMultiline(true);
    compareResultText->SetFont(font);
    AddControl(compareResultText);

    SafeRelease(font);
}


Sprite * PVRTest::CreateSpriteFromTexture(const String &texturePathname)
{
    Sprite *createdSprite = NULL;
    
    
    Texture *texture = Texture::CreateFromFile(texturePathname);
    if(texture)
    {
        createdSprite = Sprite::CreateFromTexture(texture, 0, 0, (float32)texture->GetWidth(), (float32)texture->GetHeight());
        texture->Release();
    }
    
    return createdSprite;
}


void PVRTest::UnloadResources()
{
    RemoveAllControls();
    
    SafeRelease(compareResultText);
    
    SafeRelease(pngSprite);
    SafeRelease(pvrSprite);
    SafeRelease(decompressedPNGSprite);
}

void PVRTest::TestFunction(PerfFuncData * data)
{
    DVASSERT(currentTest < TESTS_COUNT);
    
    if(IsCurrentTestAccepted())
    {
        ReloadSprites();
        
        CompareResult result = CompareSprites(decompressedPNGSprite, pvrSprite);
        float32 differencePersentage = ((float32)result.difference / ((float32)result.bytesCount * 256.f)) * 100.f;
        
        PixelFormatDescriptor formatDescriptor = Texture::GetPixelFormatDescriptor(formats[currentTest]);
        data->testData.message = Format("\nDifference: %f%%\nCoincidence: %f%%",
                                        result.difference, differencePersentage, 100.f - differencePersentage);
        
        compareResultText->SetText(StringToWString(data->testData.message));
        Logger::Debug(data->testData.message.c_str());
        
        TEST_VERIFY(differencePersentage < (float32)ACCETABLE_DELTA_IN_PERSENTS);
    }
    
    ++currentTest;
}

bool PVRTest::IsCurrentTestAccepted()
{
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();
    
    if((formats[currentTest] == FORMAT_RGBA16161616) && !deviceCaps.isFloat16Supported)
    {
        return false;
    }
    
    if((formats[currentTest] == FORMAT_RGBA32323232) && !deviceCaps.isFloat32Supported)
    {
        return false;
    }

    return true;
}


void PVRTest::ReloadSprites()
{
    SafeRelease(pngSprite);
    SafeRelease(pvrSprite);
    SafeRelease(decompressedPNGSprite);
    
    Image::EnableAlphaPremultiplication(false);
    
    pngSprite = CreateSpriteFromTexture(String(Format("~res:/PVRTest/PNG/number_%d.png", currentTest)));
    pvrSprite = CreateSpriteFromTexture(String(Format("~res:/PVRTest/PVR/number_%d.pvr", currentTest)));
    decompressedPNGSprite = CreateSpriteFromTexture(String(Format("~res:/PVRTest/DecompressedPNG/number_%d.png", currentTest)));
    
    Image::EnableAlphaPremultiplication(true);
}

void PVRTest::Draw(const DAVA::UIGeometricData &geometricData)
{
    RenderManager::Instance()->ClearWithColor(0.f, 0.0f, 0.f, 1.f);
    
    RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);

    if(pngSprite)
    {
        pngSprite->SetPosition(0, 0);
        pngSprite->SetScaleSize(256.f, 256.f);
        pngSprite->Draw();
    }
    
    if(pvrSprite)
    {
        pvrSprite->SetPosition(260.f, 0);
        pvrSprite->SetScaleSize(256.f, 256.f);
        pvrSprite->Draw();
    }
    
    TestTemplate<PVRTest>::Draw(geometricData);
}

PVRTest::CompareResult PVRTest::CompareSprites(Sprite *first, Sprite *second)
{
    DVASSERT(first->GetHeight() == second->GetHeight());
    DVASSERT(first->GetWidth() == second->GetWidth());
    
    Image *firstComparer = CreateImageAsRGBA8888(first);
    Image *secondComparer = CreateImageAsRGBA8888(second);

    CompareResult compareResult = {0};

    
    int32 imageSizeInBytes = (int32)(first->GetWidth() * first->GetHeight() * Texture::GetPixelFormatSizeInBytes(firstComparer->format));

    int32 step = 1;
    int32 startIndex = 0;
    
    if(FORMAT_A8 == formats[currentTest])
    {
        compareResult.bytesCount = (int32)(first->GetWidth() * first->GetHeight() * Texture::GetPixelFormatSizeInBytes(FORMAT_A8));
        step = 4;
        startIndex = 3;
    }
    else
    {
        compareResult.bytesCount = imageSizeInBytes;
    }

    for(int32 i = startIndex; i < imageSizeInBytes; i += step)
    {
        compareResult.difference += abs(firstComparer->GetData()[i] - secondComparer->GetData()[i]);
    }
    
    String documentsPath = FileSystem::Instance()->GetCurrentDocumentsDirectory();
    firstComparer->Save(documentsPath + Format("src_number_%d.png", currentTest));
    secondComparer->Save(documentsPath + Format("dst_number_%d.png", currentTest));
    
    
    SafeRelease(firstComparer);
    SafeRelease(secondComparer);
    return compareResult;
}

Image * PVRTest::CreateImageAsRGBA8888(Sprite *sprite)
{
    Sprite *renderTarget = Sprite::CreateAsRenderTarget(sprite->GetWidth(), sprite->GetHeight(), FORMAT_RGBA8888);
    RenderManager::Instance()->SetRenderTarget(renderTarget);
    
    sprite->SetPosition(0, 0);
    sprite->Draw();
    
    RenderManager::Instance()->RestoreRenderTarget();
    
    Texture *renderTargetTexture = renderTarget->GetTexture();
    Image *resultImage = renderTargetTexture->CreateImageFromMemory();
    
    SafeRelease(renderTarget);
    return resultImage;
}


