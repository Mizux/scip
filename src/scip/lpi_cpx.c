/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2005 Tobias Achterberg                              */
/*                                                                           */
/*                  2002-2005 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma ident "@(#) $Id: lpi_cpx.c,v 1.99 2005/08/22 18:35:40 bzfpfend Exp $"

/**@file   lpi_cpx.c
 * @brief  SCIP_LP interface for CPLEX 8.0 / 9.0
 * @author Tobias Achterberg
 */

/*--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "cplex.h"
#include "scip/bitencode.h"
#include "scip/lpi.h"
#include "scip/message.h"



#define CHECK_ZERO(x) { int _restat_;                                               \
                        if( (_restat_ = (x)) != 0 )                                 \
                        {                                                           \
                           SCIPerrorMessage("LP Error: CPLEX returned %d\n", _restat_); \
                           return SCIP_LPERROR;                                     \
                        }                                                           \
                      }

#define ABORT_ZERO(x) { int _restat_;                                               \
                        if( (_restat_ = (x)) != 0 )                                 \
                        {                                                           \
                           SCIPerrorMessage("LP Error: CPLEX returned %d\n", _restat_); \
                           SCIPABORT();                                                 \
                        }                                                           \
                      }

#define CPX_INT_MAX 2100000000 /* CPLEX doesn't accept larger values in integer parameters */


typedef SCIP_DUALPACKET COLPACKET;           /* each column needs two bits of information (basic/on_lower/on_upper) */
#define COLS_PER_PACKET SCIP_DUALPACKETSIZE
typedef SCIP_DUALPACKET ROWPACKET;           /* each row needs two bit of information (basic/on_lower/on_upper) */
#define ROWS_PER_PACKET SCIP_DUALPACKETSIZE

/* CPLEX parameter lists which can be changed */
#define NUMINTPARAM  9
static const int intparam[NUMINTPARAM] = {
   CPX_PARAM_ADVIND,
   CPX_PARAM_ITLIM,
   CPX_PARAM_FASTMIP,
   CPX_PARAM_SCAIND,
   CPX_PARAM_PREIND,
   CPX_PARAM_PPRIIND,
   CPX_PARAM_DPRIIND,
   CPX_PARAM_SIMDISPLAY,
   CPX_PARAM_SCRIND
};
#define NUMDBLPARAM  7
static const int dblparam[NUMDBLPARAM] = {
   CPX_PARAM_EPRHS,
   CPX_PARAM_EPOPT,
   CPX_PARAM_BAREPCOMP,
   CPX_PARAM_OBJLLIM,
   CPX_PARAM_OBJULIM,
   CPX_PARAM_TILIM,
   CPX_PARAM_EPMRK
};
static const double dblparammin[NUMDBLPARAM] = {
   +1e-09, /*CPX_PARAM_EPRHS*/
   +1e-09, /*CPX_PARAM_EPOPT*/
   +1e-12, /*CPX_PARAM_BAREPCOMP*/
   -1e+99, /*CPX_PARAM_OBJLLIM*/
   -1e+99, /*CPX_PARAM_OBJULIM*/
   -1e+99, /*CPX_PARAM_TILIM*/
   0.0001  /*CPX_PARAM_EPMRK*/
};

/** CPLEX parameter settings */
struct CPXParam
{
   int                   intparval[NUMINTPARAM]; /**< integer parameter values */
   double                dblparval[NUMDBLPARAM]; /**< double parameter values */
};
typedef struct CPXParam CPXPARAM;

/** SCIP_LP interface */
struct SCIP_LPi
{
   CPXLPptr         cpxlp;              /**< CPLEX SCIP_LP pointer */
   int                   solstat;            /**< solution status of last optimization call */
   CPXPARAM         cpxparam;           /**< current parameter values for this SCIP_LP */
   char*                 larray;             /**< array with 'L' entries for changing lower bounds */
   char*                 uarray;             /**< array with 'U' entries for changing upper bounds */
   char*                 senarray;           /**< array for storing row senses */
   SCIP_Real*            rhsarray;           /**< array for storing rhs values */
   SCIP_Real*            rngarray;           /**< array for storing range values */
   SCIP_Real*            valarray;           /**< array for storing coefficient values */
   int*                  rngindarray;        /**< array for storing row indices with range values */
   int*                  cstat;              /**< array for storing column basis status */
   int*                  rstat;              /**< array for storing row basis status */
   int*                  indarray;           /**< array for storing coefficient indices */
   int                   boundchgsize;       /**< size of larray and uarray */
   int                   sidechgsize;        /**< size of senarray, rngarray, and rngindarray */
   int                   valsize;            /**< size of valarray and indarray */
   int                   cstatsize;          /**< size of cstat array */
   int                   rstatsize;          /**< size of rstat array */
   int                   iterations;         /**< number of iterations used in the last solving call */
   SCIP_Bool             solisbasic;         /**< is current SCIP_LP solution a basic solution? */
};

/** SCIP_LPi state stores basis information */
struct SCIP_LPiState
{
   int                   ncols;              /**< number of SCIP_LP columns */
   int                   nrows;              /**< number of SCIP_LP rows */
   COLPACKET*       packcstat;          /**< column basis status in compressed form */
   ROWPACKET*       packrstat;          /**< row basis status in compressed form */
};


static CPXENVptr    cpxenv = NULL;      /**< CPLEX environment */
static CPXPARAM     defparam;           /**< default CPLEX parameters */
static CPXPARAM     curparam;           /**< current CPLEX parameters in the environment */
static int               numlp = 0;          /**< number of open SCIP_LP objects */



/*
 * dynamic memory arrays
 */

/** resizes larray and uarray to have at least num entries */
static
SCIP_RETCODE ensureBoundchgMem(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   num                 /**< minimal number of entries in array */
   )
{
   assert(lpi != NULL);

   if( num > lpi->boundchgsize )
   {
      int newsize;
      int i;

      newsize = MAX(2*lpi->boundchgsize, num);
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->larray, newsize) );
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->uarray, newsize) );
      for( i = lpi->boundchgsize; i < newsize; ++i )
      {
         lpi->larray[i] = 'L';
         lpi->uarray[i] = 'U';
      }
      lpi->boundchgsize = newsize;
   }
   assert(num <= lpi->boundchgsize);

   return SCIP_OKAY;
}

/** resizes senarray, rngarray, and rngindarray to have at least num entries */
static
SCIP_RETCODE ensureSidechgMem(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   num                 /**< minimal number of entries in array */
   )
{
   assert(lpi != NULL);

   if( num > lpi->sidechgsize )
   {
      int newsize;

      newsize = MAX(2*lpi->sidechgsize, num);
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->senarray, newsize) );
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->rhsarray, newsize) );
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->rngarray, newsize) );
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->rngindarray, newsize) );
      lpi->sidechgsize = newsize;
   }
   assert(num <= lpi->sidechgsize);

   return SCIP_OKAY;
}

/** resizes valarray and indarray to have at least num entries */
static
SCIP_RETCODE ensureValMem(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   num                 /**< minimal number of entries in array */
   )
{
   assert(lpi != NULL);

   if( num > lpi->valsize )
   {
      int newsize;

      newsize = MAX(2*lpi->valsize, num);
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->valarray, newsize) );
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->indarray, newsize) );
      lpi->valsize = newsize;
   }
   assert(num <= lpi->valsize);

   return SCIP_OKAY;
}

/** resizes cstat array to have at least num entries */
static
SCIP_RETCODE ensureCstatMem(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   num                 /**< minimal number of entries in array */
   )
{
   assert(lpi != NULL);

   if( num > lpi->cstatsize )
   {
      int newsize;

      newsize = MAX(2*lpi->cstatsize, num);
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->cstat, newsize) );
      lpi->cstatsize = newsize;
   }
   assert(num <= lpi->cstatsize);

   return SCIP_OKAY;
}

/** resizes rstat array to have at least num entries */
static
SCIP_RETCODE ensureRstatMem(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   num                 /**< minimal number of entries in array */
   )
{
   assert(lpi != NULL);

   if( num > lpi->rstatsize )
   {
      int newsize;

      newsize = MAX(2*lpi->rstatsize, num);
      SCIP_ALLOC( BMSreallocMemoryArray(&lpi->rstat, newsize) );
      lpi->rstatsize = newsize;
   }
   assert(num <= lpi->rstatsize);

   return SCIP_OKAY;
}

/** stores current basis in internal arrays of SCIP_LPI data structure */
static
SCIP_RETCODE getBase(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int ncols;
   int nrows;

   assert(cpxenv != NULL);
   assert(lpi != NULL);

   SCIPdebugMessage("getBase()\n");

   ncols = CPXgetnumcols(cpxenv, lpi->cpxlp);
   nrows = CPXgetnumrows(cpxenv, lpi->cpxlp);

   /* allocate enough memory for storing uncompressed basis information */
   SCIP_CALL( ensureCstatMem(lpi, ncols) );
   SCIP_CALL( ensureRstatMem(lpi, nrows) );

   /* get unpacked basis information from CPLEX */
   CHECK_ZERO( CPXgetbase(cpxenv, lpi->cpxlp, lpi->cstat, lpi->rstat) );

   return SCIP_OKAY;
}

/** loads basis stored in internal arrays of SCIP_LPI data structure into CPLEX */
static
SCIP_RETCODE setBase(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);

   SCIPdebugMessage("setBase()\n");

   /* load basis information into CPLEX */
   CHECK_ZERO( CPXcopybase(cpxenv, lpi->cpxlp, lpi->cstat, lpi->rstat) );

   return SCIP_OKAY;
}




/*
 * SCIP_LPi state methods
 */

/** returns the number of packets needed to store column packet information */
static 
int colpacketNum(
   int                   ncols               /**< number of columns to store */
   )
{
   return (ncols+(int)COLS_PER_PACKET-1)/(int)COLS_PER_PACKET;
}

/** returns the number of packets needed to store row packet information */
static 
int rowpacketNum(
   int                   nrows               /**< number of rows to store */
   )
{
   return (nrows+(int)ROWS_PER_PACKET-1)/(int)ROWS_PER_PACKET;
}

/** store row and column basis status in a packed SCIP_LPi state object */
static
void lpistatePack(
   SCIP_LPISTATE*       lpistate,            /**< pointer to SCIP_LPi state data */
   const int*           cstat,               /**< basis status of columns in unpacked format */
   const int*           rstat                /**< basis status of rows in unpacked format */
   )
{
   assert(lpistate != NULL);
   assert(lpistate->packcstat != NULL);
   assert(lpistate->packrstat != NULL);

   SCIPencodeDualBit(cstat, lpistate->packcstat, lpistate->ncols);
   SCIPencodeDualBit(rstat, lpistate->packrstat, lpistate->nrows);
}

/** unpacks row and column basis status from a packed SCIP_LPi state object */
static
void lpistateUnpack(
   const SCIP_LPISTATE* lpistate,            /**< pointer to SCIP_LPi state data */
   int*                 cstat,               /**< buffer for storing basis status of columns in unpacked format */
   int*                 rstat                /**< buffer for storing basis status of rows in unpacked format */
   )
{
   assert(lpistate != NULL);
   assert(lpistate->packcstat != NULL);
   assert(lpistate->packrstat != NULL);

   SCIPdecodeDualBit(lpistate->packcstat, cstat, lpistate->ncols);
   SCIPdecodeDualBit(lpistate->packrstat, rstat, lpistate->nrows);
}

/** creates SCIP_LPi state information object */
static
SCIP_RETCODE lpistateCreate(
   SCIP_LPISTATE**       lpistate,           /**< pointer to SCIP_LPi state */
   BMS_BLKMEM*           blkmem,             /**< block memory */
   int                   ncols,              /**< number of columns to store */
   int                   nrows               /**< number of rows to store */
   )
{
   assert(lpistate != NULL);
   assert(blkmem != NULL);
   assert(ncols >= 0);
   assert(nrows >= 0);

   SCIP_ALLOC( BMSallocBlockMemory(blkmem, lpistate) );
   SCIP_ALLOC( BMSallocBlockMemoryArray(blkmem, &(*lpistate)->packcstat, colpacketNum(ncols)) );
   SCIP_ALLOC( BMSallocBlockMemoryArray(blkmem, &(*lpistate)->packrstat, rowpacketNum(nrows)) );

   return SCIP_OKAY;
}

/** frees SCIP_LPi state information */
static
void lpistateFree(
   SCIP_LPISTATE**       lpistate,           /**< pointer to SCIP_LPi state information (like basis information) */
   BMS_BLKMEM*           blkmem              /**< block memory */
   )
{
   assert(blkmem != NULL);
   assert(lpistate != NULL);
   assert(*lpistate != NULL);

   BMSfreeBlockMemoryArray(blkmem, &(*lpistate)->packcstat, colpacketNum((*lpistate)->ncols));
   BMSfreeBlockMemoryArray(blkmem, &(*lpistate)->packrstat, rowpacketNum((*lpistate)->nrows));
   BMSfreeBlockMemory(blkmem, lpistate);
}



