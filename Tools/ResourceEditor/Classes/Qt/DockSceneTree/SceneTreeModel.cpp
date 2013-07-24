/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include <QMimeData>

#include "DockSceneTree/SceneTreeModel.h"
#include "Scene/SceneSignals.h"

// framework
#include "Scene3d/Components/ComponentHelpers.h"

const char* SceneTreeModel::mimeFormatEntity = "application/dava.entity";
const char* SceneTreeModel::mimeFormatLayer = "application/dava.particlelayer";
const char* SceneTreeModel::mimeFormatForce = "application/dava.particleforce";

SceneTreeModel::SceneTreeModel(QObject* parent /*= 0*/ )
	: QStandardItemModel(parent)
	, curScene(NULL)
	, dropAccepted(false)
{
	setColumnCount(1);
	setSupportedDragActions(Qt::MoveAction|Qt::LinkAction);

	QStringList headerLabels;
	headerLabels.append("Scene hierarchy");
	setHorizontalHeaderLabels(headerLabels);

	QObject::connect(SceneSignals::Instance(), SIGNAL(StructureChanged(SceneEditor2 *, DAVA::Entity *)), this, SLOT(StructureChanged(SceneEditor2 *, DAVA::Entity *)));
	QObject::connect(this, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(ItemChanged(QStandardItem *)));
}

SceneTreeModel::~SceneTreeModel()
{
	if(NULL != curScene)
	{
		curScene->Release();
		curScene = NULL;
	}
}

void SceneTreeModel::SetScene(SceneEditor2 *scene)
{
	// remove add rows
	removeRows(0, rowCount());

	if(NULL != curScene)
	{
		curScene->Release();
	}	

	curScene = scene;

	if(NULL != curScene)
	{
		curScene->Retain();
		ResyncStructure(invisibleRootItem(), curScene);
	}

	RebuildIndexesCache();
}

SceneEditor2* SceneTreeModel::GetScene() const
{
	return curScene;
}

void SceneTreeModel::SetSolid(const QModelIndex &index, bool solid)
{
	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));

	if(NULL != entity)
	{
		entity->SetSolid(solid);
	}
}

bool SceneTreeModel::GetSolid(const QModelIndex &index) const
{
	bool ret = false;

	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));
	if(NULL != entity)
	{
		ret = entity->GetSolid();
	}

	return ret;
}

void SceneTreeModel::SetLocked(const QModelIndex &index, bool locked)
{
	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));

	if(NULL != entity)
	{
		entity->SetLocked(locked);
	}
}

bool SceneTreeModel::GetLocked(const QModelIndex &index) const
{
	bool ret = false;
	
	DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(GetItem(index));
	if(NULL != entity)
	{
		ret = entity->GetLocked();
	}

	return ret;
}

QModelIndex SceneTreeModel::GetIndex(DAVA::Entity *entity) const
{
	return indexesCacheEntities.value(entity, QModelIndex());
}

QModelIndex SceneTreeModel::GetIndex(DAVA::ParticleLayer *layer) const
{
	return indexesCacheLayers.value(layer, QModelIndex());
}

QModelIndex SceneTreeModel::GetIndex(DAVA::ParticleForce *force) const
{
	return indexesCacheForces.value(force, QModelIndex());
}

SceneTreeItem* SceneTreeModel::GetItem(const QModelIndex &index) const
{
	return (SceneTreeItem *) itemFromIndex(index);
}

Qt::DropActions SceneTreeModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

