#include "dlConfig.h"
#if WITH_PMD

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "dlCore.h"
#include "dlSceneobject.h"
#include "dlTypes.h"
#include "dlImport.h"
#include "dlLog.h"
#include "dlTexture.h"
#include "dlAtlas.h"

/* importer */
#include "mmd_import/mmd.h"

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

/* only needed for printing comment and name as utf-8
 * in most cases uselsess.
 *
 * would be nice to have own sjis to utf-8 decoder
 * but it might bloat the code a lot */
#if ICONV_SJIS_PMD
   #include "iconv.h"
#endif

/*
 * 0 = one VBO and seperate IBO's and textures for childs
 * 1 = one atlas texture and seperate VBO's and IBO's for childs
 */
#define ATLAS_METHOD 1

#if ICONV_SJIS_PMD
/* conversion */
static void convertSjisToUtf8( char *sjis, char *utf8, size_t sjisSize )
{
   iconv_t icd;
   char *p_src, *p_dst;
   size_t n_src, n_dst;

   icd = iconv_open("UTF-8", "Shift_JIS");
   p_src = sjis;
   p_dst = utf8;
   n_src = sjisSize;
   n_dst = sjisSize * 2;

   iconv(icd, &p_src, &n_src, &p_dst, &n_dst);
   iconv_close(icd);
}
#endif

