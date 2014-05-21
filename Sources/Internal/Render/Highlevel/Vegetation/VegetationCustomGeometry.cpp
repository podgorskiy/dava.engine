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

#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/Highlevel/Vegetation/TextureSheet.h"
#include "Render/Highlevel/Vegetation/VegetationCustomGeometry.h"
#include "Utils/Random.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/Highlevel/Vegetation/VegetationPropertyNames.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{

static const FastName VEGETATION_ENTITY_VARIATION_0 = FastName("variation_0");

static const FastName VEGETATION_ENTITY_LAYER_0 = FastName("layer_0");
static const FastName VEGETATION_ENTITY_LAYER_1 = FastName("layer_1");
static const FastName VEGETATION_ENTITY_LAYER_2 = FastName("layer_2");
static const FastName VEGETATION_ENTITY_LAYER_3 = FastName("layer_3");

static const float32 MAX_ROTATION_ANGLE = 180.0f;

static FastName VEGETATION_ENTITY_LAYER_NAMES[] =
{
    VEGETATION_ENTITY_LAYER_0,
    VEGETATION_ENTITY_LAYER_1,
    VEGETATION_ENTITY_LAYER_2,
    VEGETATION_ENTITY_LAYER_3
};

void VegetationCustomGeometry::CustomMaterialTransformer::TransformMaterialOnCreate(NMaterial* mat)
{
    mat->AddNodeFlags(DataNode::NodeRuntimeFlag);
}

VegetationCustomGeometry::CustomGeometryLayerData::CustomGeometryLayerData()
{
        
}
    
VegetationCustomGeometry::CustomGeometryLayerData::CustomGeometryLayerData(const CustomGeometryLayerData& src)
{
    for(size_t i = 0; i < src.sourcePositions.size(); ++i)
    {
        sourcePositions.push_back(src.sourcePositions[i]);
    }

    for(size_t i = 0; i < src.sourceTextureCoords.size(); ++i)
    {
        sourceTextureCoords.push_back(src.sourceTextureCoords[i]);
    }

    for(size_t i = 0; i < src.sourceNormals.size(); ++i)
    {
        sourceNormals.push_back(src.sourceNormals[i]);
    }

    for(size_t i = 0; i < src.sourceIndices.size(); ++i)
    {
        sourceIndices.push_back(src.sourceIndices[i]);
    }
}

VegetationCustomGeometry::CustomGeometryEntityData::CustomGeometryEntityData() : material(NULL)
{
}

VegetationCustomGeometry::CustomGeometryEntityData::CustomGeometryEntityData(const CustomGeometryEntityData& src)
{
    material = NULL;
    SetMaterial(src.material);
    
    for(size_t i = 0; i < src.lods.size(); ++i)
    {
        lods.push_back(src.lods[i]);
    }
}
    
VegetationCustomGeometry::CustomGeometryEntityData::~CustomGeometryEntityData()
{
    SafeRelease(material);
}
    
void VegetationCustomGeometry::CustomGeometryEntityData::SetMaterial(NMaterial* mat)
{
    if(mat != material)
    {
        SafeRelease(material);
        material = SafeRetain(mat);
    }
}

int32 VegetationCustomGeometry::RandomShuffleFunc(int32 limit)
{
    return (Random::Instance()->Rand() % limit);
}

bool VegetationCustomGeometry::PolygonByDistanceCompareFunction(const PolygonSortData& a,
                                                                const PolygonSortData& b)
{
    return a.cameraDistance > b.cameraDistance; //back to front order
}

bool VegetationCustomGeometry::ClusterByMatrixCompareFunction(const ClusterResolutionData& a,
                                                              const ClusterResolutionData& b)
{
    return a.cellIndex < b.cellIndex;
}