/*
 * local methods
 */

static
SCIP_RETCODE getParameterValues(CPXPARAM* cpxparam)
{
   int i;
   
   assert(cpxenv != NULL);
   assert(cpxparam != NULL);

   SCIPdebugMessage("getParameterValues()\n");

   for( i = 0; i < NUMINTPARAM; ++i )
   {
      CHECK_ZERO( CPXgetintparam(cpxenv, intparam[i], &(cpxparam->intparval[i])) );
   }
   for( i = 0; i < NUMDBLPARAM; ++i )
   {
      CHECK_ZERO( CPXgetdblparam(cpxenv, dblparam[i], &(cpxparam->dblparval[i])) );
   }

   return SCIP_OKAY;
}
   
static
SCIP_RETCODE checkParameterValues(void)
{
#ifndef NDEBUG
   CPXPARAM par;
   int i;
   
   SCIP_CALL( getParameterValues(&par) );
   for( i = 0; i < NUMINTPARAM; ++i )
      assert(curparam.intparval[i] == par.intparval[i]);
   for( i = 0; i < NUMDBLPARAM; ++i )
      assert(MAX(curparam.dblparval[i], dblparammin[i]) == par.dblparval[i]); /*lint !e777*/
#endif

   return SCIP_OKAY;
}

static
SCIP_RETCODE setParameterValues(const CPXPARAM* cpxparam)
{
   int i;
   
   assert(cpxenv != NULL);
   assert(cpxparam != NULL);
   
   SCIPdebugMessage("setParameterValues()\n");

   for( i = 0; i < NUMINTPARAM; ++i )
   {
      if( curparam.intparval[i] != cpxparam->intparval[i] )
      {
         SCIPdebugMessage("setting CPLEX int parameter %d from %d to %d\n", 
            intparam[i], curparam.intparval[i], cpxparam->intparval[i]);
         curparam.intparval[i] = cpxparam->intparval[i];
         CHECK_ZERO( CPXsetintparam(cpxenv, intparam[i], curparam.intparval[i]) );
      }
   }
   for( i = 0; i < NUMDBLPARAM; ++i )
   {
      if( curparam.dblparval[i] != cpxparam->dblparval[i] ) /*lint !e777*/
      {
         SCIPdebugMessage("setting CPLEX dbl parameter %d from %g to %g\n", 
            dblparam[i], curparam.dblparval[i], MAX(cpxparam->dblparval[i], dblparammin[i]));
         curparam.dblparval[i] = MAX(cpxparam->dblparval[i], dblparammin[i]);
         CHECK_ZERO( CPXsetdblparam(cpxenv, dblparam[i], curparam.dblparval[i]) );
      }
   }

   SCIP_CALL( checkParameterValues() );

   return SCIP_OKAY;
}

static
void copyParameterValues(CPXPARAM* dest, const CPXPARAM* source)
{
   int i;

   for( i = 0; i < NUMINTPARAM; ++i )
      dest->intparval[i] = source->intparval[i];
   for( i = 0; i < NUMDBLPARAM; ++i )
      dest->dblparval[i] = source->dblparval[i];
}

static
int getIntParam(SCIP_LPI* lpi, const int param)
{
   int i;
   
   assert(lpi != NULL);

   for( i = 0; i < NUMINTPARAM; ++i )
      if( intparam[i] == param )
         return lpi->cpxparam.intparval[i];

   SCIPerrorMessage("unknown CPLEX integer parameter\n");
   SCIPABORT();
   return 0;
}

static
double getDblParam(SCIP_LPI* lpi, const int param)
{
   int i;

   assert(lpi != NULL);

   for( i = 0; i < NUMDBLPARAM; ++i )
      if( dblparam[i] == param )
         return lpi->cpxparam.dblparval[i];

   SCIPerrorMessage("unknown CPLEX double parameter\n");
   SCIPABORT();
   return 0.0;
}

static
void setIntParam(SCIP_LPI* lpi, const int param, int parval)
{
   int i;

   assert(lpi != NULL);

   for( i = 0; i < NUMINTPARAM; ++i )
      if( intparam[i] == param )
      {
         lpi->cpxparam.intparval[i] = parval;
         return;
      }

   SCIPerrorMessage("unknown CPLEX integer parameter\n");
   SCIPABORT();
}

static
void setDblParam(SCIP_LPI* lpi, const int param, double parval)
{
   int i;

   assert(lpi != NULL);

   for( i = 0; i < NUMDBLPARAM; ++i )
      if( dblparam[i] == param )
      {
         lpi->cpxparam.dblparval[i] = parval;
         return;
      }

   SCIPerrorMessage("unknown CPLEX double parameter\n");
   SCIPABORT();
}

static
void invalidateSolution(SCIP_LPI* lpi)
{
   assert(lpi != NULL);
   lpi->solstat = -1;
}

static
int cpxObjsen(SCIP_OBJSEN objsen)
{
   switch( objsen )
   {
   case SCIP_OBJSEN_MAXIMIZE:
      return CPX_MAX;
   case SCIP_OBJSEN_MINIMIZE:
      return CPX_MIN;
   default:
      SCIPerrorMessage("invalid objective sense\n");
      SCIPABORT();
      return 0;
   }
}

/** converts SCIP's lhs/rhs pairs into CPLEX's sen/rhs/rng */
static
void convertSides(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   nrows,              /**< number of rows */
   const SCIP_Real*      lhs,                /**< left hand side vector */
   const SCIP_Real*      rhs,                /**< right hand side vector */
   int                   indoffset,          /**< index of first row in SCIP_LP */
   int*                  rngcount            /**< pointer to store the number of range rows */
   )
{
   int i;

   assert(lpi != NULL);
   assert(nrows >= 0);
   assert(lhs != NULL);
   assert(rhs != NULL);
   assert(rngcount != NULL);

   /* convert lhs/rhs into sen/rhs/rng */
   *rngcount = 0;
   for( i = 0; i < nrows; ++i )
   {
      assert(lhs[i] <= rhs[i]);
      if( lhs[i] == rhs[i] ) /*lint !e777*/
      {
         assert(-CPX_INFBOUND < rhs[i] && rhs[i] < CPX_INFBOUND);
         lpi->senarray[i] = 'E';
         lpi->rhsarray[i] = rhs[i];
      }
      else if( lhs[i] <= -CPX_INFBOUND )
      {
         assert(-CPX_INFBOUND < rhs[i] && rhs[i] < CPX_INFBOUND);
         lpi->senarray[i] = 'L';
         lpi->rhsarray[i] = rhs[i];
      }
      else if( rhs[i] >= CPX_INFBOUND )
      {
         assert(-CPX_INFBOUND < lhs[i] && lhs[i] < CPX_INFBOUND);
         lpi->senarray[i] = 'G';
         lpi->rhsarray[i] = lhs[i];
      }
      else
      {
         /* CPLEX defines a ranged row to be within rhs and rhs+rng.
          * -> To keep SCIP's meaning of the rhs value, we would like to use negative range values: rng := lhs - rhs,
          *    but there seems to be a bug in CPLEX's presolve with negative range values:
          *    the ranged row
          *              0 <= -x <= 100000 with x >= 0 (rhs=0, rng=-100000) 
          *    would lead to the CPLEX row
          *              -x -Rg = 100000 
          *                  Rg = 0
          *    instead of the correct presolving implication  Rg = -100000.
          * -> Because of this bug, we have to use an additional rhsarray[] for the converted right hand sides and
          *    use rhsarray[i] = lhs[i] and rngarray[i] = rhs[i] - lhs[i] for ranged rows to keep the range values
          *    non-negative.
          */
         lpi->senarray[i] = 'R';
         lpi->rhsarray[i] = lhs[i];
         lpi->rngarray[*rngcount] = rhs[i] - lhs[i];
         lpi->rngindarray[*rngcount] = i + indoffset;
         (*rngcount)++;
      }
   }
}

/** converts CPLEX's sen/rhs/rng triplets into SCIP's lhs/rhs pairs */
static
void reconvertBothSides(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   nrows,              /**< number of rows */
   SCIP_Real*            lhs,                /**< buffer to store the left hand side vector */
   SCIP_Real*            rhs                 /**< buffer to store the right hand side vector */
   )
{
   int i;

   assert(lpi != NULL);
   assert(nrows >= 0);
   assert(lhs != NULL);
   assert(rhs != NULL);

   for( i = 0; i < nrows; ++i )
   {
      switch( lpi->senarray[i] )
      {
      case 'E':
         lhs[i] = lpi->rhsarray[i];
         rhs[i] = lpi->rhsarray[i];
         break;

      case 'L':
         lhs[i] = -CPX_INFBOUND;
         rhs[i] = lpi->rhsarray[i];
         break;

      case 'G':
         lhs[i] = lpi->rhsarray[i];
         rhs[i] = CPX_INFBOUND;
         break;

      case 'R':
         assert(lpi->rngarray[i] != 0.0);
         if( lpi->rngarray[i] > 0.0 )
         {
            lhs[i] = lpi->rhsarray[i];
            rhs[i] = lpi->rhsarray[i] + lpi->rngarray[i];
         }
         else
         {
            lhs[i] = lpi->rhsarray[i] + lpi->rngarray[i];
            rhs[i] = lpi->rhsarray[i];
         }
         break;
         
      default:
         SCIPerrorMessage("invalid row sense\n");
         SCIPABORT();
      }
      assert(lhs[i] <= rhs[i]);
   }
}

/** converts CPLEX's sen/rhs/rng triplets into SCIP's lhs/rhs pairs, only storing the left hand side */
static
void reconvertLhs(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   nrows,              /**< number of rows */
   SCIP_Real*            lhs                 /**< buffer to store the left hand side vector */
   )
{
   int i;

   assert(lpi != NULL);
   assert(nrows >= 0);
   assert(lhs != NULL);

   for( i = 0; i < nrows; ++i )
   {
      switch( lpi->senarray[i] )
      {
      case 'E':
         assert(lpi->rngarray[i] == 0.0);
         lhs[i] = lpi->rhsarray[i];
         break;

      case 'L':
         assert(lpi->rngarray[i] == 0.0);
         lhs[i] = -CPX_INFBOUND;
         break;

      case 'G':
         assert(lpi->rngarray[i] == 0.0);
         lhs[i] = lpi->rhsarray[i];
         break;

      case 'R':
         assert(lpi->rngarray[i] != 0.0);
         if( lpi->rngarray[i] > 0.0 )
            lhs[i] = lpi->rhsarray[i];
         else
            lhs[i] = lpi->rhsarray[i] + lpi->rngarray[i];
         break;
         
      default:
         SCIPerrorMessage("invalid row sense\n");
         SCIPABORT();
      }
   }
}

/** converts CPLEX's sen/rhs/rng triplets into SCIP's lhs/rhs pairs, only storing the right hand side */
static
void reconvertRhs(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   nrows,              /**< number of rows */
   SCIP_Real*            rhs                 /**< buffer to store the right hand side vector */
   )
{
   int i;

   assert(lpi != NULL);
   assert(nrows >= 0);
   assert(rhs != NULL);

   for( i = 0; i < nrows; ++i )
   {
      switch( lpi->senarray[i] )
      {
      case 'E':
         assert(lpi->rngarray[i] == 0.0);
         rhs[i] = lpi->rhsarray[i];
         break;

      case 'L':
         assert(lpi->rngarray[i] == 0.0);
         rhs[i] = lpi->rhsarray[i];
         break;

      case 'G':
         assert(lpi->rngarray[i] == 0.0);
         rhs[i] = CPX_INFBOUND;
         break;

      case 'R':
         assert(lpi->rngarray[i] != 0.0);
         if( lpi->rngarray[i] > 0.0 )
            rhs[i] = lpi->rhsarray[i] + lpi->rngarray[i];
         else
            rhs[i] = lpi->rhsarray[i];
         break;
         
      default:
         SCIPerrorMessage("invalid row sense\n");
         SCIPABORT();
      }
   }
}

/** converts CPLEX's sen/rhs/rng triplets into SCIP's lhs/rhs pairs */
static
void reconvertSides(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   nrows,              /**< number of rows */
   SCIP_Real*            lhs,                /**< buffer to store the left hand side vector, or NULL */
   SCIP_Real*            rhs                 /**< buffer to store the right hand side vector, or NULL */
   )
{
   if( lhs != NULL && rhs != NULL )
      reconvertBothSides(lpi, nrows, lhs, rhs);
   else if( lhs != NULL )
      reconvertLhs(lpi, nrows, lhs);
   else if( rhs != NULL )
      reconvertRhs(lpi, nrows, rhs);
}




/*
 * SCIP_LP Interface Methods
 */


/*
 * Miscellaneous Methods
 */

static char cpxname[SCIP_MAXSTRLEN];

/**@name Miscellaneous Methods */
/**@{ */

/** gets name and version of SCIP_LP solver */
const char* SCIPlpiGetSolverName(
   void
   )
{
   sprintf(cpxname, "CPLEX %.2f", (SCIP_Real)CPX_VERSION/100.0);
   return cpxname;
}

