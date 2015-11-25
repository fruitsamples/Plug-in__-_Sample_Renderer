/****************************************************************************** **																			 ** ** 	Module:		SR_MacDialog.h											 ** ** 																		 ** ** 																		 ** ** 	Purpose: 	Modal dialog routines, and other Macintosh specific		 ** ** 				routines												 ** ** 																		 ** ** 																		 ** ** 	Copyright (C) 1996 Apple Computer, Inc.  All rights reserved.		 ** ** 																		 ** ** 																		 ** *****************************************************************************/#ifndef SR_MacDialog_h#define SR_MacDialog_h#include "QD3D.h"#define SR_NAME_RESOURCE	16211	/* the ID of the resource string for									 * this renderer's name									 */TQ3Status SR_GetNameString(	unsigned char				*dataBuffer, 	unsigned long				bufferSize,	unsigned long				*actualDataSize);TQ3Status SR_MacModalDialog(	TQ3RendererObject			renderer,	TQ3DialogAnchor 			dialogAnchor, 	TQ3Boolean					*canceled,		void						*rendererPrivate);#endif  /*  SR_MacDialog_h  */