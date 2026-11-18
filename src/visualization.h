/* Final Project additions: read-only visualization helpers. */
#ifndef VISUALIZATION_H
#define VISUALIZATION_H
#include <math.h>
#define TRAIL_CAPACITY 1000
#define SPEED_MAX 4.0
typedef struct { double p[TRAIL_CAPACITY][3]; int start, count; } Trail;
static double particle_speed(const double v[3]) {
    return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}
/* Blue -> cyan -> yellow -> red, with a fixed scale in reduced LJ units. */
static void speed_color(double speed, float c[3]) {
    double t=fmax(0.0,fmin(1.0,speed/SPEED_MAX));
    if(t<1.0/3) { c[0]=0; c[1]=(float)(3*t); c[2]=1; }
    else if(t<2.0/3) { c[0]=(float)(3*t-1); c[1]=1; c[2]=(float)(2-3*t); }
    else { c[0]=1; c[1]=(float)(3-3*t); c[2]=0; }
}
static void trail_add(Trail *t, const double p[3]) {
    int index,k;
    if(t->count<TRAIL_CAPACITY) index=(t->start+t->count++)%TRAIL_CAPACITY;
    else { index=t->start; t->start=(t->start+1)%TRAIL_CAPACITY; }
    for(k=0;k<3;k++) t->p[index][k]=p[k];
}
/* Samples must be one integration step apart, with true displacement < L/2.
 * A wrapped trajectory is intentionally split, not unwrapped or interpolated. */
static int trail_connect(const double a[3],const double b[3],const double box[3]) {
    int k;
    for(k=0;k<3;k++) if(fabs(a[k]-b[k])>box[k]*0.5) return 0;
    return 1;
}
#endif
