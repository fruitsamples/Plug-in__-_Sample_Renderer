/****************************************************************************** **																			 ** ** 	Module:		SR.c													 ** ** 																		 ** ** 																		 ** ** 	Purpose: 	Generic sample renderer routines		 				 ** ** 																		 ** ** 																		 ** ** 																		 ** ** 	Copyright (C) 1996 Apple Computer, Inc.  All rights reserved.	 	 ** ** 																		 ** ** 																		 ** *****************************************************************************/  #include <assert.h>#include <stdlib.h>#include <string.h>#include "QD3D.h"#include "QD3DErrors.h"#include "QD3DView.h"#include "QD3DDrawContext.h"#include "QD3DRenderer.h"#include "QD3DExtension.h"#include "QD3DIO.h"#include "SR.h"#include "SR_Rasterizers.h"#include "SR_ConfigData.h"#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSH#include <Resources.h>#include <Aliases.h>#include "SR_MacDialog.h"#elif defined(WINDOW_SYSTEM_WIN32) && WINDOW_SYSTEM_WIN32#include "SR_WinDialog.h"#endif  /*  WINDOW_SYSTEM_MACINTOSH  *//****************************************************************************** **																			 ** **							Forward Declarations							 ** **																			 ** *****************************************************************************/#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSH/* *  Shared library initialization entry point */OSErr SR_Initialize( 	const CFragInitBlock	*initBlock);#elif defined(WINDOW_SYSTEM_WIN32) && WINDOW_SYSTEM_WIN32#endif  /*  WINDOW_SYSTEM_MACINTOSH  *//* *  Shared library exit function */TQ3Status SR_Exit( 	void);/* *  IO Functions */static TQ3RendererObject SR_Read(	TQ3FileObject		file);	static TQ3Status SR_Traverse(	TQ3RendererObject	renderer,	void				*unused,	TQ3ViewObject		view);#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSH/* * Macintosh resource fork functions */static OSErr SR_CreateAliasHandle( 	const CFragInitBlock	*initBlock);	static TQ3Status SR_FreeAliasHandle(	void);#endif  /*  WINDOW_SYSTEM_MACINTOSH  *//****************************************************************************** **																			 ** **									Globals									 ** **																			 ** *****************************************************************************/static TQ3XObjectClass	SRgRendererClass;static TQ3ObjectType	SRgClassType;static unsigned long 	SRgSharedLibrary = NULL;#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSHAliasHandle SRgAliasHandle = NULL;#endif  /*  WINDOW_SYSTEM_MACINTOSH  *//****************************************************************************** **																			 ** **							Sample Renderer routines						 ** **																			 ** *****************************************************************************//*===========================================================================*\ * *	Routine:	SR_IdleProgress() * *	Comments:	Every n primitives, call the view's idle progress method. *\*===========================================================================*/TQ3Status SR_IdleProgress(	TQ3ViewObject	view,	TSRPrivate		*srPrivate){	srPrivate->primitiveCount++;		if (srPrivate->primitiveCount % 10000 == 0) {		return (Q3XView_IdleProgress(view, 0, 0));	} else {		return (kQ3Success);	}}/*===========================================================================*\ * *	Routine:	SR_UpdateRasterFunctions() * *	Comments:	Depending on pixel type, assign rasterization routines for *				the cases of clipped and not clipped (windows). In the *				case of unsupported pixel types, set rasterizers to NULL *				NULL functions. *\*===========================================================================*/TQ3Status SR_UpdateRasterFunctions(    TSRPrivate		*srPrivate){	TQ3XClipMaskState			clipMaskState;	TQ3XDrawRegionDescriptor	*descriptor;		assert(srPrivate != NULL);	descriptor = srPrivate->descriptor;		switch (descriptor->pixelType) {		case kQ3XDevicePixelTypeARGB32	:		case kQ3XDevicePixelTypeRGB32	: {			srPrivate->rasterFunctions[SRcRasterBasic].lineFunction 		= 				SRLine_Rasterize_32;			srPrivate->rasterFunctions[SRcRasterBasic].pointFunction 		= 				SRPoint_Rasterize_32;			srPrivate->rasterFunctions[SRcRasterBasic].markerFunction 		= 				SRMarker_Rasterize_32;			srPrivate->rasterFunctions[SRcRasterBasic].pixmapMarkerFunction	= 				SRPixmapMarker_Rasterize_32;			srPrivate->rasterFunctions[SRcRasterClip].lineFunction 			= 				SRLine_Rasterize_32_WClip;			srPrivate->rasterFunctions[SRcRasterClip].pointFunction 		= 				SRPoint_Rasterize_32_WClip;			srPrivate->rasterFunctions[SRcRasterClip].markerFunction		= 				SRMarker_Rasterize_32_WClip;			srPrivate->rasterFunctions[SRcRasterClip].pixmapMarkerFunction	= 				SRPixmapMarker_Rasterize_32_WClip;			break;		}		case kQ3XDevicePixelTypeIndexed8: {			srPrivate->rasterFunctions[SRcRasterBasic].lineFunction 		= 				SRLine_Rasterize_8;			srPrivate->rasterFunctions[SRcRasterBasic].pointFunction 		= 				SRPoint_Rasterize_8;			srPrivate->rasterFunctions[SRcRasterBasic].markerFunction 		= 				SRMarker_Rasterize_8;			srPrivate->rasterFunctions[SRcRasterBasic].pixmapMarkerFunction = 				SRPixmapMarker_Rasterize_8;			srPrivate->rasterFunctions[SRcRasterClip].lineFunction 			= 				SRLine_Rasterize_8_WClip;			srPrivate->rasterFunctions[SRcRasterClip].pointFunction 		= 				SRPoint_Rasterize_8_WClip;			srPrivate->rasterFunctions[SRcRasterClip].markerFunction 		= 				SRMarker_Rasterize_8_WClip;			srPrivate->rasterFunctions[SRcRasterClip].pixmapMarkerFunction	= 				SRPixmapMarker_Rasterize_8_WClip;						break;		}		default: {			srPrivate->rasterFunctions[SRcRasterBasic].lineFunction 		= 				SRLine_Rasterize_Null;			srPrivate->rasterFunctions[SRcRasterBasic].pointFunction 		= 				SRPoint_Rasterize_Null;			srPrivate->rasterFunctions[SRcRasterClip].markerFunction 		= 				SRMarker_Rasterize_Null;			srPrivate->rasterFunctions[SRcRasterBasic].pixmapMarkerFunction = 				SRPixmapMarker_Rasterize_Null;			srPrivate->rasterFunctions[SRcRasterClip].lineFunction 			= 				SRLine_Rasterize_Null;			srPrivate->rasterFunctions[SRcRasterClip].pointFunction 		= 				SRPoint_Rasterize_Null;			srPrivate->rasterFunctions[SRcRasterClip].markerFunction 		= 				SRMarker_Rasterize_Null;			srPrivate->rasterFunctions[SRcRasterClip].pixmapMarkerFunction 	= 				SRPixmapMarker_Rasterize_Null;			fprintf(stderr, "unsupported pixel depth/type\n");						break;		}	}		/*	 *  Check if we're clipped or not, and set up the current raster function	 *  vector to the appropriate function vector.	 */	if (Q3XDrawRegion_GetClipFlags(			srPrivate->drawRegion, 			&clipMaskState) == kQ3Failure) {		return (kQ3Failure);	}			if (clipMaskState == kQ3XClipMaskFullyExposed) {		srPrivate->currentRasterFunctions = 			&(srPrivate->rasterFunctions[SRcRasterBasic]);	} else {		srPrivate->currentRasterFunctions = 			&(srPrivate->rasterFunctions[SRcRasterClip]);	}		return (kQ3Success);}/*===========================================================================*\ * *	Routine:	SR_New() * *	Comments:	Initialize the private state. *\*===========================================================================*/static TQ3Status SR_New(	TQ3RendererObject 		renderer,	TSRPrivate				*srPrivate,	void					*initData){	UNUSED(renderer);	UNUSED(initData);		assert(renderer != NULL);	assert(srPrivate != NULL);	/*	 *  Zero everything out.	 */	memset(srPrivate, 0, sizeof(*srPrivate));			/*	 *  Set the rasterizers to the "null" rasterization functions.	 */	srPrivate->rasterFunctions[SRcRasterBasic].lineFunction 		= 		SRLine_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterBasic].pointFunction 		= 		SRPoint_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterBasic].markerFunction 		= 		SRMarker_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterBasic].pixmapMarkerFunction = 		SRPixmapMarker_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].lineFunction 			= 		SRLine_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].pointFunction 		= 		SRPoint_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].markerFunction 		= 		SRMarker_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].pixmapMarkerFunction 	= 		SRPixmapMarker_Rasterize_Null;			return (kQ3Success);}/*===========================================================================*\ * *	Routine:	SR_Delete() * *	Comments:	Get rid of anything we have a reference to. *\*===========================================================================*/static void SR_Delete(	TQ3RendererObject	renderer,	TSRPrivate			*srPrivate){	UNUSED(renderer);		assert(renderer != NULL);	assert(srPrivate != NULL);		if (srPrivate->viewHighlightAttributeSet != NULL) {		Q3Object_Dispose(srPrivate->viewHighlightAttributeSet);		srPrivate->viewHighlightAttributeSet = NULL;	}		if (srPrivate->camera != NULL) {		Q3Object_Dispose(srPrivate->camera);		srPrivate->camera = NULL;	}}/*===========================================================================*\ * *	Routine:	SR_StartFrame() * *	Comments:	Called by the View at the start of the frame, set up *				the initial data. *\*===========================================================================*/static TQ3Status SR_StartFrame(	TQ3ViewObject			view,	TSRPrivate 				*srPrivate,	TQ3DrawContextObject	drawContext){	TQ3CameraObject		camera;	TQ3XDrawRegion		drawRegion;	TQ3Boolean			isActive;		UNUSED(view);		assert(view != NULL);	assert(srPrivate != NULL);	assert(drawContext != NULL);			if (Q3View_GetCamera(view, &camera) == kQ3Failure) {		return (kQ3Failure);	}	assert(camera != NULL);		/*	 *  Set primitive counter to zero	 */	srPrivate->primitiveCount = 0;		/*	 *  Initial camera and transform setup	 */	SR_SetupPipelineInitCamera(srPrivate, camera);	Q3Object_Dispose(camera);		/*	 *  Get the current draw context	 */	srPrivate->currentDrawContext = Q3Shared_GetReference(drawContext);	/*	 *  Clear update flags	 */	Q3XDrawContext_ClearValidationFlags(srPrivate->currentDrawContext);	/*	 *  Get the first draw region	 */	Q3XDrawContext_GetDrawRegion(		srPrivate->currentDrawContext,		&drawRegion);		Q3XDrawRegion_IsActive(drawRegion, &isActive);	if (isActive != kQ3True) {		/* 		 *  Need to find the first active region if there is one 		 */		while (drawRegion != NULL) {			Q3XDrawRegion_GetNextRegion(drawRegion, &drawRegion);			if (drawRegion != NULL) {				Q3XDrawRegion_IsActive(drawRegion, &isActive);				if (isActive) {					break;				}			}				}	}		if (drawRegion != NULL) {		Q3XDrawRegion_IsActive(drawRegion, &isActive);		if (isActive) {			srPrivate->drawRegion = drawRegion;		} else {			srPrivate->drawRegion = NULL;		}	} else { 		srPrivate->drawRegion = NULL;	}		srPrivate->descriptor 	= NULL;	srPrivate->image		= NULL;					return (kQ3Success);}/*===========================================================================*\ * *	Routine:	SR_Pass() * *	Comments:	Called at the start of each pass. *\*===========================================================================*/ static TQ3Status SR_Pass(	TQ3ViewObject			view,	TSRPrivate 				*srPrivate,	TQ3CameraObject			camera,	TQ3GroupObject			lightGroup){	TQ3Boolean	isActive;		UNUSED(view);	UNUSED(lightGroup);	UNUSED(camera);		assert(srPrivate != NULL);		if (srPrivate->drawRegion == NULL) {		/*		 *  No active region		 */		return (kQ3Success);	}		/*	 *  Call the "startRegion" function, if there is one.	 */	Q3XDrawRegion_IsActive(srPrivate->drawRegion, &isActive);		if (isActive) {		TQ3XClipMaskState clipMaskState;				/*		 *  Get the descriptor, and tell the draw region you're starting up		 */		if (Q3XDrawRegion_StartAccessToImageBuffer(				srPrivate->drawRegion,				kQ3XDrawRegionServicesClearFlag, 				&srPrivate->descriptor, 				&srPrivate->image) == kQ3Failure) {			goto errorCondition;		}				/*		 *  Find out if you're exposed or not. If not exposed, no need to go 		 *	any further.		 */		Q3XDrawRegion_GetClipFlags(			srPrivate->drawRegion, 			&clipMaskState);		if (clipMaskState == kQ3XClipMaskNotExposed) {			return (kQ3Success);		}				/*		 *  Make sure we've got a raster to draw to		 */		assert(srPrivate->image);		if (srPrivate->image == NULL) {			goto errorCondition;		}		/*		 *  Set up the frustum-to-device transformation		 */		SR_SetupRegionDependentTransformations(srPrivate);		if (SR_UpdateRasterFunctions(srPrivate) == kQ3Failure) {			goto errorCondition;		}	}	return (kQ3Success);errorCondition:	/*	 *  On error, set rasterizer vector entries to NULL functions.	 */	srPrivate->rasterFunctions[SRcRasterBasic].lineFunction 		= 		SRLine_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterBasic].pointFunction 		= 		SRPoint_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterBasic].markerFunction 		= 		SRMarker_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterBasic].pixmapMarkerFunction = 		SRPixmapMarker_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].lineFunction 			= 		SRLine_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].pointFunction 		= 		SRPoint_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].markerFunction 		= 			SRMarker_Rasterize_Null;	srPrivate->rasterFunctions[SRcRasterClip].pixmapMarkerFunction 	= 		SRPixmapMarker_Rasterize_Null;		return (kQ3Failure);}/*===========================================================================*\ * *	Routine:	SR_EndPass() * *	Comments:	Called by the view at the end of each pass. Multiple passes *				will be used if we have multiple draw regions (i.e., there is *				more than one monitor) *\*===========================================================================*/static TQ3ViewStatus SR_EndPass(	TQ3ViewObject 	view,	TSRPrivate 		*srPrivate){	TQ3XDrawRegion		currentRegion;	TQ3Boolean			isActive;		UNUSED(view);		assert(srPrivate != NULL);	if (srPrivate->drawRegion) {		/*		 *  We are done with this region, call the "endRegion" function, 		 *	if there is one.		 */		if (Q3XDrawRegion_IsActive(				srPrivate->drawRegion, 				&isActive) == kQ3Failure) {			return (kQ3ViewStatusError);		}		if (isActive) {			if (Q3XDrawRegion_End(					srPrivate->drawRegion) == kQ3Failure) {				return (kQ3ViewStatusError);			}		}			/* 		 *  If there is more than one device, request another pass 		 *  for the next device 		 */		if (Q3XDrawRegion_GetNextRegion(				srPrivate->drawRegion, 				&currentRegion) == kQ3Failure) {			return (kQ3ViewStatusError);		}				 		while (currentRegion != NULL) {			if (Q3XDrawRegion_IsActive(					currentRegion, 					&isActive) == kQ3Failure) {				return (kQ3ViewStatusError);			}			if (isActive) {				srPrivate->drawRegion = currentRegion;				return (kQ3ViewStatusRetraverse);			}						if (Q3XDrawRegion_GetNextRegion(					currentRegion, 					&currentRegion) == kQ3Failure) {				return (kQ3ViewStatusError);			}		} 	}			SR_SetupPipelineExit(srPrivate);	/*	 *  We are done with the draw context	 */	Q3Object_Dispose(srPrivate->currentDrawContext);	srPrivate->currentDrawContext = NULL;		return (kQ3ViewStatusDone);}/*===========================================================================*\ * *	Routine:	SR_Cancel() * *	Comments:	Called when the view is cancelled. Dispose of the reference *				to the draw context. *\*===========================================================================*/static void SR_Cancel(	TQ3ViewObject 	view,	TSRPrivate		*srPrivate){	UNUSED(view);		assert(srPrivate != NULL);	SR_SetupPipelineExit(srPrivate);	/*	 *  We are done with the draw context	 */	if (srPrivate->currentDrawContext) {		Q3Object_Dispose(srPrivate->currentDrawContext);		srPrivate->currentDrawContext = NULL;	}}/*===========================================================================*\ * *	Routine:	SR_Geometry_MetaHandler() * *	Comments:	Provide entry points for geometric primitive rendering *				functions. *\*===========================================================================*/static TQ3XFunctionPointer SR_Geometry_MetaHandler(	TQ3XMethodType			methodType){	switch (methodType) {		case kQ3GeometryTypeLine: {			return (TQ3XFunctionPointer) SR_Geometry_Line;			break;		}		case kQ3GeometryTypeMarker: {			return (TQ3XFunctionPointer) SR_Geometry_Marker;			break;		}		case kQ3GeometryTypePixmapMarker: {			return (TQ3XFunctionPointer) SR_Geometry_PixmapMarker;			break;		}		case kQ3GeometryTypePoint: {			return (TQ3XFunctionPointer) SR_Geometry_Point;			break;		}		case kQ3GeometryTypeTriangle: {			return (TQ3XFunctionPointer) SR_Geometry_Triangle;			break;		}		default: {			return NULL;			break;		}	}}/*===========================================================================*\ * *	Routine:	SR_Attribute_MetaHandler() * *	Comments:	Provide callbacks for updating attributes the sample *				renderer cares about. Since it's only a simple wireframe *				renderer, we only deal with diffuse color and highlight state. *\*===========================================================================*/static TQ3XFunctionPointer SR_Attribute_MetaHandler(	TQ3XMethodType			methodType){	switch (methodType) {		case kQ3AttributeTypeDiffuseColor: {			return (TQ3XFunctionPointer) SR_Update_DiffuseColor;			break;		}		case kQ3AttributeTypeHighlightState: {			return (TQ3XFunctionPointer) SR_Update_HighlightState;			break;		}		default: {			return NULL;			break;		}	}}/*===========================================================================*\ * *	Routine:	SR_Matrix_MetaHandler() * *	Comments:	Provide callbacks for updating the various transformation *				states. *\*===========================================================================*/static TQ3XFunctionPointer SR_Matrix_MetaHandler(	TQ3XMethodType			methodType){	switch (methodType) {		case kQ3XMethodTypeRendererUpdateMatrixLocalToWorld: {			return ((TQ3XFunctionPointer) SR_Update_LocalToWorldMatrix);			break;		}		case kQ3XMethodTypeRendererUpdateMatrixWorldToFrustum: {			return ((TQ3XFunctionPointer) SR_Update_WorldToFrustumMatrix);			break;		}		case kQ3XMethodTypeRendererUpdateMatrixLocalToFrustum: {			return ((TQ3XFunctionPointer) SR_Update_LocalToFrustumMatrix);			break;		}		default: {			return (NULL);			break;		}	}}/*===========================================================================*\ * *	Routine:	SR_Style_MetaHandler() * *	Comments:	Provide callbacks for updating the styles. *				Only backfacing, hightlight, orientation, and fill style *				are dealt with in this sample renderer. *\*===========================================================================*/static TQ3XFunctionPointer SR_Style_MetaHandler(	TQ3XMethodType			methodType){	switch (methodType) {		case kQ3StyleTypeBackfacing: {			return (TQ3XFunctionPointer) SR_Update_BackfacingStyle;			break;		}		case kQ3StyleTypeHighlight: {			return (TQ3XFunctionPointer) SR_Update_HighlightStyle;			break;		}		case kQ3StyleTypeOrientation: {			return (TQ3XFunctionPointer) SR_Update_OrientationStyle;			break;		}		case kQ3StyleTypeFill: {			return (TQ3XFunctionPointer) SR_Update_FillStyle;			break;		}		default: {			return NULL;			break;		}	}}/*===========================================================================*\ * *	Routine:	SR_MetaHandler() * *	Comments:	Main metahandler. Provides entry point for various *				required and optional renderer functions, and for the *				metahandlers for geometric primitive rendering and *				style, matrix, and attribute updates. *\*===========================================================================*/static TQ3XFunctionPointer SR_MetaHandler(	TQ3XMethodType		methodType){	switch (methodType) {		/* 		 *  Object 		 */		case kQ3XMethodTypeObjectNew: {			return (TQ3XFunctionPointer) SR_New;			break;		}		case kQ3XMethodTypeObjectDelete: {			return (TQ3XFunctionPointer) SR_Delete;			break;		}					/* 		 *  I/O 		 */		case kQ3XMethodTypeObjectRead: {			return (TQ3XFunctionPointer) SR_Read;			break;		}		case kQ3XMethodTypeObjectAttach: {			return (TQ3XFunctionPointer) NULL;			break;		}		case kQ3XMethodTypeObjectTraverse: {			return (TQ3XFunctionPointer) SR_Traverse;			break;		}		case kQ3XMethodTypeObjectWrite: {			return (TQ3XFunctionPointer) NULL;			break;		}					/* 		 *  Renderer 		 */		case kQ3XMethodTypeRendererStartFrame: {			return (TQ3XFunctionPointer) SR_StartFrame;			break;		}		case kQ3XMethodTypeRendererStartPass: {			return (TQ3XFunctionPointer) SR_Pass;			break;		}		case kQ3XMethodTypeRendererEndPass: {			return (TQ3XFunctionPointer) SR_EndPass;			break;		}		case kQ3XMethodTypeRendererCancel: {			return (TQ3XFunctionPointer) SR_Cancel;			break;		}					/* 		 *  Renderer Draw 		 */		case kQ3XMethodTypeRendererSubmitGeometryMetaHandler: {			return (TQ3XFunctionPointer) SR_Geometry_MetaHandler;			break;		}		/* 		 *  Renderer Update 		 */		case kQ3XMethodTypeRendererUpdateStyleMetaHandler: {			return (TQ3XFunctionPointer) SR_Style_MetaHandler;			break;		}		case kQ3XMethodTypeRendererUpdateAttributeMetaHandler: {			return (TQ3XFunctionPointer) SR_Attribute_MetaHandler;			break;		}		case kQ3XMethodTypeRendererUpdateMatrixMetaHandler: {			return (TQ3XFunctionPointer) SR_Matrix_MetaHandler;			break;		}				/*		 *  Modal dialog		 */		case kQ3XMethodTypeRendererModalConfigure: {#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSH			return (TQ3XFunctionPointer) SR_MacModalDialog;#elif defined (WINDOW_SYSTEM_WIN32) && WINDOW_SYSTEM_WIN32			return (TQ3XFunctionPointer) SR_WinModalDialog;#endif  /*  WINDOW_SYSTEM_MACINTOSH  */			break;		}				/*		 *  "isInteractive" control		 */		case kQ3XMethodTypeRendererIsInteractive: {			return (TQ3XFunctionPointer) kQ3True;			break;		}				/*		 *  renderer name string		 */		case kQ3XMethodTypeRendererGetNickNameString: {			return (TQ3XFunctionPointer) SR_GetNameString;			break;		}				/*		 *  Configuration data		 */		case kQ3XMethodTypeRendererGetConfigurationData: {			return (TQ3XFunctionPointer) SR_GetConfigurationData;			break;		}		case kQ3XMethodTypeRendererSetConfigurationData: {			return (TQ3XFunctionPointer) SR_SetConfigurationData;			break;		}		default: {			return NULL;			break;		}	}}/*===========================================================================*\ * *	Routine:	SR_Read() * *	Comments:	 *\*===========================================================================*/static TQ3RendererObject SR_Read(	TQ3FileObject		file){	UNUSED(file);		return Q3Renderer_NewFromType(SRgClassType);}/*===========================================================================*\ * *	Routine:	SR_Traverse() * *	Comments:	 *\*===========================================================================*/static TQ3Status SR_Traverse(	TQ3RendererObject	renderer,	void				*unused,	TQ3ViewObject		view){	UNUSED(unused);	UNUSED(renderer);		return Q3XView_SubmitWriteData(view, 0, NULL, NULL);}/*===========================================================================*\ * *	Routine:	SR_Register() * *	Comments:	Create/register this sample renderer, as a subclass of  *				kQ3SharedTypeRenderer *\*===========================================================================*/static TQ3Status SR_Register(	void){	/*	 *  Create/register the class	 */	SRgRendererClass = 		Q3XObjectHierarchy_RegisterClass(			kQ3SharedTypeRenderer,			&SRgClassType,			"SampleRenderer",			SR_MetaHandler,			NULL,			0,			sizeof(TSRPrivate));	/*	 *  Make sure it worked	 */	if (SRgRendererClass == NULL) {		return (kQ3Failure);	}	return (kQ3Success);}#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSH/*===========================================================================*\ * *	Routine:	SR_Initialize() * *	Comments:	This is the initialization routine called by the shared library *				manager.  This function passes SR_Register to QuickDraw 3D *\*===========================================================================*/OSErr SR_Initialize( 	const CFragInitBlock	*initBlock){	TQ3XSharedLibraryInfo	sharedLibraryInfo;	OSErr					err = noErr;		sharedLibraryInfo.registerFunction 	= SR_Register;	sharedLibraryInfo.sharedLibrary 	= (unsigned long)												initBlock->connectionID;													Q3XSharedLibrary_Register(&sharedLibraryInfo);		SRgSharedLibrary = (unsigned long)initBlock->connectionID;		err = SR_CreateAliasHandle(initBlock);		return (err);}#endif#if defined(WINDOW_SYSTEM_WIN32) && WINDOW_SYSTEM_WIN32HINSTANCE	hinstMyDLL = NULL;/*===========================================================================*\ * *	Routine:	DllMain() * *	Comments:	The Win32 extension entry point *\*===========================================================================*/BOOL WINAPI DllMain(	HINSTANCE	hinstDLL,	DWORD		fdwReason,	LPVOID		lpvReserved){	TQ3XSharedLibraryInfo	sharedLibraryInfo;		if (fdwReason == DLL_PROCESS_ATTACH) {		hinstMyDLL = hinstDLL;		sharedLibraryInfo.registerFunction = SR_Register;		sharedLibraryInfo.sharedLibrary = (unsigned long)hinstDLL;		if (Q3XSharedLibrary_Register(&sharedLibraryInfo) == kQ3Success) {			return TRUE;		} else {			return FALSE;		}	}		if (fdwReason == DLL_PROCESS_DETACH) {		Q3XSharedLibrary_Unregister((unsigned long)hinstDLL);	}	return (TRUE);}#endif /* WINDOW_SYSTEM_WIN32 *//*===========================================================================*\ * *	Routine:	SR_Exit() * *	Comments:	Called on exiting from QD3D, this function unregisters this *				plug-in renderer. *\*===========================================================================*/TQ3Status SR_Exit( 	void){	if (SRgSharedLibrary != NULL) {		Q3XSharedLibrary_Unregister(SRgSharedLibrary);		SRgSharedLibrary = NULL;	}	#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSH	SR_FreeAliasHandle();#endif  /*  WINDOW_SYSTEM_MACINTOSH  */		return (kQ3Success);}#if defined(WINDOW_SYSTEM_MACINTOSH) && WINDOW_SYSTEM_MACINTOSH/*===========================================================================*\ * *	Routine:	SR_CreateAliasHandle() * *	Comments:	 *\*===========================================================================*/static OSErr SR_CreateAliasHandle( 	const CFragInitBlock	*initBlock){		OSErr		err = noErr;		if (initBlock->fragLocator.where == kDataForkCFragLocator) {				err = NewAlias(					NULL, 					initBlock->fragLocator.u.onDisk.fileSpec, 					&SRgAliasHandle);	}		return (err);}/*===========================================================================*\ * *	Routine:	SR_FreeAliasHandle() * *	Comments:	 *\*===========================================================================*/static TQ3Status SR_FreeAliasHandle(	void){	if (SRgAliasHandle != NULL) {		DisposeHandle((Handle) SRgAliasHandle);	}		return (kQ3Success);}#endif  /*  WINDOW_SYSTEM_MACINTOSH  */