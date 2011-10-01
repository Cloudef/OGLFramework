#include "dlAtlasPacker.h"
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

typedef struct
{
   int x1;
   int x2;
   int y1;
   int y2;
} rect_t;

static void rectSet( rect_t *rect, int x1, int y1, int x2, int y2 )
{
   rect->x1 = x1;
   rect->y1 = y1;
   rect->x2 = x2;
   rect->y2 = y2;
}


static int rectIntersects( rect_t *r1, rect_t *r2 )
{
   int intersect = 1;

   if( r1->x2 < r2->x1 ||
         r1->x1 > r2->x2 ||
         r1->y2 < r2->y1 ||
         r1->y1 > r2->y2 )
   {
      intersect  = 0;
   }

   return( intersect );
}

typedef struct texture_tt
{
   int width;
   int height;
   int x;
   int y;
   int longestEdge;
   int area;

   int flipped;
   int placed;
} texture_t;


static void textureSet( texture_t *t, int wid, int hit )
{
   t->width   = wid;
   t->height  = hit;
   t->x       = 0;
   t->y       = 0;
   t->flipped = 0;
   t->placed  = 0;
   t->area    = wid * hit;

   if ( wid >= hit )
      t->longestEdge = wid;
   else
      t->longestEdge = hit;
}

static void texturePlace( texture_t *t, int x, int y, int flipped )
{
   t->x       = x;
   t->y       = y;
   if(flipped == 1)
      t->flipped = 1;
   else
      t->flipped = 0;
   t->placed  = 1;
}

typedef struct node_tt
{
   struct node_tt *next;
   int x;
   int y;
   int width;
   int height;
} node_t;

static void nodeSet( node_t *n, int x, int y, int wid, int hit )
{
   n->x       = x;
   n->y       = y;
   n->width   = wid;
   n->height  = hit;
   n->next    = NULL;
}

static int nodeFits( node_t *n, int wid, int hit, int *edgeCount)
{
   int ret = 0;
   int eC  = 0;

   if( wid == n->width || hit == n->height || wid == n->height || hit == n->width )
   {
      if( wid == n->width )
      {
         eC++;
         if( hit == n->height )
            eC++;
      }
      else if( wid == n->height )
      {
         eC++;
         if( hit == n->width )
         {
            eC++;
         }
      }
      else if( hit == n->width )
      {
         eC++;
      }
      else if( hit == n->height )
      {
         eC++;
      }
   }

   if( wid <= n->width && hit <= n->height )
   {
      ret = 1;
   }
   else if( hit <= n->width && wid <= n->height )
   {
      ret = 1;
   }

   *edgeCount = eC;
   return( ret );
}

void nodeGetRect( node_t *n, rect_t *r )
{
   rectSet( r, n->x, n->y, n->x + n->width - 1, n->y + n->height - 1 );
}

void nodeValidate( node_t *n1, node_t *n2 )
{
   rect_t r1;
   rect_t r2;
   nodeGetRect( n1, &r1 );
   nodeGetRect( n2, &r2 );

   assert( !rectIntersects( &r1, &r2 ) );
}

int nodeMerge( node_t *n1, node_t *n2 )
{
   int ret = 0;

   rect_t r1;
   rect_t r2;

   nodeGetRect( n1, &r1 );
   nodeGetRect( n2, &r2 );

   r1.x2++;
   r1.y2++;
   r2.x2++;
   r2.y2++;

   /* if we share the top edge then.. */
   if( r1.x1 == r2.x1 && r1.x2 == r2.x2 && r1.y1 == r2.y2 )
   {
      n1->y        = n2->y;
      n1->height  += n2->height;
      ret          = 1;
   }
   else if( r1.x1 == r2.x1 && r1.x2 == r2.x2 && r1.y2 == r2.y1 ) /* if we share the bottom edge  */
   {
      n1->height  += n2->height;
      ret          = 1;
   }
   else if( r1.y1 == r2.y1 && r1.y2 == r2.y1 && r1.x1 == r2.x2 ) /* if we share the left edge */
   {
      n1->x        = n2->x;
      n1->width   += n2->width;
      ret          = 1;
   }
   else if( r1.y1 == r2.y1 && r1.y2 == r2.y1 && r1.x2 == r2.x1 ) /* if we share the left edge */
   {
      n1->width   += n2->width;
      ret          = 1;
   }

   return( ret );
}

dlTexturePacker* dlNewTexturePacker( void )
{
   dlTexturePacker *tp;

   tp = calloc( 1, sizeof( dlTexturePacker ) );
   if(!tp)
      return( NULL );

   /* null */
   tp->freeList = NULL;
   tp->textures = NULL;

   return( tp );
}

