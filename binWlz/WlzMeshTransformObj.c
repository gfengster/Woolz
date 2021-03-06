#if defined(__GNUC__)
#ident "University of Edinburgh $Id$"
#else
static char _WlzMeshTransformObj_c[] = "University of Edinburgh $Id$";
#endif
/*!
* \file         binWlz/WlzMeshTransformObj.c
* \author       Richard Baldock
* \date         March 2005
* \version      $Id$
* \par
* Address:
*               MRC Human Genetics Unit,
*               MRC Institute of Genetics and Molecular Medicine,
*               University of Edinburgh,
*               Western General Hospital,
*               Edinburgh, EH4 2XU, UK.
* \par
* Copyright (C), [2012],
* The University Court of the University of Edinburgh,
* Old College, Edinburgh, UK.
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
* \brief	Applies a mesh transform to an object.
* \ingroup	BinWlz
*
* \par Binary
* \ref wlzmeshtransformobj "WlzMeshTransformObj"
*/

/*!
\ingroup BinWlz
\defgroup wlzmeshtransformobj WlzMeshTransformObj
\par Name
WlzMeshTransformObj - applies a mesh transform to an object.
\par Synopsis
\verbatim
WlzMeshTransformObj [-i] [-L] -m <mesh transform file> [-o <output file>]
                    [-h] <2D object input file>
\endverbatim
\par Options
<table width="500" border="0">
  <tr> 
    <td><b>-i</b></td>
    <td>Use the inverse transformation.</td>
  </tr>
  <tr> 
    <td><b>-L</b></td>
    <td>Use linear interpolation, default is nearest-neighbour.</td>
  </tr>
  <tr> 
    <td><b>-m</b></td>
    <td>Mesh transform object.</td>
  </tr>
  <tr> 
    <td><b>-o</b></td>
    <td>Output filename, default is stdout.</td>
  </tr>
  <tr> 
    <td><b>-h</b></td>
    <td>Help, prints usage message.</td>
  </tr>
</table>
\par Description
Applies a mesh transform to an object.
\par Examples
\verbatim
WlzMeshTransformObj -m mesh.wlz -o out.wlz obj.wlz
\endverbatim
Uses the mesh transform read from mesh.wlz to transform the 2D domain object
with read from obj.wlz. The resulting warped object is written to out.wlz.
\par File
\ref WlzMeshTransformObj.c "WlzMeshTransformObj.c"
\par See Also
\ref BinWlz "WlzIntro(1)"
\ref wlzbasisfntransformobj "WlzBasisFnTransformObj(1)"
\ref WlzMeshTransformObj "WlzMeshTransformObj(3)"
*/

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Wlz.h>

/* externals required by getopt  - not in ANSI C standard */
#ifdef __STDC__ /* [ */
extern int      getopt(int argc, char * const *argv, const char *optstring);
 
extern int 	optind, opterr, optopt;
extern char     *optarg;
#endif /* __STDC__ ] */

WlzErrorNum WlzSetMeshInverse(
  WlzMeshTransform	*meshTr)
{
  WlzErrorNum	errNum=WLZ_ERR_NONE;
  int		i;

  /* check the mesh transform */
  if( meshTr ){
    if( meshTr->nodes == NULL ){
      errNum = WLZ_ERR_PARAM_NULL;
    }
  }
  else {
    errNum = WLZ_ERR_PARAM_NULL;
  }

  /* loop through nodes,add the displacement then invert displacement */
  if( errNum == WLZ_ERR_NONE ){
    for(i=0; i < meshTr->nNodes; i++){
      meshTr->nodes[i].position.vtX += meshTr->nodes[i].displacement.vtX;
      meshTr->nodes[i].position.vtY += meshTr->nodes[i].displacement.vtY;
      meshTr->nodes[i].displacement.vtX *= -1.0;
      meshTr->nodes[i].displacement.vtY *= -1.0;
    }
  }

  return errNum;
}

