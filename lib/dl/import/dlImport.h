#ifndef DL_IMPORT_H
#define DL_IMPORT_H

#include "dlSceneobject.h"
#include "dlMaterial.h"
#include "dlConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Wraps around the other imports by figuring out the header
 * 1. pointer to sceneobject
 * 2. filename
 * 3. 1 = import animation data, 0 = don't import
 */
int dlImportModel( dlObject*, const char *file, int bAnimated );

#if WITH_OPENCTM
/* OpenCTM http://openctm.sourceforge.net/ */
int dlImportOCTM( dlObject*, const char *file, int bAnimated );
#endif /* WITH_OCTM */

#if WITH_PMD
/* MikuMikuDance PMD */
int dlImportPMD( dlObject*, const char *file, int bAnimated );
#endif /* WITH_PMD */

#if WITH_ASSIMP
/* Assimp wrapper */
int dlImportASSIMP( dlObject*, const char *file, int bAnimated );
#endif /* WITH_ASSIMP */

/* Not using own importers anymore, SOIL ftw :)
 * 1. pointer to texture object
 * 2. filename
 */
int dlImportImage( dlTexture*, const char *file, unsigned int flags );

/* Common helper functions.'
 * don't add here if it does not apply to other formats or importers */

/* Try to figure out real texture path
 * 1. Texture name/filename/path
 * 2. Model path
 *
 * free the returned string
 */
char* dlImportTexturePath( const char*, const char* );

#ifdef __cplusplus
}
#endif

#endif /* DL_IMPORT_H */

/* EoF */
