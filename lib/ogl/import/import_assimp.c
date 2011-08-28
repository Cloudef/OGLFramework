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

const struct aiNode* findNode( const struct aiNode *nd, const char *name )
{
   unsigned int f;

   f = 0;
   for(; f != nd->mNumChildren; ++f)
   {
      if(strcmp( nd->mChildren[f]->mName.data, name ) == 0)
         return( nd->mChildren[f] );
      else
      {
         const struct aiNode *node = findNode( nd->mChildren[f], name );
         if(node) return( node );
      }
   }

   return( NULL );
}

const struct aiBone* findBone( const struct aiMesh *mesh, const char *name )
{
   unsigned int f;

   f = 0;
   for(; f != mesh->mNumBones; ++f)
   {
      if(strcmp( mesh->mBones[f]->mName.data, name ) == 0)
         return( mesh->mBones[f] );
   }

   return( NULL );
}

static glBone*
add_bone(glAnimator *object, const struct aiMesh *mesh, const struct aiNode *bND, const struct aiNode *nd);
static glBone*
add_bone(glAnimator *object, const struct aiMesh *mesh, const struct aiNode *bND, const struct aiNode *nd)
{
   glBone *bone, *child;
   const struct aiBone* aBone;
   unsigned int f = 0;

   /* invalid bone node */
   if(!bND)
      return( NULL );

   assert( bND->mName.data );
   if(!strlen(bND->mName.data))
         return( NULL );

   /* could not find bone */
   aBone = findBone( mesh, bND->mName.data );

   /* add bone */
   bone = glAnimatorAddBone( object );
   if(!bone)
      return( NULL );

   /* assign name */
   bone->name = strdup(bND->mName.data);
   if(!bone->name) { glFreeBone( bone ); return( NULL ); }
   logBlue(); glPrint( "[ASSIMP] %s\n",  bone->name ); logNormal();

   /* only if found, otherwise create dummy bone with only relative translation info */
   if(aBone)
   {
      /* offset matrix */
      bone->offsetMatrix.mat[0]  = aBone->mOffsetMatrix.a1;
      bone->offsetMatrix.mat[1]  = aBone->mOffsetMatrix.a2;
      bone->offsetMatrix.mat[2]  = aBone->mOffsetMatrix.a3;
      bone->offsetMatrix.mat[3]  = aBone->mOffsetMatrix.a4;

      bone->offsetMatrix.mat[4]  = aBone->mOffsetMatrix.b1;
      bone->offsetMatrix.mat[5]  = aBone->mOffsetMatrix.b2;
      bone->offsetMatrix.mat[6]  = aBone->mOffsetMatrix.b3;
      bone->offsetMatrix.mat[7]  = aBone->mOffsetMatrix.b4;

      bone->offsetMatrix.mat[8]  = aBone->mOffsetMatrix.c1;
      bone->offsetMatrix.mat[9]  = aBone->mOffsetMatrix.c2;
      bone->offsetMatrix.mat[10] = aBone->mOffsetMatrix.c3;
      bone->offsetMatrix.mat[11] = aBone->mOffsetMatrix.c4;

      bone->offsetMatrix.mat[12] = aBone->mOffsetMatrix.d1;
      bone->offsetMatrix.mat[13] = aBone->mOffsetMatrix.d2;
      bone->offsetMatrix.mat[14] = aBone->mOffsetMatrix.d3;
      bone->offsetMatrix.mat[15] = aBone->mOffsetMatrix.d4;

      /* default global */
      bone->globalMatrix = bone->offsetMatrix;

      /* add weights */
      f = 0;
      for(; f != aBone->mNumWeights; ++f)
      {
         if(!glBoneAddWeight( bone, aBone->mWeights[f].mVertexId,
                              aBone->mWeights[f].mWeight ))
         { glFreeBone( bone ); return( NULL ); }
      }
   }

   /* relative matrix */
   bone->relativeMatrix.mat[0]  = bND->mTransformation.a1;
   bone->relativeMatrix.mat[1]  = bND->mTransformation.a2;
   bone->relativeMatrix.mat[2]  = bND->mTransformation.a3;
   bone->relativeMatrix.mat[3]  = bND->mTransformation.a4;

   bone->relativeMatrix.mat[4]  = bND->mTransformation.b1;
   bone->relativeMatrix.mat[5]  = bND->mTransformation.b2;
   bone->relativeMatrix.mat[6]  = bND->mTransformation.b3;
   bone->relativeMatrix.mat[7]  = bND->mTransformation.b4;

   bone->relativeMatrix.mat[8]  = bND->mTransformation.c1;
   bone->relativeMatrix.mat[9]  = bND->mTransformation.c2;
   bone->relativeMatrix.mat[10] = bND->mTransformation.c3;
   bone->relativeMatrix.mat[11] = bND->mTransformation.c4;

   bone->relativeMatrix.mat[12] = bND->mTransformation.d1;
   bone->relativeMatrix.mat[13] = bND->mTransformation.d2;
   bone->relativeMatrix.mat[14] = bND->mTransformation.d3;
   bone->relativeMatrix.mat[15] = bND->mTransformation.d4;

   /* assing childs */
   f = 0;
   for(; f != bND->mNumChildren; ++f)
   {
      child = add_bone( object, mesh, findNode( nd, bND->mChildren[f]->mName.data ), nd );
      glBoneAddChild( bone, child );
   }

   return(bone);
}