static void dlResetTexturePacker( dlTexturePacker* tp )
{
   tp->textureCount = 0;
   tp->longestEdge  = 0;
   tp->totalArea    = 0;
   tp->textureIndex = 0;
   node_t *next;
   node_t *kill;

   if(tp->textures) free( tp->textures ); tp->textures = NULL;
   if( tp->freeList )
   {
      next = tp->freeList;
      while ( next )
      {
         kill = next;
         next = next->next;
         free( kill ); kill = NULL;
      }
   }
   tp->freeList = NULL;
}

void dlFreeTexturePacker( dlTexturePacker *tp )
{
   dlResetTexturePacker( tp );
   free( tp ); tp = NULL;
}

void dlTexturePackerSetCount( dlTexturePacker *tp, int tcount )
{
   dlResetTexturePacker( tp );

   tp->textureCount = tcount;
   tp->textures     = calloc( tcount, sizeof( texture_t ) );
}

int dlTexturePackerAdd( dlTexturePacker *tp, int wid, int hit )
{
   assert( tp->textureIndex <= tp->textureCount );
   if( tp->textureIndex < tp->textureCount )
   {
      textureSet( &tp->textures[tp->textureIndex], wid, hit );
      tp->textureIndex++;
      if( wid > tp->longestEdge )  tp->longestEdge = wid;
      if( hit > tp->longestEdge )  tp->longestEdge = hit;
      tp->totalArea += wid * hit;
   }
   return( tp->textureIndex - 1 );
}


static void dlTexturePackerNewNode( dlTexturePacker *tp, int x, int y, int wid, int hit )
{
   node_t *node;

   node         = malloc( sizeof(node_t) );
   nodeSet( node, x, y, wid, hit );
   node->next   = tp->freeList;

   tp->freeList = node;
}

static int nextPow2( int v )
{
   int p = 1;
   while ( p < v )
   {
      p = p * 2;
   }

   return( p );
}

static void validate( dlTexturePacker *tp )
{
#ifdef DEBUG
   node_t *c;
   node_t *f = tp->freeList;

   while ( f )
   {
      c = tp->freeList;
      while ( c )
      {
         if ( f != c )
         {
            nodeValidate( f, c );
         }
         c = c->next;
      }
      f = f->next;
   }
#endif
}

static int mergeNodes( dlTexturePacker *tp )
{
   node_t *prev;
   node_t *c;
   node_t *f = tp->freeList;

   while( f )
   {
      prev  = NULL;
      c     = tp->freeList;
      while ( c )
      {
         if ( f != c )
         {
            if( nodeMerge(f, c) )
            {
               assert(prev);
               prev->next = c->next;
               free( c ); c = NULL;
               return( 1 );
            }
         }
         prev = c;
         c = c->next;
      }
      f = f->next;
   }

   return( 0 );
}

int dlTexturePackerGetLocation( dlTexturePacker* tp, int index, int *inX, int *inY, int *inW, int *inH )
{
   int ret  = 0;
   int x    = 0;
   int y    = 0;
   int wid  = 0;
   int hit  = 0;

   texture_t *t;

   assert( index < tp->textureCount );
   if( index < tp->textureCount )
   {
      t = &tp->textures[index];
      x = t->x;
      y = t->y;

      if( t->flipped )
      {
         wid = t->height;
         hit = t->width;
      }
      else
      {
         wid = t->width;
         hit = t->height;
      }
      ret = t->flipped;
   }

   *inX = x;
   *inY = y;
   *inW = wid;
   *inH = hit;

   return( ret );
}

