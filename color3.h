#pragma once

/* Definition of color3 struct
   usable in GPU/CPU context

   defines an rgb tuple.
   each component has value in range 0-255
   (0,0,0) -> black
   (255,0,0) -> red
   (255,255,255) -> white

*/

/* TODO:
     add namespace to prevent collisions?
     add helper functions?
*/

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} color3;