/**@} */




/*
 * SCIP_LPI Creation and Destruction Methods
 */

/**@name SCIP_LPI Creation and Destruction Methods */
/**@{ */

/** creates an SCIP_LP problem object */
SCIP_RETCODE SCIPlpiCreate(
   SCIP_LPI**            lpi,                /**< pointer to an SCIP_LP interface structure */
   const char*           name,               /**< problem name */
   SCIP_OBJSEN           objsen              /**< objective sense */
   )
{
   int          restat;

   assert(sizeof(SCIP_Real) == sizeof(double)); /* CPLEX only works with doubles as floating points */
   assert(sizeof(SCIP_Bool) == sizeof(int));    /* CPLEX only works with ints as bools */
   assert(lpi != NULL);
   assert(numlp >= 0);

   SCIPdebugMessage("SCIPlpiCreate()\n");

   /* create environment */
   if( cpxenv == NULL )
   {
      assert(numlp == 0);
      cpxenv = CPXopenCPLEX(&restat);
      CHECK_ZERO( restat );

#if 0 /* turning presolve off seems to be faster than turning it off on demand (if presolve detects infeasibility) */
      /* turn presolve off, s.t. for an infeasible problem, a ray is always available */
      CHECK_ZERO( CPXsetintparam(cpxenv, CPX_PARAM_PREIND, CPX_OFF) );
#endif

      /* get default parameter values */
      SCIP_CALL( getParameterValues(&defparam) );
      copyParameterValues(&curparam, &defparam);
   }
   assert(cpxenv != NULL);

   /* create SCIP_LP */
   SCIP_ALLOC( BMSallocMemory(lpi) );
   (*lpi)->larray = NULL;
   (*lpi)->uarray = NULL;
   (*lpi)->senarray = NULL;
   (*lpi)->rhsarray = NULL;
   (*lpi)->rngarray = NULL;
   (*lpi)->valarray = NULL;
   (*lpi)->rngindarray = NULL;
   (*lpi)->cstat = NULL;
   (*lpi)->rstat = NULL;
   (*lpi)->indarray = NULL;
   (*lpi)->boundchgsize = 0;
   (*lpi)->sidechgsize = 0;
   (*lpi)->valsize = 0;
   (*lpi)->cstatsize = 0;
   (*lpi)->rstatsize = 0;
   (*lpi)->iterations = 0;
   (*lpi)->solisbasic = TRUE;
   (*lpi)->cpxlp = CPXcreateprob(cpxenv, &restat, name);
   CHECK_ZERO( restat );
   invalidateSolution(*lpi);
   copyParameterValues(&((*lpi)->cpxparam), &defparam);
   numlp++;

   /* set objective sense */
   SCIP_CALL( SCIPlpiChgObjsen(*lpi, objsen) );

   return SCIP_OKAY;
}

/** deletes an SCIP_LP problem object */
SCIP_RETCODE SCIPlpiFree(
   SCIP_LPI**            lpi                 /**< pointer to an SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(*lpi != NULL);

   SCIPdebugMessage("SCIPlpiFree()\n");

   /* free SCIP_LP */
   CHECK_ZERO( CPXfreeprob(cpxenv, &((*lpi)->cpxlp)) );

   /* free memory */
   BMSfreeMemoryArrayNull(&(*lpi)->larray);
   BMSfreeMemoryArrayNull(&(*lpi)->uarray);
   BMSfreeMemoryArrayNull(&(*lpi)->senarray);
   BMSfreeMemoryArrayNull(&(*lpi)->rhsarray);
   BMSfreeMemoryArrayNull(&(*lpi)->rngarray);
   BMSfreeMemoryArrayNull(&(*lpi)->rngindarray);
   BMSfreeMemoryArrayNull(&(*lpi)->cstat);
   BMSfreeMemoryArrayNull(&(*lpi)->rstat);
   BMSfreeMemory(lpi);

   /* free environment */
   numlp--;
   if( numlp == 0 )
   {
      CHECK_ZERO( CPXcloseCPLEX(&cpxenv) );
   }

   return SCIP_OKAY;
}

/**@} */




/*
 * Modification Methods
 */

/**@name Modification Methods */
/**@{ */

/** copies SCIP_LP data with column matrix into SCIP_LP solver */
SCIP_RETCODE SCIPlpiLoadColLP(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_OBJSEN           objsen,             /**< objective sense */
   int                   ncols,              /**< number of columns */
   const SCIP_Real*      obj,                /**< objective function values of columns */
   const SCIP_Real*      lb,                 /**< lower bounds of columns */
   const SCIP_Real*      ub,                 /**< upper bounds of columns */
   char**                colnames,           /**< column names, or NULL */
   int                   nrows,              /**< number of rows */
   const SCIP_Real*      lhs,                /**< left hand sides of rows */
   const SCIP_Real*      rhs,                /**< right hand sides of rows */
   char**                rownames,           /**< row names, or NULL */
   int                   nnonz,              /**< number of nonzero elements in the constraint matrix */
   const int*            beg,                /**< start index of each column in ind- and val-array */
   const int*            ind,                /**< row indices of constraint matrix entries */
   const SCIP_Real*      val                 /**< values of constraint matrix entries */
   )
{
   int* cnt;
   int rngcount;
   int c;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("loading SCIP_LP in column format into CPLEX: %d cols, %d rows\n", ncols, nrows);

   invalidateSolution(lpi);

   SCIP_CALL( ensureSidechgMem(lpi, nrows) );

   /* convert lhs/rhs into sen/rhs/range tuples */
   convertSides(lpi, nrows, lhs, rhs, 0, &rngcount);

   /* calculate column lengths */
   SCIP_ALLOC( BMSallocMemoryArray(&cnt, ncols) );
   for( c = 0; c < ncols-1; ++c )
   {
      cnt[c] = beg[c+1] - beg[c];
      assert(cnt[c] >= 0);
   }
   cnt[ncols-1] = nnonz - beg[ncols-1];
   assert(cnt[ncols-1] >= 0);

   /* copy data into CPLEX */
   CHECK_ZERO( CPXcopylpwnames(cpxenv, lpi->cpxlp, ncols, nrows, cpxObjsen(objsen), obj, 
                  lpi->rhsarray, lpi->senarray, beg, cnt, ind, val, lb, ub, lpi->rngarray, colnames, rownames) );

   /* free temporary memory */
   BMSfreeMemoryArray(&cnt);

   assert(CPXgetnumcols(cpxenv, lpi->cpxlp) == ncols);
   assert(CPXgetnumrows(cpxenv, lpi->cpxlp) == nrows);
   assert(CPXgetnumnz(cpxenv, lpi->cpxlp) == nnonz);

   return SCIP_OKAY;
}

/** adds columns to the SCIP_LP */
SCIP_RETCODE SCIPlpiAddCols(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   ncols,              /**< number of columns to be added */
   const SCIP_Real*      obj,                /**< objective function values of new columns */
   const SCIP_Real*      lb,                 /**< lower bounds of new columns */
   const SCIP_Real*      ub,                 /**< upper bounds of new columns */
   char**                colnames,           /**< column names, or NULL */
   int                   nnonz,              /**< number of nonzero elements to be added to the constraint matrix */
   const int*            beg,                /**< start index of each column in ind- and val-array, or NULL if nnonz == 0 */
   const int*            ind,                /**< row indices of constraint matrix entries, or NULL if nnonz == 0 */
   const SCIP_Real*      val                 /**< values of constraint matrix entries, or NULL if nnonz == 0 */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("adding %d columns with %d nonzeros to CPLEX\n", ncols, nnonz);

   invalidateSolution(lpi);

   if( nnonz > 0 )
   {
      CHECK_ZERO( CPXaddcols(cpxenv, lpi->cpxlp, ncols, nnonz, obj, beg, ind, val, lb, ub, colnames) );
   }
   else
   {
      CHECK_ZERO( CPXnewcols(cpxenv, lpi->cpxlp, ncols, obj, lb, ub, NULL, colnames) );
   }

   return SCIP_OKAY;
}

/** deletes all columns in the given range from SCIP_LP */
SCIP_RETCODE SCIPlpiDelCols(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   firstcol,           /**< first column to be deleted */
   int                   lastcol             /**< last column to be deleted */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(0 <= firstcol && firstcol <= lastcol && lastcol < CPXgetnumcols(cpxenv, lpi->cpxlp));

   SCIPdebugMessage("deleting %d columns from CPLEX\n", lastcol - firstcol + 1);

   invalidateSolution(lpi);

   CHECK_ZERO( CPXdelcols(cpxenv, lpi->cpxlp, firstcol, lastcol) );

   return SCIP_OKAY;   
}

/** deletes columns from SCIP_LP; the new position of a column must not be greater that its old position */
SCIP_RETCODE SCIPlpiDelColset(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  dstat               /**< deletion status of columns
                                              *   input:  1 if column should be deleted, 0 if not
                                              *   output: new position of column, -1 if column was deleted */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("deleting a column set from CPLEX\n");

   invalidateSolution(lpi);

   CHECK_ZERO( CPXdelsetcols(cpxenv, lpi->cpxlp, dstat) );

   return SCIP_OKAY;   
}

/** adds rows to the SCIP_LP */
SCIP_RETCODE SCIPlpiAddRows(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   nrows,              /**< number of rows to be added */
   const SCIP_Real*      lhs,                /**< left hand sides of new rows */
   const SCIP_Real*      rhs,                /**< right hand sides of new rows */
   char**                rownames,           /**< row names, or NULL */
   int                   nnonz,              /**< number of nonzero elements to be added to the constraint matrix */
   const int*            beg,                /**< start index of each row in ind- and val-array, or NULL if nnonz == 0 */
   const int*            ind,                /**< column indices of constraint matrix entries, or NULL if nnonz == 0 */
   const SCIP_Real*      val                 /**< values of constraint matrix entries, or NULL if nnonz == 0 */
   )
{
   int rngcount;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("adding %d rows with %d nonzeros to CPLEX\n", nrows, nnonz);

   invalidateSolution(lpi);

   SCIP_CALL( ensureSidechgMem(lpi, nrows) );

   /* convert lhs/rhs into sen/rhs/range tuples */
   convertSides(lpi, nrows, lhs, rhs, CPXgetnumrows(cpxenv, lpi->cpxlp), &rngcount);

   /* add rows to SCIP_LP */
   if( nnonz > 0 )
   {
      CHECK_ZERO( CPXaddrows(cpxenv, lpi->cpxlp, 0, nrows, nnonz, lpi->rhsarray, lpi->senarray, beg, ind, val, NULL,
            rownames) );
   }
   else
   {
      CHECK_ZERO( CPXnewrows(cpxenv, lpi->cpxlp, nrows, lpi->rhsarray, lpi->senarray, NULL, rownames) );
   }
   if( rngcount > 0 )
   {
      CHECK_ZERO( CPXchgrngval(cpxenv, lpi->cpxlp, rngcount, lpi->rngindarray, lpi->rngarray) );
   }

   return SCIP_OKAY;
}

/** deletes all rows in the given range from SCIP_LP */
SCIP_RETCODE SCIPlpiDelRows(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   firstrow,           /**< first row to be deleted */
   int                   lastrow             /**< last row to be deleted */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(0 <= firstrow && firstrow <= lastrow && lastrow < CPXgetnumrows(cpxenv, lpi->cpxlp));

   SCIPdebugMessage("deleting %d rows from CPLEX\n", lastrow - firstrow + 1);

   invalidateSolution(lpi);

   CHECK_ZERO( CPXdelrows(cpxenv, lpi->cpxlp, firstrow, lastrow) );

   return SCIP_OKAY;   
}

/** deletes rows from SCIP_LP; the new position of a row must not be greater that its old position */
SCIP_RETCODE SCIPlpiDelRowset(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  dstat               /**< deletion status of rows
                                              *   input:  1 if row should be deleted, 0 if not
                                              *   output: new position of row, -1 if row was deleted */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("deleting a row set from CPLEX\n");

   invalidateSolution(lpi);

   CHECK_ZERO( CPXdelsetrows(cpxenv, lpi->cpxlp, dstat) );

   return SCIP_OKAY;   
}

/** clears the whole SCIP_LP */
SCIP_RETCODE SCIPlpiClear(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int ncols;
   int nrows;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("clearing CPLEX LP\n");

   invalidateSolution(lpi);

   ncols = CPXgetnumcols(cpxenv, lpi->cpxlp);
   nrows = CPXgetnumrows(cpxenv, lpi->cpxlp);
   if( ncols >= 1 )
   {
      CHECK_ZERO( CPXdelcols(cpxenv, lpi->cpxlp, 0, ncols-1) );
   }
   if( nrows >= 1 )
   {
      CHECK_ZERO( CPXdelrows(cpxenv, lpi->cpxlp, 0, nrows-1) );
   }

   return SCIP_OKAY;
}

