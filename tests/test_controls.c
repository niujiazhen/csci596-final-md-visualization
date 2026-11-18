/* Exercise the actual viewer event handler and simulation tick, without a GL context. */
#define VIEWER_ENTRY viewer_main
#include "../src/md_visualizer.c"
#include <assert.h>
int main(void) {
    double before_r[108][3],before_v[108][3];
    int count,step;
    assert(freopen("examples/default.in","r",stdin));
    InitParams(); InitConf(); ComputeAccel(); stepCount=0;
    trail_add(&trail,r[0]); properties();
    advance_batch();
    count=trail.count; step=stepCount;
    memcpy(before_r,r,sizeof(before_r)); memcpy(before_v,rv,sizeof(before_v));
    events(NULL,WM_KEYDOWN,VK_SPACE,0); assert(paused);
    for(int i=0;i<20;i++) advance_batch();
    assert(stepCount==step && trail.count==count);
    assert(!memcmp(before_r,r,sizeof(before_r)) && !memcmp(before_v,rv,sizeof(before_v)));
    events(NULL,WM_KEYDOWN,'C',0); assert(!color_mode);
    events(NULL,WM_KEYDOWN,'T',0); assert(!show_trail);
    assert(stepCount==step && trail.count==count);
    events(NULL,WM_KEYDOWN,'C',0); events(NULL,WM_KEYDOWN,'T',0);
    assert(color_mode && show_trail);
    events(NULL,WM_KEYDOWN,VK_SPACE,0); advance_batch();
    assert(stepCount>step && trail.count>count);
    events(NULL,WM_KEYDOWN,VK_ESCAPE,0); assert(!alive);
    puts("PASS: actual key handler, pause/resume, unchanged paused state/history, C/T toggles, exit");
    return 0;
}