QMimeData * SceneTreeModel::mimeData(const QModelIndexList & indexes) const
{
	QMimeData* ret = NULL;

	if(indexes.size() > 0)
	{
		if(AreSameType(indexes))
		{
			SceneTreeItem *firstItem = GetItem(indexes.at(0));
			if(NULL != firstItem)
			{
				switch (firstItem->ItemType())
				{
				case SceneTreeItem::EIT_Entity:
					{
						QVector<void*> data;
						foreach(QModelIndex index, indexes)
						{
							data.push_back(SceneTreeItemEntity::GetEntity(GetItem(index)));
						}

						ret = EncodeMimeData(data, mimeFormatEntity);
					}
					break;
				case SceneTreeItem::EIT_Layer:
					{
						QVector<void*> data;
						foreach(QModelIndex index, indexes)
						{
							data.push_back(SceneTreeItemParticleLayer::GetLayer(GetItem(index)));
						}

						ret = EncodeMimeData(data, mimeFormatLayer);
					}
					break;
				case SceneTreeItem::EIT_Force:
					{
						QVector<void*> data;
						foreach(QModelIndex index, indexes)
						{
							data.push_back(SceneTreeItemParticleForce::GetForce(GetItem(index)));
						}

						ret = EncodeMimeData(data, mimeFormatForce);
					}
					break;
				default:
					break;
				}
			}
		}
		else
		{
			// empty mime data
			ret = new QMimeData();
		}
	}

	return ret;
}

QStringList SceneTreeModel::mimeTypes() const
{
	QStringList types;

	types << SceneTreeModel::mimeFormatEntity;
	types << SceneTreeModel::mimeFormatLayer;
	types << SceneTreeModel::mimeFormatForce;

	return types;
}

bool SceneTreeModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
	bool ret = false;

	SceneTreeItem* parentItem = GetItem(parent);
	SceneTreeItem *beforeItem = GetItem(index(row, column, parent));

	int dropType = GetDropType(data);
	switch (dropType)
	{
	case DropingEntity:
		{
			DAVA::Entity *parentEntity = SceneTreeItemEntity::GetEntity(parentItem);
			DAVA::Entity *beforeEntity = SceneTreeItemEntity::GetEntity(beforeItem);

			if(NULL == parentEntity)
			{
				parentEntity = curScene;
			}

			QVector<void*> *entitiesV = DecodeMimeData(data, mimeFormatEntity);
			if(NULL != entitiesV && entitiesV->size() > 0)
			{
				EntityGroup entityGroup;
				for (int i = 0; i < entitiesV->size(); ++i)
				{
					entityGroup.Add((DAVA::Entity*) entitiesV->at(i));
				}

				curScene->structureSystem->Move(&entityGroup, parentEntity, beforeEntity);
				ret = true;
			}

			if(NULL != entitiesV)
			{
				delete entitiesV;
			}
		}
		break;
	case DropingLayer:
		{
			DAVA::Entity *parentEntity = SceneTreeItemEntity::GetEntity(parentItem);
			DAVA::ParticleEmitter* emitter = DAVA::GetEmitter(parentEntity);
			QVector<void*> *layersV = DecodeMimeData(data, mimeFormatLayer);

			if(NULL != emitter && NULL != layersV && layersV->size() > 0)
			{
				DAVA::ParticleLayer* beforeLayer = NULL;

				if(row >= 0 && row < (int) emitter->GetLayers().size())
				{
					beforeLayer = emitter->GetLayers()[row];
				}

				DAVA::Vector<DAVA::ParticleLayer*> layersGroup;
				for(int i = 0; i < layersV->size(); ++i)
				{
					layersGroup.push_back((DAVA::ParticleLayer *) layersV->at(i));
				}

				curScene->structureSystem->MoveLayer(layersGroup, emitter, beforeLayer);
				ret = true;
			}

			if(NULL != layersV)
			{
				delete layersV;
			}
		}
		break;
	case DropingForce:
		{
			DAVA::ParticleLayer *newLayer = SceneTreeItemParticleLayer::GetLayer(parentItem);
			QVector<void*> *forcesV = DecodeMimeData(data, mimeFormatForce);

			if(NULL != newLayer && NULL != forcesV && forcesV->size() > 0)
			{
				DAVA::Vector<DAVA::ParticleForce*> forcesGroup;
				DAVA::Vector<DAVA::ParticleLayer*> layersGroup;

				for(int i = 0; i < forcesV->size(); ++i)
				{
					QModelIndex forceIndex = GetIndex((DAVA::ParticleForce *) forcesV->at(i));
					DAVA::ParticleLayer *oldLayer = SceneTreeItemParticleLayer::GetLayer(GetItem(forceIndex.parent()));

					forcesGroup.push_back((DAVA::ParticleForce *) forcesV->at(i));
					layersGroup.push_back(oldLayer);
				}

				curScene->structureSystem->MoveForce(forcesGroup, layersGroup, newLayer);
				ret = true;
			}

			if(NULL != forcesV)
			{
				delete forcesV;
			}
		}
		break;
	default:
		break;
	}

	dropAccepted = ret;
	return ret;
}

