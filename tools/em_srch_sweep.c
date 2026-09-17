#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "../driver/egis_match.h"

/* The driver's constants are #ifndef-guarded, so -DEM_SRCH=N -DEM_MIN_OVERLAP=N
 * on the command line rebuilds the scorer at any operating point. Mirror the
 * same defaults here purely so the report line can print what was built. */
#ifndef EM_SRCH
#define EM_SRCH 6
#endif
#ifndef EM_MIN_OVERLAP
#define EM_MIN_OVERLAP 800
#endif

#define FINGERS 5
#define PRESSES 8
#define ENR 5

static int load_raw(const char *p, uint8_t *b){
  FILE *f=fopen(p,"rb"); if(!f) return -1;
  size_t n=fread(b,1,EM_N,f); fclose(f); return n==EM_N?0:-1;
}
static int cmpd(const void*a,const void*b){
  double x=*(const double*)a,y=*(const double*)b; return x<y?-1:x>y?1:0;
}
static double median(double *v,int n){
  double *c=malloc(n*sizeof(double));
  for(int i=0;i<n;i++)c[i]=v[i];
  qsort(c,n,sizeof(double),cmpd);
  double m = n%2? c[n/2] : (c[n/2-1]+c[n/2])/2;
  free(c); return m;
}

int main(int argc,char**argv){
  const char*dir = argc>1?argv[1]:"dataset";
  double fixed_th = argc>2?atof(argv[2]):0.53;
  static EmFrame fr[FINGERS][PRESSES];
  for(int f=0;f<FINGERS;f++)for(int p=0;p<PRESSES;p++){
    char path[256]; uint8_t raw[EM_N];
    snprintf(path,sizeof path,"%s/f%d_%02d.bin",dir,f+1,p);
    if(load_raw(path,raw)){fprintf(stderr,"cannot load %s\n",path);return 1;}
    em_frame_compute(raw,&fr[f][p]);
  }
  /* multi-template: enroll first ENR, probe rest, best-of-N */
  static double mg[FINGERS*PRESSES], mi[FINGERS*FINGERS*PRESSES];
  int nmg=0,nmi=0;
  for(int f=0;f<FINGERS;f++)for(int p=ENR;p<PRESSES;p++){
    double best=-1;
    for(int e=0;e<ENR;e++){double s=em_match(&fr[f][p],&fr[f][e]); if(s>best)best=s;}
    mg[nmg++]=best;
  }
  for(int f=0;f<FINGERS;f++)for(int g=0;g<FINGERS;g++){
    if(g==f)continue;
    for(int p=ENR;p<PRESSES;p++){
      double best=-1;
      for(int e=0;e<ENR;e++){double s=em_match(&fr[g][p],&fr[f][e]); if(s>best)best=s;}
      mi[nmi++]=best;
    }
  }
  int far=0,frr=0;
  for(int i=0;i<nmi;i++) far += mi[i]>=fixed_th;
  for(int i=0;i<nmg;i++) frr += mg[i]<fixed_th;
  /* strictest zero-FAR threshold */
  double imax=-1; for(int i=0;i<nmi;i++) if(mi[i]>imax) imax=mi[i];
  double zth = imax + 0.001;
  int zfrr=0; for(int i=0;i<nmg;i++) zfrr += mg[i]<zth;

  printf("SRCH=%-3d OVL=%-5d | th %.2f: FRR %5.1f%%  FAR %5.1f%% | gen med %.3f  imp med %.3f  imp max %.3f | zeroFAR th %.3f -> FRR %5.1f%% | n_gen %d n_imp %d\n",
    EM_SRCH, EM_MIN_OVERLAP, fixed_th,
    100.0*frr/nmg, 100.0*far/nmi,
    median(mg,nmg), median(mi,nmi), imax,
    zth, 100.0*zfrr/nmg, nmg, nmi);
  return 0;
}
