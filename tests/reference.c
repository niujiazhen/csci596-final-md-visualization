/* Independent batch driver: original force/integrator, no visualization helpers. */
#define main original_batch_main
#include "../original/md.c"
#undef main
#include <stdlib.h>
int main(int argc,char **argv) {
    FILE *f,*m; int i,k; double kinetic;
    if(argc!=4 || !freopen(argv[1],"r",stdin)) return 1;
    f=fopen(argv[2],"w"); m=fopen(argv[3],"w"); if(!f || !m) return 2;
    InitParams(); InitConf(); ComputeAccel();
    fprintf(m,"step,total_per_atom\n");
    for(stepCount=0;stepCount<=StepLimit;stepCount++) {
        if(stepCount) SingleStep();
        kinetic=0;
        for(i=0;i<nAtom;i++) for(k=0;k<3;k++) kinetic+=rv[i][k]*rv[i][k];
        kinetic*=0.5/nAtom; /* Same arithmetic order as the supplied EvalProps. */
        fprintf(m,"%d,%.17g\n",stepCount,kinetic+potEnergy/nAtom);
    }
    fprintf(f,"atom,x,y,z,vx,vy,vz\n");
    for(i=0;i<nAtom;i++) {
        fprintf(f,"%d",i);
        for(k=0;k<3;k++) fprintf(f,",%.17g",r[i][k]);
        for(k=0;k<3;k++) fprintf(f,",%.17g",rv[i][k]);
        fprintf(f,"\n");
    }
    fclose(f); fclose(m); return 0;
}
