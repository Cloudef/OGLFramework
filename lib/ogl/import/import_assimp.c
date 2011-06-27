#include "config.h"
#if WITH_ASSIMP

#include <stdio.h>
#include <malloc.h>
#include <assimp/assimp.h>        // Plain-C interface
#include <assimp/aiScene.h>       // Output data structure
#include <assimp/aiPostProcess.h> // Post processing flags

#include "sceneobject.h"
#include "vbo.h"
#include "types.h"
#include "import.h"
#include "texture.h"
#include "logfile_wrapper.h"
#include "core.h"

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

/* Set material */
static int setMaterial(const char *file, glObject *object, struct aiMaterial *mtl)
{
   unsigned int i, numDiffuse;

   struct aiString         textureName;
   enum aiTextureMapping   textureMapping;
   enum aiTextureOp        op;
   enum aiTextureMapMode   textureMapMode[3];
   unsigned int            uvwIndex, flags;
   float                   blend;

   char *texturePath;
   glTexture *texture;

   numDiffuse = aiGetMaterialTextureCount( mtl, aiTextureType_DIFFUSE );
   if(!numDiffuse)
      return( RETURN_NOTHING );

   if(numDiffuse > _glCore.info.maxTextureUnits)
   {
      logBlue();
      glPrint("[ASSIMP] This model has more than %d Textures, some textures won't be shown", _glCore.info.maxTextureUnits );
      logNormal();

      numDiffuse = _glCore.info.maxTextureUnits;
   }

   i = 0;
   for(; i != numDiffuse; ++i)
   {
      if( aiGetMaterialTexture( mtl, aiTextureType_DIFFUSE, i,
                                &textureName, &textureMapping,
                                &uvwIndex, &blend, &op,
                                textureMapMode, &flags ) != AI_SUCCESS )
      {
         logRed();
         glPuts("[ASSIMP] Failed to fetch texture info from material");
         logNormal();

         return( RETURN_FAIL );
      }

      texturePath = glImportTexturePath( textureName.data,
                                         file );
      if( texturePath )
      {
         texture = glNewTexture( texturePath, 0 );
         if(texture)
            glObjectAddTexture( object, i, texture );

         free( texturePath );
      }
   }

   return( RETURN_OK );
}

/* Construct our model */
static int construct(const char *file, glObject *object, const struct aiScene *sc, const struct aiMesh *mesh)
{
   unsigned int i = 0;
   unsigned int index = 0;
   unsigned int f = 0, t = 0;

   /* Check out what our mesh has */
   if(mesh->mVertices)
      glResetVertexBuffer( object->vbo, mesh->mNumVertices );
   if(mesh->mNormals)
      glResetNormalBuffer( object->vbo, mesh->mNumVertices );

   object->vbo->v_use = object->vbo->v_num;
   object->vbo->n_use = object->vbo->n_num;

   /* Texture coords */
   i = 0;
   while( i != _glCore.info.maxTextureUnits )
   {
      if(mesh->mTextureCoords[i])
         glResetCoordBuffer( object->vbo, i, mesh->mNumVertices );

      object->vbo->uvw[i].c_use = object->vbo->uvw[i].c_num;
      i++;
   }

   /* We don't know how many indices there is total
    * so let the glInsertIndex command to allocate dynamically */
   glResetIndexBuffer( object->ibo, 1 );

   /* Material check */
   if(mesh->mMaterialIndex)
   {
      setMaterial( file, object,  sc->mMaterials[mesh->mMaterialIndex] );
   }

   /* Yush! Then assing the data to our structure */
   for(; f != mesh->mNumFaces; ++f)
   {
      /* That's some beautiful face */
      const struct aiFace *face = &mesh->mFaces[f];
      if(!face)
         return( RETURN_FAIL );

      i = 0;
      for(; i != face->mNumIndices; ++i)
      {
         index = face->mIndices[i];

         if(mesh->mVertices)
         {
            object->vbo->vertices[ index ].x =
               mesh->mVertices[index].x;
            object->vbo->vertices[ index ].y =
               mesh->mVertices[index].y;
            object->vbo->vertices[ index ].z =
               mesh->mVertices[index].z;
         }

         if(mesh->mNormals)
         {
            object->vbo->normals[ index ].x =
               mesh->mNormals[index].x;
            object->vbo->normals[ index ].y =
               mesh->mNormals[index].y;
            object->vbo->normals[ index ].z =
               mesh->mNormals[index].z;
         }

         t = 0;
         while( t != _glCore.info.maxTextureUnits )
         {
            if(mesh->mTextureCoords[t])
            {
               object->vbo->uvw[t].coords[ index ].x =
                  mesh->mTextureCoords[t][ index ].x;
               object->vbo->uvw[t].coords[ index ].y =
                  mesh->mTextureCoords[t][ index ].y;
            }

            t++;
         }

         /* finally index */
         glInsertIndex( object->ibo, index );

      }
   }

   /* return our beautiful object */
   return( RETURN_OK );
}


/* Avoid overriding main object */
static int bConstructed = 0;

/* Process our model */
static int process(const char *file, glObject *object, const struct aiScene *sc, const struct aiNode *nd)
{
   unsigned int m = 0;
   for(; m != nd->mNumMeshes; ++m)
   {
      const struct aiMesh *mesh = sc->mMeshes[nd->mMeshes[m]];

      if(!bConstructed)
      {
         /* Construct */
         if(construct( file, object, sc, mesh ) != RETURN_OK)
            return( RETURN_FAIL );

         /* We modified main object */
         bConstructed = 1;
      }
      else
      {
         /* Add childs */
      }
   }

   /* Actual child code is above ^^ */
   m = 0;
   for(; m != nd->mNumChildren; ++m)
   {
      if(process( file, object, sc, nd->mChildren[m] ) != RETURN_OK)
         return( RETURN_FAIL );
   }

   return( RETURN_OK );
}

/* Import using Assimp */
int glImportASSIMP( glObject *object, const char *file, int bAnimated )
{
   logBlue(); glPrint("[ASSIMP] attempt to load: %s\n", file ); logNormal();

   // Start the import on the given file with some example postprocessing
   // Usually - if speed is not the most important aspect for you - you'll t
   // probably to request more postprocessing than we do in this example.
   const struct aiScene* scene = aiImportFile( file,
                                 aiProcess_Triangulate               |
                                 aiProcess_JoinIdenticalVertices     |
                                 aiProcess_SortByPType               |
                                 aiProcess_CalcTangentSpace          |
                                 aiProcess_JoinIdenticalVertices     |
                                 aiProcess_GenSmoothNormals          |
                                 aiProcess_LimitBoneWeights          |
                                 aiProcess_RemoveRedundantMaterials  |
                                 aiProcess_OptimizeMeshes            |
                                 aiProcess_GenUVCoords               |
                                 aiProcess_TransformUVCoords            );

   /* Import failed */
   if(!scene)
   {
      logRed();
      glPrint("[ASSIMP] %s\n", aiGetErrorString());
      logNormal();

      return( RETURN_FAIL );
   }

   /* We haven't done nothing yet */
   bConstructed = 0;

   /* Process it */
   if(process( file, object, scene, scene->mRootNode ) != RETURN_OK)
   {
      logRed();
      glPuts("[ASSIMP] processing of mesh failed");
      logNormal();

      return( RETURN_FAIL );
   }

   /* Release */
   aiReleaseImport( scene );

   /* Set to triangles
    * Add tristripper code? */
   object->primitive_type = GL_TRIANGLES;

   return( RETURN_OK );
}

#endif /* WITH_ASSIMP */
