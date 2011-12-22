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

#define DL_DEBUG_CHANNEL "IMPORT_PMD"

/* only needed for printing comment and name as utf-8
 * in most cases uselsess.
 *
 * would be nice to have own sjis to utf-8 decoder
 * but it might bloat the code a lot */
#if ICONV_SJIS_PMD
   #include "iconv.h"
   #include <errno.h>
#endif

/* 0 = one VBO and seperate IBO's and textures for childs
 * 1 = one atlas texture and seperate VBO's and IBO's for childs */
#define ATLAS_METHOD 1

#if ICONV_SJIS_PMD

/* conversion from SJIS to UTF8, remember to free the returned pointer.
 * returns NULL on failure */
static char* convertSjisToUtf8( char *sjis )
{
   const char *INSET  = "UTF-8";
   const char *OUTSET = "Shift_JIS";
   iconv_t icd;
   size_t iret;
   char *p_src, *p_dst, *p_start;
   size_t n_src, n_dst;

   CALL("%p", sjis);

   p_src = sjis;
   n_src = strlen(p_src);
   n_dst = n_src * 2;

   if(!p_src || !n_src)
   {
      LOGERR("ICONV :: String null or empty");

      RET("%p", NULL);
      return( NULL );
   }

   p_dst = calloc( n_dst, 1 );
   if(!p_dst)
   {
      LOGERR("ICONV :: Failed to alloc UTF8 string");;

      RET("%p", NULL);
      return( NULL );
   }

   icd = iconv_open(INSET, OUTSET);
   if(!icd)
   {
      logRed();
      if(errno == EINVAL)
      { LOGERRP("ICONV :: Conversion from '%s' to '%s' is not supported.",INSET,OUTSET); }
      else
      { LOGERRP("ICONV :: Initialization failure: %s", strerror(errno)); }

      RET("%p", NULL);
      return( NULL );
   }
   p_start = p_dst;
   iret = iconv(icd, &p_src, &n_src, &p_dst, &n_dst);
   if(iret == (size_t)-1)
   {
      LOGERR("ICONV :: Conversion failure.");
      switch (errno)
      {
         /* See "man 3 iconv" for an explanation. */
         case EILSEQ:
            LOGERR("Invalid multibyte sequence.");
            break;
         case EINVAL:
            LOGERR("Incomplete multibyte sequence.");
            break;
         case E2BIG:
            LOGERR("No more room.");
            break;
         default:
            LOGERRP("%s", strerror (errno));
      }
      free(p_start);

      RET("%p", NULL);
      return( NULL );
   }

   iconv_close(icd);

   RET("%s", p_start);
   return( p_start );
}
#endif

