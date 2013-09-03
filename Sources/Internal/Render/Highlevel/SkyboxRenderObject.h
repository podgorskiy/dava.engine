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
#ifndef __DAVAENGINE_SKYBOXRENDEROBJECT_H__
#define __DAVAENGINE_SKYBOXRENDEROBJECT_H__

#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
	class SkyboxRenderObject : public RenderObject, public IRenderUpdatable
	{
	public:
		
		class SkyboxTextureValidator
		{
		public:
			
			virtual bool IsValid(const FilePath& descriptorPath) = 0;
		};
		
	public:
		
		SkyboxRenderObject();
		virtual ~SkyboxRenderObject();
		
		//this method should be called by external editor to initialize skybox
		//after adding skybox to scene and setting up its paramaters it becames self-sufficent
		void Initialize(AABBox3& box);
		
		virtual void RenderUpdate(Camera *camera, float32 timeElapsed);
		
		RenderObject * Clone(RenderObject *newObject);
		virtual void Save(KeyedArchive *archive, SceneFileV2 *sceneFile);
		virtual void Load(KeyedArchive *archive, SceneFileV2 *sceneFile);
		virtual void SetRenderSystem(RenderSystem * renderSystem);
		
		void SetTexture(const FilePath& texPath);
		FilePath GetTexture();
		void SetOffsetZ(const float32& offset);
		float32 GetOffsetZ();
		void SetRotationZ(const float32& rotation);
		float32 GetRotationZ();
		
		//set validator in resourceeditor during skybox creation in order to filter out non-skybox textures files
		SkyboxRenderObject::SkyboxTextureValidator* GetTextureValidator();
		void SetTextureValidator(SkyboxRenderObject::SkyboxTextureValidator* validator);
		
		//INTROSPECTION used intentionally instead of INTROSPECTION_EXTEND in order to hide underlying details of SkyboxRenderObject implementation
		INTROSPECTION(SkyboxRenderObject,
					  PROPERTY("texture", "Texture Path", GetTexture, SetTexture, I_SAVE | I_VIEW | I_EDIT)
					  PROPERTY("verticalOffset", "Vertical Offset", GetOffsetZ, SetOffsetZ, I_SAVE | I_VIEW | I_EDIT)
					  PROPERTY("rotationAngle", "Rotation", GetRotationZ, SetRotationZ, I_SAVE | I_VIEW | I_EDIT)
					  );

		
	private:
		
		void CreateRenderData();
		void BuildSkybox();
		void UpdateMaterial();

	private:
		
		FilePath texturePath;
		float32 offsetZ;
		float32 rotationZ;
		float32 nonClippingDistance;
		
		SkyboxTextureValidator* textureValidator; //this field is set in editor environment only
	};
};

#endif //__DAVAENGINE_SKYBOXRENDEROBJECT_H__