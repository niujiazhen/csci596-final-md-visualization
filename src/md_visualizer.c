/* CSCI596 Final Project, based on the AS07 C/OpenGL viewer.
 * Simulation: the supplied md.c/md.h, included unchanged below.
 * Rendering: makeFastNiceSphere from the supplied atomv.c; coordinates
 * are unified to md.c's nAtom/r arrays, as in Visual.pdf pp.14-15.
 * Win32/WGL replaces GLUT window/event plumbing on Windows. No trajectory
 * playback is involved: SingleStep runs before each new rendered frame.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <io.h>

#define main supplied_md_batch_main
#define printf(...) fprintf(stderr, __VA_ARGS__)
#include "../original/md.c"
#include "visualization.h"
#undef printf
#undef main

#define M_PI PI
static int nlon=36, nlat=18;
#include "atomv_sphere.h"

enum { WIDTH=1280, HEIGHT=720 };
static HDC dc;
static HGLRC context;
static HWND window;
static GLuint sphereid, fontid;
static int paused=0, alive=1;
static int headless=0, color_mode=1, show_trail=1, snapshot_step=-1;
static const char *snapshot_prefix=NULL;
static Trail trail={0};
static int frame_count=0, boundary_breaks=0, max_trail=0;
static double max_displacement_ratio=0;
static const char *output_path="output/run";
static double initial_energy, max_energy_drift=0;
static char output_file[1024];
static double yaw=32, pitch=20;
static FILE *metrics;
static unsigned char pixels[WIDTH*HEIGHT*3];

static void fail(const char *s) { fprintf(stderr,"%s\n",s); exit(1); }
static LRESULT CALLBACK events(HWND w, UINT msg, WPARAM key, LPARAM arg) {
    if(msg==WM_CLOSE) { alive=0; return 0; }
    if(msg==WM_KEYDOWN) {
        if(key==VK_ESCAPE) alive=0;
        if(key==VK_SPACE) paused=!paused;
        if(key=='C') color_mode=!color_mode;
        if(key=='T') show_trail=!show_trail;
        if(key==VK_LEFT) yaw-=5;
        if(key==VK_RIGHT) yaw+=5;
        if(key==VK_UP) pitch-=5;
        if(key==VK_DOWN) pitch+=5;
        return 0;
    }
    return DefWindowProc(w,msg,key,arg);
}
static void initGL(void) {
    WNDCLASS wc={0}; RECT rect={0,0,WIDTH,HEIGHT};
    PIXELFORMATDESCRIPTOR pf={0}; int format;
    GLfloat light[]={-1,2,3,0}, diffuse[]={.9f,.94f,1,1};
    GLfloat ambient[]={.23f,.25f,.3f,1}, spec[]={.45f,.5f,.55f,1};
    HFONT font;
    wc.style=CS_OWNDC; wc.lpfnWndProc=events;
    wc.hInstance=GetModuleHandle(NULL); wc.lpszClassName="AS07MD";
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    if(!RegisterClass(&wc)) fail("Cannot register window class");
    AdjustWindowRect(&rect,WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,0);
    window=CreateWindow(wc.lpszClassName,"CSCI596 Final - C: colors | T: trail | Space: pause | Arrows: rotate | Esc: exit",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,80,80,rect.right-rect.left,rect.bottom-rect.top,
        NULL,NULL,wc.hInstance,NULL);
    if(!window) fail("Cannot create OpenGL window");
    dc=GetDC(window); pf.nSize=sizeof(pf); pf.nVersion=1;
    pf.dwFlags=PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    pf.iPixelType=PFD_TYPE_RGBA; pf.cColorBits=24; pf.cDepthBits=24;
    format=ChoosePixelFormat(dc,&pf);
    if(!format || !SetPixelFormat(dc,format,&pf)) fail("Cannot set pixel format");
    context=wglCreateContext(dc);
    if(!context || !wglMakeCurrent(dc,context)) fail("Cannot create OpenGL context");
    fprintf(stderr,"OpenGL: %s | %s\n",glGetString(GL_RENDERER),glGetString(GL_VERSION));
    glClearColor(.035f,.055f,.09f,1); glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE); glEnable(GL_COLOR_MATERIAL);
    glLightfv(GL_LIGHT0,GL_POSITION,light); glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuse);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spec);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,48);
    glEnable(GL_LIGHT0);
    sphereid=glGenLists(1); makeFastNiceSphere(sphereid,.27);
    fontid=glGenLists(96);
    font=CreateFontA(-22,0,0,0,FW_NORMAL,0,0,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,"Segoe UI");
    { HGDIOBJ old=SelectObject(dc,font);
      if(!wglUseFontBitmapsA(dc,32,96,fontid)) fail("Cannot create text font");
      SelectObject(dc,old); DeleteObject(font); }
    if(!snapshot_prefix) ShowWindow(window,SW_SHOW);
}
static void label(int x,int y,const char *s,float red,float green,float blue) {
    glColor3f(red,green,blue); glRasterPos2i(x,y);
    glListBase(fontid-32); glCallLists((GLsizei)strlen(s),GL_UNSIGNED_BYTE,s);
}
static void properties(void) {
    int i,k; double kinetic=0, energy, temp;
    /* Read-only: avoid EvalProps(), which normalizes potEnergy in place. */
    for(i=0;i<nAtom;i++) for(k=0;k<3;k++) {
        if(!isfinite(r[i][k]) || !isfinite(rv[i][k]) || r[i][k]<0 || r[i][k]>=Region[k])
            fail("Nonfinite state or coordinate outside periodic box");
        kinetic+=rv[i][k]*rv[i][k];
    }
    kinetic*=0.5/nAtom;
    energy=kinetic+potEnergy/nAtom; temp=2*kinetic/3;
    if(stepCount==0) initial_energy=energy;
    if(fabs(energy-initial_energy)>max_energy_drift) max_energy_drift=fabs(energy-initial_energy);
    if(metrics) fprintf(metrics,"%d,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g,%.17g\n",
        stepCount,stepCount*DeltaT,r[0][0],r[0][1],r[0][2],particle_speed(rv[0]),
        temp,energy,kinetic,potEnergy/nAtom);
}
static void observe_step(void) {
    int k,idx; double displacement,ratio;
    if(trail.count) {
        idx=(trail.start+trail.count-1)%TRAIL_CAPACITY;
        if(!trail_connect(trail.p[idx],r[0],Region)) boundary_breaks++;
    }
    /* Current half-step drift can be reconstructed from the final velocity.
     * Check ALL particles: this also guards the original one-box wrap. */
    for(int i=0;i<nAtom;i++) for(k=0;k<3;k++) {
        displacement=DeltaT*(rv[i][k]-DeltaTH*ra[i][k]);
        ratio=fabs(displacement)/RegionH[k];
        if(ratio>max_displacement_ratio) max_displacement_ratio=ratio;
        if(ratio>=1) fail("Integration step too large for reliable periodic trajectory; reduce dt");
    }
    trail_add(&trail,r[0]);
    if(trail.count>max_trail) max_trail=trail.count;
    properties();
}
static void save_image(const char *suffix) {
    unsigned char *row; int y; FILE *f;
    snprintf(output_file,sizeof(output_file),"%s_%s.ppm",snapshot_prefix,suffix);
    f=fopen(output_file,"wb"); if(!f) fail("Cannot create snapshot");
    glReadBuffer(GL_BACK); glPixelStorei(GL_PACK_ALIGNMENT,1);
    glReadPixels(0,0,WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,pixels);
    fprintf(f,"P6\n%d %d\n255\n",WIDTH,HEIGHT);
    for(y=HEIGHT-1;y>=0;y--) { row=pixels+y*WIDTH*3; fwrite(row,1,WIDTH*3,f); }
    fclose(f);
}
static void render(void) {
    int i,k; char text[128]; double extent=fmax(Region[0],fmax(Region[1],Region[2]));
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0,100,900,520);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(36,900.0/520,.1,100*extent);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    gluLookAt(0,0,3.1*extent,0,0,0,0,1,0);
    glRotated(pitch,1,0,0); glRotated(yaw,0,1,0);
    glTranslated(-Region[0]/2,-Region[1]/2,-Region[2]/2);
    glEnable(GL_LIGHTING);
    /* atomv.c makeAtoms pattern, using live MD arrays instead of md.conf. */
    for(i=0;i<nAtom;i++) {
        float c[3];
        if(color_mode) { speed_color(particle_speed(rv[i]),c); glColor3fv(c); }
        else glColor3f(.13f,.72f,.88f);
        glPushMatrix(); glTranslated(r[i][0],r[i][1],r[i][2]);
        glCallList(sphereid); glPopMatrix();
    }
    glDisable(GL_LIGHTING);
    if(show_trail && trail.count>1) {
        glLineWidth(2.5f); glColor3f(1,1,1); glBegin(GL_LINES);
        for(i=1;i<trail.count;i++) {
            int a=(trail.start+i-1)%TRAIL_CAPACITY, b=(trail.start+i)%TRAIL_CAPACITY;
            if(trail_connect(trail.p[a],trail.p[b],Region)) {
                glVertex3dv(trail.p[a]); glVertex3dv(trail.p[b]);
            }
        }
        glEnd();
    }
    /* Screen-facing halo in world space, outside the selected atom. */
    glPushMatrix(); glTranslated(r[0][0],r[0][1],r[0][2]);
    glRotated(-yaw,0,1,0); glRotated(-pitch,1,0,0);
    glColor3f(1,1,1); glLineWidth(2); glBegin(GL_LINE_LOOP);
    for(i=0;i<64;i++) { double angle=2*PI*i/64; glVertex3d(.34*cos(angle),.34*sin(angle),0); }
    glEnd(); glPopMatrix();
    glColor3f(.34f,.43f,.55f); glLineWidth(1.5f);
    glBegin(GL_LINES);
    for(i=0;i<8;i++) for(k=0;k<3;k++) if(!(i&(1<<k))) {
        glVertex3d((i&1)?Region[0]:0,(i&2)?Region[1]:0,(i&4)?Region[2]:0);
        {int j=i|(1<<k); glVertex3d((j&1)?Region[0]:0,(j&2)?Region[1]:0,(j&4)?Region[2]:0);}
    }
    glEnd();
    glViewport(0,0,WIDTH,HEIGHT); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    label(38,675,"CSCI596 FINAL PROJECT",.55f,.65f,.77f);
    label(38,638,"Particle speeds and trajectories",.94f,.96f,1);
    label(924,594,"C / OpenGL / Live MD",.2f,.8f,.92f);
    sprintf(text,"%d atoms | periodic",nAtom); label(924,553,text,.85f,.89f,.94f);
    sprintf(text,"Step %d / %d",stepCount,StepLimit); label(924,515,text,1,1,1);
    sprintf(text,"Time %.3f tau",stepCount*DeltaT); label(924,481,text,1,1,1);
    label(924,432,color_mode?"Speed | fixed scale":"Uniform atom colors",.85f,.89f,.94f);
    if(color_mode) {
        glBegin(GL_QUADS);
        for(i=0;i<200;i++) {
            float c[3]; speed_color(SPEED_MAX*i/199,c); glColor3fv(c);
            glVertex2i(924+i,396); glVertex2i(925+i,396);
            glVertex2i(925+i,414); glVertex2i(924+i,414);
        }
        glEnd();
        label(924,367,"0       2       >=4",.8f,.85f,.9f);
        label(924,339,"LJ reduced speed",.6f,.7f,.8f);
    }
    sprintf(text,"Atom 0: v*=%.3f",particle_speed(rv[0])); label(924,294,text,1,1,1);
    sprintf(text,"Trail: %s (%d)",show_trail?"ON":"OFF",trail.count); label(924,261,text,.8f,.85f,.9f);
    sprintf(text,"rho*=%.2f dt*=%.3f",Density,DeltaT); label(924,221,text,.6f,.7f,.8f);
    label(924,182,"White halo: atom 0",.8f,.85f,.9f);
    label(38,76,"C: colors | T: trail | Space: pause | Arrows: rotate | Esc: exit",.8f,.85f,.9f);
    label(38,42,"Wrapped paths split at periodic boundaries. Colors indicate speed, not temperature.",.6f,.7f,.8f);
    glEnable(GL_DEPTH_TEST); glFinish();
    if(!snapshot_prefix) SwapBuffers(dc);
    frame_count++;
}
static void finish_outputs(void) {
    FILE *f; int i,k;
    if(metrics) fclose(metrics);
    snprintf(output_file,sizeof(output_file),"%s_state.csv",output_path);
    f=fopen(output_file,"w"); if(!f) fail("Cannot create state CSV");
    fprintf(f,"atom,x,y,z,vx,vy,vz\n");
    for(i=0;i<nAtom;i++) { fprintf(f,"%d",i); for(k=0;k<3;k++) fprintf(f,",%.17g",r[i][k]);
        for(k=0;k<3;k++) fprintf(f,",%.17g",rv[i][k]); fprintf(f,"\n"); }
    fclose(f);
    snprintf(output_file,sizeof(output_file),"%s_summary.json",output_path);
    f=fopen(output_file,"w"); if(!f) fail("Cannot create summary");
    fprintf(f,"{\"steps\":%d,\"atoms\":%d,\"trail_samples\":%d,\"boundary_breaks\":%d,"
        "\"max_trail_samples\":%d,\"frames\":%d,\"max_drift_over_half_box\":%.17g,"
        "\"max_abs_energy_drift_per_atom\":%.17g,\"initial_energy_per_atom\":%.17g}\n",
        stepCount,nAtom,trail.count,boundary_breaks,max_trail,frame_count,
        max_displacement_ratio,max_energy_drift,initial_energy);
    fclose(f);
}
static void advance_batch(void) {
    int i;
    if(!paused && stepCount<StepLimit) {
        int batch=headless||snapshot_prefix?1:StepAvg;
        for(i=0;i<batch && stepCount<StepLimit;i++) {
            SingleStep(); ++stepCount; observe_step();
            if(snapshot_prefix && stepCount==snapshot_step) break;
        }
    }
}
#ifndef VIEWER_ENTRY
#define VIEWER_ENTRY main
#endif
int VIEWER_ENTRY(int argc,char **argv) {
    MSG msg; int i; const char *input="examples/default.in";
    for(i=1;i<argc;i++) {
        if(!strcmp(argv[i],"--headless")) headless=1;
        else if(!strcmp(argv[i],"--no-color")) color_mode=0;
        else if(!strcmp(argv[i],"--no-trail")) show_trail=0;
        else if(!strcmp(argv[i],"--output") && i+1<argc) output_path=argv[++i];
        else if(!strcmp(argv[i],"--snapshot") && i+1<argc) snapshot_prefix=argv[++i];
        else if(!strcmp(argv[i],"--snapshot-step") && i+1<argc) snapshot_step=atoi(argv[++i]);
        else if(argv[i][0]=='-') fail("Unknown or incomplete option");
        else input=argv[i];
    }
    if(headless && snapshot_prefix) fail("Snapshots require OpenGL; omit --headless");
    if(!freopen(input,"r",stdin)) fail("Cannot open MD input file");
    { int nx,ny,nz,limit,avg; double density,temp,dt;
      if(scanf("%d%d%d%lf%lf%lf%d%d",&nx,&ny,&nz,&density,&temp,&dt,&limit,&avg)!=8
         || nx<1 || ny<1 || nz<1 || (double)nx*ny*nz>NMAX/4
         || !isfinite(density) || density<=0 || !isfinite(temp) || temp<0
         || !isfinite(dt) || dt<=0 || limit<1 || avg<1) fail("Invalid MD input");
      rewind(stdin); }
    InitParams();
    for(i=0;i<3;i++) if(!isfinite(Region[i]) || RegionH[i]<=RCUT)
        fail("Periodic half-box must exceed LJ cutoff");
    if(snapshot_prefix && (snapshot_step<0 || snapshot_step>StepLimit))
        fail("Snapshot step must be between 0 and StepLimit");
    InitConf(); ComputeAccel(); stepCount=0;
    snprintf(output_file,sizeof(output_file),"%s_metrics.csv",output_path);
    metrics=fopen(output_file,"w"); if(!metrics) fail("Cannot create metrics; create output directory first");
    fprintf(metrics,"step,time_LJ,x0,y0,z0,speed0_LJ,temperature_LJ,total_per_atom,kinetic_per_atom,potential_per_atom\n");
    trail_add(&trail,r[0]); max_trail=1; properties();
    if(!headless) initGL();
    while(alive) {
        if(!headless) {
            while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
            if(!alive) break;
            if(snapshot_prefix && stepCount==snapshot_step) {
                color_mode=0; show_trail=0; render(); save_image("uniform");
                color_mode=1; render(); save_image("speed");
                show_trail=1; render(); save_image("trail");
                break;
            }
            if(!snapshot_prefix) { render(); }
        }
        if(stepCount==StepLimit && (headless || snapshot_prefix)) break;
        advance_batch();
        if(!headless && !snapshot_prefix) Sleep(33);
    }
    finish_outputs();
    if(!headless) {
        glDeleteLists(sphereid,1); glDeleteLists(fontid,96);
        wglMakeCurrent(NULL,NULL); wglDeleteContext(context);
        ReleaseDC(window,dc); DestroyWindow(window);
    }
    return 0;
}
