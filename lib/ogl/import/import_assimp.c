#include "config.h"
#if WITH_ASSIMP

#include <stdio.h>
#include <malloc.h>

#include <assimp/assimp.h>
#include <assimp/aiScene.h>
#include <assimp/aiPostProcess.h>

#include "sceneobject.h"
#include "vbo.h"
#include "types.h"
#include "import.h"
#include "texture.h"
#include "logfile_wrapper.h"
#include "core.h"
#include "alloc.h"

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

/* Construct bones */
static int construct_bones(glAnimator *object, const struct aiScene *sc, const struct aiMesh *mesh)
{
   unsigned int i, f;
   glBone **bone;

   /* no bones, no milk */
   if(!mesh->mNumBones)
      return( RETURN_NOTHING );

    bone = glAnimatorResizeBones( object, mesh->mNumBones );
    if(!bone)
    {
       /* no anims nor bones for us, sucks to be low on memory */
       return( RETURN_NOTHING );
    }

    logBlue(); glPrint("[ASSIMP] Found %d bones\n", mesh->mNumBones); logNormal();

    i = 0;
    for(; i != mesh->mNumBones; ++i)
    {
      if(mesh->mBones[i]->mName.data)
      {
         bone[i]->name = strdup(mesh->mBones[i]->mName.data);
         logBlue(); glPrint( "[ASSIMP] %s\n",  bone[i]->name ); logNormal();
      }

      bone[i]->offsetMatrix.mat[0] = mesh->mBones[i]->mOffsetMatrix.a1;
      bone[i]->offsetMatrix.mat[1] = mesh->mBones[i]->mOffsetMatrix.b1;
      bone[i]->offsetMatrix.mat[2] = mesh->mBones[i]->mOffsetMatrix.c1;
      bone[i]->offsetMatrix.mat[3] = mesh->mBones[i]->mOffsetMatrix.d1;

      bone[i]->offsetMatrix.mat[4] = mesh->mBones[i]->mOffsetMatrix.a2;
      bone[i]->offsetMatrix.mat[5] = mesh->mBones[i]->mOffsetMatrix.b2;
      bone[i]->offsetMatrix.mat[6] = mesh->mBones[i]->mOffsetMatrix.c2;
      bone[i]->offsetMatrix.mat[7] = mesh->mBones[i]->mOffsetMatrix.d2;

      bone[i]->offsetMatrix.mat[8] = mesh->mBones[i]->mOffsetMatrix.a3;
      bone[i]->offsetMatrix.mat[9] = mesh->mBones[i]->mOffsetMatrix.b3;
      bone[i]->offsetMatrix.mat[10] = mesh->mBones[i]->mOffsetMatrix.c3;
      bone[i]->offsetMatrix.mat[11] = mesh->mBones[i]->mOffsetMatrix.d3;

      bone[i]->offsetMatrix.mat[12] = mesh->mBones[i]->mOffsetMatrix.a4;
      bone[i]->offsetMatrix.mat[13] = mesh->mBones[i]->mOffsetMatrix.b4;
      bone[i]->offsetMatrix.mat[14] = mesh->mBones[i]->mOffsetMatrix.c4;
      bone[i]->offsetMatrix.mat[15] = mesh->mBones[i]->mOffsetMatrix.d4;

      bone[i]->num_weights  = mesh->mBones[i]->mNumWeights;

      bone[i]->weight
         = glCalloc( mesh->mBones[i]->mNumWeights, sizeof(glVertexWeight) );

      f = 0;
      for(; f != mesh->mBones[i]->mNumWeights; ++f)
      {
         bone[i]->weight[f].vertex = mesh->mBones[i]->mWeights[f].mVertexId;
         bone[i]->weight[f].weight = mesh->mBones[i]->mWeights[f].mWeight;
      }
    }

    object->bone = bone;
    return( RETURN_OK );
}

