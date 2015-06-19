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


#include "NotPassableTerrainProxy.h"

NotPassableTerrainProxy::NotPassableTerrainProxy(int32 heightmapSize)
:	enabled(false)
{
	LoadColorsArray();
	 
	notPassableAngleTan = (float32)tan(DegToRad((float32)NOT_PASSABLE_ANGLE));
	notPassableTexture = Texture::CreateFBO(2048, 2048, DAVA::FORMAT_RGBA8888);
    
    uint32 bufferSize = heightmapSize * heightmapSize * 4 * sizeof(float32) * 4;
    gridBufferHandle = rhi::CreateVertexBuffer(bufferSize);
}

NotPassableTerrainProxy::~NotPassableTerrainProxy()
{
	SafeRelease(notPassableTexture);
    rhi::DeleteVertexBuffer(gridBufferHandle);
}

void NotPassableTerrainProxy::LoadColorsArray()
{
	YamlParser* parser = YamlParser::Create("~res:/Configs/LandscapeAngle.yaml");
	
	if (parser != 0)
	{
		YamlNode* rootNode = parser->GetRootNode();
		int32 anglesCount = rootNode->GetCount();
		
        angleColor.reserve(anglesCount);
		for (int32 i = 0; i < anglesCount; ++i)
		{
			const YamlNode* node = rootNode->Get(i);
			if (!node || node->GetCount() != 3)
			{
				continue;
			}
			
			float32 angle1 = node->Get(0)->AsFloat();
			float32 angle2 = node->Get(1)->AsFloat();
			
			angle1 = Min(angle1, 89.f);
			angle2 = Min(angle2, 89.f);
			
			float32 tangentMin = tan(DegToRad(angle1));
			float32 tangentMax = tan(DegToRad(angle2));
			
			const YamlNode* colorNode = node->Get(2);
			if (!colorNode || colorNode->GetCount() != 4)
			{
				continue;
			}
			
			Color color(colorNode->Get(0)->AsFloat()/255.f,
						colorNode->Get(1)->AsFloat()/255.f,
						colorNode->Get(2)->AsFloat()/255.f,
						colorNode->Get(3)->AsFloat()/255.f);
			
			angleColor.push_back(TerrainColor(Vector2(tangentMin, tangentMax), color));
		}
	}
	
	SafeRelease(parser);
}

bool NotPassableTerrainProxy::PickColor(float32 tan, Color& color) const
{
	for (uint32 i = 0; i < angleColor.size(); ++i)
	{
		if(tan >= angleColor[i].angleRange.x && tan < angleColor[i].angleRange.y)
		{
			color = angleColor[i].color;
			return true;
		}
	}
	return false;
}

bool NotPassableTerrainProxy::Enable()
{
	if (enabled)
	{
		return true;
	}
	
	enabled = true;
	
	return true;
}

bool NotPassableTerrainProxy::Disable()
{
	if (!enabled)
	{
		return true;
	}
	
	enabled = false;
	
	return true;
}

bool NotPassableTerrainProxy::IsEnabled() const
{
	return enabled;
}

Texture* NotPassableTerrainProxy::GetTexture()
{
	return notPassableTexture;
}

