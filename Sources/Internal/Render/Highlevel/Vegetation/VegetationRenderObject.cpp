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

#include <cfloat>

#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Material/NMaterial.h"
#include "Utils/Random.h"
#include "Render/ImageLoader.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/RenderHelper.h"

#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"
#include "Render/Highlevel/Vegetation/VegetationFixedGeometry.h"

namespace DAVA
{

static const uint32 MAX_CLUSTER_TYPES = 4;
static const uint32 MAX_DENSITY_LEVELS = 16;
static const float32 CLUSTER_SCALE_NORMALIZATION_VALUE = 15.0f;
    
static const size_t MAX_RENDER_CELLS = 512;
static const float32 MAX_VISIBLE_CLIPPING_DISTANCE = 70.0f * 70.0f; //meters * meters (square length)
static const float32 MAX_VISIBLE_SCALING_DISTANCE = 50.0f * 50.0f;

//static const float32 MAX_VISIBLE_CLIPPING_DISTANCE = 130.0f * 130.0f; //meters * meters (square length)
//static const float32 MAX_VISIBLE_SCALING_DISTANCE = 100.0f * 100.0f;
    
static const uint32 FULL_BRUSH_VALUE = 0xFFFFFFFF;
    
static Vector3 LOD_RANGES_SCALE = Vector3(0.0f, 2.0f, 6.0f);
//static Vector3 LOD_RANGES_SCALE = Vector3(0.0f, 2.0f, 12.0f);

static float32 RESOLUTION_SCALE[] =
{
    1.0f,
    2.0f,
    4.0f,
};

static uint32 RESOLUTION_INDEX[] =
{
    0,
    1,
    2
};

static uint32 RESOLUTION_CELL_SQUARE[] =
{
    1,
    4,
    16
};

static uint32 RESOLUTION_TILES_PER_ROW[] =
{
    4,
    2,
    1
};

//#define VEGETATION_DRAW_LOD_COLOR

static Color RESOLUTION_COLOR[] =
{
    Color(0.5f, 0.0f, 0.0f, 1.0f),
    Color(0.0f, 0.5f, 0.0f, 1.0f),
    Color(0.0f, 0.0f, 0.5f, 1.0f),
};

#ifdef VEGETATION_DRAW_LOD_COLOR
static const FastName UNIFORM_LOD_COLOR = FastName("lodColor");
static const FastName FLAG_VEGETATION_DRAW_LOD_COLOR = FastName("VEGETATION_DRAW_LOD_COLOR");
#endif

inline uint32 MapCellSquareToResolutionIndex(uint32 cellSquare)
{
    uint32 index = 0;
    uint32 resolutionCount = COUNT_OF(RESOLUTION_CELL_SQUARE);
    for(uint32 i = 0; i < resolutionCount; ++i)
    {
        if(cellSquare == RESOLUTION_CELL_SQUARE[i])
        {
            index = RESOLUTION_INDEX[i];
            break;
        }
    }
    
    return index;
}


    
VegetationRenderObject::VegetationRenderObject() :
    vegetationMap(NULL),
    heightmap(NULL),
    clusterLimit(0),
    halfWidth(0),
    halfHeight(0),
    maxPerturbationDistance(1000000.0f),
    layerVisibilityMask(0xFF),
    vegetationVisible(true),
    vegetationGeometry(NULL),
    heightmapTexture(NULL)
{
    bbox.AddPoint(Vector3(0, 0, 0));
    bbox.AddPoint(Vector3(1, 1, 1));
    
    type = RenderObject::TYPE_VEGETATION;
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
    
    unitWorldSize.resize(COUNT_OF(RESOLUTION_SCALE));
    resolutionRanges.resize(COUNT_OF(RESOLUTION_INDEX));
    
    uint32 maxParams = 4 * 2 * RESOLUTION_CELL_SQUARE[COUNT_OF(RESOLUTION_CELL_SQUARE) - 1];
    shaderScaleDensityUniforms.resize(maxParams);
    
    maxVisibleQuads = MAX_RENDER_CELLS;
    lodRanges = LOD_RANGES_SCALE;
    ResetVisibilityDistance();
}

VegetationRenderObject::~VegetationRenderObject()
{
    if(renderData.size())
    {
        DVASSERT(vegetationGeometry);
        vegetationGeometry->ReleaseRenderData(renderData);
    }
    
    SafeDelete(vegetationGeometry);

    SafeRelease(vegetationMap);
    SafeRelease(heightmap);
    SafeRelease(heightmapTexture);
}
    
RenderObject* VegetationRenderObject::Clone(RenderObject *newObject)
{
    if(!newObject)
    {
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(this), "Can clone only from VegetationRenderObject");
        newObject = new VegetationRenderObject();
    }
    else
    {
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(this), "Can clone only from VegetationRenderObject");
        DVASSERT_MSG(IsPointerToExactClass<VegetationRenderObject>(newObject), "Can clone only to VegetationRenderObject");
    }
    
    VegetationRenderObject* vegetationRenderObject = static_cast<VegetationRenderObject*>(newObject);
    
    vegetationRenderObject->SetHeightmap(GetHeightmap());
    vegetationRenderObject->SetVegetationMap(GetVegetationMap());
    vegetationRenderObject->SetTextureSheet(GetTextureSheetPath());
    vegetationRenderObject->SetClusterLimit(GetClusterLimit());
    vegetationRenderObject->SetVegetationMapPath(GetVegetationMapPath());
    vegetationRenderObject->SetHeightmapPath(GetHeightmapPath());
    vegetationRenderObject->SetLightmap(GetLightmapPath());
    vegetationRenderObject->SetVegetationTexture(GetVegetationTexture());
    vegetationRenderObject->SetWorldSize(GetWorldSize());
    
    vegetationRenderObject->AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    vegetationRenderObject->AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);


    return vegetationRenderObject;
}

