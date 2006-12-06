#if defined(__GNUC__)
#ident "MRC HGU $Id$"
#else
#if defined(__SUNPRO_C) || defined(__SUNPRO_CC)
#pragma ident "MRC HGU $Id$"
#else
static char _WlzBackground_c[] = "MRC HGU $Id$";
#endif
#endif
/*!
* \file         libWlz/WlzBackground.c
* \author       Richard Baldock
* \date         March 1999
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C) 2005 Medical research Council, UK.
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be
* useful but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
* PURPOSE.  See the GNU General Public License for more
* details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA.
* \brief	Functions to get and set the background value of objects.
* \ingroup	WlzValuesUtils
* \todo         -
* \bug          None known.
*/

#include <stdlib.h>
#include <Wlz.h>

/*!
* \return	Woolz error code.
* \ingroup	WlzValuesUtils
* \brief	Sets the backgound value of an image object.
* \param	obj			Given object in which to set the
*					background value.
* \param	bgd			Required background value.
*/
WlzErrorNum WlzSetBackground(WlzObject	*obj,
			     WlzPixelV	bgd)
{
  WlzPlaneDomain	*planedmn;
  WlzVoxelValues	*voxtab;
  WlzObject		*obj1;
  int			i;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  /* check the object */
  if( obj == NULL ){
    return( WLZ_ERR_OBJECT_NULL );
  }

  switch( obj->type ){

  case WLZ_2D_DOMAINOBJ:
  case WLZ_3D_DOMAINOBJ:
    break;

  case WLZ_TRANS_OBJ:
    return( WlzSetBackground(obj->values.obj, bgd) );

  case WLZ_EMPTY_OBJ:
    return( WLZ_ERR_NONE );

  default:
    return( WLZ_ERR_OBJECT_TYPE );

  }    

  if( obj->domain.core == NULL ){
    return( WLZ_ERR_DOMAIN_NULL );
  }
  if( obj->values.core == NULL ){
    return( WLZ_ERR_NONE );
  }

  /* check the background type */
  switch( bgd.type ){

  case WLZ_GREY_INT:
  case WLZ_GREY_SHORT:
  case WLZ_GREY_UBYTE:
  case WLZ_GREY_FLOAT:
  case WLZ_GREY_DOUBLE:
  case WLZ_GREY_RGBA:
    break;

  default:
    return( WLZ_ERR_GREY_TYPE );

  }
   

  (void )WlzValueConvertPixel(
    &bgd, bgd,
    WlzGreyTableTypeToGreyType(obj->values.core->type, &errNum));
  switch( obj->type ){

  case WLZ_2D_DOMAINOBJ:
    switch( WlzGreyTableTypeToTableType(obj->values.core->type, &errNum) ){

    case WLZ_GREY_TAB_RAGR:
      obj->values.v->bckgrnd = bgd;
      break;

    case WLZ_GREY_TAB_RECT:
      obj->values.r->bckgrnd = bgd;
      break;

    case WLZ_GREY_TAB_INTL:
      obj->values.i->bckgrnd = bgd;
      break;

    default:
      return( errNum );
    }
    break;

  case WLZ_3D_DOMAINOBJ:
    planedmn = obj->domain.p;
    if( planedmn->type != WLZ_PLANEDOMAIN_DOMAIN ){
      return( WLZ_ERR_PLANEDOMAIN_TYPE );
    }

    voxtab = obj->values.vox;
    if( voxtab->type != WLZ_VOXELVALUETABLE_GREY ){
      return( WLZ_ERR_VOXELVALUES_TYPE );
    }

    for(i=0; i <= planedmn->lastpl - planedmn->plane1; i++){
      if( planedmn->domains[i].core == NULL
	  || voxtab->values[i].core == NULL){
	continue;
      }
      obj1 = WlzMakeMain(WLZ_2D_DOMAINOBJ, planedmn->domains[i],
			 voxtab->values[i], NULL, NULL, &errNum);
      WlzSetBackground( obj1, bgd );

      WlzFreeObj( obj1 );
    }
    voxtab->bckgrnd = bgd;
    break;

  }

  return( errNum );
}

/*!
* \return	The background value. If the returned pixel value type is
* 		WLZ_GREY_ERROR then an error has occurred.
* \ingroup	WlzValuesUtils
* \brief	Gets the background value of the given object.
* \param	obj			Given object.
* \param	dstErr			Destination error pointer, may be NULL.
*/
WlzPixelV WlzGetBackground(
  WlzObject	*obj,
  WlzErrorNum	*dstErr)
{
  WlzPlaneDomain	*planedmn;
  WlzVoxelValues	*voxtab;
  WlzPixelV		bgd;
  WlzErrorNum		errNum=WLZ_ERR_NONE;

  /* set up an invalid background return */
  bgd.type = WLZ_GREY_ERROR;
  bgd.v.inv = 0;
  
  if( obj == NULL ){
    errNum = WLZ_ERR_OBJECT_NULL;
  }


  if( errNum == WLZ_ERR_NONE ){
    switch( obj->type ){

    case WLZ_2D_DOMAINOBJ:
      if( obj->domain.i == NULL ){
	errNum = WLZ_ERR_DOMAIN_NULL;
	break;
      }

      if( obj->values.v == NULL ){
	bgd.type = WLZ_GREY_INT;
	bgd.v.inv = 0;
	break;
      }
      
      switch( WlzGreyTableTypeToTableType(obj->values.core->type,
					  NULL) ){

      case WLZ_GREY_TAB_RAGR:
	bgd = obj->values.v->bckgrnd;
	break;

      case WLZ_GREY_TAB_RECT:
	bgd = obj->values.r->bckgrnd;
	break;

      case WLZ_GREY_TAB_INTL:
	bgd = obj->values.i->bckgrnd;
	break;

      default:
	errNum = WLZ_ERR_VALUES_TYPE;
	break;
      }
      break;

    case WLZ_3D_DOMAINOBJ:
      if( obj->domain.p == NULL ){
	errNum = WLZ_ERR_DOMAIN_NULL;
	break;
      }

      if( obj->values.vox == NULL ){
	bgd.type = WLZ_GREY_INT;
	bgd.v.inv = 0;
	errNum = WLZ_ERR_NONE;
	break;
      }

      planedmn = obj->domain.p;
      if( planedmn->type != WLZ_PLANEDOMAIN_DOMAIN ){
	errNum = WLZ_ERR_PLANEDOMAIN_TYPE;
	break;
      }

      voxtab = obj->values.vox;
      if( voxtab->type != WLZ_VOXELVALUETABLE_GREY ){
	errNum = WLZ_ERR_VOXELVALUES_TYPE;
	break;
      }
      bgd = voxtab->bckgrnd;
      break;

    case WLZ_TRANS_OBJ:
      return( WlzGetBackground(obj->values.obj, dstErr) );

    case WLZ_EMPTY_OBJ:
      bgd.type = WLZ_GREY_INT;
      bgd.v.inv = 0;
      errNum = WLZ_ERR_NONE;
      break;

    default:
      errNum = WLZ_ERR_OBJECT_TYPE;
      break;
    }
  }

  if( dstErr ){
    *dstErr = errNum;
  }
  return bgd;
}