int dlTexturePackerPack( dlTexturePacker *tp, int *inWidth, int *inHeight, int forcePowerOfTwo, int onePixelBorder )
{
   int width  = 0;
   int height = 0;
   int count;
   int i, i2;

   int leastY = 0x7FFFFFFF;
   int leastX = 0x7FFFFFFF;

   node_t *previousBestFit = 0;
   node_t *bestFit         = 0;
   node_t *previous        = 0;
   node_t *search          = NULL;
   int edgeCount           = 0;
   int ec                  = 0;

   int index;
   int longestEdge;
   int mostArea;

   int flipped;
   int wid;
   int hit;

   int y;

   texture_t *t;

   if( onePixelBorder )
   {
      i = 0;
      for(; i != tp->textureCount; ++i)
      {
         t = &tp->textures[i];
         t->width  += 2;
         t->height += 2;
      }
      tp->longestEdge += 2;
   }

   if( forcePowerOfTwo )
   {
      tp->longestEdge = nextPow2( tp->longestEdge );
   }

   width = tp->longestEdge;
   count = tp->totalArea / (tp->longestEdge * tp->longestEdge);
   height = (count + 2) * tp->longestEdge;

   tp->debugCount = 0;
   dlTexturePackerNewNode( tp, 0, 0, width, height );

   i = 0;
   for(; i != tp->textureCount; ++i)
   {
      index       = 0;
      longestEdge = 0;
      mostArea    = 0;

      i2 = 0;
      for(; i2 != tp->textureCount; ++i2)
      {
         t = &tp->textures[i2];

         if( !t->placed )
         {
            if( t->longestEdge > longestEdge )
            {
               mostArea    = t->area;
               longestEdge = t->longestEdge;
               index       = i2;
            }
            else if( t->longestEdge == longestEdge )
            {
               if( t->area > mostArea )
               {
                  mostArea = t->area;
                  index    = i2;
               }
            }
         }
      }

      leastY = 0x7FFFFFFF;
      leastX = 0x7FFFFFFF;

      t = &tp->textures[index];

      previousBestFit   = NULL;
      bestFit           = NULL;
      previous          = NULL;
      search            = tp->freeList;
      edgeCount         = 0;

      while ( search )
      {
         ec = 0;
         if( nodeFits( search, t->width, t->height, &ec ) )
         {
            if( ec == 2 )
            {
               previousBestFit = previous;
               bestFit         = search;
               edgeCount       = ec;
               break;
            }
            if( search->y < leastY )
            {
               leastY            = search->y;
               leastX            = search->x;
               previousBestFit   = previous;
               bestFit           = search;
               edgeCount         = ec;
            }
            else if( search->y == leastY && search->x < leastX )
            {
               leastX            = search->x;
               previousBestFit   = previous;
               bestFit           = search;
               edgeCount         = ec;
            }
         }
         previous = search;
         search   = search->next;
      }
      assert( bestFit ); /* should always find a fit */

      if( bestFit )
      {
         validate( tp );

         switch( edgeCount )
         {
            case 0:
               if ( t->longestEdge <= bestFit->width )
               {
                  flipped  = 0;
                  wid      = t->width;
                  hit      = t->height;

                  if ( hit > wid )
                  {
                     wid      = t->height;
                     hit      = t->width;
                     flipped  = 1;
                  }

                  texturePlace( t, bestFit->x, bestFit->y, flipped );
                  dlTexturePackerNewNode( tp, bestFit->x, bestFit->y + hit, bestFit->width , bestFit->height - hit );

                  bestFit->x        += wid;
                  bestFit->width    -= wid;
                  bestFit->height    = hit;
                  validate( tp );
               }
               else
               {
                  assert( t->longestEdge <= bestFit->height );

                  flipped  = 0;
                  wid      = t->width;
                  hit      = t->height;

                  if( hit < wid )
                  {
                     wid      = t->height;
                     hit      = t->width;
                     flipped  = 1;
                  }

                  texturePlace( t, bestFit->x, bestFit->y, flipped );
                  dlTexturePackerNewNode( tp, bestFit->x, bestFit->y + hit, bestFit->width, bestFit->height - hit );

                  bestFit->x     += wid;
                  bestFit->width -= wid;
                  bestFit->height = hit;
                  validate( tp );
               }
               break;
            case 1:
               {
                  if( t->width == bestFit->width )
                  {
                     texturePlace( t, bestFit->x, bestFit->y, 0 );
                     bestFit->y        += t->height;
                     bestFit->height   -= t->height;
                     validate( tp );
                  }
                  else if( t->height == bestFit->height )
                  {
                     texturePlace( t, bestFit->x, bestFit->y, 0 );
                     bestFit->x     += t->width;
                     bestFit->width -= t->width;
                     validate( tp );
                  }
                  else if( t->width == bestFit->height )
                  {
                     texturePlace( t, bestFit->x, bestFit->y, 1 );
                     bestFit->x     += t->height;
                     bestFit->width -= t->height;
                     validate( tp );
                  }
                  else if( t->height == bestFit->width )
                  {
                     texturePlace( t, bestFit->x, bestFit->y, 1 );
                     bestFit->y        += t->width;
                     bestFit->height   -= t->width;
                     validate( tp );
                  }
               }
               break;
            case 2:
               {
                  flipped = t->width != bestFit->width || t->height != bestFit->height;
                  texturePlace( t, bestFit->x, bestFit->y, flipped );
                  if( previousBestFit )
                  {
                     previousBestFit->next = bestFit->next;
                  }
                  else
                  {
                     tp->freeList = bestFit->next;
                  }
                  free( bestFit ); bestFit = NULL;
                  validate( tp );
               }
               break;
         }
         while ( mergeNodes( tp ) ); /* merge as much as we can */
      }
   }

   height = 0;
   i = 0;
   for(; i< tp->textureCount; ++i)
   {
      t = &tp->textures[i];
      if( onePixelBorder )
      {
         t->width    -=2;
         t->height   -=2;
         t->x++;
         t->y++;
      }

      y = 0;
      if(t->flipped)
      {
         y = t->y + t->width;
      }
      else
      {
         y = t->y + t->height;
      }

      if( y > height )
         height = y;
   }

   if( forcePowerOfTwo )
   {
      height = nextPow2(height);
   }

   *inWidth    = width;
   *inHeight   = height;

   return( (width * height) - tp->totalArea );
}
