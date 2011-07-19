#include "config.h"
#if WITH_PMD

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "core.h"
#include "sceneobject.h"
#include "types.h"
#include "import.h"
#include "logfile_wrapper.h"
#include "texture.h"
#include "atlas.h"

/* importer */
#include "mmd_import/mmd.h"

/* maybe map the basic GL enums
 * to own structure, so these become useless */
#ifdef GLES2
#	include <GLES2/gl2.h>
#elif  GLES1
#  include <GLES/gl.h>
#  include <GLES/glext.h>
#else
#	include <GL/glew.h>
#  include <GL/gl.h>
#endif

/*
 * 0 = one VBO and seperate IBO's and textures for childs
 * 1 = one atlas texture and seperate VBO's and IBO's for childs
 */
#define ATLAS_METHOD 1

/* Import MikuMikuDance PMD file */
int glImportPMD( glObject* object, const char *file, int bAnimated )
{
   FILE *f;
   unsigned int i, i2;
   char *texturePath;
   glTexture   *texture;
   unsigned int num_faces;
   unsigned int start = 0;

#if ATLAS_METHOD
   unsigned int ix;
   glAtlas *atlas;
   glTexture **textureList;
#else
   glObject    *mObject;
#endif

   mmd_header header;
   mmd_data   *mmd;

   logBlue(); glPrint("[PMD] attempt to load: %s\n", file ); logNormal();

   /* Yush! Open the file */
   f = fopen( (char*)file, "rb" );
   if(!f)
   {
      logRed();
      glPrint("[PMD] file: %s, could not open\n", file );
      logNormal();

      return( RETURN_FAIL );
   }

   if( mmd_readHeader( f, &header ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read header");
      logNormal();

      return( RETURN_FAIL );
   }

   /* print info about our PMD */
#if ICONV_SJIS_PMD
   glPuts("");
   glPuts( header.name );
   glPuts("");
   glPuts( header.comment );
   glPuts("");
#endif

   /* ok, lets create MMD data structure that holds
    * everything we need */
   mmd = newMMD();
   if(!mmd)
   {
      /* yikes, failed to allocate */
      logRed();
      glPuts("[PMD] failed to allocate MMD data structure");
      logNormal();

      fclose(f);
      return( RETURN_FAIL );
   }

   /* read vertex data */
   if( mmd_readVertexData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read vertex data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read index data */
   if( mmd_readIndexData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read index data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read material data */
   if( mmd_readMaterialData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read material data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read bone data */
   if( mmd_readBoneData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read bone data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read IK data */
   if( mmd_readIKData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read IK data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read Skin data */
   if( mmd_readSkinData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read Skin data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read Skin display data */
   if( mmd_readSkinDisplayData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read Skin display data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* read bone name data */
   if( mmd_readBoneNameData( f, mmd ) != RETURN_OK )
   {
      logRed();
      glPuts("[PMD] failed to read bone name data");
      logNormal();

      fclose(f);
      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* print vertex info */
   glPrint("V: %d\n", mmd->num_vertices);
   glPrint("I: %d\n", mmd->num_indices);
   glPrint("B: %d\n", mmd->num_bones);
   glPrint("BN: %d\n", mmd->num_bone_names);
   glPrint("IK: %d\n", mmd->num_ik);
   glPrint("S: %d\n", mmd->num_skins);
   glPrint("SD: %d\n", mmd->num_skin_displays);

   /* close file */
   fclose(f);

#if ATLAS_METHOD

   /* texture list which we use to retive transformed coords */
   textureList = malloc( sizeof(glTexture*) * mmd->num_materials );
   if(!textureList)
   {
      logRed();
      glPuts( "[PMD] failed to allocate texture list for atlas usage" );
      logNormal();

      freeMMD( mmd );
      return( RETURN_FAIL );
   }

   /* create atlas */
   atlas = glNewAtlas();
   if(!atlas)
   {
      logRed();
      glPuts( "[PMD] failed to allocate texture atlas" );
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

      texturePath = glImportTexturePath( mmd->texture[i].file, file );
      if(texturePath)
      {
         texture = glNewTexture( texturePath, SOIL_FLAG_NONE );
         if(texture)
            glAtlasAddTexture( atlas, texture );
         textureList[i] = texture;

         free( texturePath );
      }
   }

   /* pack textures */
   texture = glAtlasPack( atlas, 1, 0  );
   glRefTexture( texture );

   /* assign atlas texture to object */
   glObjectAddTexture( object, 0, texture );

   /* reset buffers */
   glResetIndexBuffer( object->ibo,    mmd->num_indices );
   glResetVertexBuffer( object->vbo,   mmd->num_vertices );
   glResetNormalBuffer( object->vbo,   mmd->num_vertices );
   glResetCoordBuffer( object->vbo, 0, mmd->num_vertices );

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
         glInsertVertex( object->vbo,
               mmd->vertices[ ix * 3     ],
               mmd->vertices[ ix * 3 + 1 ],
               mmd->vertices[ ix * 3 + 2 ] );

         glInsertCoord( object->vbo, 0,
               mmd->coords[ ix * 2     ],
               mmd->coords[ ix * 2 + 1 ] );

         glInsertNormal( object->vbo,
               mmd->normals[ ix * 3     ],
               mmd->normals[ ix * 3 + 1 ],
               mmd->normals[ ix * 3 + 2 ] );

         /* fix coords */
         if(object->vbo->uvw[0].coords[i2].y < 0.0f)
            object->vbo->uvw[0].coords[i2].y += 1;
         if(object->vbo->uvw[0].coords[i2].x < 0.0f)
            object->vbo->uvw[0].coords[i2].x += 1;

         //glPrint( "%f, %f\n", object->vbo->uvw[0].coords[i2].x, object->vbo->uvw[0].coords[i2].y);
         glAtlasGetTransformed( atlas, textureList[i], &object->vbo->uvw[0].coords[i2] );

         glInsertIndex( object->ibo, i2 );

      }
      //glPuts("");
      start += num_faces;
   }

   /* free atlas */
   glFreeAtlas( atlas );
   free( textureList );

   /* GL_TRIANGLES object */
   object->primitive_type = GL_TRIANGLES;

#else

   glResetVertexBuffer( object->vbo,   mmd->num_vertices );
   glResetNormalBuffer( object->vbo,   mmd->num_vertices );
   glResetCoordBuffer( object->vbo, 0, mmd->num_vertices );

   i = 0;
   for(; i != mmd->num_vertices; ++i)
   {
         glInsertVertex( object->vbo,
               mmd->vertices[ i * 3     ],
               mmd->vertices[ i * 3 + 1 ],
               mmd->vertices[ i * 3 + 2 ] );

         glInsertCoord( object->vbo, 0,
               mmd->coords[ i * 2     ],
               mmd->coords[ i * 2 + 1 ] );

         glInsertNormal( object->vbo,
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
         mObject = glNewObject();
         if(!mObject)
         {
            logRed();
            glPuts( "[PMD] could not allocate child object" );
            logNormal();

            freeMMD( mmd );
            return( RETURN_FAIL );
         }

         /* ref VBO new IBO */
         mObject->vbo = glRefVBO( object->vbo );
         mObject->ibo = glNewIBO();
         if(!mObject->ibo)
         {
            logRed();
            glPuts( "[PMD] could not allocate child IBO" );
            logNormal();

            glFreeObject( mObject );
            freeMMD( mmd );
            return( RETURN_FAIL );
         }

         glObjectAddChild( object, mObject );
      }
      else  mObject = object;

      /* texture */
      texturePath = glImportTexturePath( mmd->texture[i].file, file );
      if(texturePath)
      {
         texture = glNewTexture( texturePath, 0 );
         if(texture)
            glObjectAddTexture( mObject, 0, texture );

         free( texturePath );
      }

      /* feed indices */
      num_faces = mmd->face[i];
      glResetIndexBuffer( mObject->ibo,    num_faces );

      i2 = start;
      for(; i2 != start + num_faces; ++i2)
      {
         glInsertIndex( mObject->ibo, mmd->indices[i2] );
      }
      start += num_faces;

      /* GL_TRIANGLES object */
      mObject->primitive_type = GL_TRIANGLES;
   }

#endif

   /* free mmd_data structure */
   freeMMD( mmd );

   return( RETURN_OK );
}

#endif /* WITH_PMD */
