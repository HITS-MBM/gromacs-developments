/*
 *       @(#) copyrgt.c 1.12 9/30/97
 *
 *       This source code is part of
 *
 *        G   R   O   M   A   C   S
 *
 * GROningen MAchine for Chemical Simulations
 *
 *            VERSION 2.0b
 * 
 * Copyright (c) 1990-1997,
 * BIOSON Research Institute, Dept. of Biophysical Chemistry,
 * University of Groningen, The Netherlands
 *
 * Please refer to:
 * GROMACS: A message-passing parallel molecular dynamics implementation
 * H.J.C. Berendsen, D. van der Spoel and R. van Drunen
 * Comp. Phys. Comm. 91, 43-56 (1995)
 *
 * Also check out our WWW page:
 * http://rugmd0.chem.rug.nl/~gmx
 * or e-mail to:
 * gromacs@chem.rug.nl
 *
 * And Hey:
 * Grunge ROck MAChoS
 */
#ifndef _idef_h
#define _idef_h

#define MAXATOMLIST	4
#define MAXFORCEPARAM	6
#define NR_RBDIHS	6

typedef atom_id t_iatom;

/* this MUST correspond to the 
   t_interaction_function[F_NRE] in gmxlib/ifunc.c */
enum {
  F_ANGLES, 
  F_BHAM,
  F_BONDS,
  F_MORSE,
  F_WPOL,
  F_DISRES, 
  F_IDIHS, 
  F_LJ,
  F_LJ14,
  F_LR,
  F_LJLR,
  F_PDIHS, 
  F_POSRES,
  F_RBDIHS,
  F_SHAKE,
  F_SETTLE,
  F_DUMMY1,
  F_DUMMY2,
  F_DUMMY2FD,
  F_DUMMY2FAD,
  F_DUMMY3,
  F_SR,
  F_EPOT,
  F_EKIN,
  F_ETOT,
  F_TEMP,
  F_PRES,
  F_DVDL,
  F_DVDLKIN,
  F_NRE		/* This number is for the total number of energies	*/
};
  
typedef union
{
  /* Some parameters have A and B values for free energy calculations.
   * The B values are not used for regular simulations of course.
   * Free Energy for nonbondeds can be computed by changing the atom type.
   * The harmonic type is used for all harmonic potentials:
   * bonds, angles and improper dihedrals
   */
  struct {real a,b,c;				} bham;
  struct {real rA,krA,rB,krB;           	} harmonic; 
  /* No free energy supported for WPOL */ 
  struct {real kx,ky,kz,rOH,rHH,rOD;            } wpol; 
  struct {real c6,c12;				} lj;
  struct {real c6A,c12A,c6B,c12B;		} lj14;
  /* Proper dihedrals can not have different multiplicity when
   * doing free energy calculations, because the potential would not
   * be periodic anymore.
   */ 
  struct {real phiA,cpA;int mult;real phiB,cpB; } pdihs;
  struct {real dA,dB;		        	} shake;
  /* Settle can not be used for Free energy calculations.
   * Use shake (or lincs) instead.
   * The rest of the things cannot (yet) be used in FEP studies either.
   */
  struct {real doh,dhh;                         } settle;
  struct {real b0,cb,beta;            	 	} morse;
  struct {real pos0[DIM],fc[DIM];	        } posres;
  struct {real rbc[NR_RBDIHS];			} rbdihs;
  struct {real a,b,c,d,e,f;                     } dummy;   
  struct {real rx0,rx1,rx2,rx3; int type,index; } disres; 
  struct {real buf[MAXFORCEPARAM];		} generic; /* Conversion */
} t_iparams;

typedef int t_functype;

typedef struct
{
  int nr;
  int multinr[MAXPROC];
  t_iatom *iatoms;
} t_ilist;

/*
 * The struct t_ilist defines a list of atoms with their interactions. 
 * General field description:
 *   int nr
 *	the size (nr elements) of the interactions array (iatoms[]). This 
 *      equals multinr[MAXPROC-1].
 *   int multinr[MAXPROC]
 * 	specifies the number of type and atom id sequences in the iatoms[] 
 *	array. Every element specifies the index of the first interaction
 *      for the next processor. The first processor starts at zero. So for 
 *      n=0, the interactions run from 0 upto multinr[0]. The interactions
 *      for processor n (n>0) run from multinr[n-1] to index[n] (not including 
 *      multinr[n]).
 *   t_iatom *iatoms
 * 	specifies which atoms are involved in an interaction of a certain 
 *       type. The layout of this array is as follows:
 *
 *	  +-----+---+---+---+-----+---+---+-----+---+---+---+-----+---+---+...
 *	  |type1|at1|at2|at3|type2|at1|at2|type1|at1|at2|at3|type3|at1|at2|
 *	  +-----+---+---+---+-----+---+---+-----+---+---+---+-----+---+---+...
 *
 * 	So for interaction type type1 3 atoms are needed, and for type2 and 
 *      type3 only 2. The type identifier is used to select the function to 
 *	calculate the interaction and its actual parameters. This type 
 *	identifier is an index in a params[] and functype[] array.
 * The multinr[] array will be initialised for MAXPROC in such a way that up 
 * to the actual number of processors (during creation time), the array is
 * filled with the indices, the remaining processors get empty parts by 
 * setting the indices to the largest value. In that way it is always possible
 * to run this system on a larger multiprocessor ring however only the
 * configured number of processors will we used. Running on less processors
 * than configured is also possible by taking together adjacent
 * processors. Note that in this case the load balance might get worse.
 * The single processor version is implemented by simply using the complete
 * configuration as one piece.
 */

typedef struct
{
  int ntypes;
  int pid;
  int atnr;
  t_functype *functype;
  t_iparams  *iparams;

  t_ilist il[F_NRE];
} t_idef;

/*
 * The struct t_idef defines all the interactions for the complete
 * simulation. The structure is setup in such a way that the multiprocessor
 * version of the program  can use it as easy as the single processor version.
 * General field description:
 *   int ntypes
 *	defines the number of elements in functype[] and param[].
 *   int pid
 *      the processor id (if parallel machines)
 *   int atnr
 *      the number of atomtypes
 *   t_functype *functype
 *	array of length ntypes, defines for every force type what type of 
 *      function to use. Every "bond" with the same function but different 
 *	force parameters is a different force type. The type identifier in the 
 *	forceatoms[] array is an index in this array.
     t_iparams *iparams;
 *	array of length ntypes, defines the parameters for every interaction
 *      type. The type identifier in the actual interaction list
 *      (bondeds.iatoms[] or shakes.iatoms[]) is an index in this array.
 *   t_ilist il[F_NRE]
 *      The list of interactions for each type. Note that some,
 *      such as LJ and COUL will have 0 entries.
 */

#endif