void VegetationRenderObject::Save(KeyedArchive *archive, SerializationContext *serializationContext)
{
    //VI: need to remove render batches since they are temporary
    ClearRenderBatches();
    
    RenderObject::Save(archive, serializationContext);
    
    archive->SetUInt32("vro.clusterLimit", GetClusterLimit());

	if(vegetationMapPath.IsEmpty() == false)
	{
		archive->SetString("vro.vegmap", vegetationMapPath.GetRelativePathname(serializationContext->GetScenePath()));
	}

	if(textureSheetPath.IsEmpty() == false)
	{
		archive->SetString("vro.texturesheet", textureSheetPath.GetRelativePathname(serializationContext->GetScenePath()));
	}
    
	if(albedoTexturePath.IsEmpty() == false)
	{
		archive->SetString("vro.vegtexture", albedoTexturePath.GetRelativePathname(serializationContext->GetScenePath()));
	}

	if(lightmapTexturePath.IsEmpty() == false)
	{
		archive->SetString("vro.lightmap", lightmapTexturePath.GetRelativePathname(serializationContext->GetScenePath()));
	}
}
    
void VegetationRenderObject::Load(KeyedArchive *archive, SerializationContext *serializationContext)
{
    RenderObject::Load(archive, serializationContext);
    
    RenderManager::Caps deviceCaps = RenderManager::Instance()->GetCaps();
    
    bool shouldLoadData = deviceCaps.isVertexTextureUnitsSupported;
    
    FastName currentQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(VegetationPropertyNames::VEGETATION_QUALITY_GROUP_NAME);
    bool qualityAllowsVegetation = (VegetationPropertyNames::VEGETATION_QUALITY_NAME_HIGH == currentQuality);
    
    shouldLoadData = shouldLoadData && qualityAllowsVegetation;
    
    RenderManager::Instance()->GetOptions()->SetOption(RenderOptions::VEGETATION_DRAW, shouldLoadData);
    
#if defined(__DAVAENGINE_MACOS__)  || defined(__DAVAENGINE_WIN32__)
    shouldLoadData = true;
#endif
    
    if(shouldLoadData)
    {
        SetClusterLimit(archive->GetUInt32("vro.clusterLimit"));

		String vegmap = archive->GetString("vro.vegmap");
		if(vegmap.empty() == false)
		{
			SetVegetationMap(serializationContext->GetScenePath() + vegmap);
		}

		String texturesheet = archive->GetString("vro.texturesheet");
		if(texturesheet.empty() == false)
		{
			SetTextureSheet(serializationContext->GetScenePath() + texturesheet);
		}

		String vegtexture = archive->GetString("vro.vegtexture");
		if(vegtexture.empty() == false)
		{
			SetVegetationTexture(serializationContext->GetScenePath() + vegtexture);
		}

		String lightmap = archive->GetString("vro.lightmap");
		if(lightmap.empty() == false)
		{
			SetLightmap(serializationContext->GetScenePath() + lightmap);
		}
    }
    
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);
}

void VegetationRenderObject::PrepareToRender(Camera *camera)
{
    bool renderFlag = RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::VEGETATION_DRAW);
    
#if defined(__DAVAENGINE_MACOS__)  || defined(__DAVAENGINE_WIN32__)
//VI: case when vegetation was turned off and then qualit changed from low t high is not a real-world scenario
//VI: real-world scenario is in resource editor when quality has been changed.
    FastName currentQuality = QualitySettingsSystem::Instance()->GetCurMaterialQuality(VegetationPropertyNames::VEGETATION_QUALITY_GROUP_NAME);
    bool qualityAllowsVegetation = (VegetationPropertyNames::VEGETATION_QUALITY_NAME_HIGH == currentQuality);
    
    renderFlag = (renderFlag && qualityAllowsVegetation);
