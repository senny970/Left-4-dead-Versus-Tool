//============ Copyright (c) Valve Corporation, All rights reserved. ============
//
// Functionality to render a glowing outline around client renderable objects.
//
//===============================================================================

//
//------------------------------------glow_outline_effect.h--------------------------------------------------//
//

#if defined( COMPILER_MSVC )
#pragma once
#endif

#include "utlvector.h"
#include "mathlib/vector.h"

class C_BaseEntity;
class CViewSetup;
class CMatRenderContextPtr;

static const int GLOW_FOR_ALL_SPLIT_SCREEN_SLOTS = -1;

class CGlowObjectManager
{
public:
	CGlowObjectManager() :
	m_nFirstFreeSlot( GlowObjectDefinition_t::END_OF_FREE_LIST )
	{
	}

	int RegisterGlowObject( C_BaseEntity *pEntity, const Vector &vGlowColor, float flGlowAlpha, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded, int nSplitScreenSlot )
	{
		int nIndex;
		if ( m_nFirstFreeSlot == GlowObjectDefinition_t::END_OF_FREE_LIST )
		{
			nIndex = m_GlowObjectDefinitions.AddToTail();
		}
		else
		{
			nIndex = m_nFirstFreeSlot;
			m_nFirstFreeSlot = m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot;
		}
		
		m_GlowObjectDefinitions[nIndex].m_pEntity = pEntity;
		m_GlowObjectDefinitions[nIndex].m_vGlowColor = vGlowColor;
		m_GlowObjectDefinitions[nIndex].m_flGlowAlpha = flGlowAlpha;
		m_GlowObjectDefinitions[nIndex].m_bRenderWhenOccluded = bRenderWhenOccluded;
		m_GlowObjectDefinitions[nIndex].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
		m_GlowObjectDefinitions[nIndex].m_bFullBloomRender = false;
		m_GlowObjectDefinitions[nIndex].m_nFullBloomStencilTestValue = 0;
		m_GlowObjectDefinitions[nIndex].m_nSplitScreenSlot = nSplitScreenSlot;
		m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot = GlowObjectDefinition_t::ENTRY_IN_USE;

		return nIndex;
	}

	void UnregisterGlowObject( int nGlowObjectHandle )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );

		m_GlowObjectDefinitions[nGlowObjectHandle].m_nNextFreeSlot = m_nFirstFreeSlot;
		m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = NULL;
		m_nFirstFreeSlot = nGlowObjectHandle;
	}

	void SetEntity( int nGlowObjectHandle, C_BaseEntity *pEntity )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = pEntity;
	}

	void SetColor( int nGlowObjectHandle, const Vector &vGlowColor ) 
	{ 
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_vGlowColor = vGlowColor;
	}

	void SetAlpha( int nGlowObjectHandle, float flAlpha ) 
	{ 
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_flGlowAlpha = flAlpha;
	}

	void SetRenderFlags( int nGlowObjectHandle, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded = bRenderWhenOccluded;
		m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
	}

	void SetFullBloomRender( int nGlowObjectHandle, bool bFullBloomRender, int nStencilTestValue )
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		m_GlowObjectDefinitions[nGlowObjectHandle].m_bFullBloomRender = bFullBloomRender;
		m_GlowObjectDefinitions[nGlowObjectHandle].m_nFullBloomStencilTestValue = nStencilTestValue;
	}

	bool IsRenderingWhenOccluded( int nGlowObjectHandle ) const
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded;
	}
	
	bool IsRenderingWhenUnoccluded( int nGlowObjectHandle ) const
	{
		Assert( !m_GlowObjectDefinitions[nGlowObjectHandle].IsUnused() );
		return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded;
	}

	void RenderGlowEffects( const CViewSetup *pSetup, int nSplitScreenSlot );

