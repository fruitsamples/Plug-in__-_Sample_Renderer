/****************************************************************************** **																			 ** ** 	Module:		SR_Point.c												 ** ** 																		 ** ** 																		 ** ** 	Purpose: 	Point routines			 								 ** ** 																		 ** ** 																		 ** ** 																		 ** ** 	Copyright (C) 1996 Apple Computer, Inc.  All rights reserved.		 ** ** 																		 ** ** 																		 ** *****************************************************************************/#include <assert.h>#include "QD3D.h"#include "SR.h"/*===========================================================================*\ * *	Routine:	SR_Geometry_Point() * *	Comments:	 *\*===========================================================================*/TQ3Status SR_Geometry_Point(	TQ3ViewObject 			view, 	TSRPrivate 				*srPrivate,	TQ3GeometryObject 		point,	const TQ3PointData		*pointData){	TQ3Boolean				highlightState;	TQ3XClipMaskState		clipMaskState;	TQ3ColorRGB				color;		UNUSED(point);	assert(view	 		!= NULL);	assert(srPrivate	!= NULL);	assert(pointData	!= NULL);		/*	 *  Call the application's idle progress method, via the view.  If	 *  the app's method returns kQ3Failure, then we don't go on.	 */	if (SR_IdleProgress(view, srPrivate) == kQ3Failure) {		return (kQ3Success);	}	if (srPrivate->drawRegion == NULL) {		return (kQ3Success);	}	/*	 *  Find out if we're clipped out or not. No reason to go any	 *  further if the region is obscured or entirely off-screen. 	 */	Q3XDrawRegion_GetClipFlags(srPrivate->drawRegion, &clipMaskState);	if (clipMaskState == kQ3XClipMaskNotExposed) {		return (kQ3Failure);	}	/*	 *  Lazy-evaluate the various transforms for the pipeline	 */	if (SR_UpdatePipeline(srPrivate) == kQ3Failure) {		return (kQ3Failure);	}		/*	 *  Highlight state and  color are from the view, unless	 *  overridden by the lineAttributeSet	 */	highlightState	= srPrivate->viewHighlightState;	color 			= srPrivate->viewDiffuseColor;	/*	 *  Check if we have a point attribute set.	 *  If so, then see if we can get a color and highlight state	 *  out of it. 	 */	if (pointData->pointAttributeSet != NULL) {		TQ3XAttributeMask	attributeMask;				attributeMask = Q3XAttributeSet_GetMask(pointData->pointAttributeSet);				if (attributeMask & kQ3XAttributeMaskDiffuseColor) {			Q3AttributeSet_Get(				pointData->pointAttributeSet, 				kQ3AttributeTypeDiffuseColor, 				&color);		}		if (attributeMask & kQ3XAttributeMaskHighlightState) {			Q3AttributeSet_Get(				pointData->pointAttributeSet, 				kQ3AttributeTypeHighlightState, 				&highlightState);		}	}	/*	 *  If we're highlighting, then see if we can get a highlight color	 *  out of the view's attribute set. Use that as the color, if it's there.	 */	if ((highlightState == kQ3True) &&		(srPrivate->viewHighlightAttributeSet != NULL)) {		TQ3XAttributeMask	attributeMask;				attributeMask = Q3XAttributeSet_GetMask(							srPrivate->viewHighlightAttributeSet);									if (attributeMask & kQ3XAttributeMaskDiffuseColor) {			Q3AttributeSet_Get( 				srPrivate->viewHighlightAttributeSet, 				kQ3AttributeTypeDiffuseColor, 				&color);		}	}		if (SR_PointPipe(			srPrivate, 			(TQ3Point3D *) &pointData->point, 			1, 			sizeof(TQ3Vertex3D),			&color,			NULL,			DO_POLYLINE) == kQ3Failure) {		return (kQ3Failure);	}				   	return (kQ3Success);}