#endif

    visibleCells.clear();
    ClearRenderBatches();
    
    if(!ReadyToRender(renderFlag))
    {
        return;
    }
    
    BuildVisibleCellList(camera->GetPosition(), camera->GetFrustum(), visibleCells);
    
    std::sort(visibleCells.begin(), visibleCells.end(), CellByDistanceCompareFunction);
    
    Vector4 posScale(0.0f,
                     0.0f,
                     worldSize.x,
                     worldSize.y);
    Vector2 switchLodScale;
    
    //Vector3 billboardDirection = -1.0f * camera->GetLeft();
    //billboardDirection.Normalize();
    //vegetationMaterial->SetPropertyValue(UNIFORM_BILLBOARD_DIRECTION,
    //                                     Shader::UT_FLOAT_VEC3,
    //                                     1,
    //                                     billboardDirection.data);
    
    size_t renderDataCount = renderData.size();
    size_t visibleCellCount = visibleCells.size();
    
    for(size_t cellIndex = 0; cellIndex < visibleCellCount; ++cellIndex)
    {
        AbstractQuadTreeNode<SpatialData>* treeNode = visibleCells[cellIndex];
        
        for(size_t layerIndex = 0; layerIndex < renderDataCount; ++layerIndex)
        {
            VegetationRenderData* renderDataObj = renderData[layerIndex];
            Vector<Vector<Vector<SortedBufferItem> > >& indexRenderDataObject = renderDataObj->GetIndexBuffers();
            
            RenderBatch* rb = renderBatchPool.Get(renderDataObj->GetMaterial());
            NMaterial* mat = rb->GetMaterial();
            
            AddRenderBatch(rb);
            
            uint32 resolutionIndex = MapCellSquareToResolutionIndex(treeNode->data.width * treeNode->data.height);
            
            Vector<Vector<SortedBufferItem> >& rdoVector = indexRenderDataObject[resolutionIndex];
            
            uint32 indexBufferIndex = treeNode->data.rdoIndex;
            
            DVASSERT(indexBufferIndex >= 0 && indexBufferIndex < rdoVector.size());
            
            size_t directionIndex = SelectDirectionIndex(camera, rdoVector[indexBufferIndex]);
            rb->SetRenderDataObject(rdoVector[indexBufferIndex][directionIndex].rdo);
            
            SetupNodeUniforms(treeNode, treeNode, treeNode->data.cameraDistance, shaderScaleDensityUniforms);
            
            posScale.x = treeNode->data.bbox.min.x - unitWorldSize[resolutionIndex].x * (indexBufferIndex % RESOLUTION_TILES_PER_ROW[resolutionIndex]);
            posScale.y = treeNode->data.bbox.min.y - unitWorldSize[resolutionIndex].y * (indexBufferIndex / RESOLUTION_TILES_PER_ROW[resolutionIndex]);
            
            switchLodScale.x = resolutionIndex;
            switchLodScale.y = Clamp(1.0f - (treeNode->data.cameraDistance / resolutionRanges[resolutionIndex].y), 0.0f, 1.0f);
            
            
            mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_SWITCH_LOD_SCALE,
                                  Shader::UT_FLOAT_VEC2,
                                  1,
                                  switchLodScale.data);
            
            mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_TILEPOS,
                                  Shader::UT_FLOAT_VEC4,
                                  1,
                                  posScale.data);
            
            mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_CLUSTER_SCALE_DENSITY_MAP,
                                  Shader::UT_FLOAT,
                                  shaderScaleDensityUniforms.size(),
                                  &shaderScaleDensityUniforms[0]);
            
#ifdef VEGETATION_DRAW_LOD_COLOR
            mat->SetPropertyValue(UNIFORM_LOD_COLOR, Shader::UT_FLOAT_VEC3, 1, &RESOLUTION_COLOR[resolutionIndex]);
#endif
        }
    }
}

void VegetationRenderObject::SetVegetationMap(VegetationMap* map)
{
    if(map != vegetationMap)
    {
        SafeRelease(vegetationMap);
        vegetationMap = SafeRetain(map);
        
        UpdateVegetationSetup();
    }
}

void VegetationRenderObject::SetVegetationMap(const FilePath& path)
{
    if(path.Exists())
    {
        Vector<Image*> images;
        ImageLoader::CreateFromFileByExtension(path, images);
            
        DVASSERT(images.size());
            
        if(images.size())
        {
            VegetationMap* vegMap = images[0];
            
            SetVegetationMap(vegMap);
            SetVegetationMapPath(path);
                
            for(size_t i = 0; i < images.size(); ++i)
            {
                SafeRelease(images[i]);
            }
        }
    }
}
    
VegetationMap* VegetationRenderObject::GetVegetationMap() const
{
    return vegetationMap;
}
    
void VegetationRenderObject::SetVegetationMapPath(const FilePath& path)
{
    vegetationMapPath = path;
}

const FilePath& VegetationRenderObject::GetVegetationMapPath() const
{
    return vegetationMapPath;
}

void VegetationRenderObject::SetTextureSheet(const FilePath& path)
{
    textureSheetPath = path;
    
    UpdateVegetationSetup();
}

