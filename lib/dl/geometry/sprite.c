                #include <stdio.h>
#include "dlSceneobject.h"
#include "dlVbo.h"
#include "dlLog.h"

/* create new sprite */
dlObject* dlNewSprite( const char *image, unsigned int flags, int center )
{
   dlTexture *texture;

   float x1, y1;
   float x2, y2;
   float width, height;

   kmVec3 min, max;
   kmAABB aabb_box;

   /* load image */
   texture = dlNewTexture( image, flags );
   if(!texture)
      return(NULL);

   width  = (float)texture->width  / 42000.0f;
   height = (float)texture->height / 42000.0f;

   /* new sceneobject */
   dlObject *object = dlNewObject();
   if(!object)
      return(NULL);

   /* check VBO */
   if(!object->vbo)
      object->vbo = dlNewVBO();

   if(!center)
   {
      x1 = 0;
      y1 = 0;
      x2 = width;
      y2 = height;
   }
   else
   {
      x1 = width  / 2;
      y1 = height / 2;
      x2 = width  / 2;
      y2 = height / 2;
   }

   /* reset vertex buffer and insert vertices */
   dlResetVertexBuffer( object->vbo, 4 );
   dlInsertVertex( object->vbo, +x2, +y2, 0 );
   dlInsertVertex( object->vbo, -x1, +y2, 0 );
   dlInsertVertex( object->vbo, +x2, -y1, 0 );
   dlInsertVertex( object->vbo, -x1, -y1, 0 );

   /* TO-DO:
    * Missing normals */

   /* reset texture coord buffer and insert coords */
   dlResetCoordBuffer( object->vbo, 0, 4 );
   dlInsertCoord( object->vbo, 0, 1, 1 );
   dlInsertCoord( object->vbo, 0, 0, 1 );
   dlInsertCoord( object->vbo, 0, 1, 0 );
   dlInsertCoord( object->vbo, 0, 0, 0 );

   /* needs some normals */

   /* we just use the info above to get bounding box,
    * for complex models (eg. import) you can just use dlCalculateAABB(object); */

   min.x = -x1; min.y = -y1; min.z = 0;
   max.x = +x2; max.y = +y2; max.z = 0;

   aabb_box.min = min;
   aabb_box.max = max;

   /* Assign aabb */
   object->aabb_box = aabb_box;

   /* ^ No need to calculate */
   /* dlCalculateAABB(object); */

   /* Assign texture */
   object->material = dlNewMaterialFromTexture(texture);
   if(!object->material)
   {
      dlFreeTexture(texture);
      dlFreeObject(object);
      object = NULL;
   }

   return(object);
}
