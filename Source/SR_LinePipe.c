/****************************************************************************** **																			 ** ** 	Module:		SR_LinePipe.c										     ** ** 																		 ** ** 																		 ** ** 	Purpose: 	Sample Renderer polygon edge pipeline code 				 ** ** 																		 ** ** 																		 ** ** 																		 ** ** 	Copyright (C) 1996 Apple Computer, Inc.  All rights reserved.	 	 ** ** 																		 ** ** 																		 ** *****************************************************************************/#include <stdlib.h>#include <assert.h>#include "QD3D.h"#include "QD3DErrors.h"#include "QD3DMath.h"#include "SR_Math.h"#include "SR.h"#include "SR_ClipUtilities.h"/*===========================================================================*\ * *	Routine:	SR_LinePipe() * *	Comments:	Handles lines as well as triangles. *\*===========================================================================*/TQ3Status SR_LinePipe(	TSRPrivate 			*srPrivate, 	TQ3Point3D			*localVertices,	long				numVertices,	long				sizeOfLocalVertices,	TQ3ColorRGB			*color,	TQ3Vector3D			*normal,	long				mode){	TQ3Status			status;	unsigned long		i;	TQ3RationalPoint4D	*deviceVertices 		= NULL;	TQ3RationalPoint4D	*clippedVertices 		= NULL;	TQ3RationalPoint4D	*renderVertices 		= NULL;	long				clipFound, allOut;	unsigned long		*clipFlags 				= NULL;	long				*clippedVerticesFlags 	= NULL;	LineFunction2D		lineFunction;			/*	 *  Get the appropriate rasterization function for the lines	 *  (depth, whether window is clipped or not, etc.	 */	lineFunction = ((TSRRasterFunctions *)						(srPrivate->currentRasterFunctions))->lineFunction;		/*	 *  If we're in "DO_POLYGON" mode, then we have to consider whether	 *  we need to cull or not.	 */	if (mode == DO_POLYGON) {		assert(normal != NULL);				/*		 *  We will attempt to do culling if the rank of the upper-left 3x3 		 *  submatrix of the local-to-world matrix is at least 2.  A rank of 3 		 *  means that volumes are transformed into volumes.  A rank of 2 means 		 *	that volumes are transformed onto a plane.  For a rank of 1 (line) or 		 *	0 (point), we bypass the culling stage and just render the line or 		 *  point.		 */		if (srPrivate->backfacingStyle == kQ3BackfacingStyleRemove &&			(srPrivate->normalLocalToWorldRank >= 2)) {									if (normal->x * normal->x + 				normal->y * normal->y + 				normal->z * normal->z < 1.e-10) {				/* 				 *  Normal vector's length is < 1.e-10 meaning the polygon is 				 *  degenerate 				 */				return (kQ3Success);			}						/*			 *  If the local-to-world matrix has a rank of 2, then the eye			 *  in local coordinates is a vector even if the view is a 			 *	perspective transformation.			 */			if ((srPrivate->cameraType == kQ3CameraTypeOrthographic) ||				(srPrivate->normalLocalToWorldRank == 2)) {				/* Orthographic projection */				if ((normal->x * srPrivate->eyeVectorInLocalCoords.x + 					 normal->y * srPrivate->eyeVectorInLocalCoords.y + 					 normal->z * srPrivate->eyeVectorInLocalCoords.z < 0.0) ^					((srPrivate->orientationStyle == 						kQ3OrientationStyleClockwise))) {					/* 					 *  Backfacing, so cull it 					 */					return (kQ3Success);				}			} else {				TQ3Point3D				*point;				TQ3Vector3D				eyeVector;								/* Perspective projection */				point = localVertices;				eyeVector.x = srPrivate->eyePointInLocalCoords.x - point->x;				eyeVector.y = srPrivate->eyePointInLocalCoords.y - point->y;				eyeVector.z = srPrivate->eyePointInLocalCoords.z - point->z;				if ((normal->x * eyeVector.x + 					 normal->y * eyeVector.y + 					 normal->z * eyeVector.z < 0.0) ^					((srPrivate->orientationStyle == 						kQ3OrientationStyleClockwise))) {					/* 					 *  Backfacing, so cull it 					 */					return (kQ3Success);				}			}		}	}	/*	 *  Status set to success - if a subsequent memory allocation fails, or a 	 *  called function fails, set to failure and bail out.	 */	status = kQ3Success;	/*	 *  Allocate device-coordinate vertices	 */	deviceVertices = malloc(numVertices * sizeof(TQ3RationalPoint4D));	if (deviceVertices == NULL) {		status = kQ3Failure;		goto bail;	}	/*	 *  Transform local-space line vertices to device space	 */	Q3Point3D_To4DTransformArray(		localVertices,		&srPrivate->transforms.localToDC,  		deviceVertices,		numVertices,		sizeOfLocalVertices, 		sizeof(TQ3RationalPoint4D));			/*	 *  renderVertices are those that will be rendered. If there is a clip,	 *  then we'll set this to the clipped vertices instead.	 */	renderVertices = deviceVertices;		/*	 *  Allocate a clip flag for each vertex. Bail if allocation fails.	 */	clipFlags = malloc(numVertices * sizeof(unsigned long));	if (clipFlags == NULL) {		status = kQ3Failure;		goto bail;	}	/*	 *  See if we have a clip	 */	SRPointList_ClipTestVertices(		deviceVertices, 		clipFlags, 		numVertices, 		&srPrivate->clipPlanesInDC[0], 		&clipFound, 		&allOut, 		sizeof(TQ3RationalPoint4D));	if (allOut) {		/*		 *  Return now, as nothing need be drawn		 */		goto bail;	}		/*	 *  We've found a clip, so generate the clipped vertext list	 */	if (clipFound) {		long	numberOfClippedVertices;		/*		 *  Allocate space for clipped vertices. There will be at most		 *  twice as many of these as there are in the original geometric		 *  primitive. Bail if allocation fails.		 */		clippedVertices = 			malloc((numVertices << 1) * sizeof(TQ3RationalPoint4D));		if (clippedVertices == NULL) {			status = kQ3Failure;			goto bail;		}				/*		 *  Allocate space for clip flags for vertices. Bail if allocation		 *  fails.		 */		clippedVerticesFlags = malloc((numVertices << 1) * sizeof(long));		if (clippedVerticesFlags == NULL) {			status = kQ3Failure;			goto bail;		}		/*		 *  Generate the clipped vertex list		 */		SRPointList_ClipVertices(			deviceVertices, 			sizeof(TQ3RationalPoint4D),			clippedVertices, 			sizeof(TQ3RationalPoint4D),			clipFlags, 			clippedVerticesFlags,			numVertices, 			&numberOfClippedVertices,			&srPrivate->clipPlanesInDC[0],			mode);		if (numberOfClippedVertices == 0) {			goto bail;		}				renderVertices = clippedVertices;		numVertices = numberOfClippedVertices;	}	/*	 *  Divide by homogeneous coordinate, but only if	 *	we've NOT found a clip - in that case, the w divide has	 *	already been done...	 */	if (!clipFound) {		SRPointList_WDivide(			renderVertices, 			numVertices, 			sizeof(TQ3RationalPoint4D));	}		#if DEBUGGING	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[0].x) < srPrivate->clipPlanesInDC[0]) {		goto bail;			}	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[0].x) > srPrivate->clipPlanesInDC[1]) {		goto bail;			}	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[0].y) < srPrivate->clipPlanesInDC[2]) {		goto bail;			}	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[0].y) > srPrivate->clipPlanesInDC[3]) {		goto bail;			}	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[1].x) < srPrivate->clipPlanesInDC[0]) {		goto bail;			}	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[1].x) > srPrivate->clipPlanesInDC[1]) {		goto bail;			}	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[1].y) < srPrivate->clipPlanesInDC[2]) {		goto bail;			}	if (FLOAT_ROUND_TO_LONG_POSITIVE(renderVertices[1].y) > srPrivate->clipPlanesInDC[3]) {		goto bail;			}#endif  /*  DEBUGGING  */	/*	 *  Draw line segments between each pair of vertices, taking into	 *  account the clipping information where appropriate.	 */	for (i = 0; i < numVertices - 1; i++) {		if (!clipFound 				|| 			(clippedVerticesFlags 	&& 			 clippedVerticesFlags[i] & SR_DRAW_NEXT)) {			if ((*lineFunction)(					srPrivate,					(TQ3Point3D *)&renderVertices[i], 					(TQ3Point3D *)&renderVertices[i + 1], 					color) == kQ3Failure) {				status = kQ3Failure;				goto bail;			}		}	}	/*	 *  If we're in "polygon" mode, then we have to connect the last	 *  vertex with the first.	 */	if (mode == DO_POLYGON) {		if (!clipFound 				|| 			(clippedVerticesFlags 	&& 			 clippedVerticesFlags[numVertices - 1] & SR_DRAW_NEXT)) {			if ((*lineFunction)(					srPrivate,					(TQ3Point3D *)&renderVertices[numVertices - 1], 					(TQ3Point3D *)&renderVertices[0], 					color) == kQ3Failure) {				status = kQ3Failure;				goto bail;			}		}	}bail:	/*	 *  Free up any allocated memory	 */	if (deviceVertices != NULL) {		free(deviceVertices);	}		if (clipFlags != NULL) {		free(clipFlags);	}		if (clippedVertices != NULL) {		free(clippedVertices);	}		if (clippedVerticesFlags != NULL) {		free(clippedVerticesFlags);	}		/*	 *  Status may be success or failure, the latter being the case if 	 *  an allocation failed or if the line rasterization function returns	 *  failure.	 */	return (status);}