/* construct animation */
static int construct_animation(glAnimator *object, const struct aiScene *sc, const struct aiMesh *mesh)
{
   unsigned int i, f, t;
   glAnim **anim;

#if USE_KEY_ANIMATION
   /* mesh->mAnimMeshes
    * mesh->mNumAnimMeshes */
#endif

   /* no need to do this */
   if(!sc->mNumAnimations)
      return( RETURN_NOTHING );

   anim = glAnimatorResizeAnims( object, sc->mNumAnimations );
   if(!anim)
   {
      /* It's ok, we just don't have animations */
      return( RETURN_NOTHING );
   }

   logBlue(); glPrint("[ASSIMP] Found %d animations\n", sc->mNumAnimations); logNormal();

   i = 0;
   for(; i != sc->mNumAnimations; ++i)
   {
      anim[i]->skeletal     = glCalloc( sc->mAnimations[i]->mNumChannels,
                                                sizeof(glNodeAnim) );
      anim[i]->num_skeletal = sc->mAnimations[i]->mNumChannels;

      anim[i]->ticksPerSecond = sc->mAnimations[i]->mTicksPerSecond;
      anim[i]->duration       = sc->mAnimations[i]->mDuration;

      if(sc->mAnimations[i]->mName.data)
      {
         anim[i]->name           = strdup(sc->mAnimations[i]->mName.data);
         logBlue(); glPrint( "[ASSIMP] %s\n", anim[i]->name ); logNormal();
      }

      f = 0;
      for(; f != sc->mAnimations[i]->mNumChannels; ++f)
      {
         anim[i]->skeletal[f].num_position
            = sc->mAnimations[i]->mChannels[f]->mNumPositionKeys;

         anim[i]->skeletal[f].num_rotation
            = sc->mAnimations[i]->mChannels[f]->mNumRotationKeys;

         anim[i]->skeletal[f].num_scale
            = sc->mAnimations[i]->mChannels[f]->mNumScalingKeys;

         anim[i]->skeletal[f].positionKey
            = glCalloc( sc->mAnimations[i]->mChannels[f]->mNumPositionKeys, sizeof(glVectorKey) );

         anim[i]->skeletal[f].rotationKey
            = glCalloc( sc->mAnimations[i]->mChannels[f]->mNumRotationKeys, sizeof(glQuatKey) );

         anim[i]->skeletal[f].scaleKey
            = glCalloc( sc->mAnimations[i]->mChannels[f]->mNumScalingKeys, sizeof(glVectorKey) );

         t = 0;
         for(; t != sc->mAnimations[i]->mChannels[f]->mNumPositionKeys; ++t)
         {
            anim[i]->skeletal[f].positionKey[t].time
               = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mTime;
            anim[i]->skeletal[f].positionKey[t].value.x
               = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.x;
            anim[i]->skeletal[f].positionKey[t].value.y
               = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.y;
            anim[i]->skeletal[f].positionKey[t].value.z
               = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.z;
         }
      }
   }

   object->anim = anim;
   return( RETURN_OK );
}

/* Construct our model */
static int construct(const char *file, glObject *object, const struct aiScene *sc, const struct aiMesh *mesh, int bAnimated)
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

   /* be done, if we only want static stuff */
   if(!bAnimated || object->animator)
      return( RETURN_OK );

   /* create new animator */
   object->animator = glNewAnimator();

   /* assign bones and animations */
   construct_bones( object->animator, sc, mesh );
   construct_animation( object->animator, sc, mesh );

   /* set animation to 0 */
   glObjectSetAnimation( object, 0 );

   /* return our beautiful object */
   return( RETURN_OK );
}


/* Avoid overriding main object */
static int bConstructed = 0;

/* Process our model */
static int process(const char *file, glObject *object, const struct aiScene *sc, const struct aiNode *nd, int bAnimated)
{
   glObject *mObject = object;

   unsigned int m = 0;
   for(; m != nd->mNumMeshes; ++m)
   {
      const struct aiMesh *mesh = sc->mMeshes[nd->mMeshes[m]];

      if(!bConstructed)
      {
         /* Construct */
         if(construct( file, mObject, sc, mesh, bAnimated ) != RETURN_OK)
            return( RETURN_FAIL );

         /* We modified main object */
         bConstructed = 1;
      }
      else
      {
         mObject = glNewObject();
         if(!mObject)
            return( RETURN_FAIL );

         mObject->vbo = glNewVBO();
         if(!mObject->vbo)
         {
            glFreeObject( mObject );
            return( RETURN_FAIL );
         }
         mObject->ibo = glNewIBO();
         if(!mObject->ibo)
         {
            glFreeObject( mObject );
            return( RETURN_FAIL );
         }

         /* refence animation */
         mObject->animator = glRefAnimator( object->animator );

         if(construct( file, mObject, sc, mesh, bAnimated ) != RETURN_OK)
         {
            glFreeObject( mObject );
            return( RETURN_FAIL );
         }
      }
   }

   /* Actual child code is above ^^ */
   m = 0;
   for(; m != nd->mNumChildren; ++m)
   {
      if(process( file, mObject, sc, nd->mChildren[m], bAnimated ) != RETURN_OK)
         return( RETURN_FAIL );
   }

   return( RETURN_OK );
}

/* Import using Assimp */
int glImportASSIMP( glObject *object, const char *file, int bAnimated )
{
   logBlue(); glPrint("[ASSIMP] attempt to load: %s\n", file ); logNormal();

   /* Assimp magic */
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
   if(process( file, object, scene, scene->mRootNode, bAnimated ) != RETURN_OK)
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

   /* if was animated */
   if(object->animator)
   {
      glAnimatorCopyIdleVertices( object->animator, object->vbo );
   }

   return( RETURN_OK );
}

#endif /* WITH_ASSIMP */