VegetationCustomGeometry::VegetationCustomGeometry(const Vector<uint32>& _maxClusters,
                                                   uint32 _maxDensityLevels,
                                                   const Vector2& _unitSize,
                                                   const FilePath& _dataPath,
                                                   const uint32* _resolutionCellSquare,
                                                   uint32 _resolutionCellSquareCount,
                                                   const float32* _resolutionScale,
                                                   uint32 _resolutionScaleCount,
                                                   const uint32* _resolutionTilesPerRow,
                                                   uint32 _resolutionTilesPerRowCount,
                                                   const uint32* _resolutionClusterStride,
                                                   uint32 _resolutionClusterStrideCount,
                                                   const Vector3& _worldSize)
{
    maxClusters.reserve(_maxClusters.size());
    for(size_t i = 0; i < _maxClusters.size(); ++i)
    {
        maxClusters.push_back(_maxClusters[i]);
    }
    
    maxDensityLevels = _maxDensityLevels;
    unitSize = _unitSize;
    sourceDataPath = _dataPath;
    
    resolutionCellSquare.reserve(_resolutionCellSquareCount);
    for(uint32 i = 0; i < _resolutionCellSquareCount; ++i)
    {
        resolutionCellSquare.push_back(_resolutionCellSquare[i]);
    }
    
    resolutionScale.reserve(_resolutionScaleCount);
    for(uint32 i = 0; i < _resolutionScaleCount; ++i)
    {
        resolutionScale.push_back(_resolutionScale[i]);
    }
    
    resolutionTilesPerRow.reserve(_resolutionTilesPerRowCount);
    for(uint32 i = 0; i < _resolutionTilesPerRowCount; ++i)
    {
        resolutionTilesPerRow.push_back(_resolutionTilesPerRow[i]);
    }
    
    resolutionClusterStride.reserve(_resolutionClusterStrideCount);
    for(uint32 i = 0; i < _resolutionClusterStrideCount; ++i)
    {
        resolutionClusterStride.push_back(_resolutionClusterStride[i]);
    }
    
    worldSize = _worldSize;
    resolutionCount = resolutionClusterStride.size();
    
    materialTransform = new CustomMaterialTransformer();
}
    
VegetationCustomGeometry::~VegetationCustomGeometry()
{
    markedRenderData.clear();
}