/* Construct bones */
static int construct_bones(glAnimator *object, const struct aiNode *nd, const struct aiMesh *mesh)
{
   /* no bones, no milk */
   if(!mesh->mNumBones)
      return( RETURN_NOTHING );

   /* assign bones */
   logBlue(); glPrint("[ASSIMP] Found %d bones\n", mesh->mNumBones); logNormal();
   if(!add_bone( object, mesh, nd, nd ))
   { logRed(); glPuts("[ASSIMP] Bones failed"); logNormal(); return( RETURN_FAIL ); }

   return( RETURN_OK );
}

/* construct animation */
static int construct_animation(glAnimator *object, const struct aiScene *sc, const struct aiMesh *mesh)
{
   unsigned int i, f, t;
   glBone         *bone;
   glAnim         *anim;
   glNodeAnim     *node;
   kmVec3         vec3value;
   kmQuaternion   quatvalue;

#if USE_KEY_ANIMATION
   /* mesh->mAnimMeshes
    * mesh->mNumAnimMeshes */
#endif

   /* no need to do this */
   if(!sc->mNumAnimations)
      return( RETURN_NOTHING );

   logBlue(); glPrint("[ASSIMP] Found %d animations\n", sc->mNumAnimations); logNormal();

   /* add animations */
   i = 0;
   for(; i != sc->mNumAnimations; ++i)
   {
      anim = glAnimatorAddAnim( object );
      if(!anim) return( RETURN_FAIL );

      anim->ticksPerSecond = sc->mAnimations[i]->mTicksPerSecond;
      anim->duration       = sc->mAnimations[i]->mDuration;

      /* check name */
      assert( sc->mAnimations[i]->mName.data );
      anim->name = strdup(sc->mAnimations[i]->mName.data);
      if(!anim->name) return(RETURN_FAIL);

      logBlue(); glPrint( "[ASSIMP] %s\n", anim->name );
                 glPrint( "[ASSIMP] %d channels\n", sc->mAnimations[i]->mNumChannels ); logNormal();
      /* add nodes */
      f = 0;
      for(; f != sc->mAnimations[i]->mNumChannels; ++f)
      {
         bone = glAnimatorGetBone( object, sc->mAnimations[i]->mChannels[f]->mNodeName.data );
         if(!bone || !bone->name) continue;

         node = glAnimAddNode( anim );
         if(!node) return( RETURN_FAIL );

         /* pointer to affected bone */
         node->bone = bone;

         /* add translation keys */
         t = 0;
         for(; t != sc->mAnimations[i]->mChannels[f]->mNumPositionKeys; ++t)
         {
            vec3value.x = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.x;
            vec3value.y = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.y;
            vec3value.z = sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mValue.z;

            if(!glNodeAddTranslationKey( node, vec3value,
                sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mTime ))
               return( RETURN_FAIL );
         }

         /* add rotation keys */
         t = 0;
         for(; t != sc->mAnimations[i]->mChannels[f]->mNumRotationKeys; ++t)
         {
            quatvalue.x = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.x;
            quatvalue.y = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.y;
            quatvalue.z = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.z;
            quatvalue.w = sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mValue.w;
            if(!glNodeAddRotationKey( node, quatvalue,
                sc->mAnimations[i]->mChannels[f]->mRotationKeys[t].mTime ))
               return( RETURN_FAIL );
         }

         /* add scaling keys */
         t = 0;
         for(; t != sc->mAnimations[i]->mChannels[f]->mNumScalingKeys; ++t)
         {
            vec3value.x = sc->mAnimations[i]->mChannels[f]->mScalingKeys[t].mValue.x;
            vec3value.y = sc->mAnimations[i]->mChannels[f]->mScalingKeys[t].mValue.y;
            vec3value.z = sc->mAnimations[i]->mChannels[f]->mScalingKeys[t].mValue.z;
            if(!glNodeAddScalingKey( node, vec3value,
                sc->mAnimations[i]->mChannels[f]->mPositionKeys[t].mTime ))
               return( RETURN_FAIL );
         }
      }
   }

   /* return */
   return( RETURN_OK );
}

/* Construct our model */
static int construct(const char *file, glObject *object, const struct aiScene *sc, const struct aiNode *nd, const struct aiMesh *mesh, int bAnimated)
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
   if(construct_bones( object->animator, sc->mRootNode, mesh )     == RETURN_FAIL)
   { glFreeAnimator( object->animator ); object->animator = NULL; return( RETURN_OK ); }
   if(construct_animation( object->animator, sc, mesh ) == RETURN_FAIL)
   { glFreeAnimator( object->animator ); object->animator = NULL; return( RETURN_OK ); }

   /* calculate global transforms */
   glAnimatorCalculateGlobalTransformations( object->animator );

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
         if(construct( file, mObject, sc, nd, mesh, bAnimated ) != RETURN_OK)
            return( RETURN_FAIL );

         /* We modified main object */
         bConstructed = 1;
      }
      else
      {
#if 0
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

         if(construct( file, mObject, sc, nd, mesh, bAnimated ) != RETURN_OK)
         {
            glFreeObject( mObject );
            return( RETURN_FAIL );
         }
#endif
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
      glVBOPrepareTstance( object->vbo );

   return( RETURN_OK );
}

#endif /* WITH_ASSIMP */