private:

	void RenderGlowModels( const CViewSetup *pSetup, int nSplitScreenSlot, CMatRenderContextPtr &pRenderContext );
	void ApplyEntityGlowEffects( const CViewSetup *pSetup, int nSplitScreenSlot, CMatRenderContextPtr &pRenderContext, float flBloomScale, int x, int y, int w, int h );

	struct GlowObjectDefinition_t
	{
		bool ShouldDraw( int nSlot ) const { return m_pEntity && ( m_nSplitScreenSlot == GLOW_FOR_ALL_SPLIT_SCREEN_SLOTS || m_nSplitScreenSlot == nSlot ) && ( m_bRenderWhenOccluded || m_bRenderWhenUnoccluded )/* && m_pEntity->ShouldDraw()*/; }
		bool IsUnused() const { return m_nNextFreeSlot != GlowObjectDefinition_t::ENTRY_IN_USE; }
		void DrawModel();

		C_BaseEntity* m_pEntity;
		Vector m_vGlowColor;
		float m_flGlowAlpha;

		bool m_bRenderWhenOccluded;
		bool m_bRenderWhenUnoccluded;
		bool m_bFullBloomRender;
		int m_nFullBloomStencilTestValue; // only render full bloom objects if stencil is equal to this value (value of -1 implies no stencil test)
		int m_nSplitScreenSlot;

		// Linked list of free slots
		int m_nNextFreeSlot;

		// Special values for GlowObjectDefinition_t::m_nNextFreeSlot
		static const int END_OF_FREE_LIST = -1;
		static const int ENTRY_IN_USE = -2;
	};

	CUtlVector< GlowObjectDefinition_t > m_GlowObjectDefinitions;
	int m_nFirstFreeSlot;
};

extern CGlowObjectManager g_GlowObjectManager;

class CGlowObject
{
public:
	CGlowObject( C_BaseEntity *pEntity, const Vector &vGlowColor = Vector( 1.0f, 1.0f, 1.0f ), float flGlowAlpha = 1.0f, bool bRenderWhenOccluded = false, bool bRenderWhenUnoccluded = false, int nSplitScreenSlot = GLOW_FOR_ALL_SPLIT_SCREEN_SLOTS )
	{
		m_nGlowObjectHandle = g_GlowObjectManager.RegisterGlowObject( pEntity, vGlowColor, flGlowAlpha, bRenderWhenOccluded, bRenderWhenUnoccluded, nSplitScreenSlot );
	}

	~CGlowObject()
	{
		g_GlowObjectManager.UnregisterGlowObject( m_nGlowObjectHandle );
	}

	void SetEntity( C_BaseEntity *pEntity )
	{
		g_GlowObjectManager.SetEntity( m_nGlowObjectHandle, pEntity );
	}

	void SetColor( const Vector &vGlowColor )
	{
		g_GlowObjectManager.SetColor( m_nGlowObjectHandle, vGlowColor );
	}

	void SetAlpha( float flAlpha )
	{
		g_GlowObjectManager.SetAlpha( m_nGlowObjectHandle, flAlpha );
	}

	void SetRenderFlags( bool bRenderWhenOccluded, bool bRenderWhenUnoccluded )
	{
		g_GlowObjectManager.SetRenderFlags( m_nGlowObjectHandle, bRenderWhenOccluded, bRenderWhenUnoccluded );
	}

	void SetFullBloomRender( bool bFullBloomRender, int nStencilTestValue = -1 )
	{
		return g_GlowObjectManager.SetFullBloomRender( m_nGlowObjectHandle, bFullBloomRender, nStencilTestValue );
	}

	bool IsRenderingWhenOccluded() const
	{
		return g_GlowObjectManager.IsRenderingWhenOccluded( m_nGlowObjectHandle );
	}

	bool IsRenderingWhenUnoccluded() const
	{
		return g_GlowObjectManager.IsRenderingWhenUnoccluded( m_nGlowObjectHandle );
	}

	bool IsRendering() const
	{
		return IsRenderingWhenOccluded() || IsRenderingWhenUnoccluded();
	}

	// Add more accessors/mutators here as needed

private:
	int m_nGlowObjectHandle;

	// Assignment & copy-construction disallowed
	CGlowObject( const CGlowObject &other );
	CGlowObject& operator=( const CGlowObject &other );
};

//
//------------------------------------glow_outline_effect.cpp--------------------------------------------------//
//

#include "model_types.h"
#include "shaderapi/ishaderapi.h"
#include "materialsystem/imaterialvar.h"
#include "view_shared.h"

//OwnFix
#include "materialsystem/itexture.h"
#include "../L4DVersusTool.h"
#include "iclientrenderable.h"

