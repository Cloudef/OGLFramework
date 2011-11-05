#include <stdio.h>
#include "dlSceneobject.h"
#include "dlVbo.h"
#include "dlLog.h"

/* create plane */
dlObject* dlNewPlane( const kmScalar width, const kmScalar height, int center )
{
   float x1, y1;
   float x2, y2;

   kmVec3 min, max;
   kmAABB aabb_box;

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

   return(object);
}

/* create grid plane */
dlObject* dlNewGrid( int rows,
                     int columns,
                     const kmScalar size )
{
   int i, i2;
   kmVec3 min, max;
   kmAABB aabb_box;

   /* check */
   if(!rows || !columns)
   {
      dlPuts("[GRID] rows && columns must be over 0");
      return(NULL);
   }

   /* new sceneobject */
   dlObject *object = dlNewObject();
   if(!object)
      return(NULL);

   /* check VBO */
   if(!object->vbo)
      object->vbo = dlNewVBO();

   /* check IBO */
   if(!object->ibo)
      object->ibo = dlNewIBO();

   /* reset vertex buffer and insert vertices */
   dlResetVertexBuffer( object->vbo, rows * columns * 2 );

   i = -rows;
   for(; i != rows + 1; ++i)
   {
      i2 = -columns;
      for(; i2 != columns + 1; ++i2)
          dlInsertVertex( object->vbo, (float)i * size, (float)i2 * size, 0 );
   }

   /* we can get AABB here */
   min.x = (float)-rows    * size;
   min.y = (float)-columns * size;
   max.x = (float)rows     * size;
   max.y = (float)columns  * size;

   rows     *= 2; rows    += 1;
   columns  *= 2; columns += 1;

   /* indices */
   dlResetIndexBuffer( object->ibo, rows * columns );

   i = 0;
   for(; i != rows - 1; ++i)
   {
      dlInsertIndex( object->ibo, i * columns);

      i2 = 0;
      for(; i2 != columns; ++i2)
      {
         dlInsertIndex( object->ibo, i * columns + i2 );
         dlInsertIndex( object->ibo, (i + 1) * columns + i2 );
      }
      dlInsertIndex( object->ibo, (i + 1) * columns + (columns - 1) );
   }

   /* we just use the info above to get bounding box,
    * for complex models (eg. import) you can just use dlCalculateAABB(object); */
   aabb_box.min = min;
   aabb_box.max = max;

   /* Assign aabb */
   object->aabb_box = aabb_box;

   /* ^ No need to calculate */
   /* dlCalculateAABB(object); */

   return(object);
}
