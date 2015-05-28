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


#include "Base/GlobalEnum.h"
#include "Render/Texture.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/Entity.h"
#include "Platform/DeviceInfo.h"
#include "DLC/DLC.h"
#include "UI/UIControlBackground.h"
#include "UI/UIStaticText.h"
#include "Render/2D/TextBlock.h"
#include "UI/UIList.h"
#include "UI/UITextField.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

ENUM_DECLARE(eGPUFamily)
{
	ENUM_ADD_DESCR(GPU_POWERVR_IOS, "PowerVR_iOS");
	ENUM_ADD_DESCR(GPU_POWERVR_ANDROID, "PowerVR_Android");
	ENUM_ADD_DESCR(GPU_TEGRA, "tegra");
	ENUM_ADD_DESCR(GPU_MALI, "mali");
	ENUM_ADD_DESCR(GPU_ADRENO, "adreno");
	ENUM_ADD_DESCR(GPU_PNG, "PNG");
}

ENUM_DECLARE(Texture::TextureWrap)
{
	ENUM_ADD_DESCR(Texture::WRAP_CLAMP_TO_EDGE, "WRAP_CLAMP_TO_EDGE");
	ENUM_ADD_DESCR(Texture::WRAP_REPEAT, "WRAP_REPEAT");
}

ENUM_DECLARE(Texture::TextureFilter)
{
	ENUM_ADD_DESCR(Texture::FILTER_LINEAR, "LINEAR");
	ENUM_ADD_DESCR(Texture::FILTER_NEAREST, "NEAREST");
	ENUM_ADD_DESCR(Texture::FILTER_NEAREST_MIPMAP_NEAREST, "NEAREST_MIPMAP_NEAREST");
	ENUM_ADD_DESCR(Texture::FILTER_LINEAR_MIPMAP_NEAREST, "LINEAR_MIPMAP_NEAREST");
	ENUM_ADD_DESCR(Texture::FILTER_NEAREST_MIPMAP_LINEAR, "NEAREST_MIPMAP_LINEAR");
	ENUM_ADD_DESCR(Texture::FILTER_LINEAR_MIPMAP_LINEAR, "LINEAR_MIPMAP_LINEAR");
}

ENUM_DECLARE(PixelFormat)
{
	ENUM_ADD_DESCR(FORMAT_RGBA8888, "RGBA8888");
	ENUM_ADD_DESCR(FORMAT_RGBA5551, "RGBA5551");
	ENUM_ADD_DESCR(FORMAT_RGBA4444, "RGBA4444");
	ENUM_ADD_DESCR(FORMAT_RGB888, "RGB888");
	ENUM_ADD_DESCR(FORMAT_RGB565, "RGB565");
	ENUM_ADD_DESCR(FORMAT_A8, "A8");
	ENUM_ADD_DESCR(FORMAT_A16, "A16");
	ENUM_ADD_DESCR(FORMAT_PVR4, "PVR4");
	ENUM_ADD_DESCR(FORMAT_PVR2, "PVR2");
	ENUM_ADD_DESCR(FORMAT_RGBA16161616, "RGBA16161616");
	ENUM_ADD_DESCR(FORMAT_RGBA32323232, "RGBA32323232");
	ENUM_ADD_DESCR(FORMAT_DXT1, "DXT1");
	ENUM_ADD_DESCR(FORMAT_DXT1A, "DXT1A");
	ENUM_ADD_DESCR(FORMAT_DXT3, "DXT3");
	ENUM_ADD_DESCR(FORMAT_DXT5, "DXT5");
	ENUM_ADD_DESCR(FORMAT_DXT5NM, "DXT5NM");
	ENUM_ADD_DESCR(FORMAT_ETC1, "ETC1");
	ENUM_ADD_DESCR(FORMAT_ATC_RGB, "ATC_RGB");
	ENUM_ADD_DESCR(FORMAT_ATC_RGBA_EXPLICIT_ALPHA, "ATC_RGBA_EXPLICIT_ALPHA");
	ENUM_ADD_DESCR(FORMAT_ATC_RGBA_INTERPOLATED_ALPHA, "ATC_RGBA_INTERPOLATED_ALPHA");

    ENUM_ADD_DESCR(FORMAT_PVR2_2, "PVR2_2");
	ENUM_ADD_DESCR(FORMAT_PVR4_2, "PVR4_2");
	ENUM_ADD_DESCR(FORMAT_EAC_R11_UNSIGNED, "EAC_R11_UNSIGNED");
	ENUM_ADD_DESCR(FORMAT_EAC_R11_SIGNED, "EAC_R11_SIGNED");
	ENUM_ADD_DESCR(FORMAT_EAC_RG11_UNSIGNED, "EAC_RG11_UNSIGNED");
	ENUM_ADD_DESCR(FORMAT_EAC_RG11_SIGNED, "EAC_RG11_SIGNED");
	ENUM_ADD_DESCR(FORMAT_ETC2_RGB, "ETC2_RGB");
	ENUM_ADD_DESCR(FORMAT_ETC2_RGBA, "ETC2_RGBA");
	ENUM_ADD_DESCR(FORMAT_ETC2_RGB_A1, "ETC2_RGB_A1");
}

