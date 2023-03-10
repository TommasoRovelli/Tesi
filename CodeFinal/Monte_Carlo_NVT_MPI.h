#ifndef __Montecarlo_
#define __Montecarlo_

#include<iostream>
#include<fstream>
#include<ostream>
#include<cmath>
#include<iomanip>
#include<string>
#include<sstream>
#include<vector>
#include"mpi.h"

using namespace std;

//==========================================
// Global variables
//==========================================

// potential parameters
double khc, r_hc, r_hc2;									//height, radius and raidus^2

bool p_check=true;

// thermodynamical state
int npart, ncluster, nspin;
double beta,temp,vol,rho,boxx,boxy,rcut,rcut2, rverlet;
bool check_verlet;
double u1x, u2x, u2y, tga;										//base vector sides and tangent angle
double qminx, qmino, qminy;										//fundamental cell size in reciprocal space, x, y and oblique axes
int nx,ny;													//number of rectangular macrocells which compose the overall simulation box

// simulation length and delta ranges for the moves
int nstep, nblk, nblkt;
const int n_delta=5;
double alpha, deltaB, deltaCM, deltaAngle, deltaAspectRatio;
double delta[n_delta];


//Restart_parameters
bool restart;
int oblk;									//number of blocks from which to start (used to take into account the previous runs)
long double glob_epot, glob_epot2, glob_heat, glob_heat2;	//old runs accumulators


// MPI_parameters
int size, my_rank, rank_0=0;

//print parameters
int prec=17, wd=25;											//significant digits
const double pi=3.1415927;								//pi
double incr=1.00001;

//observables vector
int n_props;
int n_obs=4;												//number of observables
int iv=0, iv2=1, iv3=2, iv4=3;											//index for potential, virial and potential^2 and order parameter 1-<cos^2(theta)> 
long double vtail;											//tail corrections
int nbins=500, nbinqx,nbinqy, nbinqo; 					//g(r) bins, S(q) bins along x,y,o, 
int nbins_angle;                                    //bins for distribution of the box angle gang(a)
int nbins_ld_cm_x, nbins_ld_cm_o1, nbins_ld_cm_o2;     //bins for linar distribution of the cm distancies, to check the geometry of the lattice
int nbins_ene;
int nbins_box;
double bin_size;											//physical length of bins for g(r) and gspin(r)				
double bin_size_angle;							//wideness of the angular bins 
double bin_size_ene;
double bin_size_boxx;
double bin_size_boxy;
double left_value_angle;
double left_value_ene;
double left_value_boxx;
double left_value_boxy;
double bin_size_ld_cm_x, bin_size_ld_cm_o1, bin_size_ld_cm_o2;
double left_value_ld_x, left_value_ld_o1, left_value_ld_o2;
int* bin_counter_x;									//counter to normalize gspin(r) along x
int* bin_counter_obl;                                    //counter to normalize gspin(r) along o
int igofr, igspin_x, igspin_obl, isofqx, isofqo, isofqy, isofq_clx, isofq_cly, isofq_clo, igang, ildcm_x, ildcm_o1, ildcm_o2, iene, iboxx, iboxy;	//starting index of the observables inside walker
double angle= 1.04864133; //angle in radians between x and oblique axis - this value is overwritten by input file value
double sinang = sin(angle);
double cosang = cos(angle);
double my_boxx;
double tanang = tan(angle);

long double* walker;										//observables array (step by step, overwritten)
long double* blk_av; 	long double* mpi_blk_av;			//observables array (accumulate and normalize within each block)
double blk_norm; 	double mpi_blk_norm;					//inner block normalization
long double* glob_av;										//observables array (amongst various blocks) - progressive average
long double* glob_av2;										//observables^2 array (amongst various blocks)
long double* ene_dist; 
long double* angle_dist;
long double* boxx_dist;
long double* boxy_dist;

// acceptance rate
const int n_moves=n_delta+3, ir=3, ib=4, icm=5, ia=6, ias=7;
int accepted[n_moves], mpi_accepted[n_moves];
int attempted[n_moves], mpi_attempted[n_moves];

//Particle struct
struct Particle
{
	double x, y; 											    //coordinates
	int pointer, i_cluster;									//coupled particle and cluster index
	double xm, ym, phi, dr;									//spin variables (spin = dimer)
};
Particle* particle;										//particles array

//Define MPI_Particle
const int num_var=8;
int var_len[num_var];
MPI_Aint var_add[num_var];
MPI_Datatype var_type[num_var];
MPI_Datatype MPI_PARTICLE;

//Store neighbors
vector<vector<int> > neighbors;

//**********************************************************************************************************

//==========================================
// Functions
//==========================================

//Input
void Input(void);										//read data from external file
void Def_neighbors();									//define neighboring particles exploiting Verlet lists
void Update_neighbors();								//update verlet neighbors
void Check_Configuration();								//check possible collapses of spins and fix it

//Reset
void Reset(int);										// Reset values after each block for each rank
void Reset_mpi();										// Reset MPI averages

//Monte Carlo statistics
void Measure(void);										//evaluate observables and save them into walker
void Accumulate(void);									//sum values within a block
void Averages(int);										//evaluate block averages and print results
long double Error(long double,long double,int);			//compute statistical error with data blocking

//Monte Carlo Moves
long double Boltzmann(double, double, int);				//Computes energy difference related to single particle moves
long double Boltzmann_2(double, double, int, 			//Computes energy difference related to 2-particle moves
                        double, double, int);
void Boltzmann_Print(double xx, double yy, double xx_old, double yy_old, int ip);

void Move(int);											// single particle translation
void Move_Rotate();										// particle pair rotation
void Move_Break();										// internal dimer displacement
void Move_Cluster();									// move cluster barycenter
void Move_Angle();										// angle changing, area is constant 
void Move_AspectRatio();								// modify the ratio base/height 

//Print
string ConvertStr(int&);								// Convert numbers into strings
void ConfFinal(void);									// Print final config
void ConfPartial(int&, bool);							// Print intermediate configuration (true=equilibration)
void Print_Parameters();								// Print observables
void Print_Cores(int);

//Cluster
void Make_Spin(int&);									// Generate dimers, coupling the closest neighbors
void Cluster();											// optimize particles coupling and define spin variables
double Distance(Particle&, Particle&);					// Square distance between 2 particles
double Phi_spin(Particle&, Particle&);					// Returns angle between 2 particles in Pbca

//Potenziale
long double V_Pot(double);								//(ok)-> Potenziale di interazione di coppia
long double V_Tail();									//(ok)-> Code del potenziale

//PBC
double Pbcx(double, double);							// PBC x for non-rectangular box
double Pbcy(double);									// PBC y for non-rectangular box
double Pbc_cart_x(double);                        		// Usual PBC x for rectangular box
double Pbc_cart_y(double);                          	// Usual PBC y for rectangular box
double Pbca(double);									// Angular PBC
int Pbc_icl_x(int,int,int);                            	// PBC for integer cluster indexes along x
int Pbc_icl_obl(int,int,int,int);	                 	// PBC for integer cluster indexes along obl. axis
#endif
