#include "dlConfig.h"
#if WITH_OPENCTM

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <openctm.h>

#include "dlSceneobject.h"
#include "dlTypes.h"
#include "dlImport.h"
#include "dlLog.h"

/* maybe map the basic GL enums
 * to own structure, so these become useless */
#ifdef GLES2
#  include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#define DL_DEBUG_CHANNEL "IMPORT_OCTM"

#define COLOR_ATTRIB "Color"

/* We are totally in control with this custom read function */
static CTMuint readOCTMFile( void *buf, CTMuint toRead, void *f )
{
   CALL("%p, %u, %p", buf, toRead, f);
   return( (CTMuint) fread( buf, 1, (size_t) toRead, (FILE *) f ) );
}

/* Import OpenCTM file */
int dlImportOCTM( dlObject* object, const char *file, int bAnimated )
{
   FILE *f;
   CTMcontext context;
   CTMuint  vertCount, triCount, uvCount, attribCount;
   const CTMuint *indices;
   const CTMfloat *vertices, *normals, *coords, *attribs;
   CTMenum  error;
   unsigned int i, i2; /* the god of iterators, and his child */
   const char *textureFilename, *comment, *attribName;
   char *texturePath = NULL;

   LOGINFOP("Attempt to load: %s", file);

   /* obiviously we need a import context
    * HEY! Maybe CTM_EXPORT would work!? */
   context = ctmNewContext( CTM_IMPORT );

   /* check if we fail at context creation */
   if(!context)
   {
      LOGERR("I suck at context creation");

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* Yush! Open the file */
   f = fopen( (char*)file, "rb" );
   if(!f)
   {
      LOGERRP("File: %s, could not open", file);

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* I think, i fell love with this API */
   ctmLoadCustom( context, (CTMreadfn)readOCTMFile, f );

   /* What's the magic word to close it? */
   fclose(f);

   /* lets just hope that it does not have any errors */
   if((error = ctmGetError(context)) != CTM_NONE)
   {
      /* Go! Pikachu! Use the printf! */
      LOGERRP("%s", ctmErrorString( error ));
      ctmFreeContext(context);

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* so far all good, lets do this! */
   vertCount   = ctmGetInteger( context, CTM_VERTEX_COUNT );
   vertices    = ctmGetFloatArray( context, CTM_VERTICES );
   triCount    = ctmGetInteger( context, CTM_TRIANGLE_COUNT );
   indices     = ctmGetIntegerArray( context, CTM_INDICES );
   uvCount     = ctmGetInteger( context, CTM_UV_MAP_COUNT );
   attribCount = ctmGetInteger( context, CTM_ATTRIB_MAP_COUNT );

   comment   = ctmGetString( context, CTM_FILE_COMMENT );
   if( comment )
      dlPuts(comment);

   /* ok, we go the info..
    * now.. how to deal with it? */
   dlResetVertexBuffer( object->vbo, vertCount );
   dlResetIndexBuffer( object->ibo, triCount * 3 );

   /* indices */
   i = 0;
   for(; i != triCount * 3; ++i)
      dlInsertIndex( object->ibo,
                     indices[i]  );

   /* vertices */
   i = 0;
   for(; i != vertCount; ++i)
      dlInsertVertex( object->vbo,
                      vertices[ i * 3 ],
                      vertices[ i * 3 + 1 ],
                      vertices[ i * 3 + 2 ]  );

   /* texture coords */
   i = 0;
   while( i != uvCount )
   {
      coords = ctmGetFloatArray( context, CTM_UV_MAP_1 + i );
      dlResetCoordBuffer( object->vbo, i, vertCount );

      /* get texture filename */
      textureFilename = ctmGetUVMapString( context, CTM_UV_MAP_1 + i, CTM_FILE_NAME );
      if(!textureFilename)
         textureFilename = ctmGetUVMapString( context, CTM_UV_MAP_1 + i, CTM_NAME );

      /* check path */
      if(textureFilename)
         texturePath = dlImportTexturePath( textureFilename,
                                            file );

      /* load if exists */
      if(texturePath)
      {
         if(object->material) dlFreeMaterial(object->material);
         object->material = dlNewMaterialFromImage( texturePath, SOIL_FLAG_DEFAULTS );
         free( texturePath );
      }

      i2 = 0;
      for(; i2 != vertCount; ++i2)
      {
         dlInsertCoord( object->vbo, i,
                        coords[ i2 * 2 ],
                        coords[ i2 * 2 + 1 ] );
      }

      ++i;
   }

   /* normals */
   if(ctmGetInteger(context, CTM_HAS_NORMALS) == CTM_TRUE)
   {
       dlResetNormalBuffer( object->vbo, vertCount );
       normals = ctmGetFloatArray( context, CTM_NORMALS );

       i = 0;
       for(; i != vertCount; ++i)
          dlInsertNormal( object->vbo,
                          normals[ i * 3 ],
                          normals[ i * 3 + 1 ],
                          normals[ i * 3 + 2 ]  );
   }

   /* custom attribs, only for vertex colors atm */
   i = 0;
   for(; i != attribCount; ++i)
   {
      attribName = ctmGetAttribMapString( context, CTM_ATTRIB_MAP_1 + i, CTM_NAME );
      attribs     = ctmGetFloatArray( context, CTM_ATTRIB_MAP_1 + i );

      if(attribName)
         LOGINFO(attribName);

#if VERTEX_COLOR
      if( strcmp( attribName, COLOR_ATTRIB ) == 0 )
      {
         dlResetColorBuffer( object->vbo, vertCount );

         i2 = 0;
         for(; i2 != vertCount; ++i2)
            dlInsertColor( object->vbo,
                           attribs[ i2 * 4 ],
                           attribs[ i2 * 4 + 1 ],
                           attribs[ i2 * 4 + 2 ],
                           attribs[ i2 * 4 + 3 ]  );
      }
#endif
   }


   /* this is now GL_TRIANGLES object
    * maybe add tristripper option? */
   object->primitive_type = GL_TRIANGLES;

   /* free the bird */
   ctmFreeContext(context);

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

#endif /* WITH_OCTM */