ENUM_DECLARE(Light::eType)
{
	ENUM_ADD_DESCR(Light::TYPE_DIRECTIONAL, "Directional");
	ENUM_ADD_DESCR(Light::TYPE_SPOT, "Spot");
	ENUM_ADD_DESCR(Light::TYPE_POINT, "Point");
    ENUM_ADD_DESCR(Light::TYPE_SKY, "Sky");
    ENUM_ADD_DESCR(Light::TYPE_AMBIENT, "Ambient");
}

ENUM_DECLARE(Entity::EntityFlags)
{
    ENUM_ADD_DESCR(Entity::NODE_WORLD_MATRIX_ACTUAL, "NODE_WORLD_MATRIX_ACTUAL");
    ENUM_ADD_DESCR(Entity::NODE_VISIBLE, "NODE_VISIBLE");
    ENUM_ADD_DESCR(Entity::NODE_UPDATABLE, "NODE_UPDATABLE");
    ENUM_ADD_DESCR(Entity::NODE_IS_LOD_PART, "NODE_IS_LOD_PART");
    ENUM_ADD_DESCR(Entity::NODE_LOCAL_MATRIX_IDENTITY, "NODE_LOCAL_MATRIX_IDENTITY");
    ENUM_ADD_DESCR(Entity::BOUNDING_VOLUME_AABB, "BOUNDING_VOLUME_AABB");
    ENUM_ADD_DESCR(Entity::BOUNDING_VOLUME_OOB, "BOUNDING_VOLUME_OOB");
    ENUM_ADD_DESCR(Entity::BOUNDING_VOLUME_SPHERE, "BOUNDING_VOLUME_SPHERE");
    ENUM_ADD_DESCR(Entity::NODE_CLIPPED_PREV_FRAME, "NODE_CLIPPED_PREV_FRAME");
    ENUM_ADD_DESCR(Entity::NODE_CLIPPED_THIS_FRAME, "NODE_CLIPPED_THIS_FRAME");
    ENUM_ADD_DESCR(Entity::NODE_INVALID, "NODE_INVALID");
    ENUM_ADD_DESCR(Entity::TRANSFORM_NEED_UPDATE, "TRANSFORM_NEED_UPDATE");
    ENUM_ADD_DESCR(Entity::TRANSFORM_DIRTY, "TRANSFORM_DIRTY");
    ENUM_ADD_DESCR(Entity::NODE_DELETED, "NODE_DELETED");
    ENUM_ADD_DESCR(Entity::SCENE_LIGHTS_MODIFIED, "SCENE_LIGHTS_MODIFIED");
}