bool SceneTreeModel::DropCanBeAccepted(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) const
{
	bool ret = false;

	SceneTreeItem* parentItem = GetItem(parent);

	int dropType = GetDropType(data);
	switch (dropType)
	{
	case DropingEntity:
		{
			ret = true;

			// 1. don't accept entity to be dropped anywhere except other entity
			if(NULL != parentItem && parentItem->ItemType() != SceneTreeItem::EIT_Entity)
			{
				ret = false;
			}
			else
			{
				// go thought entities and check them
				QVector<void*> *entities = DecodeMimeData(data, mimeFormatEntity);
				if(NULL != entities)
				{
					for (int i = 0; i < entities->size(); ++i)
					{
						DAVA::Entity *entity = (DAVA::Entity *) entities->at(i);

						// 2. we don't accept drops if it has locked items
						if(NULL != entity && entity->GetLocked())
						{
							ret = false;
							break;
						}
					}

					delete entities;
				}
			}
		}
		break;
	case DropingLayer:
		{
			// accept layer to be dropped only to entity with particle emitter
			if(NULL != parentItem && parentItem->ItemType() == SceneTreeItem::EIT_Entity)
			{
				DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(parentItem);
				if(NULL != DAVA::GetEmitter(entity))
				{
					ret = true;
				}
			}
		}
		break;
	case DropingForce:
		{
			// accept force to be dropped only to particle layer
			if(NULL != parentItem && parentItem->ItemType() == SceneTreeItem::EIT_Layer) 
			{
				// accept only add (no insertion)
				if(-1 == row && -1 == column)
				{
					ret = true;
				}
			}
		}
		break;

	case DropingUnknown:
	case DropingMixed:
	default:
		break;
	}

	return ret;
}

bool SceneTreeModel::DropAccepted() const
{
	return dropAccepted;
}

void SceneTreeModel::StructureChanged(SceneEditor2 *scene, DAVA::Entity *parent)
{
	if(curScene == scene)
	{
		ResyncStructure(invisibleRootItem(), curScene);
		RebuildIndexesCache();
	}
}

void SceneTreeModel::ItemChanged(QStandardItem * item)
{
	SceneTreeItem *treeItem = dynamic_cast<SceneTreeItem *>(item);
	if(NULL != treeItem)
	{
		if(treeItem->ItemType() == SceneTreeItem::EIT_Layer)
		{
			SceneTreeItemParticleLayer *itemLayer = (SceneTreeItemParticleLayer *) treeItem;
			itemLayer->layer->SetDisabled(item->checkState() == Qt::Unchecked);
		}
	}
}

void SceneTreeModel::ResyncStructure(QStandardItem *item, DAVA::Entity *entity)
{
	SceneTreeItemEntity::DoSync(item, entity);
}

void SceneTreeModel::RebuildIndexesCache()
{
	indexesCacheEntities.clear();
	indexesCacheForces.clear();
	indexesCacheForces.clear();

	for(int i = 0; i < rowCount(); ++i)
	{
		AddIndexesCache(GetItem(index(i, 0)));
	}
}