void VegetationRenderObject::SetVegetationTexture(const FilePath& texturePath)
{
    albedoTexturePath = texturePath;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetString(NMaterial::TEXTURE_ALBEDO.c_str(), albedoTexturePath.GetAbsolutePathname());
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}

const FilePath& VegetationRenderObject::GetVegetationTexture() const
{
    return albedoTexturePath;
}
    
const FilePath& VegetationRenderObject::GetTextureSheetPath() const
{
    return textureSheetPath;
}

void VegetationRenderObject::SetClusterLimit(const uint32& maxClusters)
{
    clusterLimit = maxClusters;
    
    UpdateVegetationSetup();
}

uint32 VegetationRenderObject::GetClusterLimit() const
{
    return clusterLimit;
}

void VegetationRenderObject::SetHeightmap(Heightmap* _heightmap)
{
    if(heightmap != _heightmap)
    {
        SafeRelease(heightmap);
        heightmap = (_heightmap->Data()) ? SafeRetain(_heightmap) : NULL;
        
        if(heightmap)
        {
            InitHeightTextureFromHeightmap(heightmap);
        }
        
        UpdateVegetationSetup();
    }
}

Heightmap* VegetationRenderObject::GetHeightmap() const
{
    return heightmap;
}
    
const FilePath& VegetationRenderObject::GetHeightmapPath() const
{
    return heightmapPath;
}

void VegetationRenderObject::SetHeightmapPath(const FilePath& path)
{
    heightmapPath = path;
}
    
void VegetationRenderObject::SetLightmap(const FilePath& filePath)
{
    lightmapTexturePath = filePath;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetString(VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP.c_str(), lightmapTexturePath.GetAbsolutePathname());
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}
    
const FilePath& VegetationRenderObject::GetLightmapPath() const
{
    return lightmapTexturePath;
}

void VegetationRenderObject::SetWorldSize(const Vector3 size)
{
    worldSize = size;
    
    UpdateVegetationSetup();
}
    
const Vector3& VegetationRenderObject::GetWorldSize() const
{
    return worldSize;
}

Vector2 VegetationRenderObject::GetVegetationUnitWorldSize(float32 resolution) const
{
    DVASSERT(vegetationMap);
    return Vector2((worldSize.x / vegetationMap->width) * resolution,
                   (worldSize.y / vegetationMap->height) * resolution);
}
    
void VegetationRenderObject::BuildSpatialStructure(VegetationMap* vegMap)
{
    DVASSERT(vegMap);
    DVASSERT(heightmap);
    DVASSERT(IsPowerOf2(vegMap->GetWidth()));
    DVASSERT(IsPowerOf2(vegMap->GetHeight()));
    DVASSERT(vegMap->GetWidth() == vegMap->GetHeight());
    
    uint32 mapSize = vegMap->GetWidth();
    uint32 heightmapSize = heightmap->Size();
    
    halfWidth = mapSize / 2;
    halfHeight = mapSize / 2;
    
    heightmapToVegetationMapScale = Vector2((1.0f * heightmapSize) / mapSize,
                                            (1.0f * heightmapSize) / mapSize);
    
    uint32 treeDepth = FastLog2(mapSize);
    
    quadTree.Init(treeDepth);
    AbstractQuadTreeNode<SpatialData>* node = quadTree.GetRoot();
    
    uint32 halfSize = mapSize >> 1;
    BuildSpatialQuad(node, NULL, -1 * halfSize, -1 * halfSize, mapSize, mapSize, node->data.bbox);
}
    