ENUM_DECLARE(DeviceInfo::ePlatform)
{
    ENUM_ADD_DESCR(DeviceInfo::PLATFORM_IOS, "iOS");
    ENUM_ADD_DESCR(DeviceInfo::PLATFORM_IOS_SIMULATOR, "iOS Simulator");
    ENUM_ADD_DESCR(DeviceInfo::PLATFORM_MACOS, "MacOS");
    ENUM_ADD_DESCR(DeviceInfo::PLATFORM_ANDROID, "Android");
    ENUM_ADD_DESCR(DeviceInfo::PLATFORM_WIN32, "Win32");
    ENUM_ADD_DESCR(DeviceInfo::PLATFORM_UNKNOWN, "Unknown");
}

ENUM_DECLARE(DLC::DLCError)
{
    ENUM_ADD(DLC::DE_NO_ERROR);
    ENUM_ADD(DLC::DE_WAS_CANCELED);
    ENUM_ADD(DLC::DE_INIT_ERROR);
    ENUM_ADD(DLC::DE_CHECK_ERROR);
    ENUM_ADD(DLC::DE_READ_ERROR);
    ENUM_ADD(DLC::DE_WRITE_ERROR);
    ENUM_ADD(DLC::DE_CONNECT_ERROR);
    ENUM_ADD(DLC::DE_DOWNLOAD_ERROR);
    ENUM_ADD(DLC::DE_PATCH_ERROR_LITE);
    ENUM_ADD(DLC::DE_PATCH_ERROR_FULL);
}

ENUM_DECLARE(UIControlBackground::eDrawType)
{
    ENUM_ADD_DESCR(UIControlBackground::DRAW_ALIGNED, "DRAW_ALIGNED");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_SCALE_TO_RECT, "DRAW_SCALE_TO_RECT");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_SCALE_PROPORTIONAL, "DRAW_SCALE_PROPORTIONAL");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_SCALE_PROPORTIONAL_ONE, "DRAW_SCALE_PROPORTIONAL_ONE");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_FILL, "DRAW_FILL");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_STRETCH_HORIZONTAL, "DRAW_STRETCH_HORIZONTAL");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_STRETCH_VERTICAL, "DRAW_STRETCH_VERTICAL");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_STRETCH_BOTH, "DRAW_STRETCH_BOTH");
    ENUM_ADD_DESCR(UIControlBackground::DRAW_TILED, "DRAW_TILED");
}

ENUM_DECLARE(eAlign)
{
    ENUM_ADD_DESCR(ALIGN_LEFT, "LEFT");
    ENUM_ADD_DESCR(ALIGN_HCENTER, "HCENTER");
    ENUM_ADD_DESCR(ALIGN_RIGHT, "RIGHT");
    ENUM_ADD_DESCR(ALIGN_TOP, "TOP");
    ENUM_ADD_DESCR(ALIGN_VCENTER, "VCENTER");
    ENUM_ADD_DESCR(ALIGN_BOTTOM, "BOTTOM");
    ENUM_ADD_DESCR(ALIGN_HJUSTIFY, "HJUSTIFY");
}

ENUM_DECLARE(UIControlBackground::eColorInheritType)
{
    ENUM_ADD_DESCR(UIControlBackground::COLOR_MULTIPLY_ON_PARENT, "COLOR_MULTIPLY_ON_PARENT");
    ENUM_ADD_DESCR(UIControlBackground::COLOR_ADD_TO_PARENT, "COLOR_ADD_TO_PARENT");
    ENUM_ADD_DESCR(UIControlBackground::COLOR_REPLACE_TO_PARENT, "COLOR_REPLACE_TO_PARENT");
    ENUM_ADD_DESCR(UIControlBackground::COLOR_IGNORE_PARENT, "COLOR_IGNORE_PARENT");
    ENUM_ADD_DESCR(UIControlBackground::COLOR_MULTIPLY_ALPHA_ONLY, "COLOR_MULTIPLY_ALPHA_ONLY");
    ENUM_ADD_DESCR(UIControlBackground::COLOR_REPLACE_ALPHA_ONLY, "COLOR_REPLACE_ALPHA_ONLY");
}

