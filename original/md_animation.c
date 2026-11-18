/* CSCI596 AS07 Part I, Option 1: in situ C/OpenGL MD animation.
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
#include "../md.c"
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
static int paused=0, capture=0, alive=1;
static double yaw=32, pitch=20;
static FILE *metrics;
static unsigned char pixels[WIDTH*HEIGHT*3];

static void fail(const char *s) { fprintf(stderr,"%s\n",s); exit(1); }
static LRESULT CALLBACK events(HWND w, UINT msg, WPARAM key, LPARAM arg) {
    if(msg==WM_CLOSE) { alive=0; return 0; }
    if(msg==WM_KEYDOWN) {
        if(key==VK_ESCAPE) alive=0;
        if(key==VK_SPACE) paused=!paused;
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
    window=CreateWindow(wc.lpszClassName,"CSCI596 - In situ MD (Space: pause, arrows: rotate, Esc: exit)",
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
    if(!capture) ShowWindow(window,SW_SHOW);
}
static void label(int x,int y,const char *s,float red,float green,float blue) {
    glColor3f(red,green,blue); glRasterPos2i(x,y);
    glListBase(fontid-32); glCallLists((GLsizei)strlen(s),GL_UNSIGNED_BYTE,s);
}
static void properties(void) {
    int i,k; double momentum[3]={0,0,0};
    /* EvalProps converts potEnergy to per-atom units; recompute before each
     * evaluation, including a paused/initial state. SingleStep also recomputes. */
    ComputeAccel(); EvalProps();
    for(i=0;i<nAtom;i++) for(k=0;k<3;k++) {
        if(!isfinite(r[i][k]) || !isfinite(rv[i][k]) || r[i][k]<0 || r[i][k]>=Region[k])
            fail("Nonfinite state or coordinate outside periodic box");
        momentum[k]+=rv[i][k];
    }
    if(metrics) fprintf(metrics,"%d,%.9g,%.12g,%.12g,%.12g,%.12g,%.12g,%.12g\n",
        stepCount,stepCount*DeltaT,temperature,potEnergy,totEnergy,
        momentum[0],momentum[1],momentum[2]);
}
static void render(void) {
    int i,k; char text[128]; double extent=fmax(Region[0],fmax(Region[1],Region[2]));
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,900,HEIGHT);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(36,900.0/HEIGHT,.1,100*extent);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    gluLookAt(0,0,3.1*extent,0,0,0,0,1,0);
    glRotated(pitch,1,0,0); glRotated(yaw,0,1,0);
    glTranslated(-Region[0]/2,-Region[1]/2,-Region[2]/2);
    glEnable(GL_LIGHTING); glColor3f(.13f,.72f,.88f);
    /* atomv.c makeAtoms pattern, using live MD arrays instead of md.conf. */
    for(i=0;i<nAtom;i++) {
        glPushMatrix(); glTranslated(r[i][0],r[i][1],r[i][2]);
        glCallList(sphereid); glPopMatrix();
    }
    glDisable(GL_LIGHTING); glColor3f(.34f,.43f,.55f); glLineWidth(1.5f);
    glBegin(GL_LINES);
    for(i=0;i<8;i++) for(k=0;k<3;k++) if(!(i&(1<<k))) {
        glVertex3d((i&1)?Region[0]:0,(i&2)?Region[1]:0,(i&4)?Region[2]:0);
        {int j=i|(1<<k); glVertex3d((j&1)?Region[0]:0,(j&2)?Region[1]:0,(j&4)?Region[2]:0);}
    }
    glEnd();
    glViewport(0,0,WIDTH,HEIGHT); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0,WIDTH,0,HEIGHT,-1,1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    label(38,675,"CSCI596  /  VISUALIZING SIMULATIONS",.55f,.65f,.77f);
    label(38,638,"PART I   Lennard-Jones molecular dynamics",.94f,.96f,1);
    label(924,594,"LIVE C / OpenGL",.2f,.8f,.92f);
    label(924,553,"108 atoms | periodic box",.85f,.89f,.94f);
    sprintf(text,"Step  %04d / %d",stepCount,StepLimit); label(924,487,text,1,1,1);
    sprintf(text,"Time  %.3f tau",stepCount*DeltaT); label(924,450,text,1,1,1);
    sprintf(text,"T*       %.4f",temperature); label(924,381,text,.75f,.83f,.9f);
    sprintf(text,"U*/N   %.4f",potEnergy); label(924,344,text,.75f,.83f,.9f);
    sprintf(text,"E*/N   %.4f",totEnergy); label(924,307,text,.75f,.83f,.9f);
    label(924,235,"rho* = 0.8 | dt* = 0.005",.55f,.65f,.77f);
    label(924,198,"Velocity-Verlet integration",.55f,.65f,.77f);
    label(924,161,"Force-shifted LJ | rc = 2.5",.55f,.65f,.77f);
    label(38,42,"Atom positions evolve during rendering; boundary crossings wrap periodically.",.6f,.7f,.8f);
    glEnable(GL_DEPTH_TEST); glFinish();
    if(capture) {
        glReadBuffer(GL_BACK); glPixelStorei(GL_PACK_ALIGNMENT,1);
        glReadPixels(0,0,WIDTH,HEIGHT,GL_RGB,GL_UNSIGNED_BYTE,pixels);
        if(fwrite(pixels,1,sizeof(pixels),stdout)!=sizeof(pixels)) fail("Video pipe closed");
    }
    SwapBuffers(dc);
}
int main(int argc,char **argv) {
    MSG msg; int i; const char *input="md_animation.in";
    capture=(argc>1 && strcmp(argv[1],"--capture")==0);
    if(argc>(capture?2:1)) input=argv[capture?2:1];
    if(!freopen(input,"r",stdin)) fail("Cannot open MD input file");
    /* Validate before supplied InitConf writes the fixed-size arrays. */
    { int nx,ny,nz,limit,avg; double density,temp,dt;
      if(scanf("%d%d%d%lf%lf%lf%d%d",&nx,&ny,&nz,&density,&temp,&dt,&limit,&avg)!=8
         || nx<1 || ny<1 || nz<1 || (double)nx*ny*nz>NMAX/4
         || !isfinite(density) || density<=0 || !isfinite(temp) || temp<0
         || !isfinite(dt) || dt<=0 || limit<1 || avg<1) fail("Invalid MD input");
      rewind(stdin); }
    InitParams(); InitConf(); ComputeAccel(); stepCount=0;
    for(i=0;i<3;i++) if(RegionH[i]<RCUT) fail("Periodic half-box must exceed LJ cutoff");
    if(capture) {
        _setmode(_fileno(stdout),_O_BINARY);
        metrics=fopen("output/md_metrics.csv","w");
        if(!metrics) fail("Create output directory first");
        fprintf(metrics,"step,time_LJ,temperature_LJ,potential_per_atom,total_per_atom,px,py,pz\n");
    }
    initGL(); properties(); render();
    while(alive && (stepCount<StepLimit || !capture)) {
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        if(!alive) break;
        if(stepCount<StepLimit && !paused) {
            for(i=0;i<StepAvg && stepCount<StepLimit;i++) { SingleStep(); ++stepCount; }
            properties(); render();
        } else if(!capture) render();
        if(!capture) Sleep(33);
    }
    if(metrics) fclose(metrics);
    glDeleteLists(sphereid,1); glDeleteLists(fontid,96);
    wglMakeCurrent(NULL,NULL); wglDeleteContext(context);
    ReleaseDC(window,dc); DestroyWindow(window);
    return 0;
}