void VegetationCustomGeometry::ReleaseRenderData(Vector<VegetationRenderData*>& renderDataArray)
{
    markedRenderData.clear(); //marked render data only stores references
        
    VegetationGeometry::ReleaseRenderData(renderDataArray);
}

    
void VegetationCustomGeometry::Build(Vector<VegetationRenderData*>& renderDataArray, const FastNameSet& materialFlags)
{
    Vector<CustomGeometryEntityData> customGeometryData;
    LoadCustomData(customGeometryData);
    
    markedRenderData.resize(customGeometryData.size());
    renderDataArray.resize(customGeometryData.size());
    
    size_t layerCount = customGeometryData.size();
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        CustomGeometryEntityData& layerGeometryData = customGeometryData[layerIndex];
        
        VegetationRenderData* renderData = new VegetationRenderData();
        renderData->SetMaterial(layerGeometryData.material);
        
        layerGeometryData.material->SetFlag(VegetationPropertyNames::FLAG_GRASS_OPAQUE, NMaterial::FlagOn);
        
        FastNameSet::iterator end = materialFlags.end();
        for(FastNameSet::iterator it = materialFlags.begin(); it != end; ++it)
        {
            layerGeometryData.material->SetFlag(it->first, NMaterial::FlagOn);
        }

        layerGeometryData.material->SetPropertyValue(VegetationPropertyNames::UNIFORM_WORLD_SIZE,
                                             Shader::UT_FLOAT_VEC3,
                                             1,
                                             &worldSize);
        
        renderDataArray[layerIndex] = renderData;
        markedRenderData[layerIndex].renderData = renderData;
        
        Vector<VegetationVertex>& vertexData = renderData->GetVertices();
        Vector<VegetationIndex>& indexData = renderData->GetIndices();
        
        BuildLayer(layerIndex,
                   layerGeometryData,
                   vertexData,
                   indexData,
                   markedRenderData[layerIndex].vertexOffset,
                   markedRenderData[layerIndex].indexOffset);
    }
    
    for(size_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        VegetationRenderData* renderData = renderDataArray[layerIndex];
        Vector<VegetationVertex>& vertexData = renderData->GetVertices();
        Vector<VegetationIndex>& indexData = renderData->GetIndices();
        
        Vector<Vector<Vector<SortedBufferItem> > >& indexBuffers = renderData->GetIndexBuffers();
        
        for(size_t resolutionIndex = 0; resolutionIndex < resolutionCount; ++resolutionIndex)
        {
            Vector<VertexRangeData>& vertexBuffers = markedRenderData[layerIndex].vertexOffset[resolutionIndex];
            Vector<Vector<SortBufferData> >& sortedIndexBuffers = markedRenderData[layerIndex].indexOffset[resolutionIndex];
            
            indexBuffers.push_back(Vector<Vector<SortedBufferItem> >());
            Vector<Vector<SortedBufferItem> >& currentResolutionIndexBuffers = indexBuffers[indexBuffers.size() - 1];
            
            size_t cellCount = sortedIndexBuffers.size();
            for(size_t cellIndex = 0; cellIndex < cellCount; ++cellIndex)
            {
                VertexRangeData& rangeData = vertexBuffers[cellIndex];
                
                RenderDataObject* vertexRDO = new RenderDataObject();
                vertexRDO->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[rangeData.index].coord));
                vertexRDO->SetStream(EVF_NORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[rangeData.index].normal));
                vertexRDO->SetStream(EVF_BINORMAL, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[rangeData.index].binormal));
                vertexRDO->SetStream(EVF_TANGENT, TYPE_FLOAT, 3, sizeof(VegetationVertex), &(vertexData[rangeData.index].tangent));
                vertexRDO->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[rangeData.index].texCoord0));
                vertexRDO->SetStream(EVF_TEXCOORD1, TYPE_FLOAT, 2, sizeof(VegetationVertex), &(vertexData[rangeData.index].texCoord1));
                vertexRDO->BuildVertexBuffer(rangeData.size, true);
            
                currentResolutionIndexBuffers.push_back(Vector<SortedBufferItem>());
                Vector<SortedBufferItem>& sortedIndexBufferItems = currentResolutionIndexBuffers[currentResolutionIndexBuffers.size() - 1];
                
                Vector<SortBufferData>& directionIndexBuffers = sortedIndexBuffers[cellIndex];
                
                size_t directionCount = directionIndexBuffers.size();
                for(size_t directionIndex = 0; directionIndex < directionCount; ++directionIndex)
                {
                    SortBufferData& sortData = directionIndexBuffers[directionIndex];
                
                    sortedIndexBufferItems.push_back(SortedBufferItem());
                    SortedBufferItem& sortBufferItem = sortedIndexBufferItems[directionIndex];
                    
                    RenderDataObject* indexBuffer = new RenderDataObject();
                    indexBuffer->SetIndices(VEGETATION_INDEX_TYPE, (uint8*)(&indexData[sortData.indexOffset]), sortData.size);
                    
                    sortBufferItem.SetRenderDataObject(indexBuffer);
                    sortBufferItem.SetRenderDataObjectAttachment(vertexRDO);
                    sortBufferItem.sortDirection = sortData.sortDirection;
                    
                    sortBufferItem.rdo->BuildIndexBuffer(true);
                    sortBufferItem.rdo->AttachVertices(vertexRDO);
                    
                    SafeRelease(indexBuffer);
                }
                
                SafeRelease(vertexRDO);
            }
        }
    }
}
    
void VegetationCustomGeometry::OnVegetationPropertiesChanged(Vector<VegetationRenderData*>& renderDataArray, KeyedArchive* props)
{
    size_t markedArraySize = markedRenderData.size();
    for(size_t renderDataIndex = 0; renderDataIndex < markedArraySize; ++renderDataIndex)
    {
        NMaterial* mat = markedRenderData[renderDataIndex].renderData->GetMaterial();
        
        if(mat)
        {
            String lightmapKeyName = VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP.c_str();
            if(props->IsKeyExists(lightmapKeyName))
            {
                FilePath lightmapPath = props->GetString(lightmapKeyName);
                mat->SetTexture(VegetationPropertyNames::UNIFORM_SAMPLER_VEGETATIONMAP, lightmapPath);
            }
            
            String heightmapKeyName = NMaterial::TEXTURE_HEIGHTMAP.c_str();
            if(props->IsKeyExists(heightmapKeyName))
            {
                Texture* heightmap = (Texture*)props->GetUInt64(heightmapKeyName);
                mat->SetTexture(NMaterial::TEXTURE_HEIGHTMAP, heightmap);
            }
            
            String heightmapScaleKeyName = VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE.c_str();
            if(props->IsKeyExists(heightmapScaleKeyName))
            {
                Vector2 heightmapScale = props->GetVector2(heightmapScaleKeyName);
                mat->SetPropertyValue(VegetationPropertyNames::UNIFORM_HEIGHTMAP_SCALE,
                                      Shader::UT_FLOAT_VEC2,
                                      1,
                                      heightmapScale.data);
            }
        }
    }
}