void SceneTreeModel::AddIndexesCache(SceneTreeItem *item)
{
	// go thought all items and remember entities indexes
	switch(item->ItemType())
	{
	case SceneTreeItem::EIT_Entity:
		{
			DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(item);
			if(NULL != entity)
			{
				indexesCacheEntities.insert(entity, item->index());
			}
		}
		break;
	case SceneTreeItem::EIT_Layer:
		{
			DAVA::ParticleLayer *layer = SceneTreeItemParticleLayer::GetLayer(item);
			if(NULL != layer)
			{
				indexesCacheLayers.insert(layer, item->index());
			}
		}
		break;
	case SceneTreeItem::EIT_Force:
		{
			DAVA::ParticleForce *force = SceneTreeItemParticleForce::GetForce(item);
			if(NULL != force)
			{
				indexesCacheForces.insert(force, item->index());
			}
		}
		break;
	}

	for(int i = 0; i < item->rowCount(); ++i)
	{
		AddIndexesCache((SceneTreeItem *) item->child(i));
	}
}

bool SceneTreeModel::AreSameType(const QModelIndexList & indexes) const
{
	bool ret = true;

	SceneTreeItem *firstItem = GetItem(indexes.at(0));

	// check if all items are same type
	if(NULL != firstItem)
	{
		foreach(QModelIndex index, indexes)
		{
			SceneTreeItem *item = GetItem(index);
			if(NULL != item && firstItem->ItemType() != item->ItemType())
			{
				ret = false;
				break;
			}
		}
	}

	return ret;
}

int SceneTreeModel::GetDropType(const QMimeData *data) const
{
	int ret = DropingUnknown;

	if(NULL != data)
	{
		if(data->formats().size() > 1)
		{
			// more than one format in data
			ret = DropingMixed;
		}
		else if(data->hasFormat(mimeFormatEntity))
		{
			ret = DropingEntity;
		}
		else if(data->hasFormat(mimeFormatLayer))
		{
			ret = DropingLayer;
		}
		else if(data->hasFormat(mimeFormatForce))
		{
			ret = DropingForce;
		}
	}

	return ret;
}

QMimeData* SceneTreeModel::EncodeMimeData(const QVector<void*> &data, const QString &format) const
{
	QMimeData *mimeData = NULL;
	
	if(data.size() > 0)
	{
		mimeData = new QMimeData();
		QByteArray encodedData;

		QDataStream stream(&encodedData, QIODevice::WriteOnly);
		for (int i = 0; i < data.size(); ++i)
		{
			stream.writeRawData((char *) &data[i], sizeof(void*));
		}

		mimeData->setData(format, encodedData);
	}

	return mimeData;
}

QVector<void*>* SceneTreeModel::DecodeMimeData(const QMimeData* data, const QString &format) const
{
	QVector<void*> *ret = NULL;

	if(data->hasFormat(format))
	{
		void* entity = NULL;
		QByteArray encodedData = data->data(format);
		QDataStream stream(&encodedData, QIODevice::ReadOnly);

		ret = new QVector<void*>();
		while(!stream.atEnd())
		{
			stream.readRawData((char *) &entity, sizeof(void*));
			ret->push_back(entity);
		}
	}

	return ret;
}


SceneTreeFilteringModel::SceneTreeFilteringModel(SceneTreeModel *_treeModel, QObject *parent /* = NULL */)
	: QSortFilterProxyModel(parent)
	, treeModel(_treeModel)
{
	setSourceModel(treeModel);
}

bool SceneTreeFilteringModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	if(NULL != treeModel)
	{
		// check self accept
		if(selfAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}

		//accept if any of the parents is accepted
		QModelIndex parent = sourceParent;
		while(parent.isValid()) 
		{
			if(selfAcceptRow(parent.row(), parent.parent()))
			{
				return true;
			}

			parent = parent.parent();
		}

		// accept if any child is accepted
		if(childrenAcceptRow(sourceRow, sourceParent))
		{
			return true;
		}
	}

	return false;
}

bool SceneTreeFilteringModel::selfAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool SceneTreeFilteringModel::childrenAcceptRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool ret = false;

	QModelIndex index = treeModel->index(sourceRow, 0, sourceParent);
	if(treeModel->rowCount(index) > 0)
	{
		for(int i = 0; i < treeModel->rowCount(index); i++)
		{
			if(selfAcceptRow(i, index) || childrenAcceptRow(i, index))
			{
				ret = true;
				break;
			}
		}
	}

	return ret;
}