/** changes lower and upper bounds of columns */
SCIP_RETCODE SCIPlpiChgBounds(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   ncols,              /**< number of columns to change bounds for */
   const int*            ind,                /**< column indices */
   const SCIP_Real*      lb,                 /**< values for the new lower bounds */
   const SCIP_Real*      ub                  /**< values for the new upper bounds */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("changing %d bounds in CPLEX\n", ncols);
#ifdef SCIP_DEBUG
   {
      int i;
      for( i = 0; i < ncols; ++i )
         SCIPdebugPrintf("  col %d: [%g,%g]\n", ind[i], lb[i], ub[i]);
   }
#endif

   invalidateSolution(lpi);

   SCIP_CALL( ensureBoundchgMem(lpi, ncols) );

   CHECK_ZERO( CPXchgbds(cpxenv, lpi->cpxlp, ncols, ind, lpi->larray, (SCIP_Real*)lb) );
   CHECK_ZERO( CPXchgbds(cpxenv, lpi->cpxlp, ncols, ind, lpi->uarray, (SCIP_Real*)ub) );

   return SCIP_OKAY;
}

/** changes left and right hand sides of rows */
SCIP_RETCODE SCIPlpiChgSides(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   nrows,              /**< number of rows to change sides for */
   const int*            ind,                /**< row indices */
   const SCIP_Real*      lhs,                /**< new values for left hand sides */
   const SCIP_Real*      rhs                 /**< new values for right hand sides */
   )
{
   int rngcount;
   int i;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("changing %d sides in CPLEX\n", nrows);

   invalidateSolution(lpi);

   SCIP_CALL( ensureSidechgMem(lpi, nrows) );

   /* convert lhs/rhs into sen/rhs/range tuples */
   convertSides(lpi, nrows, lhs, rhs, 0, &rngcount);

   /* change row sides */
   CHECK_ZERO( CPXchgsense(cpxenv, lpi->cpxlp, nrows, ind, lpi->senarray) );
   CHECK_ZERO( CPXchgrhs(cpxenv, lpi->cpxlp, nrows, ind, lpi->rhsarray) );
   if( rngcount > 0 )
   {
      /* adjust the range count indices to the correct row indices */
      for( i = 0; i < rngcount; ++i )
      {
         assert(0 <= lpi->rngindarray[i] && lpi->rngindarray[i] < nrows);
         assert(lpi->senarray[i] == 'R');
         lpi->rngindarray[i] = ind[lpi->rngindarray[i]];
      }

      /* change the range values in CPLEX */
      CHECK_ZERO( CPXchgrngval(cpxenv, lpi->cpxlp, rngcount, lpi->rngindarray, lpi->rngarray) );
   }

   return SCIP_OKAY;
}

/** changes a single coefficient */
SCIP_RETCODE SCIPlpiChgCoef(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   row,                /**< row number of coefficient to change */
   int                   col,                /**< column number of coefficient to change */
   SCIP_Real             newval              /**< new value of coefficient */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("changing coefficient row %d, column %d in CPLEX to %g\n", row, col, newval);

   invalidateSolution(lpi);

   CHECK_ZERO( CPXchgcoef(cpxenv, lpi->cpxlp, row, col, newval) );

   return SCIP_OKAY;
}

/** changes the objective sense */
SCIP_RETCODE SCIPlpiChgObjsen(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_OBJSEN           objsen              /**< new objective sense */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("changing objective sense in CPLEX to %d\n", objsen);

   invalidateSolution(lpi);
   
   CPXchgobjsen(cpxenv, lpi->cpxlp, cpxObjsen(objsen));

   return SCIP_OKAY;
}

/** changes objective values of columns in the SCIP_LP */
SCIP_RETCODE SCIPlpiChgObj(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   ncols,              /**< number of columns to change objective value for */
   int*                  ind,                /**< column indices to change objective value for */
   SCIP_Real*            obj                 /**< new objective values for columns */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("changing %d objective values in CPLEX\n", ncols);

   CHECK_ZERO( CPXchgobj(cpxenv, lpi->cpxlp, ncols, ind, obj) );

   return SCIP_OKAY;
}

/** multiplies a row with a non-zero scalar; for negative scalars, the row's sense is switched accordingly */
SCIP_RETCODE SCIPlpiScaleRow(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   row,                /**< row number to scale */
   SCIP_Real             scaleval            /**< scaling multiplier */
   )
{
   SCIP_Real lhs;
   SCIP_Real rhs;
   int nnonz;
   int beg;
   int i;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(scaleval != 0.0);

   SCIPdebugMessage("scaling row %d with factor %g in CPLEX\n", row, scaleval);

   invalidateSolution(lpi);

   SCIP_CALL( ensureValMem(lpi, CPXgetnumcols(cpxenv, lpi->cpxlp)) );

   /* get the row */
   SCIP_CALL( SCIPlpiGetRows(lpi, row, row, &lhs, &rhs, &nnonz, &beg, lpi->indarray, lpi->valarray) );

   /* scale row coefficients */
   for( i = 0; i < nnonz; ++i )
   {
      SCIP_CALL( SCIPlpiChgCoef(lpi, row, lpi->indarray[i], lpi->valarray[i] * scaleval) );
   }

   /* scale row sides */
   if( lhs > -CPX_INFBOUND )
      lhs *= scaleval;
   else if( scaleval < 0.0 )
      lhs = CPX_INFBOUND;
   if( rhs < CPX_INFBOUND )
      rhs *= scaleval;
   else if( scaleval < 0.0 )
      rhs = -CPX_INFBOUND;
   if( scaleval > 0.0 )
   {
      SCIP_CALL( SCIPlpiChgSides(lpi, 1, &row, &lhs, &rhs) );
   }
   else
   {
      SCIP_CALL( SCIPlpiChgSides(lpi, 1, &row, &rhs, &lhs) );
   }

   return SCIP_OKAY;
}

/** multiplies a column with a non-zero scalar; the objective value is multiplied with the scalar, and the bounds
 *  are divided by the scalar; for negative scalars, the column's bounds are switched
 */
SCIP_RETCODE SCIPlpiScaleCol(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   col,                /**< column number to scale */
   SCIP_Real             scaleval            /**< scaling multiplier */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;
   SCIP_Real obj;
   int nnonz;
   int beg;
   int i;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(scaleval != 0.0);

   SCIPdebugMessage("scaling column %d with factor %g in CPLEX\n", col, scaleval);

   invalidateSolution(lpi);

   SCIP_CALL( ensureValMem(lpi, CPXgetnumcols(cpxenv, lpi->cpxlp)) );

   /* get the column */
   SCIP_CALL( SCIPlpiGetCols(lpi, col, col, &lb, &ub, &nnonz, &beg, lpi->indarray, lpi->valarray) );

   /** get objective coefficient */
   SCIP_CALL( SCIPlpiGetObj(lpi, col, col, &obj) );

   /* scale column coefficients */
   for( i = 0; i < nnonz; ++i )
   {
      SCIP_CALL( SCIPlpiChgCoef(lpi, lpi->indarray[i], col, lpi->valarray[i] * scaleval) );
   }

   /* scale objective value */
   obj *= scaleval;
   SCIP_CALL( SCIPlpiChgObj(lpi, 1, &col, &obj) );

   /* scale column bounds */
   if( lb > -CPX_INFBOUND )
      lb /= scaleval;
   else if( scaleval < 0.0 )
      lb = CPX_INFBOUND;
   if( ub < CPX_INFBOUND )
      ub /= scaleval;
   else if( scaleval < 0.0 )
      ub = -CPX_INFBOUND;
   if( scaleval > 0.0 )
   {
      SCIP_CALL( SCIPlpiChgBounds(lpi, 1, &col, &lb, &ub) );
   }
   else
   {
      SCIP_CALL( SCIPlpiChgBounds(lpi, 1, &col, &ub, &lb) );
   }

   return SCIP_OKAY;
}

/**@} */




/*
 * Data Accessing Methods
 */

/**@name Data Accessing Methods */
/**@{ */

/** gets the number of rows in the SCIP_LP */
SCIP_RETCODE SCIPlpiGetNRows(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  nrows               /**< pointer to store the number of rows */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(nrows != NULL);

   SCIPdebugMessage("getting number of rows\n");

   *nrows = CPXgetnumrows(cpxenv, lpi->cpxlp);

   return SCIP_OKAY;
}

/** gets the number of columns in the SCIP_LP */
SCIP_RETCODE SCIPlpiGetNCols(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  ncols               /**< pointer to store the number of cols */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(ncols != NULL);

   SCIPdebugMessage("getting number of columns\n");

   *ncols = CPXgetnumcols(cpxenv, lpi->cpxlp);

   return SCIP_OKAY;
}

/** gets the number of nonzero elements in the SCIP_LP constraint matrix */
SCIP_RETCODE SCIPlpiGetNNonz(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  nnonz               /**< pointer to store the number of nonzeros */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(nnonz != NULL);

   SCIPdebugMessage("getting number of non-zeros\n");

   *nnonz = CPXgetnumnz(cpxenv, lpi->cpxlp);

   return SCIP_OKAY;
}

/** gets columns from SCIP_LP problem object; the arrays have to be large enough to store all values
 *  Either both, lb and ub, have to be NULL, or both have to be non-NULL,
 *  either nnonz, beg, ind, and val have to be NULL, or all of them have to be non-NULL.
 */
SCIP_RETCODE SCIPlpiGetCols(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   firstcol,           /**< first column to get from SCIP_LP */
   int                   lastcol,            /**< last column to get from SCIP_LP */
   SCIP_Real*            lb,                 /**< buffer to store the lower bound vector, or NULL */
   SCIP_Real*            ub,                 /**< buffer to store the upper bound vector, or NULL */
   int*                  nnonz,              /**< pointer to store the number of nonzero elements returned, or NULL */
   int*                  beg,                /**< buffer to store start index of each column in ind- and val-array, or NULL */
   int*                  ind,                /**< buffer to store column indices of constraint matrix entries, or NULL */
   SCIP_Real*            val                 /**< buffer to store values of constraint matrix entries, or NULL */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(0 <= firstcol && firstcol <= lastcol && lastcol < CPXgetnumcols(cpxenv, lpi->cpxlp));

   SCIPdebugMessage("getting columns %d to %d\n", firstcol, lastcol);

   if( lb != NULL )
   {
      assert(ub != NULL);

      CHECK_ZERO( CPXgetlb(cpxenv, lpi->cpxlp, lb, firstcol, lastcol) );
      CHECK_ZERO( CPXgetub(cpxenv, lpi->cpxlp, ub, firstcol, lastcol) );
   }
   else
      assert(ub == NULL);

   if( nnonz != NULL )
   {
      int surplus;

      assert(beg != NULL);
      assert(ind != NULL);
      assert(val != NULL);

      /* get matrix entries */
      CHECK_ZERO( CPXgetcols(cpxenv, lpi->cpxlp, nnonz, beg, ind, val, CPXgetnumnz(cpxenv, lpi->cpxlp), &surplus, 
                     firstcol, lastcol) );
      assert(surplus >= 0);
   }
   else
   {
      assert(beg == NULL);
      assert(ind == NULL);
      assert(val == NULL);
   }

   return SCIP_OKAY;
}

/** gets rows from SCIP_LP problem object; the arrays have to be large enough to store all values.
 *  Either both, lhs and rhs, have to be NULL, or both have to be non-NULL,
 *  either nnonz, beg, ind, and val have to be NULL, or all of them have to be non-NULL.
 */
SCIP_RETCODE SCIPlpiGetRows(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   firstrow,           /**< first row to get from SCIP_LP */
   int                   lastrow,            /**< last row to get from SCIP_LP */
   SCIP_Real*            lhs,                /**< buffer to store left hand side vector, or NULL */
   SCIP_Real*            rhs,                /**< buffer to store right hand side vector, or NULL */
   int*                  nnonz,              /**< pointer to store the number of nonzero elements returned, or NULL */
   int*                  beg,                /**< buffer to store start index of each row in ind- and val-array, or NULL */
   int*                  ind,                /**< buffer to store row indices of constraint matrix entries, or NULL */
   SCIP_Real*            val                 /**< buffer to store values of constraint matrix entries, or NULL */
   )
{
   int retcode;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(0 <= firstrow && firstrow <= lastrow && lastrow < CPXgetnumrows(cpxenv, lpi->cpxlp));

   SCIPdebugMessage("getting rows %d to %d\n", firstrow, lastrow);

   if( lhs != NULL || rhs != NULL )
   {
      /* get row sense, rhs, and ranges */
      SCIP_CALL( ensureSidechgMem(lpi, lastrow - firstrow + 1) );
      CHECK_ZERO( CPXgetsense(cpxenv, lpi->cpxlp, lpi->senarray, firstrow, lastrow) );
      CHECK_ZERO( CPXgetrhs(cpxenv, lpi->cpxlp, lpi->rhsarray, firstrow, lastrow) );
      retcode = CPXgetrngval(cpxenv, lpi->cpxlp, lpi->rngarray, firstrow, lastrow);
      if( retcode != CPXERR_NO_RNGVAL ) /* ignore "No range values" error */
      {
         CHECK_ZERO( retcode );
      }
      else
         BMSclearMemoryArray(lpi->rngarray, lastrow-firstrow+1);

      /* convert sen/rhs/range into lhs/rhs tuples */
      reconvertSides(lpi, lastrow - firstrow + 1, lhs, rhs);
   }

   if( nnonz != NULL )
   {
      int surplus;

      assert(beg != NULL);
      assert(ind != NULL);
      assert(val != NULL);

      /* get matrix entries */
      CHECK_ZERO( CPXgetrows(cpxenv, lpi->cpxlp, nnonz, beg, ind, val, CPXgetnumnz(cpxenv, lpi->cpxlp), &surplus, 
                     firstrow, lastrow) );
      assert(surplus >= 0);
   }
   else
   {
      assert(beg == NULL);
      assert(ind == NULL);
      assert(val == NULL);
   }

   return SCIP_OKAY;
}