void VegetationCustomGeometry::GenerateClusterPositionData(uint32 layerMaxClusters, Vector<ClusterPositionData>& clusters)
{
    clusters.clear();
    
    uint32 clusterRowSize = resolutionTilesPerRow[0] * layerMaxClusters;
    uint32 totalClusterCount = clusterRowSize * clusterRowSize;
    Vector2 vegetationInstanceSize(unitSize.x / layerMaxClusters, unitSize.y / layerMaxClusters);
    
    Vector<uint32> densityId;
    densityId.resize(totalClusterCount);
    for(uint32 i = 0; i < totalClusterCount; ++i)
    {
        densityId[i] = (i % maxDensityLevels);
    }
    std::random_shuffle(densityId.begin(), densityId.end(), RandomShuffleFunc);
    
    clusters.resize(totalClusterCount);
    for(uint32 clusterIndex = 0; clusterIndex < totalClusterCount; ++clusterIndex)
    {
        ClusterPositionData& cluster = clusters[clusterIndex];
        
        uint32 cellX = clusterIndex % clusterRowSize;
        uint32 cellY = clusterIndex / clusterRowSize;
        
        uint32 matrixCellX = cellX / layerMaxClusters;
        uint32 matrixCellY = cellY / layerMaxClusters;
        
        float32 randomX = unitSize.x * Random::Instance()->RandFloat();
        float32 randomY = unitSize.y * Random::Instance()->RandFloat();
        
        cluster.pos = Vector3((matrixCellX * unitSize.x) + randomX,
                              (matrixCellY * unitSize.y) + randomY,
                              0.0f);
        cluster.rotation = MAX_ROTATION_ANGLE * (Random::Instance()->RandFloat() - 0.5f);
        cluster.densityId = densityId[clusterIndex];
        cluster.matrixIndex = matrixCellX + matrixCellY * resolutionTilesPerRow[0];
        
        DVASSERT(cluster.matrixIndex >= 0 && cluster.matrixIndex < (resolutionTilesPerRow[0] * resolutionTilesPerRow[0]));
    }
}

void VegetationCustomGeometry::GenerateClusterResolutionData(uint32 layerId,
                                                             uint32 layerMaxClusters,
                                                             uint32 resolutionId,
                                                             Vector<ClusterPositionData>& clusterPosition,
                                                             Vector<ClusterResolutionData>& clusterResolution)
{
    clusterResolution.clear();
    
    uint32 clusterRowSize = resolutionTilesPerRow[0] * layerMaxClusters;
    uint32 currentTilesPerRowCount = resolutionTilesPerRow[resolutionId];
    uint32 resolutionRowSize = (resolutionTilesPerRow[0] / currentTilesPerRowCount) * layerMaxClusters;
    
    size_t totalClusterCount = clusterPosition.size();
    size_t currentResolutionStride = resolutionClusterStride[resolutionId];
    size_t clusterCountInResolution = totalClusterCount / (currentResolutionStride * currentResolutionStride);
    
    clusterResolution.resize(clusterCountInResolution);
    size_t clusterResolutionIndex = 0;
    for(size_t clusterY = 0; clusterY < clusterRowSize; clusterY += currentResolutionStride)
    {
        for(size_t clusterX = 0; clusterX < clusterRowSize; clusterX += currentResolutionStride)
        {
            size_t clusterIndex = clusterX + clusterY * clusterRowSize;
            
            ClusterResolutionData& resolutionData = clusterResolution[clusterResolutionIndex];
            
            uint32 absoluteCellX = clusterIndex % clusterRowSize;
            uint32 absoluteCellY = clusterIndex / clusterRowSize;
            
            uint32 resolutionCellX = absoluteCellX / resolutionRowSize;
            uint32 resolutionCellY = absoluteCellY / resolutionRowSize;
            
            resolutionData.position = clusterPosition[clusterIndex];
            resolutionData.resolutionId = PrepareResolutionId(resolutionId, clusterX, clusterY);
            resolutionData.layerId = layerId;
            resolutionData.cellIndex = resolutionCellX + resolutionCellY * currentTilesPerRowCount;
            
            clusterResolutionIndex++;
            
            DVASSERT(resolutionData.cellIndex >= 0 && resolutionData.cellIndex < (currentTilesPerRowCount * currentTilesPerRowCount));
        }
    }
    
    DVASSERT(clusterResolution.size() == clusterResolutionIndex);
}

