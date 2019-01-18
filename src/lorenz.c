#include <stdio.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

#define A 16.0
#define R 45.0
#define B 4.0

#define D 3
#define H 0.015
#define USAGE "lorenz <num-points> <num-skip> <x0> <y0> <z0>"

int lorenz3 (double t, const double x[], double f[],void *params){
  f[0] = A*(x[1] - x[0]);
  f[1] = -1.0*x[0]*x[2] + R*x[0] - x[1] ;
  f[2] = x[0]*x[1] - B*x[2];
  return GSL_SUCCESS;
}

int lorenz3_jac (double t, const double x[], double *dfdy, double dfdt[], void *params){
  gsl_matrix_view dfdy_mat = gsl_matrix_view_array (dfdy, D, D);
  gsl_matrix * m = &dfdy_mat.matrix;
 
  gsl_matrix_set (m, 0, 0, -1.0*A);
  gsl_matrix_set (m, 0, 1, A);
  gsl_matrix_set (m, 0, 2, 0.0);
  gsl_matrix_set (m, 1, 0, R-x[2]);
  gsl_matrix_set (m, 1, 1, -1.0);
  gsl_matrix_set (m, 1, 2, -1.0*x[0]);
  gsl_matrix_set (m, 2, 0, x[1]);
  gsl_matrix_set (m, 2, 1, x[0]);
  gsl_matrix_set (m, 1, 1, -1.0*B);

  dfdt[0] = 0.0;
  dfdt[1] = 0.0;
  dfdt[2] = 0.0;

  return GSL_SUCCESS;
}

void go_blooey(const char *s){
  fprintf(stderr,"%s\n",s);
  exit(1);
}

int main(int argc,char **argv){
  // http://www.gnu.org/software/gsl/manual/html_node/ODE-Example-programs.html
  // http://linux.duke.edu/~mstenner/free-docs/gsl-ref-1.1/gsl-ref_25.html

  //const gsl_odeiv_step_type * T = gsl_odeiv_step_rk4;
  const gsl_odeiv_step_type * T = gsl_odeiv_step_rkf45;
  gsl_odeiv_step * s = gsl_odeiv_step_alloc (T, D);
  gsl_odeiv_system sys = {lorenz3,lorenz3_jac,D,NULL};

  if(argc < 6) go_blooey(USAGE);
  int num_points = atoi(argv[1]);
  int num_skip = atoi(argv[2]);
  double x[D] = {atof(argv[3]),atof(argv[4]),atof(argv[5])}, x_err[D];

  fprintf(stderr,"Calculating %d points of trajectory, skipping %d points of transient and starting at (%f,%f,%f)\n",num_points,num_skip,x[0],x[1],x[2]);


  double t = 0.0;
  double dydt_in[D], dydt_out[D];
     
  /* initialise dydt_in from system parameters */
  GSL_ODEIV_FN_EVAL(&sys, t, x, dydt_in);

  for(int i = 0; i < (num_points+num_skip); i++){
    int status = gsl_odeiv_step_apply(s,t,H,x,x_err,dydt_in,dydt_out,&sys);
    if(status != GSL_SUCCESS) break;

    t += H;
    dydt_in[0] = dydt_out[0];
    dydt_in[1] = dydt_out[1];
    dydt_in[2] = dydt_out[2];

    if(i < num_skip) continue;

    printf("%.09f %.09f %.09f %.09f\n",t,x[0],x[1],x[2]);   
  }

  gsl_odeiv_step_free (s);
  return 0;
}