//+Replaced Files from ASW:
//E:\Senny\Other\Left 4 dead_devs\L4D_Server_plugin\L4D_SDK\public\materialsystem\ImaterialSystem.h
//E:\Senny\Other\Left 4 dead_devs\L4D_Server_plugin\L4D_SDK\public\Modeltypes.h
//E:\Senny\Other\Left 4 dead_devs\L4D_Server_plugin\L4D_SDK\game\client\c_baseanimating.h + cpp
//E:\Senny\Other\Left 4 dead_devs\L4D_Server_plugin\L4D_SDK\game\client\c_baseentity.h + cpp
//E:\Senny\Other\Left 4 dead_devs\L4D_Server_plugin\L4D_SDK\game\client\c_baseplayer.h + cpp


#define FULL_FRAME_TEXTURE "_rt_FullFrameFB"

ConVar glow_outline_effect_enable("l4d_glow_outline_effect_enable", "1", 0, "Enable entity outline glow effects.");
ConVar glow_outline_effect_width("l4d_glow_outline_width", "6.0f", 0, "Width of glow outline effect in screen space.");

CGlowObjectManager g_GlowObjectManager;

void CGlowObjectManager::RenderGlowEffects(const CViewSetup *pSetup, int nSplitScreenSlot)
{
	if (glow_outline_effect_enable.GetBool())
	{
		CMatRenderContextPtr pRenderContext(material);

		int nX, nY, nWidth, nHeight;
		pRenderContext->GetViewport(nX, nY, nWidth, nHeight);

		PIXEvent _pixEvent(pRenderContext, "EntityGlowEffects");
		ApplyEntityGlowEffects(pSetup, nSplitScreenSlot, pRenderContext, glow_outline_effect_width.GetFloat(), nX, nY, nWidth, nHeight);
	}
}


static void SetRenderTargetAndViewPort(ITexture *rt, int w, int h)
{
	CMatRenderContextPtr pRenderContext(material);
	pRenderContext->SetRenderTarget(rt);
	pRenderContext->Viewport(0, 0, w, h);
}

// *** Keep in sync with matsys_interface.cpp, where the texture is declared ***
// Resolution for glow target chosen to be the largest that we can fit in EDRAM after 720p color/depth textures.		
#define GLOW_360_RT_WIDTH ( MIN( 1120, pSetup->width ) )
#define GLOW_360_RT_HEIGHT ( MIN( 624, pSetup->height ) )