/** gets objective coefficients from SCIP_LP problem object */
SCIP_RETCODE SCIPlpiGetObj(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   firstcol,           /**< first column to get objective coefficient for */
   int                   lastcol,            /**< last column to get objective coefficient for */
   SCIP_Real*            vals                /**< array to store objective coefficients */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(firstcol <= lastcol);
   assert(vals != NULL);
   
   SCIPdebugMessage("getting objective values %d to %d\n", firstcol, lastcol);

   CHECK_ZERO( CPXgetobj(cpxenv, lpi->cpxlp, vals, firstcol, lastcol) );

   return SCIP_OKAY;
}

/** gets current bounds from SCIP_LP problem object */
SCIP_RETCODE SCIPlpiGetBounds(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   firstcol,           /**< first column to get bounds for */
   int                   lastcol,            /**< last column to get bounds for */
   SCIP_Real*            lbs,                /**< array to store lower bound values, or NULL */
   SCIP_Real*            ubs                 /**< array to store upper bound values, or NULL */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(firstcol <= lastcol);
   
   SCIPdebugMessage("getting bounds %d to %d\n", firstcol, lastcol);

   if( lbs != NULL )
   {
      CHECK_ZERO( CPXgetlb(cpxenv, lpi->cpxlp, lbs, firstcol, lastcol) );
   }

   if( ubs != NULL )
   {
      CHECK_ZERO( CPXgetub(cpxenv, lpi->cpxlp, ubs, firstcol, lastcol) );
   }

   return SCIP_OKAY;
}

/** gets current row sides from SCIP_LP problem object */
SCIP_RETCODE SCIPlpiGetSides(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   firstrow,           /**< first row to get sides for */
   int                   lastrow,            /**< last row to get sides for */
   SCIP_Real*            lhss,               /**< array to store left hand side values, or NULL */
   SCIP_Real*            rhss                /**< array to store right hand side values, or NULL */
   )
{
   int retval;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(firstrow <= lastrow);
   
   SCIPdebugMessage("getting row sides %d to %d\n", firstrow, lastrow);

   /* get row sense, rhs, and ranges */
   SCIP_CALL( ensureSidechgMem(lpi, lastrow - firstrow + 1) );
   CHECK_ZERO( CPXgetsense(cpxenv, lpi->cpxlp, lpi->senarray, firstrow, lastrow) );
   CHECK_ZERO( CPXgetrhs(cpxenv, lpi->cpxlp, lpi->rhsarray, firstrow, lastrow) );
   retval = CPXgetrngval(cpxenv, lpi->cpxlp, lpi->rngarray, firstrow, lastrow);
   if( retval != CPXERR_NO_RNGVAL ) /* ignore "No range values" error */
   {
      CHECK_ZERO( retval );
   }
   else
      BMSclearMemoryArray(lpi->rngarray, lastrow-firstrow+1);
   
   /* convert sen/rhs/range into lhs/rhs tuples */
   reconvertSides(lpi, lastrow - firstrow + 1, lhss, rhss);

   return SCIP_OKAY;
}

/** gets a single coefficient */
SCIP_RETCODE SCIPlpiGetCoef(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   row,                /**< row number of coefficient */
   int                   col,                /**< column number of coefficient */
   SCIP_Real*            val                 /**< pointer to store the value of the coefficient */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("getting coefficient of row %d col %d\n", row, col);

   CHECK_ZERO( CPXgetcoef(cpxenv, lpi->cpxlp, row, col, val) );

   return SCIP_OKAY;
}

/**@} */




/*
 * Solving Methods
 */

/**@name Solving Methods */
/**@{ */

/** calls primal simplex to solve the SCIP_LP */
SCIP_RETCODE SCIPlpiSolvePrimal(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int retval;
   int primalfeasible;
   int dualfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("calling CPLEX primal simplex: %d cols, %d rows\n",
      CPXgetnumcols(cpxenv, lpi->cpxlp), CPXgetnumrows(cpxenv, lpi->cpxlp));

   invalidateSolution(lpi);

   SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );

   SCIPdebugMessage("calling CPXprimopt()\n");
   retval = CPXprimopt(cpxenv, lpi->cpxlp);
   switch( retval  )
   {
   case 0:
      break;
   case CPXERR_NO_MEMORY:
      return SCIP_NOMEMORY;
   default:
      return SCIP_LPERROR;
   }

   lpi->iterations = CPXgetphase1cnt(cpxenv, lpi->cpxlp) + CPXgetitcnt(cpxenv, lpi->cpxlp);
   lpi->solisbasic = TRUE;
   lpi->solstat = CPXgetstat(cpxenv, lpi->cpxlp);
   CHECK_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &primalfeasible, &dualfeasible) );
   SCIPdebugMessage(" -> CPLEX returned solstat=%d, pfeas=%d, dfeas=%d (%d iterations)\n",
      lpi->solstat, primalfeasible, dualfeasible, lpi->iterations);

   if( lpi->solstat == CPX_STAT_INForUNBD
      || (lpi->solstat == CPX_STAT_INFEASIBLE && !dualfeasible)
      || (lpi->solstat == CPX_STAT_UNBOUNDED && !primalfeasible) )
   {
      if( getIntParam(lpi, CPX_PARAM_PREIND) == CPX_ON )
      {
         /* maybe the preprocessor solved the problem; but we need a solution, so solve again without preprocessing */
         SCIPdebugMessage("presolver may have solved the problem -> calling CPLEX primal simplex again without presolve\n");
         
         /* switch off preprocessing */
         setIntParam(lpi, CPX_PARAM_PREIND, CPX_OFF);
         SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );
         
         retval = CPXprimopt(cpxenv, lpi->cpxlp);
         switch( retval  )
         {
         case 0:
            break;
         case CPXERR_NO_MEMORY:
            return SCIP_NOMEMORY;
         default:
            return SCIP_LPERROR;
         }

         lpi->iterations += CPXgetphase1cnt(cpxenv, lpi->cpxlp) + CPXgetitcnt(cpxenv, lpi->cpxlp);
         lpi->solstat = CPXgetstat(cpxenv, lpi->cpxlp);
         SCIPdebugMessage(" -> CPLEX returned solstat=%d (%d iterations)\n", lpi->solstat, lpi->iterations);

         /* switch on preprocessing again */
         setIntParam(lpi, CPX_PARAM_PREIND, CPX_ON);
      }

      if( lpi->solstat == CPX_STAT_INForUNBD )
      {
         /* preprocessing was not the problem; issue a warning message and treat SCIP_LP as infeasible */
         SCIPerrorMessage("CPLEX primal simplex returned CPX_STAT_INForUNBD after presolving was turned off\n");
      }
   }

   return SCIP_OKAY;
}

/** calls dual simplex to solve the SCIP_LP */
SCIP_RETCODE SCIPlpiSolveDual(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int retval;
   int primalfeasible;
   int dualfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("calling CPLEX dual simplex: %d cols, %d rows\n", 
      CPXgetnumcols(cpxenv, lpi->cpxlp), CPXgetnumrows(cpxenv, lpi->cpxlp));

   invalidateSolution(lpi);

   SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );

   SCIPdebugMessage("calling CPXdualopt()\n");
   retval = CPXdualopt(cpxenv, lpi->cpxlp);
   switch( retval  )
   {
   case 0:
      break;
   case CPXERR_NO_MEMORY:
      return SCIP_NOMEMORY;
   default:
      return SCIP_LPERROR;
   }

   lpi->iterations = CPXgetphase1cnt(cpxenv, lpi->cpxlp) + CPXgetitcnt(cpxenv, lpi->cpxlp);
   lpi->solisbasic = TRUE;
   lpi->solstat = CPXgetstat(cpxenv, lpi->cpxlp);
   CHECK_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &primalfeasible, &dualfeasible) );
   SCIPdebugMessage(" -> CPLEX returned solstat=%d, pfeas=%d, dfeas=%d (%d iterations)\n",
      lpi->solstat, primalfeasible, dualfeasible, lpi->iterations);

   if( lpi->solstat == CPX_STAT_INForUNBD
      || (lpi->solstat == CPX_STAT_INFEASIBLE && !dualfeasible)
      || (lpi->solstat == CPX_STAT_UNBOUNDED && !primalfeasible) )
   {
      if( getIntParam(lpi, CPX_PARAM_PREIND) == CPX_ON )
      {
         /* maybe the preprocessor solved the problem; but we need a solution, so solve again without preprocessing */
         SCIPdebugMessage("presolver may have solved the problem -> calling CPLEX dual simplex again without presolve\n");
         
         /* switch off preprocessing */
         setIntParam(lpi, CPX_PARAM_PREIND, CPX_OFF);
         SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );
         
         retval = CPXdualopt(cpxenv, lpi->cpxlp);
         switch( retval  )
         {
         case 0:
            break;
         case CPXERR_NO_MEMORY:
            return SCIP_NOMEMORY;
         default:
            return SCIP_LPERROR;
         }

         lpi->iterations += CPXgetphase1cnt(cpxenv, lpi->cpxlp) + CPXgetitcnt(cpxenv, lpi->cpxlp);
         lpi->solstat = CPXgetstat(cpxenv, lpi->cpxlp);
         CHECK_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &primalfeasible, &dualfeasible) );
         SCIPdebugMessage(" -> CPLEX returned solstat=%d (%d iterations)\n", lpi->solstat, lpi->iterations);

         /* switch on preprocessing again */
         setIntParam(lpi, CPX_PARAM_PREIND, CPX_ON);
      }

      if( lpi->solstat == CPX_STAT_INForUNBD )
      {
         /* preprocessing was not the problem; issue a warning message and treat SCIP_LP as infeasible */
         SCIPerrorMessage("CPLEX dual simplex returned CPX_STAT_INForUNBD after presolving was turned off\n");
      }
   }

#if 0
   /* this fixes the strange behaviour of CPLEX, that in case of the objective limit exceedance, it returns the
    * solution for the basis preceeding the one with exceeding objective limit
    * (using this "wrong" dual solution can cause column generation algorithms to fail to find an improving column)
    */
   if( SCIPlpiIsObjlimExc(lpi) )
   {
      SCIP_Real objval;
      SCIP_Real llim;
      SCIP_Real ulim;
      SCIP_Real eps;

      /* check, if the dual solution returned by CPLEX really exceeds the objective limit;
       * CPLEX usually returns the basis one iteration before the one that exceeds the limit
       */
      SCIP_CALL( SCIPlpiGetObjval(lpi, &objval) );
      llim = getDblParam(lpi, CPX_PARAM_OBJLLIM);
      ulim = getDblParam(lpi, CPX_PARAM_OBJULIM);
      eps = getDblParam(lpi, CPX_PARAM_EPOPT);
      if( objval >= llim - eps && objval <= ulim + eps )
      {
         int itlim;
         int advind;

         /* perform one additional simplex iteration without objective limit */
         SCIPdebugMessage("dual solution %g does not exceed objective limit [%g,%g] (%d iterations) -> calling CPLEX dual simplex again for one iteration\n",
            objval, llim, ulim, lpi->iterations);
         itlim = getIntParam(lpi, CPX_PARAM_ITLIM);
         setIntParam(lpi, CPX_PARAM_ITLIM, 1);
         advind = getIntParam(lpi, CPX_PARAM_ADVIND);
         setIntParam(lpi, CPX_PARAM_ADVIND, CPX_ON);
         setDblParam(lpi, CPX_PARAM_OBJLLIM, -CPX_INFBOUND);
         setDblParam(lpi, CPX_PARAM_OBJULIM, CPX_INFBOUND);
         SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );
         CHECK_ZERO( CPXsetintparam(cpxenv, CPX_PARAM_FINALFACTOR, FALSE) );
         
         retval = CPXdualopt(cpxenv, lpi->cpxlp);
         switch( retval  )
         {
         case 0:
            break;
         case CPXERR_NO_MEMORY:
            return SCIP_NOMEMORY;
         default:
            return SCIP_LPERROR;
         }

         lpi->iterations += CPXgetphase1cnt(cpxenv, lpi->cpxlp) + CPXgetitcnt(cpxenv, lpi->cpxlp);

         /* reset the iteration limit and objective bounds */
         setIntParam(lpi, CPX_PARAM_ITLIM, itlim);
         setIntParam(lpi, CPX_PARAM_ADVIND, advind);
         setDblParam(lpi, CPX_PARAM_OBJLLIM, llim);
         setDblParam(lpi, CPX_PARAM_OBJULIM, ulim);
         SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );
         CHECK_ZERO( CPXsetintparam(cpxenv, CPX_PARAM_FINALFACTOR, TRUE) );
         
         /* resolve SCIP_LP again in order to restore the status of exceeded objective limit */
         retval = CPXdualopt(cpxenv, lpi->cpxlp);
         switch( retval  )
         {
         case 0:
            break;
         case CPXERR_NO_MEMORY:
            return SCIP_NOMEMORY;
         default:
            return SCIP_LPERROR;
         }

         lpi->iterations += CPXgetphase1cnt(cpxenv, lpi->cpxlp) + CPXgetitcnt(cpxenv, lpi->cpxlp);
         lpi->solstat = CPXgetstat(cpxenv, lpi->cpxlp);
         SCIPdebugMessage(" -> CPLEX returned solstat=%d (%d iterations)\n", lpi->solstat, lpi->iterations);
      }
   }