/* Import MikuMikuDance PMD file */
int dlImportPMD( dlObject* object, const char *file, int bAnimated )
{
   FILE *f;
   unsigned int i, i2;
   char *texturePath;
   char tmp[256];
   dlTexture   *texture;
   unsigned int num_faces;
   unsigned int start = 0;

#if ATLAS_METHOD
   unsigned int ix;
   dlAtlas *atlas;
   dlTexture **textureList;
#else
   dlObject    *mObject;
#endif

   mmd_header header;
   mmd_data   *mmd;

   logBlue(); dlPrint("[PMD] attempt to load: %s\n", file ); logNormal();

   /* Yush! Open the file */
   f = fopen( (char*)file, "rb" );
   if(!f)
   {
      logRed();
      dlPrint("[PMD] file: %s, could not open\n", file );
      logNormal();

      return( RETURN_FAIL );
   }

   if( mmd_readHeader( f, &header ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read header");
      logNormal();

      return( RETURN_FAIL );
   }

   /* print info about our PMD */
#if ICONV_SJIS_PMD
   dlPuts("");
   convertSjisToUtf8( header.name, tmp, strlen(header.name) );
   dlPuts( tmp );
   dlPuts("");
   convertSjisToUtf8( header.comment, tmp, strlen(header.comment) );
   dlPuts( tmp );
   dlPuts("");
#endif

   /* ok, lets create MMD data structure that holds
    * everything we need */
   mmd = newMMD();
   if(!mmd)
   {
      /* yikes, failed to allocate */
      logRed();
      dlPuts("[PMD] failed to allocate MMD data structure");
      logNormal();

      fclose(f);
      return( RETURN_FAIL );
   }

   /* read vertex data */
   if( mmd_readVertexData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read vertex data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read index data */
   if( mmd_readIndexData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read index data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read material data */
   if( mmd_readMaterialData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read material data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read bone data */
   if( mmd_readBoneData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read bone data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read IK data */
   if( mmd_readIKData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read IK data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read Skin data */
   if( mmd_readSkinData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read Skin data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read Skin display data */
   if( mmd_readSkinDisplayData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read Skin display data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read bone name data */
   if( mmd_readBoneNameData( f, mmd ) != RETURN_OK )
   {
      logRed();
      dlPuts("[PMD] failed to read bone name data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* print vertex info */
   dlPrint("V: %d\n", mmd->num_vertices);
   dlPrint("I: %d\n", mmd->num_indices);
   dlPrint("B: %d\n", mmd->num_bones);
   dlPrint("BN: %d\n", mmd->num_bone_names);
   dlPrint("IK: %d\n", mmd->num_ik);
   dlPrint("S: %d\n", mmd->num_skins);
   dlPrint("SD: %d\n", mmd->num_skin_displays);

   /* close file */
   fclose(f);

#if ATLAS_METHOD

   /* texture list which we use to retive transformed coords */
   textureList = malloc( sizeof(dlTexture*) * mmd->num_materials );
   if(!textureList)
   {
      logRed();
      dlPuts( "[PMD] failed to allocate texture list for atlas usage" );
      logNormal();

      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* create atlas */
   atlas = dlNewAtlas();
   if(!atlas)
   {
      logRed();
      dlPuts( "[PMD] failed to allocate texture atlas" );
      logNormal();

      free( textureList );
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* add textures to atlas */
   i = 0;
   for(; i != mmd->num_materials; ++i)
   {
      /* default */
      textureList[i] = NULL;

      texturePath = dlImportTexturePath( mmd->texture[i].file, file );
      if(texturePath)
      {
         texture = dlNewTexture( texturePath, 0 );
         if(texture)
            dlAtlasAddTexture( atlas, texture );
         textureList[i] = texture;

         free( texturePath );
      }
   }

   /* pack textures */
   texture = dlAtlasPack( atlas, 1, 0  );
   dlRefTexture( texture );

   /* assign atlas texture to object */
   dlObjectAddTexture( object, 0, texture );

   /* reset buffers */
   dlResetIndexBuffer( object->ibo,    mmd->num_indices );
   dlResetVertexBuffer( object->vbo,   mmd->num_vertices );
   dlResetNormalBuffer( object->vbo,   mmd->num_vertices );
   dlResetCoordBuffer( object->vbo, 0, mmd->num_vertices );

   /* materials
    * each material = 1 object */
   i = 0;
   for(; i != mmd->num_materials; ++i)
   {
      /* feed vertex + index data  */
      num_faces = mmd->face[i];
      i2 = start;
      for(; i2 != start + num_faces; ++i2)
      {
         ix = mmd->indices[i2];
         dlInsertVertex( object->vbo,
               mmd->vertices[ ix * 3     ],
               mmd->vertices[ ix * 3 + 1 ],
               mmd->vertices[ ix * 3 + 2 ] );

         dlInsertCoord( object->vbo, 0,
               mmd->coords[ ix * 2     ],
               mmd->coords[ ix * 2 + 1 ] );

         dlInsertNormal( object->vbo,
               mmd->normals[ ix * 3     ],
               mmd->normals[ ix * 3 + 1 ],
               mmd->normals[ ix * 3 + 2 ] );

         /* fix coords */
         if(object->vbo->uvw[0].coords[i2].y < 0.0f)
            object->vbo->uvw[0].coords[i2].y += 1;
         if(object->vbo->uvw[0].coords[i2].x < 0.0f)
            object->vbo->uvw[0].coords[i2].x += 1;

         //dlPrint( "%f, %f\n", object->vbo->uvw[0].coords[i2].x, object->vbo->uvw[0].coords[i2].y);
         dlAtlasGetTransformed( atlas, textureList[i], &object->vbo->uvw[0].coords[i2] );

         dlInsertIndex( object->ibo, i2 );

      }
      //dlPuts("");
      start += num_faces;
   }

   /* free atlas */
   dlFreeAtlas( atlas );
   free( textureList );

   /* GL_TRIANGLES object */
   object->primitive_type = GL_TRIANGLES;

#else

   dlResetVertexBuffer( object->vbo,   mmd->num_vertices );
   dlResetNormalBuffer( object->vbo,   mmd->num_vertices );
   dlResetCoordBuffer( object->vbo, 0, mmd->num_vertices );

   i = 0;
   for(; i != mmd->num_vertices; ++i)
   {
         dlInsertVertex( object->vbo,
               mmd->vertices[ i * 3     ],
               mmd->vertices[ i * 3 + 1 ],
               mmd->vertices[ i * 3 + 2 ] );

         dlInsertCoord( object->vbo, 0,
               mmd->coords[ i * 2     ],
               mmd->coords[ i * 2 + 1 ] );

         dlInsertNormal( object->vbo,
               mmd->normals[ i * 3     ],
               mmd->normals[ i * 3 + 1 ],
               mmd->normals[ i * 3 + 2 ] );
   }

   /* materials
    * each material = 1 object */
   i = 0;
   for(; i != mmd->num_materials; ++i)
   {
      puts( mmd->texture[i].file);

      /* each material which is > 0 = new object */
      if(i)
      {
         mObject = dlNewObject();
         if(!mObject)
         {
            logRed();
            dlPuts( "[PMD] could not allocate child object" );
            logNormal();

            freeMMD( mmd );
            return( RETURN_FAIL );
         }

         /* ref VBO new IBO */
         mObject->vbo = dlRefVBO( object->vbo );
         mObject->ibo = dlNewIBO();
         if(!mObject->ibo)
         {
            logRed();
            dlPuts( "[PMD] could not allocate child IBO" );
            logNormal();

            dlFreeObject( mObject );
            freeMMD( mmd );
            return( RETURN_FAIL );
         }

         dlObjectAddChild( object, mObject );
      }
      else  mObject = object;

      /* texture */
      texturePath = dlImportTexturePath( mmd->texture[i].file, file );
      if(texturePath)
      {
         texture = dlNewTexture( texturePath, SOIL_FLAG_DEFAULTS );
         if(texture)
            dlObjectAddTexture( mObject, 0, texture );

         free( texturePath );
      }

      /* feed indices */
      num_faces = mmd->face[i];
      dlResetIndexBuffer( mObject->ibo,    num_faces );

      i2 = start;
      for(; i2 != start + num_faces; ++i2)
      {
         dlInsertIndex( mObject->ibo, mmd->indices[i2] );
      }
      start += num_faces;

      /* GL_TRIANGLES object */
      mObject->primitive_type = GL_TRIANGlES;
   }

#endif

   /* free mmd_data structure */
   freeMMD( mmd );

   return( RETURN_OK );
}

#endif /* WITH_PMD */