void NotPassableTerrainProxy::UpdateTexture(DAVA::Heightmap *heightmap, const AABBox3& landscapeBoundingBox, const DAVA::Rect &forRect)
{
	const Vector3 landSize = landscapeBoundingBox.max - landscapeBoundingBox.min;
	
	const float32 angleCellDistance = landSize.x / (float32)(heightmap->Size() - 1);
	const float32 angleHeightDelta = landSize.z / (float32)(Heightmap::MAX_VALUE - 1);
	const float32 tanCoef = angleHeightDelta / angleCellDistance;
	
    const float32 targetWidth = (float32)notPassableTexture->GetWidth();
    const float32 dx = targetWidth / (float32)(heightmap->Size() - 1);
    
    uint32 bufferSize = heightmap->Size() * heightmap->Size() * 4 * 4;
    float32 * buffer = new float32[bufferSize];
    float32 * bufferPtr = buffer;
    
    for (int32 y = 0; y < heightmap->Size(); ++y)
    {
        const int32 yOffset = y * heightmap->Size();
        const float32 ydx = (heightmap->Size() - y - 1) * dx;
        
        for (int32 x = 0; x < heightmap->Size(); ++x)
        {
            const uint16 currentPoint = heightmap->Data()[yOffset + x];
            const uint16 rightPoint = heightmap->Data()[yOffset + x + 1];
            const uint16 bottomPoint = heightmap->Data()[yOffset + x + heightmap->Size()];
            
            const uint16 deltaRight = (uint16)abs((int32)currentPoint - (int32)rightPoint);
            const uint16 deltaBottom = (uint16)abs((int32)currentPoint - (int32)bottomPoint);
            
            const float32 tanRight = (float32)deltaRight * tanCoef;
            const float32 tanBottom = (float32)deltaBottom * tanCoef;
            
            const float32 xdx = x * dx;
            
            Color color(0.f, 0.f, 0.f, 0.f);
            
            PickColor(tanRight, color);
            
            {
                *((Vector3 *)bufferPtr) = Vector3(xdx, ydx, 0.f); bufferPtr +=3;
                *(uint32*)(bufferPtr) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a); ++bufferPtr;
                *((Vector3 *)bufferPtr) = Vector3((xdx + dx), ydx, 0.f); bufferPtr +=3;
                *(uint32*)(bufferPtr) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a); ++bufferPtr;
            }
            
            PickColor(tanBottom, color);
            
            {
                *((Vector3 *)bufferPtr) = Vector3(xdx, ydx, 0.f); bufferPtr +=3;
                *(uint32*)(bufferPtr) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a); ++bufferPtr;
                *((Vector3 *)bufferPtr) = Vector3(xdx, (ydx - dx), 0.f); bufferPtr +=3;
                *(uint32*)(bufferPtr) = rhi::NativeColorRGBA(color.r, color.g, color.b, color.a); ++bufferPtr;
            }
        }
    }
    
    rhi::UpdateVertexBuffer(gridBufferHandle, buffer, 0, bufferSize * sizeof(float32));
    SafeDeleteArray(buffer);
    
    Matrix4 projMatrix;
    projMatrix.glOrtho(0.0f, (float32)notPassableTexture->GetWidth(), 0.0f, (float32)notPassableTexture->GetHeight(), -1.0f, 1.0f, false);
    
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEW, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJ, &projMatrix, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    
    rhi::Packet gridPacket;
    RenderSystem2D::DEFAULT_2D_COLOR_MATERIAL->BindParams(gridPacket);

    rhi::VertexLayout layout;
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
    
    gridPacket.vertexStreamCount = 1;
    gridPacket.vertexStream[0] = gridBufferHandle;
    gridPacket.primitiveType = rhi::PRIMITIVE_LINELIST;
    gridPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(layout);
    gridPacket.primitiveCount = heightmap->Size() * heightmap->Size() * 2;
    gridPacket.vertexCount = gridPacket.primitiveCount * 2;
    
    SafeDeleteArray(buffer);
    
    rhi::RenderPassConfig passConfig;
    passConfig.colorBuffer[0].texture = notPassableTexture->handle;
    passConfig.colorBuffer[0].clearColor[0] = 0.f;
    passConfig.colorBuffer[0].clearColor[1] = 0.f;
    passConfig.colorBuffer[0].clearColor[2] = 0.f;
    passConfig.colorBuffer[0].clearColor[3] = 0.f;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    passConfig.priority = PRIORITY_SERVICE_2D;
    passConfig.viewport.width = notPassableTexture->GetWidth();
    passConfig.viewport.height = notPassableTexture->GetHeight();
    
    rhi::HPacketList packetListHandle;
    rhi::HRenderPass passTargetHandle = rhi::AllocateRenderPass(passConfig, 1, &packetListHandle);
    
    rhi::BeginRenderPass(passTargetHandle);
    rhi::BeginPacketList(packetListHandle);
    
    rhi::AddPacket(packetListHandle, gridPacket);
    
    rhi::EndPacketList(packetListHandle);
    rhi::EndRenderPass(passTargetHandle);
}