void VegetationRenderObject::BuildSpatialQuad(AbstractQuadTreeNode<SpatialData>* node,
                          AbstractQuadTreeNode<SpatialData>* firstRenderableParent,
                          int16 x, int16 y,
                          uint16 width, uint16 height,
                          AABBox3& parentBox)
{
    DVASSERT(node);
    
    node->data.x = x;
    node->data.y = y;
    
    if(width <= RESOLUTION_SCALE[COUNT_OF(RESOLUTION_SCALE) - 1])
    {
        node->data.width = width;
        node->data.height = height;
        node->data.isVisible = !IsNodeEmpty(node,
                                            MAX_CLUSTER_TYPES,
                                            CLUSTER_SCALE_NORMALIZATION_VALUE,
                                            *vegetationMap);
        
        if(width == RESOLUTION_SCALE[COUNT_OF(RESOLUTION_SCALE) - 1])
        {
            firstRenderableParent = node;
            node->data.rdoIndex = 0;
        }
        else
        {
            int16 offsetX = abs(node->data.x - firstRenderableParent->data.x) / width;
            int16 offsetY = abs(node->data.y - firstRenderableParent->data.y) / height;
            
            node->data.rdoIndex = offsetX + (offsetY * (firstRenderableParent->data.width / width));
        }
    }
    else
    {
        node->data.width = -1;
        node->data.height = -1;
        node->data.rdoIndex = -1;
    }
    
    if(node->IsTerminalLeaf())
    {
        int32 mapX = x + halfWidth;
        int32 mapY = y + halfHeight;

        float32 heightmapHeight = SampleHeight(mapX, mapY);
        node->data.bbox.AddPoint(Vector3(x * unitWorldSize[0].x, y * unitWorldSize[0].y, (heightmapHeight - 0.5f)));
        node->data.bbox.AddPoint(Vector3((x + width) * unitWorldSize[0].x, (y + height) * unitWorldSize[0].y, (heightmapHeight + 0.5f)));
        
        parentBox.AddPoint(node->data.bbox.min);
        parentBox.AddPoint(node->data.bbox.max);
    }
    else
    {
        int16 cellHalfWidth = width >> 1;
        int16 cellHalfHeight = height >> 1;
        
        BuildSpatialQuad(node->children[0], firstRenderableParent, x, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[1], firstRenderableParent, x + cellHalfWidth, y, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[2], firstRenderableParent, x, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        BuildSpatialQuad(node->children[3], firstRenderableParent, x + cellHalfWidth, y + cellHalfHeight, cellHalfWidth, cellHalfHeight, node->data.bbox);
        
        parentBox.AddPoint(node->data.bbox.min);
        parentBox.AddPoint(node->data.bbox.max);
    }
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint,
                                                  Frustum* frustum,
                                                  Vector<AbstractQuadTreeNode<SpatialData>*>& cellList)
{
    uint8 planeMask = 0x3F;
    Vector3 cameraPosXY = cameraPoint;
    cameraPosXY.z = 0.0f;
    BuildVisibleCellList(cameraPosXY, frustum, planeMask, quadTree.GetRoot(), cellList);
}
    
void VegetationRenderObject::BuildVisibleCellList(const Vector3& cameraPoint,
                                                  Frustum* frustum,
                                                  uint8 planeMask,
                                                  AbstractQuadTreeNode<SpatialData>* node,
                                                  Vector<AbstractQuadTreeNode<SpatialData>*>& cellList)
{
    static Vector3 corners[8];
    if(node)
    {
        Frustum::eFrustumResult result = frustum->Classify(node->data.bbox, planeMask, node->data.clippingPlane);
        if(Frustum::EFR_OUTSIDE != result)
        {
            if(node->data.IsRenderable())
            {
                node->data.bbox.GetCorners(corners);
                float32 refDistance = FLT_MAX;
                for(uint32 cornerIndex = 0; cornerIndex < COUNT_OF(corners); ++cornerIndex)
                {
                    corners[cornerIndex].z = 0.0f;
                    float32 cornerDistance = (cameraPoint - corners[cornerIndex]).SquareLength();
                    if(cornerDistance < refDistance)
                    {
                        refDistance = cornerDistance;
                    }
                }
                
                node->data.cameraDistance = refDistance;
                
                uint32 resolutionId = MapToResolution(node->data.cameraDistance);
                if(node->IsTerminalLeaf() ||
                   RESOLUTION_CELL_SQUARE[resolutionId] >= node->data.GetResolutionId())
                {
                    int32 mapX = node->data.x + halfWidth;
                    int32 mapY = node->data.y + halfHeight;
                    uint32 cellDescriptionIndex = (mapY * (halfWidth << 1)) + mapX;

                    uint32 vegetationMapValue = (node->IsTerminalLeaf()) ? (*(((uint32*)vegetationMap->data) + cellDescriptionIndex)) : FULL_BRUSH_VALUE;
                    AddVisibleCell(node, MAX_VISIBLE_CLIPPING_DISTANCE,
                                   vegetationMapValue, cellList);
                }
                else if(!node->IsTerminalLeaf())
                {
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[0], cellList);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[1], cellList);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[2], cellList);
                    BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[3], cellList);
                }
            }
            else
            {
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[0], cellList);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[1], cellList);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[2], cellList);
                BuildVisibleCellList(cameraPoint, frustum, planeMask, node->children[3], cellList);
            }
        }
    }
}
        
bool VegetationRenderObject::CellByDistanceCompareFunction(const AbstractQuadTreeNode<SpatialData>* a,
                                                           const AbstractQuadTreeNode<SpatialData>*  b)
{
    return (a->data.cameraDistance > b->data.cameraDistance);
}
    