#endif

   return SCIP_OKAY;
}

/** calls barrier or interior point algorithm to solve the SCIP_LP with crossover to simplex basis */
SCIP_RETCODE SCIPlpiSolveBarrier(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Bool             crossover            /**< perform crossover */
   )
{
   int retval;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("calling CPLEX barrier: %d cols, %d rows\n",
      CPXgetnumcols(cpxenv, lpi->cpxlp), CPXgetnumrows(cpxenv, lpi->cpxlp));

   invalidateSolution(lpi);

   SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );

   SCIPdebugMessage("calling CPXhybaropt()\n");
   retval = CPXhybbaropt(cpxenv, lpi->cpxlp, crossover ? 0 : CPX_ALG_NONE);
   switch( retval  )
   {
   case 0:
      break;
   case CPXERR_NO_MEMORY:
      return SCIP_NOMEMORY;
   default:
      return SCIP_LPERROR;
   }

   lpi->iterations = CPXgetbaritcnt(cpxenv, lpi->cpxlp);
   lpi->solisbasic = crossover;
   lpi->solstat = CPXgetstat(cpxenv, lpi->cpxlp);
   SCIPdebugMessage(" -> CPLEX returned solstat=%d (%d iterations)\n", lpi->solstat, lpi->iterations);

   if( lpi->solstat == CPX_STAT_INForUNBD )
   {
      /* maybe the preprocessor solved the problem; but we need a solution, so solve again without preprocessing */
      SCIPdebugMessage("CPLEX returned INForUNBD -> calling CPLEX barrier again without presolve\n");
      
      /* switch off preprocessing */
      setIntParam(lpi, CPX_PARAM_PREIND, CPX_OFF);
      SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );

      retval = CPXhybbaropt(cpxenv, lpi->cpxlp, crossover ? 0 : CPX_ALG_NONE);
      switch( retval  )
      {
      case 0:
         break;
      case CPXERR_NO_MEMORY:
         return SCIP_NOMEMORY;
      default:
         return SCIP_LPERROR;
      }

      lpi->iterations += CPXgetbaritcnt(cpxenv, lpi->cpxlp);
      lpi->solstat = CPXgetstat(cpxenv, lpi->cpxlp);
      SCIPdebugMessage(" -> CPLEX returned solstat=%d\n", lpi->solstat);

      if( lpi->solstat == CPX_STAT_INForUNBD )
      {
         /* preprocessing was not the problem; issue a warning message and treat SCIP_LP as infeasible */
         SCIPerrorMessage("CPLEX barrier returned CPX_STAT_INForUNBD after presolving was turned off\n");
      }

      setIntParam(lpi, CPX_PARAM_PREIND, CPX_ON);
   }

   return SCIP_OKAY;
}

/** performs strong branching iterations on all candidates */
SCIP_RETCODE SCIPlpiStrongbranch(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   col,                /**< column to apply strong branching on */
   SCIP_Real             psol,               /**< current primal solution value of column */
   int                   itlim,              /**< iteration limit for strong branchings */
   SCIP_Real*            down,               /**< stores dual bound after branching column down */
   SCIP_Real*            up,                 /**< stores dual bound after branching column up */
   SCIP_Bool*            downvalid,          /**< stores whether the returned down value is a valid dual bound;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Bool*            upvalid,            /**< stores whether the returned up value is a valid dual bound;
                                              *   otherwise, it can only be used as an estimate value */
   int*                  iter                /**< stores total number of strong branching iterations, or -1; may be NULL */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(down != NULL);
   assert(up != NULL);
   assert(downvalid != NULL);
   assert(upvalid != NULL);

   SCIPdebugMessage("calling CPLEX strongbranching on variable %d (%d iterations)\n", col, itlim);

   /* results of CPLEX are valid in any case */
   *downvalid = TRUE;
   *upvalid = TRUE;

   SCIP_CALL( setParameterValues(&(lpi->cpxparam)) );

   /* if the solution value is integral, we have to apply strong branching manually; otherwise, the CPLEX
    * method CPXstrongbranch() is applicable
    */
   if( EPSISINT(psol, 1e-06) )
   {
      const char lbound = 'L';
      const char ubound = 'U';
      SCIP_Real oldlb;
      SCIP_Real oldub;
      SCIP_Real newlb;
      SCIP_Real newub;
      int objsen;
      int olditlim;
      int it;

      SCIPdebugMessage(" -> strong branching on integral variable\n");

      if( iter != NULL )
         *iter = 0;

      objsen = CPXgetobjsen(cpxenv, lpi->cpxlp);

      /* save current SCIP_LP basis and bounds*/
      SCIP_CALL( getBase(lpi) );
      CHECK_ZERO( CPXgetlb(cpxenv, lpi->cpxlp, &oldlb, col, col) );
      CHECK_ZERO( CPXgetub(cpxenv, lpi->cpxlp, &oldub, col, col) );

      /* save old iteration limit and set iteration limit to strong branching limit */
      if( itlim > CPX_INT_MAX )
         itlim = CPX_INT_MAX;
      olditlim = getIntParam(lpi, CPX_PARAM_ITLIM);
      setIntParam(lpi, CPX_PARAM_ITLIM, itlim);
      
      /* down branch */
      newub = EPSCEIL(psol-1.0, 1e-06);
      if( newub >= oldlb - 0.5 )
      {
         CHECK_ZERO( CPXchgbds(cpxenv, lpi->cpxlp, 1, &col, &ubound, &newub) );
         SCIP_CALL( SCIPlpiSolveDual(lpi) );
         if( SCIPlpiIsPrimalInfeasible(lpi) || SCIPlpiIsObjlimExc(lpi) )
            *down = objsen == CPX_MIN ? getDblParam(lpi, CPX_PARAM_OBJULIM) : getDblParam(lpi, CPX_PARAM_OBJLLIM);
         else if( SCIPlpiIsOptimal(lpi) || SCIPlpiIsIterlimExc(lpi) )
         {
            SCIP_CALL( SCIPlpiGetObjval(lpi, down) );
         }
         else
            *down = objsen == CPX_MIN ? getDblParam(lpi, CPX_PARAM_OBJLLIM) : getDblParam(lpi, CPX_PARAM_OBJULIM);
         if( iter != NULL )
         {
            SCIP_CALL( SCIPlpiGetIterations(lpi, &it) );
            *iter += it;
         }
         SCIPdebugMessage(" -> down (x%d <= %g): %g\n", col, newub, *down);

         CHECK_ZERO( CPXchgbds(cpxenv, lpi->cpxlp, 1, &col, &ubound, &oldub) );
         SCIP_CALL( setBase(lpi) );
      }
      else
         *down = objsen == CPX_MIN ? getDblParam(lpi, CPX_PARAM_OBJULIM) : getDblParam(lpi, CPX_PARAM_OBJLLIM);

      /* up branch */
      newlb = EPSFLOOR(psol+1.0, 1e-06);
      if( newlb <= oldub + 0.5 )
      {
         CHECK_ZERO( CPXchgbds(cpxenv, lpi->cpxlp, 1, &col, &lbound, &newlb) );
         SCIP_CALL( SCIPlpiSolveDual(lpi) );
         if( SCIPlpiIsPrimalInfeasible(lpi) || SCIPlpiIsObjlimExc(lpi) )
            *up = objsen == CPX_MIN ? getDblParam(lpi, CPX_PARAM_OBJULIM) : getDblParam(lpi, CPX_PARAM_OBJLLIM);
         else if( SCIPlpiIsOptimal(lpi) || SCIPlpiIsIterlimExc(lpi) )
         {
            SCIP_CALL( SCIPlpiGetObjval(lpi, up) );
         }
         else
            *up = objsen == CPX_MIN ? getDblParam(lpi, CPX_PARAM_OBJLLIM) : getDblParam(lpi, CPX_PARAM_OBJULIM);
         if( iter != NULL )
         {
            SCIP_CALL( SCIPlpiGetIterations(lpi, &it) );
            *iter += it;
         }
         SCIPdebugMessage(" -> up  (x%d >= %g): %g\n", col, newlb, *up);

         CHECK_ZERO( CPXchgbds(cpxenv, lpi->cpxlp, 1, &col, &lbound, &oldlb) );
         SCIP_CALL( setBase(lpi) );
      }
      else
         *up = objsen == CPX_MIN ? getDblParam(lpi, CPX_PARAM_OBJLLIM) : getDblParam(lpi, CPX_PARAM_OBJULIM);

      /* reset iteration limit */
      setIntParam(lpi, CPX_PARAM_ITLIM, olditlim);
   }
   else
   {
      CHECK_ZERO( CPXstrongbranch(cpxenv, lpi->cpxlp, &col, 1, down, up, itlim) );
      SCIPdebugMessage(" -> down: %g, up:%g\n", *down, *up);

      /* CPLEX is not able to return the iteration counts in strong branching */
      if( iter != NULL )
         *iter = -1;
   }

   return SCIP_OKAY;
}

/**@} */




/*
 * Solution Information Methods
 */

/**@name Solution Information Methods */
/**@{ */

/** gets information about primal and dual feasibility of the current SCIP_LP solution */
SCIP_RETCODE SCIPlpiGetSolFeasibility(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Bool*            primalfeasible,     /**< stores primal feasibility status */
   SCIP_Bool*            dualfeasible        /**< stores dual feasibility status */
   )
{
   int pfeas;
   int dfeas;

   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(primalfeasible != NULL);
   assert(dualfeasible != NULL);

   SCIPdebugMessage("getting solution feasibility\n");

   CHECK_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &pfeas, &dfeas) );
   *primalfeasible = (SCIP_Bool)pfeas;
   *dualfeasible = (SCIP_Bool)dfeas;

   return SCIP_OKAY;
}

/** returns TRUE iff SCIP_LP is proven to have a primal unbounded ray (but not necessary a primal feasible point);
 *  this does not necessarily mean, that the solver knows and can return the primal ray
 */
SCIP_Bool SCIPlpiExistsPrimalRay(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_UNBOUNDED || lpi->solstat == CPX_STAT_OPTIMAL_FACE_UNBOUNDED);
}

/** returns TRUE iff SCIP_LP is proven to have a primal unbounded ray (but not necessary a primal feasible point),
 *  and the solver knows and can return the primal ray
 */
SCIP_Bool SCIPlpiHasPrimalRay(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_UNBOUNDED && CPXgetmethod(cpxenv, lpi->cpxlp) == CPX_ALG_PRIMAL);
}

/** returns TRUE iff SCIP_LP is proven to be primal unbounded */
SCIP_Bool SCIPlpiIsPrimalUnbounded(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int primalfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("checking for primal unboundness\n");

   ABORT_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &primalfeasible, NULL) );
   
   /* If the solution status of CPLEX is CPX_STAT_UNBOUNDED, it only means, there is an unbounded ray,
    * but not necessarily a feasible primal solution. If primalfeasible == FALSE, we cannot conclude,
    * that the problem is unbounded
    */
   return ((primalfeasible && (lpi->solstat == CPX_STAT_UNBOUNDED || lpi->solstat == CPX_STAT_INForUNBD))
      || lpi->solstat == CPX_STAT_OPTIMAL_FACE_UNBOUNDED);
}

/** returns TRUE iff SCIP_LP is proven to be primal infeasible */
SCIP_Bool SCIPlpiIsPrimalInfeasible(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int dualfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("checking for primal infeasibility\n");

   ABORT_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, NULL, &dualfeasible) );

   return (lpi->solstat == CPX_STAT_INFEASIBLE || (lpi->solstat == CPX_STAT_INForUNBD && dualfeasible));
}