ENUM_DECLARE(UIControlBackground::ePerPixelAccuracyType)
{
    ENUM_ADD_DESCR(UIControlBackground::PER_PIXEL_ACCURACY_DISABLED, "PER_PIXEL_ACCURACY_DISABLED");
    ENUM_ADD_DESCR(UIControlBackground::PER_PIXEL_ACCURACY_ENABLED, "PER_PIXEL_ACCURACY_ENABLED");
    ENUM_ADD_DESCR(UIControlBackground::PER_PIXEL_ACCURACY_FORCED, "PER_PIXEL_ACCURACY_FORCED");
};

ENUM_DECLARE(UIStaticText::eMultiline)
{
    ENUM_ADD_DESCR(UIStaticText::MULTILINE_DISABLED, "MULTILINE_DISABLED");
    ENUM_ADD_DESCR(UIStaticText::MULTILINE_ENABLED, "MULTILINE_ENABLED");
    ENUM_ADD_DESCR(UIStaticText::MULTILINE_ENABLED_BY_SYMBOL, "MULTILINE_ENABLED_BY_SYMBOL");
};

ENUM_DECLARE(TextBlock::eFitType)
{
    ENUM_ADD_DESCR(TextBlock::FITTING_DISABLED, "DISABLED");
    ENUM_ADD_DESCR(TextBlock::FITTING_ENLARGE, "ENLARGE");
    ENUM_ADD_DESCR(TextBlock::FITTING_REDUCE, "REDUCE");
    ENUM_ADD_DESCR(TextBlock::FITTING_POINTS, "POINTS");
};

ENUM_DECLARE(UIList::eListOrientation)
{
    ENUM_ADD_DESCR(UIList::ORIENTATION_VERTICAL, "ORIENTATION_VERTICAL");
    ENUM_ADD_DESCR(UIList::ORIENTATION_HORIZONTAL, "ORIENTATION_HORIZONTAL");
};

ENUM_DECLARE(UIScrollBar::eScrollOrientation)
{
    ENUM_ADD_DESCR(UIScrollBar::ORIENTATION_VERTICAL, "ORIENTATION_VERTICAL");
    ENUM_ADD_DESCR(UIScrollBar::ORIENTATION_HORIZONTAL, "ORIENTATION_HORIZONTAL");
};

ENUM_DECLARE(UITextField::eAutoCapitalizationType)
{
    ENUM_ADD_DESCR(UITextField::AUTO_CAPITALIZATION_TYPE_NONE       , "AUTO_CAPITALIZATION_TYPE_NONE"     );
    ENUM_ADD_DESCR(UITextField::AUTO_CAPITALIZATION_TYPE_WORDS      , "AUTO_CAPITALIZATION_TYPE_WORDS"    );
    ENUM_ADD_DESCR(UITextField::AUTO_CAPITALIZATION_TYPE_SENTENCES  , "AUTO_CAPITALIZATION_TYPE_SENTENCES");
    ENUM_ADD_DESCR(UITextField::AUTO_CAPITALIZATION_TYPE_ALL_CHARS  , "AUTO_CAPITALIZATION_TYPE_ALL_CHARS");
};

ENUM_DECLARE(UITextField::eAutoCorrectionType)
{
    ENUM_ADD_DESCR(UITextField::AUTO_CORRECTION_TYPE_DEFAULT, "AUTO_CORRECTION_TYPE_DEFAULT");
    ENUM_ADD_DESCR(UITextField::AUTO_CORRECTION_TYPE_NO     , "AUTO_CORRECTION_TYPE_NO"     );
    ENUM_ADD_DESCR(UITextField::AUTO_CORRECTION_TYPE_YES    , "AUTO_CORRECTION_TYPE_YES"    );
};