void VegetationRenderObject::InitHeightTextureFromHeightmap(Heightmap* heightMap)
{
    SafeRelease(heightmapTexture);

    Image* originalImage = Image::CreateFromData(heightMap->Size(),
                                                 heightMap->Size(),
                                                 FORMAT_A16,
                                                 (uint8*)heightMap->Data());
    
    int32 pow2Size = heightmap->Size();
    if(!IsPowerOf2(heightmap->Size()))
    {
        EnsurePowerOf2(pow2Size);
        
        if(pow2Size > heightmap->Size())
        {
            pow2Size = pow2Size >> 1;
        }
    }
    
    Texture* tx = NULL;
    if(pow2Size != heightmap->Size())
    {
        Image* croppedImage = Image::CopyImageRegion(originalImage, pow2Size, pow2Size);
        tx = Texture::CreateFromData(FORMAT_RGBA4444, croppedImage->GetData(), pow2Size, pow2Size, false);
     
        SafeRelease(croppedImage);
    }
    else
    {
        tx = Texture::CreateFromData(FORMAT_RGBA4444, originalImage->GetData(), pow2Size, pow2Size, false);
    }
    
    SafeRelease(originalImage);
    
    heightmapScale = Vector2((1.0f * heightmap->Size()) / pow2Size,
                             (1.0f * heightmap->Size()) / pow2Size);
    
    ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN, Message(this, &VegetationRenderObject::SetupHeightmapParameters, tx));
	JobInstanceWaiter waiter(job);
	waiter.Wait();
    
    heightmapTexture = SafeRetain(tx);
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetUInt64(NMaterial::TEXTURE_HEIGHTMAP.c_str(), (uint64)heightmapTexture);
        props->SetVector2(VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE.c_str(), heightmapScale);
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
    
    SafeRelease(tx);
}
    
float32 VegetationRenderObject::SampleHeight(int16 x, int16 y)
{
    uint32 hX = heightmapToVegetationMapScale.x * x;
    uint32 hY = heightmapToVegetationMapScale.y * y;
    
    uint16 left = (hX > 0) ? *(heightmap->Data() + ((hY * heightmap->Size()) + hX - 1)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 right = (hX < halfWidth) ? *(heightmap->Data() + ((hY * heightmap->Size()) + hX + 1)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 top = (hY > 0) ? *(heightmap->Data() + (((hY - 1) * heightmap->Size()) + hX)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 down = (hY < halfHeight) ? *(heightmap->Data() + (((hY + 1) * heightmap->Size()) + hX)) : *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    uint16 center = *(heightmap->Data() + ((hY * heightmap->Size()) + hX));
    
    uint16 heightmapValue = (left + right + top + down + center) / 5;
    
    float32 height = ((float32)heightmapValue / (float32)Heightmap::MAX_VALUE) * worldSize.z;
    
    return height;
}

bool VegetationRenderObject::IsValidGeometryData() const
{
     return (worldSize.Length() > 0 &&
             heightmap != NULL &&
             vegetationMap != NULL &&
             !textureSheetPath.IsEmpty() &&
             clusterLimit > 0);
}
    
bool VegetationRenderObject::IsValidSpatialData() const
{
    return (worldSize.Length() > 0 &&
            heightmap != NULL &&
            vegetationMap != NULL);
}

void VegetationRenderObject::UpdateVegetationSetup()
{
    if(vegetationMap)
    {
        for(size_t i = 0; i < COUNT_OF(RESOLUTION_SCALE); ++i)
        {
            unitWorldSize[i] = GetVegetationUnitWorldSize(RESOLUTION_SCALE[i]);
        }
    }
    
    if(IsValidGeometryData())
    {
        CreateRenderData(clusterLimit);
    }
    
    if(IsValidSpatialData())
    {
        BuildSpatialStructure(vegetationMap);
    }
}

void VegetationRenderObject::SetVisibilityDistance(const Vector2& distances)
{
    visibleClippingDistances = distances;
}
    
const Vector2& VegetationRenderObject::GetVisibilityDistance() const
{
    return visibleClippingDistances;
}
    
void VegetationRenderObject::ResetVisibilityDistance()
{
    visibleClippingDistances.x = MAX_VISIBLE_CLIPPING_DISTANCE;
    visibleClippingDistances.y = MAX_VISIBLE_SCALING_DISTANCE;
}
    
void VegetationRenderObject::SetLodRange(const Vector3& distances)
{
    lodRanges = distances;
    
    if(IsValidSpatialData())
    {
        InitLodRanges();
    }
}
    
const Vector3& VegetationRenderObject::GetLodRange() const
{
   return lodRanges;
}
    
void VegetationRenderObject::ResetLodRanges()
{
   lodRanges = LOD_RANGES_SCALE;
 
    if(IsValidSpatialData())
    {
        InitLodRanges();
    }
}

void VegetationRenderObject::InitLodRanges()
{
    Vector2 smallestUnitSize = GetVegetationUnitWorldSize(RESOLUTION_SCALE[0]);
    
    resolutionRanges[0].x = lodRanges.x * smallestUnitSize.x;
    resolutionRanges[0].y = lodRanges.y * smallestUnitSize.x;

    resolutionRanges[1].x = lodRanges.y * smallestUnitSize.x;
    resolutionRanges[1].y = lodRanges.z * smallestUnitSize.x;

    resolutionRanges[2].x = lodRanges.z * smallestUnitSize.x;
    resolutionRanges[2].y = MAX_VISIBLE_CLIPPING_DISTANCE;//RESOLUTION_RANGES[2].x * 1000.0f;

    size_t resolutionCount = resolutionRanges.size();
    for(size_t i = 0; i < resolutionCount; ++i)
    {
        resolutionRanges[i].x *= resolutionRanges[i].x;
        resolutionRanges[i].y *= resolutionRanges[i].y;
    }
}

void VegetationRenderObject::SetMaxVisibleQuads(const uint32& _maxVisibleQuads)
{
    maxVisibleQuads = _maxVisibleQuads;
}

const uint32& VegetationRenderObject::GetMaxVisibleQuads() const
{
    return maxVisibleQuads;
}

void VegetationRenderObject::GetDataNodes(Set<DataNode*> & dataNodes)
{
    size_t layerCount = renderData.size();
    for(size_t i = 0; i < layerCount; ++i)
    {
        VegetationRenderData* renderDataObj = renderData[i];
        dataNodes.insert(renderDataObj->GetMaterial());
    }
}

void VegetationRenderObject::SetupHeightmapParameters(BaseObject * caller,
                                                    void * param,
                                                    void *callerData)
{
    Texture* tx = (Texture*)param;
    tx->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
    tx->SetMinMagFilter(Texture::FILTER_NEAREST, Texture::FILTER_NEAREST);
}

void VegetationRenderObject::CreateRenderData(uint32 maxClusters)
{
    DVASSERT(maxClusters > 0);
    
    InitLodRanges();
    
    if(renderData.size() > 0)
    {
        DVASSERT(vegetationGeometry);
        vegetationGeometry->ReleaseRenderData(renderData);
    }
    
    SafeDelete(vegetationGeometry);
    
    vegetationGeometry = new VegetationFixedGeometry(maxClusters,
                                                     MAX_DENSITY_LEVELS,
                                                     MAX_CLUSTER_TYPES,
                                                     GetVegetationUnitWorldSize(RESOLUTION_SCALE[0]),
                                                     textureSheetPath,
                                                     RESOLUTION_CELL_SQUARE,
                                                     COUNT_OF(RESOLUTION_CELL_SQUARE),
                                                     RESOLUTION_SCALE,
                                                     COUNT_OF(RESOLUTION_SCALE),
                                                     resolutionRanges,
                                                     RESOLUTION_TILES_PER_ROW,
                                                     COUNT_OF(RESOLUTION_TILES_PER_ROW),
                                                     worldSize);
    
    FastNameSet materialFlags;
    if(RenderManager::Instance()->GetCaps().isFramebufferFetchSupported)
    {
        materialFlags.Insert(VegetationPropertyNames::FLAG_FRAMEBUFFER_FETCH);
    }
    
    materialFlags.Insert(VegetationPropertyNames::FLAG_BILLBOARD_DRAW);
    
#ifdef VEGETATION_DRAW_LOD_COLOR
    
    materialFlags.Insert(VegetationPropertyNames::FLAG_VEGETATION_DRAW_LOD_COLOR);
    
#endif

    vegetationGeometry->Build(renderData, materialFlags);
    
    KeyedArchive* props = new KeyedArchive();
    props->SetUInt64(NMaterial::TEXTURE_HEIGHTMAP.c_str(), (uint64)heightmapTexture);
    props->SetVector2(VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE.c_str(), heightmapScale);
    props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE.c_str(), perturbationForce);
    props->SetFloat(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE.c_str(), maxPerturbationDistance);
    props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_POINT.c_str(), perturbationPoint);
    props->SetString(NMaterial::TEXTURE_ALBEDO.c_str(), albedoTexturePath.GetAbsolutePathname());
    props->SetString(VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP.c_str(), lightmapTexturePath.GetAbsolutePathname());
    
    vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
    
    SafeRelease(props);

    renderBatchPool.Clear();
    size_t renderDataCount = renderData.size();
    for(size_t i = 0; i < renderDataCount; ++i)
    {
        renderBatchPool.Init(renderData[i]->GetMaterial(), 16);
    }
}
    

bool VegetationRenderObject::ReadyToRender(bool externalRenderFlag)
{
    return vegetationVisible && (RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::VEGETATION_DRAW)) && (renderData.size() > 0);
}

