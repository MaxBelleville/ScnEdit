/*
!@
MIT License

Copyright (c) 2023 Skylicht Technology CO., LTD

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#include "pch.h"
#include "CDeferredSimpleRP.h"
#include "CForwardRP.h"
#include "RenderMesh/CMesh.h"
#include "Material/CMaterial.h"
#include "Material/Shader/CShaderManager.h"
#include "Material/Shader/ShaderCallback/CShaderDeferred.h"
#include "Material/Shader/ShaderCallback/CShaderSH.h"
#include "Material/Shader/ShaderCallback/CShaderMaterial.h"
#include "Shadow/CShadowRTTManager.h"
#include "Culling/CCullingSystem.h"

namespace Skylicht
{

	CDeferredSimpleRP::CDeferredSimpleRP() :
		m_albedo(NULL),
		m_position(NULL),
		m_normal(NULL),
		m_data(NULL),
		m_vertexColorShader(0),
		m_textureColorShader(0),
		m_textureLinearRGBShader(0),
		m_postProcessor(NULL)
	{
		m_type = IRenderPipeline::Deferred;
	}

	CDeferredSimpleRP::~CDeferredSimpleRP()
	{
		releaseRTT();
	}

	void CDeferredSimpleRP::initRTT(int w, int h)
	{
		IVideoDriver* driver = getVideoDriver();

		// init render target
		m_size = core::dimension2du((u32)w, (u32)h);
		m_albedo = driver->addRenderTargetTexture(m_size, "albedo", ECF_A8R8G8B8);
		m_position = driver->addRenderTargetTexture(m_size, "position", ECF_A32B32G32R32F);
		m_normal = driver->addRenderTargetTexture(m_size, "normal", ECF_A32B32G32R32F);
		m_data = driver->addRenderTargetTexture(m_size, "data", ECF_A8R8G8B8);

		m_target = driver->addRenderTargetTexture(m_size, "target", ECF_A16B16G16R16F);

		// setup multi render target
		// opengles just support 4 buffer
		m_multiRenderTarget.push_back(m_albedo);
		m_multiRenderTarget.push_back(m_position);
		m_multiRenderTarget.push_back(m_normal);
		m_multiRenderTarget.push_back(m_data);
	}

	void CDeferredSimpleRP::releaseRTT()
	{
		IVideoDriver* driver = getVideoDriver();

		if (m_albedo != NULL)
			driver->removeTexture(m_albedo);

		if (m_position != NULL)
			driver->removeTexture(m_position);

		if (m_normal != NULL)
			driver->removeTexture(m_normal);

		if (m_data != NULL)
			driver->removeTexture(m_data);

		if (m_target != NULL)
			driver->removeTexture(m_target);

		m_multiRenderTarget.clear();
	}

	void CDeferredSimpleRP::initRender(int w, int h)
	{
		IVideoDriver* driver = getVideoDriver();

		// init render target
		initRTT(w, h);

		// get basic shader
		CShaderManager* shaderMgr = CShaderManager::getInstance();
		m_textureColorShader = shaderMgr->getShaderIDByName("TextureColor");
		m_textureLinearRGBShader = shaderMgr->getShaderIDByName("TextureLinearRGB");
		m_vertexColorShader = shaderMgr->getShaderIDByName("VertexColor");

	}

	void CDeferredSimpleRP::resize(int w, int h)
	{
		releaseRTT();
		initRTT(w, h);

	}


	void CDeferredSimpleRP::disableFloatTextureFilter(SMaterial& m)
	{
		// turn off mipmap on float texture	
		m.TextureLayer[0].BilinearFilter = false;
		m.TextureLayer[0].TrilinearFilter = false;
		m.TextureLayer[0].AnisotropicFilter = 0;

		m.TextureLayer[1].BilinearFilter = false;
		m.TextureLayer[1].TrilinearFilter = false;
		m.TextureLayer[1].AnisotropicFilter = 0;

		m.TextureLayer[2].BilinearFilter = false;
		m.TextureLayer[2].TrilinearFilter = false;
		m.TextureLayer[2].AnisotropicFilter = 0;

		m.TextureLayer[3].BilinearFilter = false;
		m.TextureLayer[3].TrilinearFilter = false;
		m.TextureLayer[3].AnisotropicFilter = 0;

		// disable Z
		m.ZBuffer = video::ECFN_DISABLED;
		m.ZWriteEnable = false;
	}

	bool CDeferredSimpleRP::canRenderMaterial(CMaterial* material)
	{
		if (material->isDeferred() == true)
			return true;

		return false;
	}

	bool CDeferredSimpleRP::canRenderShader(CShader* shader)
	{
		if (shader->isDeferred() == true)
			return true;

		return false;
	}

	void CDeferredSimpleRP::drawMeshBuffer(CMesh* mesh, int bufferID, CEntityManager* entity, int entityID, bool skinnedMesh)
	{
	
			if (s_bakeMode == true)
			{
				// update texture resource
				updateTextureResource(mesh, bufferID, entity, entityID, skinnedMesh);

				IMeshBuffer* mb = mesh->getMeshBuffer(bufferID);
				IVideoDriver* driver = getVideoDriver();

				video::SMaterial irrMaterial = mb->getMaterial();
				irrMaterial.BackfaceCulling = false;

				// set irrlicht material
				driver->setMaterial(irrMaterial);

				// draw mesh buffer
				driver->drawMeshBuffer(mb);
			}
			else
			{
				// default render
				CBaseRP::drawMeshBuffer(mesh, bufferID, entity, entityID, skinnedMesh);
			}

	}

	void CDeferredSimpleRP::drawInstancingMeshBuffer(CMesh* mesh, int bufferID, CShader* instancingShader, CEntityManager* entityMgr, int entityID, bool skinnedMesh)
	{
		
		if (s_bakeMode == true)
		{
			IMeshBuffer* mb = mesh->getMeshBuffer(bufferID);
			IVideoDriver* driver = getVideoDriver();

			video::SMaterial irrMaterial = mb->getMaterial();
			irrMaterial.MaterialType = instancingShader->getMaterialRenderID();
			irrMaterial.BackfaceCulling = false;

			driver->setMaterial(irrMaterial);
			driver->drawMeshBuffer(mb);
		}
		else
		{
			CBaseRP::drawInstancingMeshBuffer(mesh, bufferID, instancingShader, entityMgr, entityID, skinnedMesh);
		}

	}

	void CDeferredSimpleRP::render(ITexture* target, CCamera* camera, CEntityManager* entityManager, const core::recti& viewport, int cubeFaceId, IRenderPipeline* lastRP)
	{
		if (camera == NULL)
			return;

		IVideoDriver* driver = getVideoDriver();

		// custom viewport
		bool useCustomViewport = false;
		core::recti customViewport;
		if (viewport.getWidth() > 0 && viewport.getHeight() > 0)
		{
			customViewport.LowerRightCorner.set(
				viewport.getWidth(),
				viewport.getHeight()
			);

			useCustomViewport = true;
		}

		// setup camera
		setCamera(camera);
		entityManager->setCamera(camera);
		entityManager->setRenderPipeline(this);

		// save projection/view for advance shader SSR
		CShaderDeferred::setProjection(driver->getTransform(video::ETS_PROJECTION));

		m_viewMatrix = driver->getTransform(video::ETS_VIEW);
		CShaderDeferred::setView(m_viewMatrix);

		// STEP 01:
		// draw baked indirect
	
		if (useCustomViewport)
			driver->setViewPort(customViewport);

		if (m_updateEntity == true)
		{
			entityManager->update();
			entityManager->render();
		}
		else
		{
			entityManager->cullingAndRender();
		}
		if (useCustomViewport)
			driver->setViewPort(customViewport);
		entityManager->render();
		

		// STEP 02:
		// Render multi target to: albedo, position, normal, data		
		driver->setRenderTarget(m_multiRenderTarget);
		if (useCustomViewport)
			driver->setViewPort(customViewport);

		// draw entity to buffer
		entityManager->render();

		// Apply uniform: uLightMultiplier
		if (CBaseRP::s_bakeMode == true && CBaseRP::s_bakeLMMode)
		{
			// default light setting
			CShaderManager::getInstance()->ShaderVec3[0] = core::vector3df(1.0f, 1.0f, 1.0f);
		}
		else
		{
			CShaderManager::getInstance()->ShaderVec3[0] = core::vector3df(1.0f, 1.0f, 1.0f);
		}

		// STEP 03:
		// save camera transform
		m_projectionMatrix = driver->getTransform(video::ETS_PROJECTION);
		m_viewMatrix = driver->getTransform(video::ETS_VIEW);

		float renderW = (float)m_size.Width;
		float renderH = (float)m_size.Height;

		if (useCustomViewport)
		{
			renderW = (float)viewport.getWidth();
			renderH = (float)viewport.getHeight();
		}

		// STEP 04
		// Render final direction lighting to screen
		driver->setRenderTarget(m_target, false, false);

		// custom viewport
		if (useCustomViewport)
			driver->setViewPort(customViewport);

		// ssr (last frame)
		bool enableSSR = false;

		if (m_postProcessor != NULL &&
			m_postProcessor->getLastFrameBuffer() &&
			m_postProcessor->isEnableScreenSpaceReflection())
		{
			enableSSR = true;
			
		}

		beginRender2D(renderW, renderH);

		
		CCullingSystem::useCacheCulling(true);

		// STEP 05
		// call forwarder rp?
		core::recti fwvp(0, 0, (int)renderW, (int)renderH);
		onNext(m_target, camera, entityManager, fwvp, cubeFaceId);

		CCullingSystem::useCacheCulling(false);

		// STEP 06
		// final pass to screen
		if (m_postProcessor != NULL && s_bakeMode == false)
		{
			ITexture* emission = NULL;
			if (m_next->getType() == IRenderPipeline::Forwarder)
				emission = ((CForwardRP*)m_next)->getEmissionTexture();

			m_postProcessor->postProcessing(target, m_target, emission, m_normal, m_position, viewport, cubeFaceId);
		}
		else
		{
			setTarget(target, cubeFaceId);

			if (useCustomViewport)
				driver->setViewPort(viewport);

			beginRender2D(renderW, renderH);
			m_finalPass.setTexture(0, m_target);
			m_finalPass.MaterialType = m_textureLinearRGBShader;
			renderBufferToTarget(0.0f, 0.0f, renderW, renderH, m_finalPass);
		}

	

		// test
		/*
		SMaterial t = m_finalPass;
		t.setTexture(0, m_data);
		t.MaterialType = m_textureColorShader;
		renderBufferToTarget(0.0f, 0.0f, renderW, renderH, 0.0f, 0.0f, renderW, renderH, t);
		*/
	}
}