void CGlowObjectManager::RenderGlowModels(const CViewSetup *pSetup, int nSplitScreenSlot, CMatRenderContextPtr &pRenderContext)
{
	//==========================================================================================//
	// This renders solid pixels with the correct coloring for each object that needs the glow.	//
	// After this function returns, this image will then be blurred and added into the frame	//
	// buffer with the objects stenciled out.													//
	//==========================================================================================//
	pRenderContext->PushRenderTargetAndViewport();

	// Save modulation color and blend
	Vector vOrigColor;
	renderView->GetColorModulation(vOrigColor.Base());
	float flOrigBlend = renderView->GetBlend();

	ITexture *pRtFullFrame = material->FindTexture(FULL_FRAME_TEXTURE, TEXTURE_GROUP_RENDER_TARGET);

	if (IsX360())
	{
		ITexture *pRtGlowTexture360 = material->FindTexture("_rt_Glows360", TEXTURE_GROUP_RENDER_TARGET);

		SetRenderTargetAndViewPort(pRtGlowTexture360, GLOW_360_RT_WIDTH, GLOW_360_RT_HEIGHT);
	}
	else
	{
		SetRenderTargetAndViewPort(pRtFullFrame, pSetup->width, pSetup->height);
	}

	pRenderContext->ClearColor3ub(0, 0, 0);
	pRenderContext->ClearBuffers(true, false, false);

	// Set override material for glow color
	IMaterial *pMatGlowColor = NULL;

	pMatGlowColor = material->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER, true);

	//==================//
	// Draw the objects //
	//==================//
	for (int i = 0; i < m_GlowObjectDefinitions.Count(); ++i)
	{
		if (m_GlowObjectDefinitions[i].IsUnused() || !m_GlowObjectDefinitions[i].ShouldDraw(nSplitScreenSlot))
			continue;

		studio_render->ForcedMaterialOverride(pMatGlowColor);

		if (m_GlowObjectDefinitions[i].m_bFullBloomRender)
		{

			// Disabled because stencil test on off-screen buffers doesn't work with MSAA on.
			// Also, the normal model render does not seem to work on the off-screen buffer

			//g_pStudioRender->ForcedMaterialOverride( NULL );

// 			ShaderStencilState_t stencilState;
// 			stencilState.m_bEnable = true;
// 			stencilState.m_nReferenceValue = m_GlowObjectDefinitions[i].m_nFullBloomStencilTestValue;
// 			stencilState.m_nTestMask = 0xFF;
// 			stencilState.m_CompareFunc = SHADER_STENCILFUNC_EQUAL;
// 			stencilState.m_PassOp = SHADER_STENCILOP_KEEP;
// 			stencilState.m_FailOp = SHADER_STENCILOP_KEEP;
// 			stencilState.m_ZFailOp = SHADER_STENCILOP_KEEP;
// 
// 			pRenderContext->SetStencilState( stencilState );
		}
		else
		{

			// Disabled because stencil test on off-screen buffers doesn't work with MSAA on
			// Most features still work, but some (e.g. partial occlusion) don't
// 			ShaderStencilState_t stencilState;
// 			stencilState.m_bEnable = true;
// 			stencilState.m_nReferenceValue = 1;
// 			stencilState.m_nTestMask = 0x1;
// 			stencilState.m_CompareFunc = SHADER_STENCILFUNC_EQUAL;
// 			stencilState.m_PassOp = SHADER_STENCILOP_KEEP;
// 			stencilState.m_FailOp = SHADER_STENCILOP_KEEP;
// 			stencilState.m_ZFailOp = SHADER_STENCILOP_KEEP;
// 
// 			pRenderContext->SetStencilState( stencilState );
		}

		renderView->SetBlend(m_GlowObjectDefinitions[i].m_flGlowAlpha);
		Vector vGlowColor = m_GlowObjectDefinitions[i].m_vGlowColor * m_GlowObjectDefinitions[i].m_flGlowAlpha;
		renderView->SetColorModulation(&vGlowColor[0]); // This only sets rgb, not alpha

		m_GlowObjectDefinitions[i].DrawModel();
	}

	studio_render->ForcedMaterialOverride(NULL);
	renderView->SetColorModulation(vOrigColor.Base());
	renderView->SetBlend(flOrigBlend);

	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	pRenderContext->SetStencilState(stencilStateDisable);

	if (IsX360())
	{
		Rect_t rect;
		rect.x = rect.y = 0;
		rect.width = GLOW_360_RT_WIDTH;
		rect.height = GLOW_360_RT_HEIGHT;

		pRenderContext->CopyRenderTargetToTextureEx(pRtFullFrame, 0, &rect, &rect);
	}

	pRenderContext->PopRenderTargetAndViewport();
}