/** returns TRUE iff SCIP_LP is proven to be primal feasible */
SCIP_Bool SCIPlpiIsPrimalFeasible(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int primalfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("checking for primal feasibility\n");

   ABORT_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &primalfeasible, NULL) );
   
   return (SCIP_Bool)primalfeasible;
}

/** returns TRUE iff SCIP_LP is proven to have a dual unbounded ray (but not necessary a dual feasible point);
 *  this does not necessarily mean, that the solver knows and can return the dual ray
 */
SCIP_Bool SCIPlpiExistsDualRay(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_INFEASIBLE);
}

/** returns TRUE iff SCIP_LP is proven to have a dual unbounded ray (but not necessary a dual feasible point),
 *  and the solver knows and can return the dual ray
 */
SCIP_Bool SCIPlpiHasDualRay(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_INFEASIBLE && CPXgetmethod(cpxenv, lpi->cpxlp) == CPX_ALG_DUAL);
}

/** returns TRUE iff SCIP_LP is proven to be dual unbounded */
SCIP_Bool SCIPlpiIsDualUnbounded(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int dualfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("checking for dual unboundness\n");

   ABORT_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, NULL, &dualfeasible) );

   return (dualfeasible && (lpi->solstat == CPX_STAT_INFEASIBLE || lpi->solstat == CPX_STAT_INForUNBD));
}

/** returns TRUE iff SCIP_LP is proven to be dual infeasible */
SCIP_Bool SCIPlpiIsDualInfeasible(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int primalfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("checking for dual infeasibility\n");

   ABORT_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &primalfeasible, NULL) );

   return (lpi->solstat == CPX_STAT_UNBOUNDED
      || lpi->solstat == CPX_STAT_OPTIMAL_FACE_UNBOUNDED
      || (lpi->solstat == CPX_STAT_INForUNBD && primalfeasible));
}

/** returns TRUE iff SCIP_LP is proven to be dual feasible */
SCIP_Bool SCIPlpiIsDualFeasible(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   int dualfeasible;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("checking for dual feasibility\n");

   ABORT_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, NULL, &dualfeasible) );
   
   return (SCIP_Bool)dualfeasible;
}

/** returns TRUE iff SCIP_LP was solved to optimality */
SCIP_Bool SCIPlpiIsOptimal(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_OPTIMAL);
}

/** returns TRUE iff current SCIP_LP basis is stable */
SCIP_Bool SCIPlpiIsStable(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("checking for stability: CPLEX solstat = %d\n", lpi->solstat);

   /* If the solution status of CPLEX is CPX_STAT_UNBOUNDED, it only means, there is an unbounded ray,
    * but not necessarily a feasible primal solution. If primalfeasible == FALSE, we interpret this
    * result as instability, s.t. the problem is resolved from scratch
    */
   if( lpi->solstat == CPX_STAT_UNBOUNDED )
   {
      int primalfeasible;
      
      ABORT_ZERO( CPXsolninfo(cpxenv, lpi->cpxlp, NULL, NULL, &primalfeasible, NULL) );

      if( !primalfeasible )
         return FALSE;
   }

   return (lpi->solstat != CPX_STAT_NUM_BEST && lpi->solstat != CPX_STAT_OPTIMAL_INFEAS);
}

/** returns TRUE iff the objective limit was reached */
SCIP_Bool SCIPlpiIsObjlimExc(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_ABORT_OBJ_LIM
      || lpi->solstat == CPX_STAT_ABORT_DUAL_OBJ_LIM
      || lpi->solstat == CPX_STAT_ABORT_PRIM_OBJ_LIM);
}

/** returns TRUE iff the iteration limit was reached */
SCIP_Bool SCIPlpiIsIterlimExc(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_ABORT_IT_LIM);
}

/** returns TRUE iff the time limit was reached */
SCIP_Bool SCIPlpiIsTimelimExc(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   return (lpi->solstat == CPX_STAT_ABORT_TIME_LIM);
}

/** returns the internal solution status of the solver */
int SCIPlpiGetInternalStatus(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   return lpi->solstat;
}

/** tries to reset the internal status of the SCIP_LP solver in order to ignore an instability of the last solving call */
SCIP_RETCODE SCIPlpiIgnoreInstability(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Bool*            success             /**< pointer to store, whether the instability could be ignored */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(success != NULL);
   assert(lpi->solstat == CPX_STAT_UNBOUNDED
      || lpi->solstat == CPX_STAT_NUM_BEST
      || lpi->solstat == CPX_STAT_OPTIMAL_INFEAS);

   /* replace instable status with optimal status */
   if( lpi->solstat == CPX_STAT_NUM_BEST || lpi->solstat == CPX_STAT_OPTIMAL_INFEAS )
      lpi->solstat = CPX_STAT_OPTIMAL;

   *success = TRUE;

   return SCIP_OKAY;
}

/** gets objective value of solution */
SCIP_RETCODE SCIPlpiGetObjval(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Real*            objval              /**< stores the objective value */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("getting solution's objective value\n");

   CHECK_ZERO( CPXgetobjval(cpxenv, lpi->cpxlp, objval) );

   return SCIP_OKAY;
}

/** gets primal and dual solution vectors */
SCIP_RETCODE SCIPlpiGetSol(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Real*            objval,             /**< stores the objective value, may be NULL if not needed */
   SCIP_Real*            primsol,            /**< primal solution vector, may be NULL if not needed */
   SCIP_Real*            dualsol,            /**< dual solution vector, may be NULL if not needed */
   SCIP_Real*            activity,           /**< row activity vector, may be NULL if not needed */
   SCIP_Real*            redcost             /**< reduced cost vector, may be NULL if not needed */
   )
{
   int dummy;

   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("getting solution\n");

   CHECK_ZERO( CPXsolution(cpxenv, lpi->cpxlp, &dummy, objval, primsol, dualsol, NULL, redcost) );
   assert(dummy == lpi->solstat);

   if( activity != NULL )
   {
      CHECK_ZERO( CPXgetax(cpxenv, lpi->cpxlp, activity, 0, CPXgetnumrows(cpxenv, lpi->cpxlp)-1) );
   }

   return SCIP_OKAY;
}

/** gets primal ray for unbounded LPs */
SCIP_RETCODE SCIPlpiGetPrimalRay(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Real*            ray                 /**< primal ray */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);

   SCIPdebugMessage("calling CPLEX get primal ray: %d cols, %d rows\n",
      CPXgetnumcols(cpxenv, lpi->cpxlp), CPXgetnumrows(cpxenv, lpi->cpxlp));

   CHECK_ZERO( CPXgetray(cpxenv, lpi->cpxlp, ray) );

   return SCIP_OKAY;
}

/** gets dual farkas proof for infeasibility */
SCIP_RETCODE SCIPlpiGetDualfarkas(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Real*            dualfarkas          /**< dual farkas row multipliers */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);
   assert(dualfarkas != NULL);

   SCIPdebugMessage("calling CPLEX dual farkas: %d cols, %d rows\n",
      CPXgetnumcols(cpxenv, lpi->cpxlp), CPXgetnumrows(cpxenv, lpi->cpxlp));

   CHECK_ZERO( CPXdualfarkas(cpxenv, lpi->cpxlp, dualfarkas, NULL) );

   return SCIP_OKAY;
}

/** gets the number of SCIP_LP iterations of the last solve call */
SCIP_RETCODE SCIPlpiGetIterations(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  iterations          /**< pointer to store the number of iterations of the last solve call */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpi->solstat >= 0);
   assert(iterations != NULL);

   *iterations = lpi->iterations;

   return SCIP_OKAY;
}

/**@} */




/*
 * SCIP_LP Basis Methods
 */

/**@name SCIP_LP Basis Methods */
/**@{ */

/** gets current basis status for columns and rows; arrays must be large enough to store the basis status */
SCIP_RETCODE SCIPlpiGetBase(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  cstat,              /**< array to store column basis status, or NULL */
   int*                  rstat               /**< array to store row basis status, or NULL */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("saving CPLEX basis into %p/%p\n", cstat, rstat);

   CHECK_ZERO( CPXgetbase(cpxenv, lpi->cpxlp, cstat, rstat) );

   /* because the basis status values are equally defined in SCIP and CPLEX, they don't need to be transformed */
   assert((int)SCIP_BASESTAT_LOWER == CPX_AT_LOWER);
   assert((int)SCIP_BASESTAT_BASIC == CPX_BASIC);
   assert((int)SCIP_BASESTAT_UPPER == CPX_AT_UPPER);
   assert((int)SCIP_BASESTAT_ZERO == CPX_FREE_SUPER);

   return SCIP_OKAY;
}

/** sets current basis status for columns and rows */
SCIP_RETCODE SCIPlpiSetBase(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  cstat,              /**< array with column basis status */
   int*                  rstat               /**< array with row basis status */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(cstat != NULL);
   assert(rstat != NULL);

   SCIPdebugMessage("loading basis %p/%p into CPLEX\n", cstat, rstat);

   invalidateSolution(lpi);

   /* because the basis status values are equally defined in SCIP and CPLEX, they don't need to be transformed */
   assert((int)SCIP_BASESTAT_LOWER == CPX_AT_LOWER);
   assert((int)SCIP_BASESTAT_BASIC == CPX_BASIC);
   assert((int)SCIP_BASESTAT_UPPER == CPX_AT_UPPER);
   assert((int)SCIP_BASESTAT_ZERO == CPX_FREE_SUPER);

   CHECK_ZERO( CPXcopybase(cpxenv, lpi->cpxlp, cstat, rstat) );

   return SCIP_OKAY;
}

/** returns the indices of the basic columns and rows */
SCIP_RETCODE SCIPlpiGetBasisInd(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int*                  bind                /**< basic column n gives value n, basic row m gives value -1-m */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("getting basis information\n");

   CHECK_ZERO( CPXgetbhead(cpxenv, lpi->cpxlp, bind, NULL) );

   return SCIP_OKAY;
}

/** get dense row of inverse basis matrix B^-1 */
SCIP_RETCODE SCIPlpiGetBInvRow(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   r,                  /**< row number */
   SCIP_Real*            coef                /**< pointer to store the coefficients of the row */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("getting binv-row %d\n", r);

   CHECK_ZERO( CPXbinvrow(cpxenv, lpi->cpxlp, r, coef) );

   return SCIP_OKAY;
}

/** get dense row of inverse basis matrix times constraint matrix B^-1 * A */
SCIP_RETCODE SCIPlpiGetBInvARow(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   int                   r,                  /**< row number */
   const SCIP_Real*      binvrow,            /**< row in (A_B)^-1 from prior call to SCIPlpiGetBInvRow(), or NULL */
   SCIP_Real*            val                 /**< vector to return coefficients */
   )
{  /*lint --e{715}*/
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("getting binva-row %d\n", r);

   CHECK_ZERO( CPXbinvarow(cpxenv, lpi->cpxlp, r, val) );

   return SCIP_OKAY;
}

/**@} */




/*
 * SCIP_LP State Methods
 */

/**@name SCIP_LP State Methods */
/**@{ */

/** stores SCIP_LPi state (like basis information) into lpistate object */
SCIP_RETCODE SCIPlpiGetState(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   BMS_BLKMEM*           blkmem,             /**< block memory */
   SCIP_LPISTATE**       lpistate            /**< pointer to SCIP_LPi state information (like basis information) */
   )
{
   int ncols;
   int nrows;

   assert(blkmem != NULL);
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpistate != NULL);

   /* if there is no basis information available (e.g. after barrier without crossover), no state can be saved */
   if( !lpi->solisbasic )
   {
      *lpistate = NULL;
      return SCIP_OKAY;
   }

   ncols = CPXgetnumcols(cpxenv, lpi->cpxlp);
   nrows = CPXgetnumrows(cpxenv, lpi->cpxlp);
   assert(ncols >= 0);
   assert(nrows >= 0);
   
   /* allocate lpistate data */
   SCIP_CALL( lpistateCreate(lpistate, blkmem, ncols, nrows) );

   SCIPdebugMessage("storing CPLEX SCIP_LPI state in %p (%d cols, %d rows)\n", *lpistate, ncols, nrows);

   /* get unpacked basis information from CPLEX */
   SCIP_CALL( getBase(lpi) );

   /* pack SCIP_LPi state data */
   (*lpistate)->ncols = ncols;
   (*lpistate)->nrows = nrows;
   lpistatePack(*lpistate, lpi->cstat, lpi->rstat);

   return SCIP_OKAY;
}