void VegetationRenderObject::SetupNodeUniforms(AbstractQuadTreeNode<SpatialData>* sourceNode,
                                               AbstractQuadTreeNode<SpatialData>* node,
                                               float32 cameraDistance,
                                               Vector<float32>& uniforms)
{
    if(node->IsTerminalLeaf())
    {
        DVASSERT(node->data.rdoIndex >= 0 && node->data.rdoIndex < uniforms.size());
        
        //int32 mapX = node->data.x + halfWidth;
        //int32 mapY = node->data.y + halfHeight;
        //uint32 cellDescriptionIndex = (mapY * (halfWidth << 1)) + mapX;
        
        float32 distanceScale = 1.0f;
        
        if(cameraDistance > MAX_VISIBLE_SCALING_DISTANCE)
        {
            distanceScale = Clamp(1.0f - ((cameraDistance - MAX_VISIBLE_SCALING_DISTANCE) / (MAX_VISIBLE_CLIPPING_DISTANCE - MAX_VISIBLE_SCALING_DISTANCE)), 0.0f, 1.0f);
        }
        
        uint8 *vegetationMapValuePtr = GetCellValue(node->data.x, node->data.y, *vegetationMap);//(vegetationMap->data + cellDescriptionIndex * 4);
        
        for(uint32 clusterType = 0; clusterType < MAX_CLUSTER_TYPES; ++clusterType)
        {
            uint8 cellLayerData = vegetationMapValuePtr[clusterType];
            
            float32 clusterScale = 0.0f;
            float32 density = 0.0f;
            GetLayerDescription(cellLayerData, CLUSTER_SCALE_NORMALIZATION_VALUE, density, clusterScale);
            
            uniforms[node->data.rdoIndex * 2 * 4 + clusterType] = density;
            uniforms[node->data.rdoIndex * 2 * 4 + 4 + clusterType] = clusterScale;
        }

    }
    else
    {
        SetupNodeUniforms(sourceNode, node->children[0], cameraDistance, uniforms);
        SetupNodeUniforms(sourceNode, node->children[1], cameraDistance, uniforms);
        SetupNodeUniforms(sourceNode, node->children[2], cameraDistance, uniforms);
        SetupNodeUniforms(sourceNode, node->children[3], cameraDistance, uniforms);
    }
}