void CGlowObjectManager::ApplyEntityGlowEffects(const CViewSetup *pSetup, int nSplitScreenSlot, CMatRenderContextPtr &pRenderContext, float flBloomScale, int x, int y, int w, int h)
{
	static bool s_bFirstPass = true;

	//=======================================================//
	// Render objects into stencil buffer					 //
	//=======================================================//

	// Set override shader to the same simple shader we use to render the glow models
	IMaterial *pMatGlowColor = material->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER, true);
	studio_render->ForcedMaterialOverride(pMatGlowColor);

	ShaderStencilState_t stencilStateDisable;
	stencilStateDisable.m_bEnable = false;
	float flSavedBlend = renderView->GetBlend();

	// Set alpha to 0 so we don't touch any color pixels
	renderView->SetBlend(0.0f);
	pRenderContext->OverrideDepthEnable(true, false);

	RenderableInstance_t instance;
	instance.m_nAlpha = 255;

	int iNumGlowObjects = 0;

	for (int i = 0; i < m_GlowObjectDefinitions.Count(); ++i)
	{
		if (m_GlowObjectDefinitions[i].IsUnused() || !m_GlowObjectDefinitions[i].ShouldDraw(nSplitScreenSlot))
			continue;

		// Full bloom rendered objects should not be stenciled out here
		if (m_GlowObjectDefinitions[i].m_bFullBloomRender)
		{
			++iNumGlowObjects;
			continue;
		}

		if (m_GlowObjectDefinitions[i].m_bRenderWhenOccluded || m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded)
		{
			if (m_GlowObjectDefinitions[i].m_bRenderWhenOccluded && m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded)
			{
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 1;
				stencilState.m_CompareFunc = SHADER_STENCILFUNC_ALWAYS;
				stencilState.m_PassOp = SHADER_STENCILOP_SET_TO_REFERENCE;
				stencilState.m_FailOp = SHADER_STENCILOP_KEEP;
				stencilState.m_ZFailOp = SHADER_STENCILOP_SET_TO_REFERENCE;

				pRenderContext->SetStencilState(stencilState);

				m_GlowObjectDefinitions[i].DrawModel();
			}
			else if (m_GlowObjectDefinitions[i].m_bRenderWhenOccluded)
			{
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 1;
				stencilState.m_CompareFunc = SHADER_STENCILFUNC_ALWAYS;
				stencilState.m_PassOp = SHADER_STENCILOP_KEEP;
				stencilState.m_FailOp = SHADER_STENCILOP_KEEP;
				stencilState.m_ZFailOp = SHADER_STENCILOP_SET_TO_REFERENCE;

				pRenderContext->SetStencilState(stencilState);

				m_GlowObjectDefinitions[i].DrawModel();
			}
			else if (m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded)
			{
				ShaderStencilState_t stencilState;
				stencilState.m_bEnable = true;
				stencilState.m_nReferenceValue = 2;
				stencilState.m_nTestMask = 0x1;
				stencilState.m_nWriteMask = 0x3;
				stencilState.m_CompareFunc = SHADER_STENCILFUNC_EQUAL;
				stencilState.m_PassOp = SHADER_STENCILOP_INCREMENT_CLAMP;
				stencilState.m_FailOp = SHADER_STENCILOP_KEEP;
				stencilState.m_ZFailOp = SHADER_STENCILOP_SET_TO_REFERENCE;

				pRenderContext->SetStencilState(stencilState);

				m_GlowObjectDefinitions[i].DrawModel();
			}
		}

		iNumGlowObjects++;
	}

	// Need to do a 2nd pass to warm stencil for objects which are rendered only when occluded
	for (int i = 0; i < m_GlowObjectDefinitions.Count(); ++i)
	{
		if (m_GlowObjectDefinitions[i].IsUnused() || !m_GlowObjectDefinitions[i].ShouldDraw(nSplitScreenSlot))
			continue;

		// Full bloom rendered objects should not be stenciled out here
		if (m_GlowObjectDefinitions[i].m_bFullBloomRender)
			continue;

		if (m_GlowObjectDefinitions[i].m_bRenderWhenOccluded && !m_GlowObjectDefinitions[i].m_bRenderWhenUnoccluded)
		{
			ShaderStencilState_t stencilState;
			stencilState.m_bEnable = true;
			stencilState.m_nReferenceValue = 2;
			stencilState.m_CompareFunc = SHADER_STENCILFUNC_ALWAYS;
			stencilState.m_PassOp = SHADER_STENCILOP_SET_TO_REFERENCE;
			stencilState.m_FailOp = SHADER_STENCILOP_KEEP;
			stencilState.m_ZFailOp = SHADER_STENCILOP_KEEP;
			pRenderContext->SetStencilState(stencilState);

			m_GlowObjectDefinitions[i].DrawModel();
		}
	}

	pRenderContext->OverrideDepthEnable(false, false);
	renderView->SetBlend(flSavedBlend);
	pRenderContext->SetStencilState(stencilStateDisable);
	studio_render->ForcedMaterialOverride(NULL);

	// If there aren't any objects to glow, don't do all this other stuff
	// this fixes a bug where if there are glow objects in the list, but none of them are glowing,
	// the whole screen blooms.
	if (iNumGlowObjects <= 0)
		return;

	//=============================================
	// Render the glow colors to _rt_FullFrameFB 
	//=============================================
	{
		PIXEvent pixEvent(pRenderContext, "RenderGlowModels");
		RenderGlowModels(pSetup, nSplitScreenSlot, pRenderContext);
	}

	//===================================
	// Setup state for downsample/bloom
	//===================================

#if defined( _X360 )
	pRenderContext->PushVertexShaderGPRAllocation(16); // Max out pixel shader threads
