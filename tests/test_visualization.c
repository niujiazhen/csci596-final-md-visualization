#include <assert.h>
#include <stdio.h>
#include "../src/visualization.h"
int main(void) {
    float c[3]; double box[]={10,10,10}, a[]={9.9,2,3},b[]={.1,2,3};
    double v[]={3,4,0},p[]={0,0,0}; Trail t={0};
    assert(particle_speed(v)==5);
    speed_color(0,c); assert(c[0]==0 && c[1]==0 && c[2]==1);
    speed_color(2,c); assert(fabs(c[0]-.5)<1e-6 && c[1]==1 && fabs(c[2]-.5)<1e-6);
    speed_color(4,c); assert(c[0]==1 && c[1]==0 && c[2]==0);
    speed_color(9,c); assert(c[0]==1 && c[1]==0 && c[2]==0);
    speed_color(-1,c); assert(c[0]==0 && c[1]==0 && c[2]==1);
    assert(!trail_connect(a,b,box)); assert(!trail_connect(b,a,box));
    for(int axis=0;axis<3;axis++) {
        double x[]={1,1,1},y[]={1,1,1}; x[axis]=9.9; y[axis]=.1;
        assert(!trail_connect(x,y,box)); assert(!trail_connect(y,x,box));
    }
    b[0]=9.8; assert(trail_connect(a,b,box));
    for(int i=0;i<1200;i++) { p[0]=i; trail_add(&t,p); }
    assert(t.count==1000 && t.p[t.start][0]==200);
    assert(t.p[(t.start+t.count-1)%TRAIL_CAPACITY][0]==1199);
    puts("PASS: speed, fixed colors/clipping, crossings in 3 axes, ring chronology/capacity");
    return 0;
}