ENUM_DECLARE(UITextField::eSpellCheckingType)
{
    ENUM_ADD_DESCR(UITextField::SPELL_CHECKING_TYPE_DEFAULT, "SPELL_CHECKING_TYPE_DEFAULT");
    ENUM_ADD_DESCR(UITextField::SPELL_CHECKING_TYPE_NO     , "SPELL_CHECKING_TYPE_NO"     );
    ENUM_ADD_DESCR(UITextField::SPELL_CHECKING_TYPE_YES    , "SPELL_CHECKING_TYPE_YES"    );
};

ENUM_DECLARE(UITextField::eKeyboardAppearanceType)
{
    ENUM_ADD_DESCR(UITextField::KEYBOARD_APPEARANCE_DEFAULT, "KEYBOARD_APPEARANCE_DEFAULT");
    ENUM_ADD_DESCR(UITextField::KEYBOARD_APPEARANCE_ALERT  , "KEYBOARD_APPEARANCE_ALERT"  );
};

ENUM_DECLARE(UITextField::eKeyboardType)
{
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_DEFAULT                , "KEYBOARD_TYPE_DEFAULT"                );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_ASCII_CAPABLE          , "KEYBOARD_TYPE_ASCII_CAPABLE"          );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION, "KEYBOARD_TYPE_NUMBERS_AND_PUNCTUATION");
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_URL                    , "KEYBOARD_TYPE_URL"                    );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_NUMBER_PAD             , "KEYBOARD_TYPE_NUMBER_PAD"             );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_PHONE_PAD              , "KEYBOARD_TYPE_PHONE_PAD"              );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_NAME_PHONE_PAD         , "KEYBOARD_TYPE_NAME_PHONE_PAD"         );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_EMAIL_ADDRESS          , "KEYBOARD_TYPE_EMAIL_ADDRESS"          );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_DECIMAL_PAD            , "KEYBOARD_TYPE_DECIMAL_PAD"            );
    ENUM_ADD_DESCR(UITextField::KEYBOARD_TYPE_TWITTER                , "KEYBOARD_TYPE_TWITTER"                );
};

ENUM_DECLARE(UITextField::eReturnKeyType)
{
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_DEFAULT       , "RETURN_KEY_DEFAULT"       );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_GO            , "RETURN_KEY_GO"            );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_GOOGLE        , "RETURN_KEY_GOOGLE"        );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_JOIN          , "RETURN_KEY_JOIN"          );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_NEXT          , "RETURN_KEY_NEXT"          );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_ROUTE         , "RETURN_KEY_ROUTE"         );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_SEARCH        , "RETURN_KEY_SEARCH"        );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_SEND          , "RETURN_KEY_SEND"          );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_YAHOO         , "RETURN_KEY_YAHOO"         );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_DONE          , "RETURN_KEY_DONE"          );
    ENUM_ADD_DESCR(UITextField::RETURN_KEY_EMERGENCY_CALL, "RETURN_KEY_EMERGENCY_CALL");
};

ENUM_DECLARE(UIComponent::eType)
{
    ENUM_ADD_DESCR(UIComponent::FAKE_COMPONENT       , "Fake"       );
    ENUM_ADD_DESCR(UIComponent::FAKE_MULTI_COMPONENT       , "FakeMultiple"       );
};

/*
void f()
{
}

GlobalEnum *globalEnum = GlobalEnum::Instance();
f();
*/
/*
->Add(DAVA::MetaInfo::Instance<DAVA::Texture::TextureWrap>(), DAVA::Texture::WRAP_CLAMP_TO_EDGE, "WRAP_CLAMP_TO_EDGE");

ENUM_ADD(DAVA::Texture::TextureWrap, DAVA::Texture::WRAP_CLAMP_TO_EDGE, "WRAP_CLAMP_TO_EDGE");
ENUM_ADD(DAVA::Texture::TextureWrap, DAVA::Texture::WRAP_REPEAT, "WRAP_REPEAT");
*/