#endif

	pRenderContext->PushRenderTargetAndViewport();

	// Get viewport
	int nSrcWidth = pSetup->width;
	int nSrcHeight = pSetup->height;
	int nViewportX, nViewportY, nViewportWidth, nViewportHeight;
	pRenderContext->GetViewport(nViewportX, nViewportY, nViewportWidth, nViewportHeight);

	// Get material and texture pointers
	IMaterial *pMatDownsample = material->FindMaterial("dev/glow_downsample", TEXTURE_GROUP_OTHER, true);
	IMaterial *pMatBlurX = material->FindMaterial("dev/glow_blur_x", TEXTURE_GROUP_OTHER, true);
	IMaterial *pMatBlurY = material->FindMaterial("dev/glow_blur_y", TEXTURE_GROUP_OTHER, true);

	ITexture *pRtFullFrame = material->FindTexture(FULL_FRAME_TEXTURE, TEXTURE_GROUP_RENDER_TARGET);
	ITexture *pRtQuarterSize0 = material->FindTexture("_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET);
	ITexture *pRtQuarterSize1 = material->FindTexture("_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET);

	//============================================
	// Downsample _rt_FullFrameFB to _rt_SmallFB0
	//============================================

	// First clear the full target to black if we're not going to touch every pixel
	if ((pRtQuarterSize0->GetActualWidth() != (pSetup->width / 4)) || (pRtQuarterSize0->GetActualHeight() != (pSetup->height / 4)))
	{
		SetRenderTargetAndViewPort(pRtQuarterSize0, pRtQuarterSize0->GetActualWidth(), pRtQuarterSize0->GetActualHeight());
		pRenderContext->ClearColor3ub(0, 0, 0);
		pRenderContext->ClearBuffers(true, false, false);
	}

	// Set the viewport
	SetRenderTargetAndViewPort(pRtQuarterSize0, pSetup->width / 4, pSetup->height / 4);

	IMaterialVar *pbloomexpvar = pMatDownsample->FindVar("$bloomexp", 0L);
	if (pbloomexpvar != NULL)
	{
		pbloomexpvar->SetFloatValue(2.5f);
	}

	IMaterialVar *pbloomsaturationvar = pMatDownsample->FindVar("$bloomsaturation", 0L);
	if (pbloomsaturationvar != NULL)
	{
		pbloomsaturationvar->SetFloatValue(1.0f);
	}

	// note the -2's below. Thats because we are downsampling on each axis and the shader
	// accesses pixels on both sides of the source coord
	int nFullFbWidth = nSrcWidth;
	int nFullFbHeight = nSrcHeight;
	if (IsX360())
	{
		nFullFbWidth = GLOW_360_RT_WIDTH;
		nFullFbHeight = GLOW_360_RT_HEIGHT;
	}
	pRenderContext->DrawScreenSpaceRectangle(pMatDownsample, 0, 0, nSrcWidth / 4, nSrcHeight / 4,
		0, 0, nFullFbWidth - 4, nFullFbHeight - 4,
		pRtFullFrame->GetActualWidth(), pRtFullFrame->GetActualHeight());

	if (IsX360())
	{
		// Need to reset viewport to full size so we can also copy the cleared black pixels around the border
		SetRenderTargetAndViewPort(pRtQuarterSize0, pRtQuarterSize0->GetActualWidth(), pRtQuarterSize0->GetActualHeight());
		pRenderContext->CopyRenderTargetToTextureEx(pRtQuarterSize0, 0, NULL, NULL);
	}

	//============================//
	// Guassian blur x rt0 to rt1 //
	//============================//

	// First clear the full target to black if we're not going to touch every pixel
	if (s_bFirstPass || (pRtQuarterSize1->GetActualWidth() != (pSetup->width / 4)) || (pRtQuarterSize1->GetActualHeight() != (pSetup->height / 4)))
	{
		// On the first render, this viewport may require clearing
		s_bFirstPass = false;
		SetRenderTargetAndViewPort(pRtQuarterSize1, pRtQuarterSize1->GetActualWidth(), pRtQuarterSize1->GetActualHeight());
		pRenderContext->ClearColor3ub(0, 0, 0);
		pRenderContext->ClearBuffers(true, false, false);
	}

	// Set the viewport
	SetRenderTargetAndViewPort(pRtQuarterSize1, pSetup->width / 4, pSetup->height / 4);

	pRenderContext->DrawScreenSpaceRectangle(pMatBlurX, 0, 0, nSrcWidth / 4, nSrcHeight / 4,
		0, 0, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
		pRtQuarterSize0->GetActualWidth(), pRtQuarterSize0->GetActualHeight());

	if (IsX360())
	{
		pRenderContext->CopyRenderTargetToTextureEx(pRtQuarterSize1, 0, NULL, NULL);
	}

	//============================//
	// Gaussian blur y rt1 to rt0 //
	//============================//
	SetRenderTargetAndViewPort(pRtQuarterSize0, pSetup->width / 4, pSetup->height / 4);
	IMaterialVar *pBloomAmountVar = pMatBlurY->FindVar("$bloomamount", NULL);
	pBloomAmountVar->SetFloatValue(flBloomScale);
	pRenderContext->DrawScreenSpaceRectangle(pMatBlurY, 0, 0, nSrcWidth / 4, nSrcHeight / 4,
		0, 0, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
		pRtQuarterSize1->GetActualWidth(), pRtQuarterSize1->GetActualHeight());

	if (IsX360())
	{
		pRenderContext->CopyRenderTargetToTextureEx(pRtQuarterSize1, 0, NULL, NULL); // copy to rt1 instead of rt0 because rt1 has linear reads enabled and works more easily with screenspace_general to fix 360 bloom issues
	}

	// Pop RT
	pRenderContext->PopRenderTargetAndViewport();

	{
		//=======================================================================================================//
		// At this point, pRtQuarterSize0 is filled with the fully colored glow around everything as solid glowy //
		// blobs. Now we need to stencil out the original objects by only writing pixels that have no            //
		// stencil bits set in the range we care about.                                                          //
		//=======================================================================================================//
		IMaterial *pMatHaloAddToScreen = material->FindMaterial("dev/halo_add_to_screen", TEXTURE_GROUP_OTHER, true);

		// Do not fade the glows out at all (weight = 1.0)
		IMaterialVar *pDimVar = pMatHaloAddToScreen->FindVar("$C0_X", NULL);
		pDimVar->SetFloatValue(1.0f);

		ShaderStencilState_t stencilState;
		stencilState.m_bEnable = true;
		stencilState.m_nWriteMask = 0x0; // We're not changing stencil
		stencilState.m_nTestMask = 0x3;
		stencilState.m_nReferenceValue = 0x0;
		stencilState.m_CompareFunc = SHADER_STENCILFUNC_EQUAL;
		stencilState.m_PassOp = SHADER_STENCILOP_KEEP;
		stencilState.m_FailOp = SHADER_STENCILOP_KEEP;
		stencilState.m_ZFailOp = SHADER_STENCILOP_KEEP;
		pRenderContext->SetStencilState(stencilState);

		// Draw quad
		pRenderContext->DrawScreenSpaceRectangle(pMatHaloAddToScreen, 0, 0, nViewportWidth, nViewportHeight,
			0.0f, -0.5f, nSrcWidth / 4 - 1, nSrcHeight / 4 - 1,
			pRtQuarterSize1->GetActualWidth(),
			pRtQuarterSize1->GetActualHeight());

		// Disable stencil
		pRenderContext->SetStencilState(stencilStateDisable);
	}

#if defined( _X360 )
	pRenderContext->PopVertexShaderGPRAllocation();
#endif
}

void CGlowObjectManager::GlowObjectDefinition_t::DrawModel()
{
	RenderableInstance_t instance;
	instance.m_nAlpha = (uint8)(m_flGlowAlpha * 255.0f);

	/*m_pEntity->DrawModel(STUDIO_RENDER | STUDIO_SKIP_FLEXES | STUDIO_DONOTMODIFYSTENCILSTATE | STUDIO_NOLIGHTING_OR_CUBEMAP | STUDIO_SKIP_DECALS, instance);
	C_BaseEntity *pAttachment = m_pEntity->FirstMoveChild();

	while (pAttachment != NULL)
	{
		if (pAttachment->ShouldDraw())
		{
			pAttachment->DrawModel(STUDIO_RENDER | STUDIO_SKIP_FLEXES | STUDIO_DONOTMODIFYSTENCILSTATE | STUDIO_NOLIGHTING_OR_CUBEMAP | STUDIO_SKIP_DECALS, instance);
		}
		pAttachment = pAttachment->NextMovePeer();
	}*/
}