/* Import MikuMikuDance PMD file */
int dlImportPMD( dlObject* object, const char *file, int bAnimated )
{
   FILE *f;
   unsigned int i, i2;
   char *texturePath;
   dlTexture   *texture;
   unsigned int num_faces;
   unsigned int start = 0;

#if ICONV_SJIS_PMD
   char *utf8;
#endif

#if ATLAS_METHOD
   unsigned int ix;
   dlAtlas *atlas;
   dlTexture **textureList;
#else
   dlObject *mObject;
#endif

   mmd_header header;
   mmd_data   *mmd;


   CALL("%p, %s, %d", object, file, bAnimated)
   LOGINFOP("Attempt to load: %s", file);

   /* Yush! Open the file */
   f = fopen( (char*)file, "rb" );
   if(!f)
   {
      LOGERRP("File: %s, could not open", file);

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   if( mmd_readHeader( f, &header ) != RETURN_OK )
   {
      LOGERR("Failed to read header");

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* print info about our PMD */
#if ICONV_SJIS_PMD
   if((utf8 = convertSjisToUtf8( header.name )))
   { LOGINFO( utf8 ); free( utf8 ); }

   if((utf8 = convertSjisToUtf8( header.comment )))
   { LOGINFO( utf8 ); free( utf8 ); }
#endif

   /* ok, lets create MMD data structure that holds
    * everything we need */
   mmd = newMMD();
   if(!mmd)
   {
      /* yikes, failed to allocate */
      LOGERR("Failed to allocate MMD data structure");
      fclose(f);

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read vertex data */
   if( mmd_readVertexData( f, mmd ) != RETURN_OK )
   {

      LOGERR("Failed to read vertex data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read index data */
   if( mmd_readIndexData( f, mmd ) != RETURN_OK )
   {
      LOGERR("Failed to read index data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read material data */
   if( mmd_readMaterialData( f, mmd ) != RETURN_OK )
   {
      LOGERR("Failed to read material data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read bone data */
   if( mmd_readBoneData( f, mmd ) != RETURN_OK )
   {
      LOGERR("Failed to read bone data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read IK data */
   if( mmd_readIKData( f, mmd ) != RETURN_OK )
   {
      LOGERR("Failed to read IK data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read Skin data */
   if( mmd_readSkinData( f, mmd ) != RETURN_OK )
   {
      LOGERR("Failed to read Skin data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read Skin display data */
   if( mmd_readSkinDisplayData( f, mmd ) != RETURN_OK )
   {
      LOGERR("Failed to read Skin display data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* read bone name data */
   if( mmd_readBoneNameData( f, mmd ) != RETURN_OK )
   {
      LOGERR("Failed to read bone name data");
      fclose(f);
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* print vertex info */
   LOGINFOP("V: %d", mmd->num_vertices);
   LOGINFOP("I: %d", mmd->num_indices);
   LOGINFOP("B: %d", mmd->num_bones);
   LOGINFOP("BN: %d", mmd->num_bone_names);
   LOGINFOP("IK: %d", mmd->num_ik);
   LOGINFOP("S: %d", mmd->num_skins);
   LOGINFOP("SD: %d", mmd->num_skin_displays);

   /* close file */
   fclose(f);

#if ATLAS_METHOD

   /* texture list which we use to retive transformed coords */
   textureList = malloc( sizeof(dlTexture*) * mmd->num_materials );
   if(!textureList)
   {
      LOGERR("Failed to allocate texture list for atlas usage");
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
      return( RETURN_FAIL );
   }

   /* create atlas */
   atlas = dlNewAtlas();
   if(!atlas)
   {
      LOGERR("Failed to allocate texture atlas");
      free( textureList );
      freeMMD( mmd );

      RET("%d", RETURN_FAIL);
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
   if(object->material) dlFreeMaterial(object->material);
   object->material = dlNewMaterialFromTexture( texture );

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
            LOGERR("Could not allocate child object");
            freeMMD( mmd );

            RET("%d", RETURN_FAIL);
            return( RETURN_FAIL );
         }

         /* ref VBO new IBO */
         mObject->vbo = dlRefVBO( object->vbo );
         mObject->ibo = dlNewIBO();
         if(!mObject->ibo)
         {
            LOGERR("Could not allocate child IBO");
            dlFreeObject( mObject );
            freeMMD( mmd );

            RET("%d", RETURN_FAIL);
            return( RETURN_FAIL );
         }

         dlObjectAddChild( object, mObject );
      }
      else  mObject = object;

      /* texture */
      texturePath = dlImportTexturePath( mmd->texture[i].file, file );
      if(texturePath)
      {
         object->material = dlNewMaterialFromImage( texturePath, SOIL_FLAG_DEFAULTS |
                                                    SOIL_FLAG_INVERT_Y |
                                                    SOIL_FLAG_TEXTURE_REPEATS);

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
      mObject->primitive_type = GL_TRIANGLES;
   }

#endif

   /* free mmd_data structure */
   freeMMD( mmd );

   RET("%d", RETURN_OK);
   return( RETURN_OK );
}

#endif /* WITH_PMD */