uint32 VegetationCustomGeometry::PrepareResolutionId(uint32 currentResolutionId, uint32 cellX, uint32 cellY) const
{
    uint32 nextResolutionId = Min(resolutionCount - 1, currentResolutionId + 1);
    uint32 rowStride = resolutionClusterStride[nextResolutionId];
    
    bool isNextResolution = (((cellX % rowStride) == 0) && ((cellY % rowStride) == 0));
    return (isNextResolution) ? nextResolutionId : currentResolutionId;
}

void VegetationCustomGeometry::GenerateVertexData(Vector<Vector3>& sourcePositions,
                                                  Vector<Vector2>& sourceTextureCoords,
                                                  Vector<Vector3>& sourceNormals,
                                                  Vector<ClusterResolutionData>& clusterResolution,
                                                  
                                                  uint32& startIndex,
                                                  uint32& endIndex,
                                                  Vector<VegetationVertex>& vertexData,
                                                  Vector<VertexRangeData>& perCellOffsets,
                                                  Vector<uint32>& perCellClusterCount)
{
    DVASSERT(sourcePositions.size() == sourceNormals.size() &&
             sourcePositions.size() == sourceTextureCoords.size());
    
    startIndex = vertexData.size();
    
    perCellOffsets.clear();
    uint32 currentMatrix = 0;
    uint32 currentCluster = 0;
    
    uint32 currentVertexIndex = startIndex;
    
    size_t vertexCount = sourcePositions.size();
    size_t clusterCount = clusterResolution.size();
    
    Vector<Vector3> transformedVertices;
    for(size_t clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
    {
        ClusterResolutionData& clusterData = clusterResolution[clusterIndex];
        
        Rotate(clusterData.position.rotation, sourcePositions, transformedVertices);
        
        for(size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
        {
            VegetationVertex vertex;
         
            vertex.coord.x = clusterData.position.pos.x + transformedVertices[vertexIndex].x;
            vertex.coord.y = clusterData.position.pos.y + transformedVertices[vertexIndex].y;
            vertex.coord.z = clusterData.position.pos.z + transformedVertices[vertexIndex].z;
            
            vertex.normal = sourceNormals[vertexIndex];
            
            vertex.binormal = clusterData.position.pos;
            
            vertex.tangent.x = clusterData.position.matrixIndex * 2.0f * 4.0f; //each cluster is described by 2 vectors
            vertex.tangent.y = clusterData.layerId;
            vertex.tangent.z = clusterData.position.densityId;
            
            vertex.texCoord0 = sourceTextureCoords[vertexIndex];
            vertex.texCoord1.x = clusterData.resolutionId;
            
            if(currentMatrix != clusterData.cellIndex)
            {
                VertexRangeData rangeData;
                rangeData.index = currentVertexIndex;
                rangeData.size = vertexData.size() - currentVertexIndex;
            
                perCellOffsets.push_back(rangeData);
                perCellClusterCount.push_back(clusterIndex - currentCluster);
                
                currentCluster = clusterIndex;
                currentMatrix = clusterData.cellIndex;
                currentVertexIndex = vertexData.size();
            }
            
            vertexData.push_back(vertex);
        }
    }
    
    endIndex = vertexData.size() - 1;
    
    VertexRangeData rangeData;
    rangeData.index = currentVertexIndex;
    rangeData.size =  vertexData.size() - currentVertexIndex;
        
    perCellOffsets.push_back(rangeData);
    perCellClusterCount.push_back(clusterCount - currentCluster);

}

void VegetationCustomGeometry::Rotate(float32 angle, Vector<Vector3>& sourcePositions, Vector<Vector3>& rotatedPositions)
{
    rotatedPositions.clear();

    Matrix4 rotMat;
    rotMat.CreateRotation(Vector3(0.0f, 0.0f, 1.0f), DegToRad(angle));
    
    size_t sourceVertexCount = sourcePositions.size();
    for(size_t vertexIndex = 0; vertexIndex < sourceVertexCount; ++vertexIndex)
    {
        Vector3 transformedVertex = sourcePositions[vertexIndex] * rotMat;
        rotatedPositions.push_back(transformedVertex);
    }
}

void VegetationCustomGeometry::GenerateIndexData(Vector<VegetationIndex>& sourceIndices,
                                                 VegetationIndex startIndex,
                                                 uint32 clusterCount,
                                                 uint32 clusterVertexCount,
                                                 Vector<VegetationVertex>& vertexData,
                                                 Vector<VegetationIndex>& indexData,
                                                 Vector<SortBufferData>& directionOffsets)
{
    uint32 totalIndexCount = sourceIndices.size() * clusterCount;
    
    size_t clusterIndexCount = sourceIndices.size();
    Vector<VegetationIndex> sourceCellIndices;
    sourceCellIndices.resize(totalIndexCount);
    for(uint32 clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
    {
        for(size_t i = 0; i < clusterIndexCount; ++i)
        {
            sourceCellIndices[clusterIndex * clusterIndexCount + i] = startIndex + clusterIndex * clusterVertexCount + sourceIndices[i];
        }
    }

    AABBox3 boundingBox;
    Vector<PolygonSortData> sortDataArray;
    size_t sortItemCount = totalIndexCount / 3; //triangle is described by 3 vertices
    sortDataArray.resize(sortItemCount);
    for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
    {
        PolygonSortData& sortData = sortDataArray[sortItemIndex];
        
        sortData.indices[0] = sourceCellIndices[sortItemIndex * 3 + 0];
        
        sortData.indices[1] = sourceCellIndices[sortItemIndex * 3 + 1];
        
        sortData.indices[2] = sourceCellIndices[sortItemIndex * 3 + 2];
        
        boundingBox.AddPoint(vertexData[sortData.indices[0]].coord);
        boundingBox.AddPoint(vertexData[sortData.indices[1]].coord);
        boundingBox.AddPoint(vertexData[sortData.indices[2]].coord);
    }
    
    Vector<Vector3> cameraPositions;
    SetupCameraPositions(boundingBox, cameraPositions);
    size_t cameraPositionCount = cameraPositions.size();
    
    for(uint32 cameraPositionIndex = 0; cameraPositionIndex < cameraPositionCount; ++cameraPositionIndex)
    {
        GenerateSortedClusterIndexData(cameraPositions[cameraPositionIndex],
                                       sortDataArray,
                                       vertexData);
        
        SortBufferData bufferData;
        bufferData.sortDirection = boundingBox.GetCenter() - cameraPositions[cameraPositionIndex];
        bufferData.indexOffset = indexData.size();
        bufferData.size = sortItemCount * 3;
        
        directionOffsets.push_back(bufferData);
        
        for(size_t sortItemIndex = 0; sortItemIndex < sortItemCount; ++sortItemIndex)
        {
            PolygonSortData& sortData = sortDataArray[sortItemIndex];
            
            indexData.push_back(sortData.indices[0] - startIndex);
            indexData.push_back(sortData.indices[1] - startIndex);
            indexData.push_back(sortData.indices[2] - startIndex);
        }
    }
}
    
void VegetationCustomGeometry::GenerateSortedClusterIndexData(Vector3& cameraPosition,
                                                              Vector<PolygonSortData>& sourceIndices,
                                                              Vector<VegetationVertex>& vertexData)
{
    size_t sortedIndicesCount = sourceIndices.size();
    for(size_t sortItemIndex = 0;
        sortItemIndex < sortedIndicesCount;
        ++sortItemIndex)
    {
        PolygonSortData& sortData = sourceIndices[sortItemIndex];
        sortData.cameraDistance = FLT_MAX;
        
        for(uint32 polygonIndex = 0; polygonIndex < 3; ++polygonIndex)
        {
            float32 distance = (vertexData[sortData.indices[polygonIndex]].coord - cameraPosition).SquareLength();
            if(distance < sortData.cameraDistance)
            {
                sortData.cameraDistance = distance;
            }
        }
    }
    
    std::stable_sort(sourceIndices.begin(), sourceIndices.end(), PolygonByDistanceCompareFunction);
}

void VegetationCustomGeometry::BuildLayer(uint32 layerId,
                                          CustomGeometryEntityData& sourceLayerData,
                                          Vector<VegetationVertex>& vertexData,
                                          Vector<VegetationIndex>& indexData,
                                          Vector<Vector<VertexRangeData> >& vertexOffsets, //resolution-cell
                                          Vector<Vector<Vector<SortBufferData> > >& indexOffsets) //resolution-cell-direction
{
    DVASSERT(layerId >= 0 && layerId < maxClusters.size());
    
    uint32 layerMaxClusters = maxClusters[layerId];
    Vector<ClusterPositionData> clusters;
    GenerateClusterPositionData(layerMaxClusters, clusters);
    
    for(uint32 resolutionIndex = 0;
        resolutionIndex < resolutionCount;
        ++resolutionIndex)
    {
        indexOffsets.push_back(Vector<Vector<SortBufferData> >());
        vertexOffsets.push_back(Vector<VertexRangeData>());
        
        Vector<Vector<SortBufferData> >& resolutionOffsets = indexOffsets[indexOffsets.size() - 1];
        Vector<VertexRangeData>& resolutionVertexOffsets = vertexOffsets[vertexOffsets.size() - 1];
        
        Vector<ClusterResolutionData> clusterResolution;
        
        GenerateClusterResolutionData(layerId,
                                      layerMaxClusters,
                                      resolutionIndex,
                                      clusters,
                                      clusterResolution);
        std::stable_sort(clusterResolution.begin(), clusterResolution.end(), ClusterByMatrixCompareFunction);
        
        uint32 vertexStartIndex = 0;
        uint32 vertexEndIndex = 0;
        Vector<uint32> perCellClusterCount;
        GenerateVertexData(sourceLayerData.lods[resolutionIndex].sourcePositions,
                           sourceLayerData.lods[resolutionIndex].sourceTextureCoords,
                           sourceLayerData.lods[resolutionIndex].sourceNormals,
                           clusterResolution,
                           vertexStartIndex,
                           vertexEndIndex,
                           vertexData,
                           vertexOffsets[resolutionIndex],
                           perCellClusterCount);
        
        uint32 cellCount = resolutionVertexOffsets.size();
        for(uint32 cellIndex = 0; cellIndex < cellCount; cellIndex++)
        {
            resolutionOffsets.push_back(Vector<SortBufferData>());
            Vector<SortBufferData>& currentIndexOffsets = resolutionOffsets[resolutionOffsets.size() - 1];
            
            GenerateIndexData(sourceLayerData.lods[resolutionIndex].sourceIndices,
                              vertexOffsets[resolutionIndex][cellIndex].index,
                              perCellClusterCount[cellIndex],
                              sourceLayerData.lods[resolutionIndex].sourcePositions.size(),
                              vertexData,
                              indexData,
                              currentIndexOffsets);
        }
    }
}

void VegetationCustomGeometry::LoadCustomData(Vector<CustomGeometryEntityData>& geometryData)
{
    Scene* scene = new Scene();
    Entity* entity = scene->GetRootNode(sourceDataPath);
    
    DVASSERT(entity && "Cannot load custom geometry scene specified!");
    
    if(entity)
    {
        //VI: wait for all scene objects to initialize
        //VI: in order to avoid crashes when scene released
        ThreadIdJobWaiter waiter(Thread::GetCurrentThreadId());
        waiter.Wait();
        
        Entity* currentVariation = SelectDataVariation(entity);
        DVASSERT(currentVariation && "Invalid scene structure: variations!");
        
        if(currentVariation)
        {
            uint32 layerCount = COUNT_OF(VEGETATION_ENTITY_LAYER_NAMES);
            for(uint32 layerIndex = 0; layerIndex < layerCount; ++layerIndex)
            {
                Entity* layerEntity = currentVariation->FindByName(VEGETATION_ENTITY_LAYER_NAMES[layerIndex]);
                
                DVASSERT(layerEntity && "Invalid scene structure: layers!");
                
                if(layerEntity)
                {
                    RenderComponent* rc = GetRenderComponent(layerEntity);
                    DVASSERT(rc && "Invalid scene structure: no render component!");
                
                    if(rc)
                    {
                        RenderObject* ro = rc->GetRenderObject();
                        DVASSERT(ro && "Invalid scene structure: no render object!");
                        
                        if(ro)
                        {
                            DVASSERT(ro->GetRenderBatchCount() > 0 && "Invalid scene structure: no render batches!");
                            
                            uint32 renderBatchCount = ro->GetRenderBatchCount();
                            if(renderBatchCount > 0)
                            {
                                geometryData.push_back(CustomGeometryEntityData());
                                CustomGeometryEntityData& geometryDataItem = geometryData[geometryData.size() - 1];
                                
                                NMaterial* parentMaterial = ro->GetRenderBatch(0)->GetMaterial()->GetParent();
                                parentMaterial->AddNodeFlags(DataNode::NodeRuntimeFlag);
                                
                                geometryDataItem.SetMaterial(parentMaterial);

                                for(uint32 resolutionIndex = 0;
                                    resolutionIndex < resolutionCount;
                                    resolutionIndex++)
                                {
                                    uint32 renderBatchIndex = Min(resolutionIndex, renderBatchCount - 1);
                                    RenderBatch* rb = ro->GetRenderBatch(renderBatchIndex);
                                    
                                    DVASSERT(rb);
                                    
                                    PolygonGroup* pg = rb->GetPolygonGroup();
                                    
                                    DVASSERT(pg);
                                    
                                    geometryDataItem.lods.push_back(CustomGeometryLayerData());
                                    CustomGeometryLayerData& geometryLayerData = geometryDataItem.lods[geometryDataItem.lods.size() - 1];
                                    
                                    Vector<Vector3>& positions = geometryLayerData.sourcePositions;
                                    Vector<Vector2>& texCoords = geometryLayerData.sourceTextureCoords;
                                    Vector<Vector3>& normals = geometryLayerData.sourceNormals;
                                    Vector<VegetationIndex>& indices = geometryLayerData.sourceIndices;
                                    
                                    int32 vertexFormat = pg->GetFormat();
                                    DVASSERT((vertexFormat & EVF_VERTEX) != 0);
                                    
                                    uint32 vertexCount = pg->GetVertexCount();
                                    for(uint32 vertexIndex = 0;
                                        vertexIndex < vertexCount;
                                        ++vertexIndex)
                                    {
                                        Vector3 coord;
                                        pg->GetCoord(vertexIndex, coord);
                                        
                                        Vector3 normal;
                                        if((vertexFormat & EVF_NORMAL) != 0)
                                        {
                                            pg->GetNormal(vertexIndex, normal);
                                        }
                                        
                                        Vector2 texCoord;
                                        if((vertexFormat & EVF_TEXCOORD0) != 0)
                                        {
                                            pg->GetTexcoord(0, vertexIndex, texCoord);
                                        }
                                        
                                        positions.push_back(coord);
                                        normals.push_back(normal);
                                        texCoords.push_back(texCoord);
                                    }
                                    
                                    uint32 indexCount = pg->GetIndexCount();
                                    for(uint32 indexIndex = 0;
                                        indexIndex < indexCount;
                                        ++indexIndex)
                                    {
                                        int32 currentIndex = 0;
                                        pg->GetIndex(indexIndex, currentIndex);
                                        
                                        indices.push_back(currentIndex);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    SafeRelease(scene);
}

Entity* VegetationCustomGeometry::SelectDataVariation(Entity* rootNode)
{
    return rootNode->FindByName(VEGETATION_ENTITY_VARIATION_0);
}

};