/** loads SCIP_LPi state (like basis information) into solver */
SCIP_RETCODE SCIPlpiSetState(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   BMS_BLKMEM*           blkmem,             /**< block memory */
   SCIP_LPISTATE*        lpistate            /**< SCIP_LPi state information (like basis information) */
   )
{
   assert(blkmem != NULL);
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(lpistate == NULL || lpistate->ncols == CPXgetnumcols(cpxenv, lpi->cpxlp));
   assert(lpistate == NULL || lpistate->nrows == CPXgetnumrows(cpxenv, lpi->cpxlp));

   /* if there was no basis information available, the SCIP_LPI state was not stored */
   if( lpistate == NULL )
      return SCIP_OKAY;

   SCIPdebugMessage("loading SCIP_LPI state %p (%d cols, %d rows) into CPLEX\n", lpistate, lpistate->ncols, lpistate->nrows);

   if( lpistate->ncols == 0 || lpistate->nrows == 0 )
      return SCIP_OKAY;   

   /* allocate enough memory for storing uncompressed basis information */
   SCIP_CALL( ensureCstatMem(lpi, lpistate->ncols) );
   SCIP_CALL( ensureRstatMem(lpi, lpistate->nrows) );

   /* unpack SCIP_LPi state data */
   lpistateUnpack(lpistate, lpi->cstat, lpi->rstat);

   /* load basis information into CPLEX */
   SCIP_CALL( setBase(lpi) );

   return SCIP_OKAY;
}

/** frees SCIP_LPi state information */
SCIP_RETCODE SCIPlpiFreeState(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   BMS_BLKMEM*           blkmem,             /**< block memory */
   SCIP_LPISTATE**       lpistate            /**< pointer to SCIP_LPi state information (like basis information) */
   )
{
   assert(lpi != NULL);
   assert(lpistate != NULL);

   if( *lpistate != NULL )
   {
      lpistateFree(lpistate, blkmem);
   }

   return SCIP_OKAY;
}

/** checks, whether the given SCIP_LP state contains simplex basis information */
SCIP_Bool SCIPlpiHasStateBasis(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_LPISTATE*        lpistate            /**< SCIP_LP state information (like basis information) */
   )
{  /*lint --e{715}*/
   return (lpistate != NULL);
}

/** reads SCIP_LP state (like basis information from a file */
SCIP_RETCODE SCIPlpiReadState(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   const char*           fname               /**< file name */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("reading SCIP_LP state from file <%s>\n", fname);

   CHECK_ZERO( CPXreadcopybase(cpxenv, lpi->cpxlp, fname) );

   return SCIP_OKAY;
}

/** writes SCIP_LP state (like basis information) to a file */
SCIP_RETCODE SCIPlpiWriteState(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   const char*           fname               /**< file name */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("writing SCIP_LP state to file <%s>\n", fname);

   CHECK_ZERO( CPXmbasewrite(cpxenv, lpi->cpxlp, fname) );

   return SCIP_OKAY;
}

/**@} */




/*
 * Parameter Methods
 */

/**@name Parameter Methods */
/**@{ */

/** gets integer parameter of SCIP_LP */
SCIP_RETCODE SCIPlpiGetIntpar(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_LPPARAM          type,               /**< parameter number */
   int*                  ival                /**< buffer to store the parameter value */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(ival != NULL);

   SCIPdebugMessage("getting int parameter %d\n", type);

   switch( type )
   {
   case SCIP_LPPAR_FROMSCRATCH:
      *ival = (getIntParam(lpi, CPX_PARAM_ADVIND) == CPX_OFF);
      break;
   case SCIP_LPPAR_FASTMIP:
      *ival = (getIntParam(lpi, CPX_PARAM_FASTMIP) == CPX_ON);
      break;
   case SCIP_LPPAR_SCALING:
      *ival = (getIntParam(lpi, CPX_PARAM_SCAIND) == 0);
      break;
   case SCIP_LPPAR_PRESOLVING:
      *ival = (getIntParam(lpi, CPX_PARAM_PREIND) == CPX_ON);
      break;
   case SCIP_LPPAR_PRICING:
      switch( getIntParam(lpi, CPX_PARAM_PPRIIND) )
      {
      case CPX_PPRIIND_FULL:
         *ival = (int)SCIP_PRICING_FULL;
         break;
      case CPX_PPRIIND_PARTIAL:
         *ival = (int)SCIP_PRICING_PARTIAL;
         break;
      case CPX_PPRIIND_STEEP:
         *ival = (int)SCIP_PRICING_STEEP;
         break;
      case CPX_PPRIIND_STEEPQSTART:
         *ival = (int)SCIP_PRICING_STEEPQSTART;
         break;
#if (CPX_VERSION >= 900)
      case CPX_PPRIIND_DEVEX:
         *ival = (int)SCIP_PRICING_DEVEX;
         break;
#endif
      default:
         *ival = (int)SCIP_PRICING_AUTO;
         break;
      }
      break;
   case SCIP_LPPAR_LPINFO:
      *ival = (getIntParam(lpi, CPX_PARAM_SCRIND) == CPX_ON);
      break;
   case SCIP_LPPAR_LPITLIM:
      *ival = getIntParam(lpi, CPX_PARAM_ITLIM);
      if( *ival >= CPX_INT_MAX )
         *ival = INT_MAX;
      break;
   default:
      return SCIP_PARAMETERUNKNOWN;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** sets integer parameter of SCIP_LP */
SCIP_RETCODE SCIPlpiSetIntpar(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_LPPARAM          type,               /**< parameter number */
   int                   ival                /**< parameter value */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("setting int parameter %d to %d\n", type, ival);

   switch( type )
   {
   case SCIP_LPPAR_FROMSCRATCH:
      assert(ival == TRUE || ival == FALSE);
      setIntParam(lpi, CPX_PARAM_ADVIND, ival == FALSE ? CPX_ON : CPX_OFF);
      break;
   case SCIP_LPPAR_FASTMIP:
      assert(ival == TRUE || ival == FALSE);
      setIntParam(lpi, CPX_PARAM_FASTMIP, ival == TRUE ? CPX_ON : CPX_OFF);
      break;
   case SCIP_LPPAR_SCALING:
      assert(ival == TRUE || ival == FALSE);
      setIntParam(lpi, CPX_PARAM_SCAIND, ival == TRUE ? 0 : -1);
      break;
   case SCIP_LPPAR_PRESOLVING:
      assert(ival == TRUE || ival == FALSE);
      setIntParam(lpi, CPX_PARAM_PREIND, ival == TRUE ? CPX_ON : CPX_OFF);
      break;
   case SCIP_LPPAR_PRICING:
      switch( (SCIP_PRICING)ival )
      {
      case SCIP_PRICING_AUTO:
	 setIntParam(lpi, CPX_PARAM_PPRIIND, CPX_PPRIIND_AUTO);
	 setIntParam(lpi, CPX_PARAM_DPRIIND, CPX_DPRIIND_AUTO);
         break;
      case SCIP_PRICING_FULL:
	 setIntParam(lpi, CPX_PARAM_PPRIIND, CPX_PPRIIND_FULL);
	 setIntParam(lpi, CPX_PARAM_DPRIIND, CPX_DPRIIND_FULL);
         break;
      case SCIP_PRICING_PARTIAL:
	 setIntParam(lpi, CPX_PARAM_PPRIIND, CPX_PPRIIND_PARTIAL);
	 setIntParam(lpi, CPX_PARAM_DPRIIND, CPX_DPRIIND_AUTO);
         break;
      case SCIP_PRICING_STEEP:
	 setIntParam(lpi, CPX_PARAM_PPRIIND, CPX_PPRIIND_STEEP);
	 setIntParam(lpi, CPX_PARAM_DPRIIND, CPX_DPRIIND_STEEP);
	 break;
      case SCIP_PRICING_STEEPQSTART:
	 setIntParam(lpi, CPX_PARAM_PPRIIND, CPX_PPRIIND_STEEPQSTART);
	 setIntParam(lpi, CPX_PARAM_DPRIIND, CPX_DPRIIND_STEEPQSTART);
	 break;
#if (CPX_VERSION >= 900)
      case SCIP_PRICING_DEVEX:
	 setIntParam(lpi, CPX_PARAM_PPRIIND, CPX_PPRIIND_DEVEX);
	 setIntParam(lpi, CPX_PARAM_DPRIIND, CPX_DPRIIND_DEVEX);
	 break;
#endif
      default:
         return SCIP_LPERROR;
      }
      break;
   case SCIP_LPPAR_LPINFO:
      assert(ival == TRUE || ival == FALSE);
      if( ival )
	 setIntParam(lpi, CPX_PARAM_SCRIND, CPX_ON);
      else 
	 setIntParam(lpi, CPX_PARAM_SCRIND, CPX_OFF);
      break;
   case SCIP_LPPAR_LPITLIM:
      ival = MIN(ival, CPX_INT_MAX);
      setIntParam(lpi, CPX_PARAM_ITLIM, ival);
      break;
   default:
      return SCIP_PARAMETERUNKNOWN;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** gets floating point parameter of SCIP_LP */
SCIP_RETCODE SCIPlpiGetRealpar(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_LPPARAM          type,               /**< parameter number */
   SCIP_Real*            dval                /**< buffer to store the parameter value */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);
   assert(dval != NULL);

   SCIPdebugMessage("getting real parameter %d\n", type);

   switch( type )
   {
   case SCIP_LPPAR_FEASTOL:
      *dval = getDblParam(lpi, CPX_PARAM_EPRHS);
      break;
   case SCIP_LPPAR_DUALFEASTOL:
      *dval = getDblParam(lpi, CPX_PARAM_EPOPT);
      break;
   case SCIP_LPPAR_BARRIERCONVTOL:
      *dval = getDblParam(lpi, CPX_PARAM_BAREPCOMP);
      break;
   case SCIP_LPPAR_LOBJLIM:
      *dval = getDblParam(lpi, CPX_PARAM_OBJLLIM);
      break;
   case SCIP_LPPAR_UOBJLIM:
      *dval = getDblParam(lpi, CPX_PARAM_OBJULIM);
      break;
   case SCIP_LPPAR_LPTILIM:
      *dval = getDblParam(lpi, CPX_PARAM_TILIM);
      break;
   case SCIP_LPPAR_MARKOWITZ:
      *dval = getDblParam(lpi, CPX_PARAM_EPMRK);
      break;
   default:
      return SCIP_PARAMETERUNKNOWN;
   }  /*lint !e788*/
   
   return SCIP_OKAY;
}

/** sets floating point parameter of SCIP_LP */
SCIP_RETCODE SCIPlpiSetRealpar(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_LPPARAM          type,               /**< parameter number */
   SCIP_Real             dval                /**< parameter value */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("setting real parameter %d to %g\n", type, dval);

   switch( type )
   {
   case SCIP_LPPAR_FEASTOL:
      setDblParam(lpi, CPX_PARAM_EPRHS, dval);
      break;
   case SCIP_LPPAR_DUALFEASTOL:
      setDblParam(lpi, CPX_PARAM_EPOPT, dval);
      break;
   case SCIP_LPPAR_BARRIERCONVTOL:
      setDblParam(lpi, CPX_PARAM_BAREPCOMP, dval);
      break;
   case SCIP_LPPAR_LOBJLIM:
      setDblParam(lpi, CPX_PARAM_OBJLLIM, dval);
      break;
   case SCIP_LPPAR_UOBJLIM:
      setDblParam(lpi, CPX_PARAM_OBJULIM, dval);
      break;
   case SCIP_LPPAR_LPTILIM:
      setDblParam(lpi, CPX_PARAM_TILIM, dval);
      break;
   case SCIP_LPPAR_MARKOWITZ:
      setDblParam(lpi, CPX_PARAM_EPMRK, dval);
      break;
   default:
      return SCIP_PARAMETERUNKNOWN;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/**@} */




/*
 * Numerical Methods
 */

/**@name Numerical Methods */
/**@{ */

/** returns value treated as infinity in the SCIP_LP solver */
SCIP_Real SCIPlpiInfinity(
   SCIP_LPI*             lpi                 /**< SCIP_LP interface structure */
   )
{  /*lint --e{715}*/
   return CPX_INFBOUND;
}

/** checks if given value is treated as infinity in the SCIP_LP solver */
SCIP_Bool SCIPlpiIsInfinity(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   SCIP_Real             val
   )
{  /*lint --e{715}*/
   return (val >= CPX_INFBOUND);
}

/**@} */




/*
 * File Interface Methods
 */

/**@name File Interface Methods */
/**@{ */

/** reads SCIP_LP from a file */
SCIP_RETCODE SCIPlpiReadLP(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   const char*           fname               /**< file name */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("reading SCIP_LP from file <%s>\n", fname);

   CHECK_ZERO( CPXreadcopyprob(cpxenv, lpi->cpxlp, fname, NULL) );

   return SCIP_OKAY;
}

/** writes SCIP_LP to a file */
SCIP_RETCODE SCIPlpiWriteLP(
   SCIP_LPI*             lpi,                /**< SCIP_LP interface structure */
   const char*           fname               /**< file name */
   )
{
   assert(cpxenv != NULL);
   assert(lpi != NULL);
   assert(lpi->cpxlp != NULL);

   SCIPdebugMessage("writing SCIP_LP to file <%s>\n", fname);

   CHECK_ZERO( CPXwriteprob(cpxenv, lpi->cpxlp, fname, NULL) );

   return SCIP_OKAY;
}

/**@} */