void VegetationRenderObject::SetPerturbation(const Vector3& point,
                                            const Vector3& force,
                                            float32 distance)
{
    perturbationForce = force;
    maxPerturbationDistance = distance;
    perturbationPoint = point;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE.c_str(), perturbationForce);
        props->SetFloat(VegetationPropertyNames::UNIFORM_PERTURBATION_FORCE_DISTANCE.c_str(), maxPerturbationDistance);
        props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_POINT.c_str(), perturbationPoint);
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}

float32 VegetationRenderObject::GetPerturbationDistance() const
{
   return maxPerturbationDistance;
}

const Vector3& VegetationRenderObject::GetPerturbationForce() const
{
    return perturbationForce;
}

const Vector3& VegetationRenderObject::GetPerturbationPoint() const
{
    return perturbationPoint;
}

void VegetationRenderObject::SetPerturbationPoint(const Vector3& point)
{
    perturbationPoint = point;
    
    if(vegetationGeometry != NULL)
    {
        KeyedArchive* props = new KeyedArchive();
        props->SetVector3(VegetationPropertyNames::UNIFORM_PERTURBATION_POINT.c_str(), perturbationPoint);
        
        vegetationGeometry->OnVegetationPropertiesChanged(renderData, props);
        
        SafeRelease(props);
    }
}

void VegetationRenderObject::SetLayerVisibilityMask(const uint8& mask)
{
    layerVisibilityMask = mask;
}
    
const uint8& VegetationRenderObject::GetLayerVisibilityMask() const
{
     return layerVisibilityMask;
}

void VegetationRenderObject::SetVegetationVisible(bool show)
{
    vegetationVisible = show;
}
    
bool VegetationRenderObject::GetVegetationVisible() const
{
    return vegetationVisible;
}

size_t VegetationRenderObject::SelectDirectionIndex(Camera* cam, Vector<SortedBufferItem>& buffers)
{
    Vector3 cameraDirection = cam->GetDirection();
    cameraDirection.Normalize();
    
    size_t index = 0;
    float32 currentCosA = 0.0f;
    size_t directionCount = buffers.size();
    for(size_t i = 0; i < directionCount; ++i)
    {
        SortedBufferItem& item = buffers[i];
        //cos (angle) = dotAB / (length A * lengthB)
        //no need to calculate (length A * lengthB) since vectors are normalized
        
        if(item.sortDirection == cameraDirection)
        {
            index = i;
            break;
        }
        else
        {
            float32 cosA = cameraDirection.DotProduct(item.sortDirection);
            if(cosA > currentCosA)
            {
                index = i;
                currentCosA = cosA;
            }
        }
    }
    
    return index;
}

void VegetationRenderObject::DebugDrawVisibleNodes()
{
    uint32 requestedBatchCount = Min(visibleCells.size(), (size_t)maxVisibleQuads);
    for(uint32 i = 0; i < requestedBatchCount; ++i)
    {
        AbstractQuadTreeNode<SpatialData>* treeNode = visibleCells[i];
        uint32 resolutionIndex = MapCellSquareToResolutionIndex(treeNode->data.width * treeNode->data.height);
        
        RenderManager::Instance()->SetColor(RESOLUTION_COLOR[resolutionIndex]);
        RenderHelper::Instance()->DrawBox(treeNode->data.bbox, 1.0f, RenderState::RENDERSTATE_3D_OPAQUE);
    }
}

void VegetationRenderObject::ClearRenderBatches()
{
    int32 batchesToRemove = GetRenderBatchCount();
    while(batchesToRemove > 0)
    {
        RemoveRenderBatch(batchesToRemove - 1);
        batchesToRemove = GetRenderBatchCount();
    }
    
    renderBatchPool.ReturnAll();
}

};