#include "MovieViewControlWin32.h"

#include "Platform/TemplateWin32/FfmpegPlayer.h"
#include "Render/PixelFormatDescriptor.h"
#include "UI/UIControl.h"

namespace DAVA
{
MovieViewControl::MovieViewControl()
    : ffmpegPlayer(new FfmpegPlayer())
    , videoBackground(new UIControlBackground())
{
    ffmpegPlayer->Initialize(Rect());
    videoBackground->SetDrawType(UIControlBackground::eDrawType::DRAW_SCALE_PROPORTIONAL);
}

MovieViewControl::~MovieViewControl()
{
    SafeDelete(ffmpegPlayer);
    SafeRelease(videoTexture);
    SafeRelease(videoBackground);
    SafeDeleteArray(videoTextureBuffer);
}

void MovieViewControl::Initialize(const Rect& rect)
{
    ffmpegPlayer->Initialize(rect);
}

void MovieViewControl::SetRect(const Rect& rect)
{
    ffmpegPlayer->SetRect(rect);
}

void MovieViewControl::SetVisible(bool isVisible)
{
    ffmpegPlayer->SetVisible(isVisible);
}

void MovieViewControl::OpenMovie(const FilePath& moviePath, const OpenMovieParams& params)
{
    ffmpegPlayer->OpenMovie(moviePath, params);
}

void MovieViewControl::Play()
{
    ffmpegPlayer->Play();
    Vector2 res = ffmpegPlayer->GetResolution();
    textureWidth = NextPowerOf2(static_cast<uint32>(res.dx));
    textureHeight = NextPowerOf2(static_cast<uint32>(res.dy));
    uint32 size = textureWidth * textureHeight * PixelFormatDescriptor::GetPixelFormatSizeInBytes(ffmpegPlayer->GetPixelFormat());

    SafeDeleteArray(videoTextureBuffer);
    videoTextureBuffer = new uint8[size];

    Memset(videoTextureBuffer, 0, size);
}

void MovieViewControl::Stop()
{
    ffmpegPlayer->Stop();
    SafeDeleteArray(videoTextureBuffer);
    SafeRelease(videoTexture);
}

void MovieViewControl::Pause()
{
    ffmpegPlayer->Pause();
}

void MovieViewControl::Resume()
{
    ffmpegPlayer->Resume();
}

bool MovieViewControl::IsPlaying() const
{
    return ffmpegPlayer->IsPlaying();
}

void MovieViewControl::Update()
{
    if (nullptr == ffmpegPlayer)
        return;

    ffmpegPlayer->Update();

    if (FfmpegPlayer::STOPPED == ffmpegPlayer->GetState())
        SafeRelease(videoTexture);

    FfmpegPlayer::DrawVideoFrameData* drawData = ffmpegPlayer->GetDrawData();

    if (nullptr == drawData || nullptr == videoTextureBuffer)
        return;

    Memcpy(videoTextureBuffer, drawData->data, drawData->dataSize);
    if (nullptr == videoTexture)
    {
        videoTexture = Texture::CreateFromData(drawData->format, videoTextureBuffer, textureWidth, textureHeight, false);
        Sprite* videoSprite = Sprite::CreateFromTexture(videoTexture, 0, 0, drawData->frameWidth, drawData->frameHeight, static_cast<float32>(drawData->frameWidth), static_cast<float32>(drawData->frameHeight));
        videoBackground->SetSprite(videoSprite);
        videoSprite->Release();
    }
    else
    {
        videoTexture->TexImage(0, textureWidth, textureHeight, videoTextureBuffer, drawData->dataSize, Texture::INVALID_CUBEMAP_FACE);
    }
    SafeDelete(drawData);
}

void MovieViewControl::Draw(const UIGeometricData& parentGeometricData)
{
    if (videoTexture)
        videoBackground->Draw(parentGeometricData);
}
}