#include "config.h"
#if WITH_OPENCTM

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <openctm.h>

#include "sceneobject.h"
#include "types.h"
#include "import.h"
#include "logfile_wrapper.h"

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

#define COLOR_ATTRIB "Color"

/* We are totally in control with this custom read function */
static CTMuint readOCTMFile( void *buf, CTMuint toRead, void *f )
{
   return( (CTMuint) fread( buf, 1, (size_t) toRead, (FILE *) f ) );
}

/* Import OpenCTM file */
int glImportOCTM( glObject* object, const char *file, int bAnimated )
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
   glTexture *texture;

   logBlue(); glPrint("[OCTM] attempt to load: %s\n", file ); logNormal();

   /* obiviously we need a import context
    * HEY! Maybe CTM_EXPORT would work!? */
   context = ctmNewContext( CTM_IMPORT );

   /* check if we fail at context creation */
   if(!context)
   {
      logRed();
      glPuts("[OCTM] i suck at context creation");
      logNormal();

      return( RETURN_FAIL );
   }

   /* Yush! Open the file */
   f = fopen( (char*)file, "rb" );
   if(!f)
   {
      logRed();
      glPrint("[OCTM] file: %s, could not open\n", file );
      logNormal();

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
      logRed();
      glPrint("[OCTM] %s\n", ctmErrorString( error ));
      logNormal();

      ctmFreeContext(context);
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
      puts(comment);

   /* ok, we go the info..
    * now.. how to deal with it? */
   glResetVertexBuffer( object->vbo, vertCount );
   glResetIndexBuffer( object->ibo, triCount * 3 );

   /* indices */
   i = 0;
   for(; i != triCount * 3; ++i)
      glInsertIndex( object->ibo,
                     indices[i]  );

   /* vertices */
   i = 0;
   for(; i != vertCount; ++i)
      glInsertVertex( object->vbo,
                      vertices[ i * 3 ],
                      vertices[ i * 3 + 1 ],
                      vertices[ i * 3 + 2 ]  );

   /* texture coords */
   i = 0;
   while( i != uvCount )
   {
      coords = ctmGetFloatArray( context, CTM_UV_MAP_1 + i );
      glResetCoordBuffer( object->vbo, i, vertCount );

      /* get texture filename */
      textureFilename = ctmGetUVMapString( context, CTM_UV_MAP_1 + i, CTM_FILE_NAME );
      if(!textureFilename)
         textureFilename = ctmGetUVMapString( context, CTM_UV_MAP_1 + i, CTM_NAME );

      /* check path */
      if(textureFilename)
         texturePath = glImportTexturePath( textureFilename,
                                            file );

      /* load if exists */
      if(texturePath)
      {
         texture = glNewTexture( texturePath, SOIL_FLAG_DEFAULTS );
         if(texture)
            glObjectAddTexture( object, i, texture );

         free( texturePath );
      }

      i2 = 0;
      for(; i2 != vertCount; ++i2)
      {
         glInsertCoord( object->vbo, i,
                        coords[ i2 * 2 ],
                        coords[ i2 * 2 + 1 ] );
      }

      ++i;
   }

   /* normals */
   if(ctmGetInteger(context, CTM_HAS_NORMALS) == CTM_TRUE)
   {
       glResetNormalBuffer( object->vbo, vertCount );
       normals = ctmGetFloatArray( context, CTM_NORMALS );

       i = 0;
       for(; i != vertCount; ++i)
          glInsertNormal( object->vbo,
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
         puts(attribName);

#if VERTEX_COLOR
      if( strcmp( attribName, COLOR_ATTRIB ) == 0 )
      {
         glResetColorBuffer( object->vbo, vertCount );

         i2 = 0;
         for(; i2 != vertCount; ++i2)
            glInsertColor( object->vbo,
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

   return( RETURN_OK );
}

#endif /* WITH_OCTM */