static void usage(char *proc_str)
{
  fprintf(stderr,
	  "Usage:\t%s -m <mesh transform file>"
	  " [-o <output file>] [-h]\n"
	  "                            [<2D object input file>]\n"
	  "\tApply a mesh transform to given input objects\n"
	  "\twriting the warped objects to standard output.\n"
	  "Version: %s\n"
	  "Options:\n"
	  "\t  -i                 Inverse transform\n"
	  "\t  -L                 Use linear interpolation instead of\n"
	  "\t                     nearest-neighbour\n"
	  "\t  -m<meshfile>       Mesh transform object\n"
	  "\t  -o<output file>    Output filename, default to stdout\n"
	  "\t  -h                 Help - this message\n",
	  proc_str,
	  WlzVersion());
  return;
}

int main(int	argc,
	 char	**argv)
{
  WlzObject	*inObj, *meshObj, *outObj;
  FILE		*inFP, *outFP, *meshFP;
  char		*outFile, *meshFile;
  char 		optList[] = "iLm:o:h";
  int		option;
  int		inverseFlg = 0;
  const char    *errMsg;
  WlzInterpolationType interp = WLZ_INTERPOLATION_NEAREST;
  WlzErrorNum	errNum=WLZ_ERR_NONE;

  /* additional defaults */
  outFile = "-";
  meshFile = NULL;

  /* read the argument list and check for an input file */
  opterr = 0;
  while( (option = getopt(argc, argv, optList)) != EOF ){
    switch( option ){

    case 'i':
      inverseFlg = 1;
      break;

    case 'L':
      interp = WLZ_INTERPOLATION_LINEAR;
      break;

    case 'm':
      meshFile = optarg;
      break;

    case 'o':
      outFile = optarg;
      break;

    case 'h':
    default:
      usage(argv[0]);
      return 0;

    }
  }

  /* check input file/stream */
  inFP = stdin;
  if( optind < argc ){
    if( (inFP = fopen(*(argv+optind), "rb")) == NULL ){
      fprintf(stderr, "%s: can't open file %s\n", argv[0], *(argv+optind));
      usage(argv[0]);
      return 1;
    }
  }

  /* check output file/stream */
  if(strcmp(outFile, "-"))
  {
    if((outFP = fopen(outFile, "wb")) == NULL)
    {
      errNum = WLZ_ERR_WRITE_EOF;
    }
  }
  else
  {
    outFP = stdout;
  }

  /* check meshfile */
  if((errNum == WLZ_ERR_NONE) && (meshFile != NULL)){
    if((meshFP = fopen(meshFile, "rb")) != NULL){
      if((meshObj = WlzReadObj(meshFP, &errNum)) != NULL){
	if(meshObj->type != WLZ_MESH_TRANS){
	  (void )fprintf(stderr,
		  "%s: mesh file does not have a mesh transform object\n",
		  argv[0]);
	  return 1;
	}
      }
      else {
	fprintf(stderr, "%s: failed to read mesh file\n",
	    argv[0]);
	return 1;
      }
    }
    else {
      fprintf(stderr, "%s: failed to open mesh file\n",
	    argv[0]);
      return 1;
    }
  }
  else {
    fprintf(stderr, "%s: mesh input file required\n",
	    argv[0]);
    usage(argv[0]);
    return 1;
  }

  /* check for inverse transform */
  if( (errNum == WLZ_ERR_NONE) && inverseFlg ){
    WlzSetMeshInverse(meshObj->domain.mt);
  }

  /* transform any suitable input objects */
  while((errNum == WLZ_ERR_NONE) &&
        ((inObj = WlzReadObj(inFP, &errNum)) != NULL))
  {
    switch( inObj->type )
    {
    case WLZ_2D_DOMAINOBJ:
      if((outObj = WlzMeshTransformObj(inObj, meshObj->domain.mt,
				       interp, &errNum)) != NULL){
	WlzWriteObj(outFP, outObj);
	WlzFreeObj(outObj);
      }
      else {
	(void )WlzStringFromErrorNum(errNum, &errMsg);
	(void )fprintf(stderr,
		       "%s: failed to transform object (%s).\n",
		       *argv, errMsg);
	return 1;
      }
      break;

    default:
      WlzWriteObj(outFP, inObj);
      break;
    }

    WlzFreeObj(inObj);
  }

  if(errNum == WLZ_ERR_READ_EOF)
  {
    errNum = WLZ_ERR_NONE;
  }

  return errNum;
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */
