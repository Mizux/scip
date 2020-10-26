#define SCIP_DEBUG
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2020 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not visit scip.zib.de.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   sepa_minor.c
 * @ingroup DEFPLUGINS_SEPA
 * @brief  principal minor separator
 * @author Dr. Benjamin Mueller
 *
 * @todo detect non-principal minors and use them to derive split cuts
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>
#include <string.h>

#include "scip/sepa_interminor.h"
#include "scip/cons_expr.h"
#include "scip/cons_expr_var.h"
#include "scip/cons_expr_pow.h"
#include "scip/cons_expr_product.h"
#include "scip/cons_expr_iterator.h"
#include "scip/cons_expr_rowprep.h"
#include "nlpi/nlpi_ipopt.h"

#define SEPA_NAME              "interminor"
#define SEPA_DESC              "separator to ensure that 2x2 minors of X (= xx') have determinant 0"
#define SEPA_PRIORITY                 0
#define SEPA_FREQ                    10
#define SEPA_MAXBOUNDDIST           1.0
#define SEPA_USESSUBSCIP          FALSE /**< does the separator use a secondary SCIP instance? */
#define SEPA_DELAY                FALSE /**< should separation method be delayed, if other separators found cuts? */

#define DEFAULT_MAXMINORSCONST     3000 /**< default constant for the maximum number of minors, i.e., max(const, fac * # quadratic terms) */
#define DEFAULT_MAXMINORSFAC       10.0 /**< default factor for the maximum number of minors, i.e., max(const, fac * # quadratic terms) */
#define DEFAULT_MINCUTVIOL         1e-4 /**< default minimum required violation of a cut */
#define DEFAULT_RANDSEED            157 /**< default random seed */
#define DEFAULT_MAXROUNDS            10 /**< maximal number of separation rounds per node (-1: unlimited) */
#define DEFAULT_MAXROUNDSROOT        -1 /**< maximal number of separation rounds in the root node (-1: unlimited) */
#define DEFAULT_IGNOREPACKINGCONSS TRUE /**< default for ignoring circle packing constraints during minor detection */
#define BINSEARCH_MAXITERS          120 /**< default iteration limit for binary search */
#define INTERCUTS_MINVIOL          1e-4 /**< minimal violation the cut needs to have to be added */

/*
 * Data structures
 */

/** separator data */
struct SCIP_SepaData
{
   SCIP_VAR**            minors;             /**< variables of 2x2 minors; each minor is stored like (auxvar_x^2,auxvar_y^2,auxvar_xy) */
   int                   nminors;            /**< total number of minors */
   int                   minorssize;         /**< size of minors array */
   int                   maxminorsconst;     /**< constant for the maximum number of minors, i.e., max(const, fac * # quadratic terms) */
   SCIP_Real             maxminorsfac;       /**< factor for the maximum number of minors, i.e., max(const, fac * # quadratic terms) */
   int                   maxrounds;          /**< maximal number of separation rounds per node (-1: unlimited) */
   int                   maxroundsroot;      /**< maximal number of separation rounds in the root node (-1: unlimited) */
   SCIP_Bool             detectedminors;     /**< has minor detection be called? */
   SCIP_Real             mincutviol;         /**< minimum required violation of a cut */
   SCIP_RANDNUMGEN*      randnumgen;         /**< random number generation */
   SCIP_Bool             ignorepackingconss; /**< whether to ignore circle packing constraints during minor detection */
};

/* these represent a row */
struct myarray
{
   int*                  vals;               /**< index of the column */
   int                   nvals;
   int                   valssize;
   SCIP_HASHMAP*         auxvars;            /**< entry of the matrix */
};

/*
 * Local methods
 */

/** helper method to store a 2x2 minor in the separation data */
static
SCIP_RETCODE sepadataAddMinor(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SEPADATA*        sepadata,           /**< separator data */
   SCIP_VAR*             auxvarxik,          /**< auxiliary variable X_ik = x_i * x_k */
   SCIP_VAR*             auxvarxil,          /**< auxiliary variable X_il = x_i * x_l */
   SCIP_VAR*             auxvarxjk,          /**< auxiliary variable X_jk = x_j * x_k */
   SCIP_VAR*             auxvarxjl           /**< auxiliary variable X_jl = x_j * x_l */
   )
{
   assert(sepadata != NULL);
   assert(auxvarxik != NULL);
   assert(auxvarxil != NULL);
   assert(auxvarxjk != NULL);
   assert(auxvarxjl != NULL);
   assert(auxvarxik != auxvarxil);
   assert(auxvarxjk != auxvarxjl);

   SCIPdebugMsg(scip, "store 2x2 minor: [%s %s, %s %s]\n", SCIPvarGetName(auxvarxik), SCIPvarGetName(auxvarxil),
         SCIPvarGetName(auxvarxjk), SCIPvarGetName(auxvarxjl));

   /* reallocate if necessary */
   if( sepadata->minorssize < 4 * (sepadata->nminors + 1) )
   {
      int newsize = SCIPcalcMemGrowSize(scip, 4 * (sepadata->nminors + 1));
      assert(newsize >= 4 * (sepadata->nminors + 1));

      SCIP_CALL( SCIPreallocBlockMemoryArray(scip, &(sepadata->minors), sepadata->minorssize, newsize) );
      sepadata->minorssize = newsize;
   }

   /* store minor */
   sepadata->minors[4 * sepadata->nminors] = auxvarxik;
   sepadata->minors[4 * sepadata->nminors + 1] = auxvarxil;
   sepadata->minors[4 * sepadata->nminors + 2] = auxvarxjk;
   sepadata->minors[4 * sepadata->nminors + 3] = auxvarxjl;
   ++(sepadata->nminors);

   /* capture variables */
   SCIP_CALL( SCIPcaptureVar(scip, auxvarxik) );
   SCIP_CALL( SCIPcaptureVar(scip, auxvarxil) );
   SCIP_CALL( SCIPcaptureVar(scip, auxvarxjk) );
   SCIP_CALL( SCIPcaptureVar(scip, auxvarxjl) );

   return SCIP_OKAY;
}

/** helper method to clear separation data */
static
SCIP_RETCODE sepadataClear(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SEPADATA*        sepadata            /**< separator data */
   )
{
   int i;

   assert(sepadata != NULL);

   SCIPdebugMsg(scip, "clear separation data\n");

   /* release captured variables */
   for( i = 0; i < 4 * sepadata->nminors; ++i )
   {
      assert(sepadata->minors[i] != NULL);
      SCIP_CALL( SCIPreleaseVar(scip, &sepadata->minors[i]) );
   }

   /* free memory */
   SCIPfreeBlockMemoryArrayNull(scip, &sepadata->minors, sepadata->minorssize);

   /* reset counters */
   sepadata->nminors = 0;
   sepadata->minorssize = 0;

   return SCIP_OKAY;
}


/** helper method to get the variables associated to a minor */
static
SCIP_RETCODE getMinorVars(
   SCIP_SEPADATA*        sepadata,           /**< separator data */
   int                   idx,                /**< index of the stored minor */
   SCIP_VAR**            auxvarxik,          /**< auxiliary variable X_ik = x_i * x_k */
   SCIP_VAR**            auxvarxil,          /**< auxiliary variable X_il = x_i * x_l */
   SCIP_VAR**            auxvarxjk,          /**< auxiliary variable X_jk = x_j * x_k */
   SCIP_VAR**            auxvarxjl           /**< auxiliary variable X_jl = x_j * x_l */
   )
{
   assert(auxvarxik != NULL);
   assert(auxvarxil != NULL);
   assert(auxvarxjk != NULL);
   assert(auxvarxjl != NULL);

   *auxvarxik = sepadata->minors[4 * idx];
   *auxvarxil = sepadata->minors[4 * idx + 1];
   *auxvarxjk = sepadata->minors[4 * idx + 2];
   *auxvarxjl = sepadata->minors[4 * idx + 3];

   return SCIP_OKAY;
}


/**
 * we have a matrix, M, indexed by the variables
 * M(xi, xk) is the auxiliary variable of xi * xk if it exists
 * We store, for each row of the matrix, the indices of the nonzero column entries (assoc with the given row) and the auxiliary variable for xi * xk
 * The nonzero column entries are stores as an array (struct myarray)
 * So we have a hasmap mapping each variable (row of the matrix) with its array representing the nonzero entries of the row.
 */
static
SCIP_RETCODE insertIndex(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HASHMAP*         rowmap,
   SCIP_VAR*             row,
   SCIP_VAR*             col,
   SCIP_VAR*             auxvar,
   int*                  rowindices,
   int*                  nrows
   )
{
   SCIPdebugMsg(scip, "inserting %s in row %s and col %s \n", SCIPvarGetName(auxvar), SCIPvarGetName(row), SCIPvarGetName(col));

   /* check whether variables has an array associated to it */
   if( SCIPhashmapExists(rowmap, (void*)row) )
   {
      struct myarray* arr;

      arr = (struct myarray*)SCIPhashmapGetImage(rowmap, (void *)row);

      /* reallocate if necessary */
      if( arr->valssize < arr->nvals + 1 )
      {
         int newsize = SCIPcalcMemGrowSize(scip, arr->nvals + 1);
         assert(newsize > arr->nvals + 1);

         SCIP_CALL( SCIPreallocBlockMemoryArray(scip, &(arr->vals), arr->valssize, newsize) );
         arr->valssize = newsize;
      }

      /* insert */
      arr->vals[arr->nvals] = SCIPvarGetProbindex(col);
      SCIP_CALL( SCIPhashmapInsert(arr->auxvars, (void*)col, (void *)auxvar) );
      arr->nvals += 1;
   }
   else
   {
      struct myarray* arr;

      /* create index array */
      SCIP_CALL( SCIPallocBlockMemory(scip, &arr) );
      arr->valssize = 10;
      arr->nvals = 0;
      SCIP_CALL( SCIPallocBlockMemoryArray(scip, &arr->vals, arr->valssize) );
      SCIP_CALL( SCIPhashmapCreate(&arr->auxvars, SCIPblkmem(scip), arr->valssize) );

      /* insert */
      arr->vals[arr->nvals] = SCIPvarGetProbindex(col);
      SCIP_CALL( SCIPhashmapInsert(arr->auxvars, (void*)col, (void *)auxvar) );
      arr->nvals += 1;

      /* store in hashmap */
      SCIP_CALL( SCIPhashmapInsert(rowmap, (void*)row, (void *)arr) );

      /* remember the new row */
      rowindices[*nrows] = SCIPvarGetProbindex(row);
      *nrows += 1;
   }

   return SCIP_OKAY;
}

/** method to detect and store principal minors */
static
SCIP_RETCODE detectMinors(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SEPADATA*        sepadata            /**< separator data */
   )
{
   SCIP_CONSHDLR* conshdlr;
   SCIP_CONSEXPR_ITERATOR* it;
   SCIP_HASHMAP* rowmap;
   SCIP_HASHMAP* quadmap;
   SCIP_VAR** xs;
   SCIP_VAR** ys;
   SCIP_VAR** auxvars;
   int* rowvars = NULL;
   int* perm = NULL;
   int* intersection;
   int nrowvars = 0;
   int c;
   int i;

#ifdef SCIP_STATISTIC
   SCIP_Real totaltime = -SCIPgetTotalTime(scip);
#endif

   assert(sepadata != NULL);

   /* check whether minor detection has been called already */
   if( sepadata->detectedminors )
      return SCIP_OKAY;

   assert(sepadata->minors == NULL);
   assert(sepadata->nminors == 0);

   /* we assume that the auxiliary variables in the expression constraint handler have been already generated */
   sepadata->detectedminors = TRUE;

   /* check whether there are expression constraints available */
   conshdlr = SCIPfindConshdlr(scip, "expr");
   if( conshdlr == NULL || SCIPconshdlrGetNConss(conshdlr) == 0 )
      return SCIP_OKAY;

   SCIPdebugMsg(scip, "call detectMinors()\n");

   /* allocate memory */
   SCIP_CALL( SCIPexpriteratorCreate(&it, conshdlr, SCIPblkmem(scip)) );
   SCIP_CALL( SCIPhashmapCreate(&quadmap, SCIPblkmem(scip), SCIPgetNVars(scip)) );
   SCIP_CALL( SCIPhashmapCreate(&rowmap, SCIPblkmem(scip), SCIPgetNVars(scip)) );
   SCIP_CALL( SCIPallocBufferArray(scip, &xs, SCIPgetNVars(scip)) );
   SCIP_CALL( SCIPallocBufferArray(scip, &ys, SCIPgetNVars(scip)) );
   SCIP_CALL( SCIPallocBufferArray(scip, &auxvars, SCIPgetNVars(scip)) );
   SCIP_CALL( SCIPallocBufferArray(scip, &rowvars, SCIPgetNVars(scip)) );
   SCIP_CALL( SCIPallocBufferArray(scip, &intersection, SCIPgetNVars(scip)) );

   /* initialize iterator */
   SCIP_CALL( SCIPexpriteratorInit(it, NULL, SCIP_CONSEXPRITERATOR_DFS, TRUE) );
   SCIPexpriteratorSetStagesDFS(it, SCIP_CONSEXPRITERATOR_ENTEREXPR);

   for( c = 0; c < SCIPconshdlrGetNConss(conshdlr); ++c )
   {
      SCIP_CONS* cons;
      SCIP_CONSEXPR_EXPR* expr;
      SCIP_CONSEXPR_EXPR* root;

      cons = SCIPconshdlrGetConss(conshdlr)[c];
      assert(cons != NULL);
      root = SCIPgetExprConsExpr(scip, cons);
      assert(root != NULL);

      for( expr = SCIPexpriteratorRestartDFS(it, root); !SCIPexpriteratorIsEnd(it); expr = SCIPexpriteratorGetNext(it) ) /*lint !e441*/
      {
         SCIP_CONSEXPR_EXPRHDLR* exprhdlr;
         SCIP_CONSEXPR_EXPR** children;
         SCIP_VAR* auxvar;

         SCIPdebugMsg(scip, "visit expression %p in constraint %s\n", (void*)expr, SCIPconsGetName(cons));

         /* check whether the expression has an auxiliary variable */
         auxvar = SCIPgetConsExprExprAuxVar(expr);
         if( auxvar == NULL )
         {
            SCIPdebugMsg(scip, "expression has no auxiliary variable -> skip\n");
            continue;
         }

         exprhdlr = SCIPgetConsExprExprHdlr(expr);
         assert(exprhdlr != NULL);
         children = SCIPgetConsExprExprChildren(expr);

         /* check for expr = (x)^2 */
         if( SCIPgetConsExprExprNChildren(expr) == 1 && exprhdlr == SCIPgetConsExprExprHdlrPower(conshdlr)
            && SCIPgetConsExprExprPowExponent(expr) == 2.0
            && SCIPgetConsExprExprAuxVar(children[0]) != NULL )
         {
            SCIP_VAR* quadvar;

            assert(children[0] != NULL);

            quadvar = SCIPgetConsExprExprAuxVar(children[0]);
            assert(quadvar != NULL);

            SCIP_CALL( insertIndex(scip, rowmap, quadvar, quadvar, auxvar, rowvars, &nrowvars) );
         }
         /* check for expr = x_i * x_k */
         else if( SCIPgetConsExprExprNChildren(expr) == 2 && exprhdlr == SCIPgetConsExprExprHdlrProduct(conshdlr)
            && SCIPgetConsExprExprAuxVar(children[0]) != NULL && SCIPgetConsExprExprAuxVar(children[1]) != NULL )
         {
            SCIP_VAR* xi;
            SCIP_VAR* xk;

            assert(children[0] != NULL);
            assert(children[1] != NULL);

            xi = SCIPgetConsExprExprAuxVar(children[0]);
            xk = SCIPgetConsExprExprAuxVar(children[1]);

            SCIP_CALL( insertIndex(scip, rowmap, xk, xi, auxvar, rowvars, &nrowvars) );
            SCIP_CALL( insertIndex(scip, rowmap, xi, xk, auxvar, rowvars, &nrowvars) );
         }
      }
   }

   /* sort the column entries */
   for( i = 0; i < nrowvars; ++i )
   {
      struct myarray* row;

      row = (struct myarray*)SCIPhashmapGetImage(rowmap, (void *)SCIPgetVars(scip)[rowvars[i]]);
      (void)SCIPsortInt(row->vals, row->nvals);
   }

   /* store 2x2 minors */
   /* TODO: we might store some minors twice since the matrix is symmetric. Handle that! (see unit test for example) */
   for( i = 0; i < nrowvars - 1; ++i )
   {
      int j;
      struct myarray* rowi;

      rowi = (struct myarray*)SCIPhashmapGetImage(rowmap, (void *)SCIPgetVars(scip)[rowvars[i]]);

      for( j = i + 1; j < nrowvars; ++j )
      {
         struct myarray* rowj;
         int ninter;

         rowj = (struct myarray*)SCIPhashmapGetImage(rowmap, (void *)SCIPgetVars(scip)[rowvars[j]]);

         SCIP_CALL( SCIPcomputeArraysIntersection(rowi->vals, rowi->nvals, rowj->vals, rowj->nvals, intersection,
                  &ninter) );

         if( ninter > 1)
         {
            int p;

            for( p = 0; p < ninter - 1; ++p )
            {
               int q;

               for( q = p + 1; q < ninter; ++q )
               {
                  SCIP_HASHMAP* rowicols;
                  SCIP_HASHMAP* rowjcols;
                  SCIP_VAR* colk;
                  SCIP_VAR* coll;
                  SCIP_VAR* auxvarik;
                  SCIP_VAR* auxvaril;
                  SCIP_VAR* auxvarjk;
                  SCIP_VAR* auxvarjl;

                  rowicols = rowi->auxvars;
                  rowjcols = rowj->auxvars;

                  colk = SCIPgetVars(scip)[intersection[p]];
                  coll = SCIPgetVars(scip)[intersection[q]];

                  auxvarik = SCIPhashmapGetImage(rowicols, colk);
                  auxvaril = SCIPhashmapGetImage(rowicols, coll);
                  auxvarjk = SCIPhashmapGetImage(rowjcols, colk);
                  auxvarjl = SCIPhashmapGetImage(rowjcols, coll);

                  SCIP_CALL( sepadataAddMinor(scip, sepadata, auxvarik, auxvaril, auxvarjk, auxvarjl) );
               }
            }
         }
      }
   }

   ///* permute bilinear terms if there are too many of them; the motivation for this is that we don't want to
   // * prioritize variables because of the order in the bilinear terms where they appear; however, variables that
   // * appear more often in bilinear terms might be more important than others so the corresponding bilinear terms
   // * are more likely to be chosen
   // */
   //if( maxminors < nbilinterms && maxminors < SQR(nquadterms) )
   //{
   //   SCIP_CALL( SCIPallocBufferArray(scip, &perm, nbilinterms) );

   //   for( i = 0; i < nbilinterms; ++i )
   //      perm[i] = i;

   //   /* permute array */
   //   SCIPrandomPermuteIntArray(sepadata->randnumgen, perm, 0, nbilinterms);
   //}

   SCIPdebugMsg(scip, "found %d principal minors in total\n", sepadata->nminors);

   /* free memory */
   SCIPfreeBufferArrayNull(scip, &perm);
   SCIPfreeBufferArray(scip, &intersection);
   SCIPfreeBufferArray(scip, &rowvars);
   SCIPfreeBufferArray(scip, &auxvars);
   SCIPfreeBufferArray(scip, &ys);
   SCIPfreeBufferArray(scip, &xs);
   SCIPhashmapFree(&rowmap);
   SCIPhashmapFree(&quadmap);
   SCIPexpriteratorFree(&it);

#ifdef SCIP_STATISTIC
   totaltime += SCIPgetTotalTime(scip);
   SCIPstatisticMessage("MINOR DETECT %s %f %d %d\n", SCIPgetProbName(scip), totaltime, sepadata->nminors, maxminors);
#endif

   return SCIP_OKAY;
}

/** constructs map between lp position of a basic variable and its row in the tableau */
/* TODO for Antonia: maybe think of making things nicer to avoid code duplication */
static
SCIP_RETCODE constructBasicVars2TableauRowMap(
   SCIP*                 scip,               /**< SCIP data structure */
   int*                  map                 /**< buffer to store the map */
   )
{
   int* basisind;
   int nrows;
   int i;

   nrows = SCIPgetNLPRows(scip);
   SCIP_CALL( SCIPallocBufferArray(scip, &basisind, nrows) );

   SCIP_CALL( SCIPgetLPBasisInd(scip, basisind) );
   for( i = 0; i < nrows; ++i )
   {
      if( basisind[i] >= 0 )
         map[basisind[i]] = i;
   }

   SCIPfreeBufferArray(scip, &basisind);

   return SCIP_OKAY;
}

/** The restriction of the function representing the maximal S-free set to zlp + t * ray has the form
 * SQRT(A t^2 + B t + C) - (D t + E).
 * This function computes the coefficients A, B, C, D, E for the given ray.
 */
static
void computeRestrictionToRay(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real*            ray,                /**< coefficients of ray */
   SCIP_VAR**            vars,               /**< variables */
   SCIP_Real*            coefs               /**< buffer to store A, B, C, D, and E */
   )
{
   SCIP_Real eigenvectors[16] = {1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0};
   SCIP_Real eigenvalues[4] = {0.5, 0.5, -0.5, -0.5};
   SCIP_Real coefeigenvector = 0.70710678118;
   SCIP_Real* a;
   SCIP_Real* b;
   SCIP_Real* c;
   SCIP_Real* d;
   SCIP_Real* e;
   int i;

   /* set all coefficients to zero */
   memset(coefs, 0, 5 * sizeof(SCIP_Real));

   a = coefs;
   b = coefs + 1;
   c = coefs + 2;
   d = coefs + 3;
   e = coefs + 4;

   for( i = 0; i < 4; ++i )
   {
      int j;
      SCIP_Real vzlp;
      SCIP_Real vdotray;

      vzlp = 0;
      vdotray = 0;

      /* compute eigenvec * ray and eigenvec * solution */
      for( j = 0; j < 4; ++j )
      {
         vdotray += coefeigenvector * eigenvectors[4 * i + j] * ray[j];
         vzlp += coefeigenvector * eigenvectors[4 * i + j] * SCIPvarGetLPSol(vars[j]);
      }

      if( eigenvalues[i] > 0 )
      {
         /* positive eigenvalue: compute D and E */
         *d += eigenvalues[i] * vzlp * vdotray;
         *e += eigenvalues[i] * SQR( vzlp );
      }
      else
      {
         /* negative eigenvalue: compute A, B, and C */
         *a -= eigenvalues[i] * SQR( vdotray );
         *b -= 2.0 * eigenvalues[i] * vzlp * vdotray;
         *c -= eigenvalues[i] * SQR( vzlp );
      }
   }

   /* finish computation of D and E */
   assert(*e > 0);
   *e = SQRT( *e );
   *d /= *e;

   /* some sanity checks */
   assert(*c >= 0); /* radicand at zero */
   assert(SQRT( *c ) - *e < 0); /* the function at 0 must be negative */
   assert(*a >= 0); /* the function inside the root is convex */

#ifdef  DEBUG_INTERSECTIONCUT
   SCIPinfoMessage(scip, NULL, "Restriction yields: a,b,c,d,e %g %g %g %g %g\n", coefs[0], coefs[1], coefs[2], coefs[3], coefs[4]);
#endif
}

/** returns phi(zlp + t * ray) = SQRT(A t^2 + B t + C) - (D t + E) */
static
SCIP_Real evalPhiAtRay(
   SCIP_Real             t,                  /**< argument of phi restricted to ray */
   SCIP_Real             a,                  /**< value of A */
   SCIP_Real             b,                  /**< value of B */
   SCIP_Real             c,                  /**< value of C */
   SCIP_Real             d,                  /**< value of D */
   SCIP_Real             e                   /**< value of E */
   )
{
#ifdef INTERCUTS_DBLDBL
   SCIP_Real QUAD(lin);
   SCIP_Real QUAD(disc);
   SCIP_Real QUAD(tmp);
   SCIP_Real QUAD(root);

   /* d * t + e */
   SCIPquadprecProdDD(lin, d, t);
   SCIPquadprecSumQD(lin, lin, e);

   /* a * t * t */
   SCIPquadprecSquareD(disc, t);
   SCIPquadprecProdQD(disc, disc, a);

   /* b * t */
   SCIPquadprecProdDD(tmp, b, t);

   /* a * t * t + b * t */
   SCIPquadprecSumQQ(disc, disc, tmp);

   /* a * t * t + b * t + c */
   SCIPquadprecSumQD(disc, disc, c);

   /* sqrt(above): can't take sqrt of 0! */
   if( QUAD_TO_DBL(disc) == 0 )
   {
      QUAD_ASSIGN(root, 0.0);
   }
   else
   {
      SCIPquadprecSqrtQ(root, disc);
   }

   /* final result */
   QUAD_SCALE(lin, -1.0);
   SCIPquadprecSumQQ(tmp, root, lin);

   assert(t != 1e20 || QUAD_TO_DBL(tmp) <= 0);

   return  QUAD_TO_DBL(tmp);
#else
   return SQRT( a * t * t + b * t + c ) - ( d * t + e );
#endif
}

/** helper function of computeRoot: we want phi to be <= 0 */
static
void doBinarySearch(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             a,                  /**< value of A */
   SCIP_Real             b,                  /**< value of B */
   SCIP_Real             c,                  /**< value of C */
   SCIP_Real             d,                  /**< value of D */
   SCIP_Real             e,                  /**< value of E */
   SCIP_Real*            sol                 /**< buffer to store solution; also gives initial point */
   )
{
   SCIP_Real lb = 0.0;
   SCIP_Real ub = *sol;
   SCIP_Real curr;
   int i;

   for( i = 0; i < BINSEARCH_MAXITERS; ++i )
   {
      SCIP_Real phival;

      curr = (lb + ub) / 2.0;
      phival = evalPhiAtRay(curr, a, b, c, d, e);
#ifdef INTERCUT_MOREDEBUG
      printf("%d: lb,ub %.10f, %.10f. curr = %g -> phi at curr %g -> phi at lb %g \n", i, lb, ub, curr, phival, evalPhiAtRay(lb, a, b, c, d, e));
#endif

      if( phival <= 0.0 )
      {
         lb = curr;
         if( SCIPisFeasZero(scip, phival) || SCIPisFeasEQ(scip, ub, lb) )
            break;
      }
      else
         ub = curr;
   }

   *sol = lb;

}

/**  finds smallest positive root phi by finding the smallest positive root of
 * (A - D^2) t^2 + (B - 2 D*E) t + (C - E^2) = 0
 * However, we are conservative and want a solution such that phi is negative, but close to 0;
 * thus we correct the result with a binary search
 */
static
SCIP_Real computeRoot(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real*            coefs               /**< value of A */
   )
{
   SCIP_Real sol;
   SCIP_INTERVAL bounds;
   SCIP_INTERVAL result;
   SCIP_Real a = coefs[0];
   SCIP_Real b = coefs[1];
   SCIP_Real c = coefs[2];
   SCIP_Real d = coefs[3];
   SCIP_Real e = coefs[4];

   /* there is an intersection point if and only if SQRT(A) > D: here we are beliving in math, this might cause
    * numerical issues
    */
   if( SQRT( a ) <= d )
   {
      sol = SCIPinfinity(scip);

      /* if SQRT(a) <= d, but a > d * d --> numerics are weird and phi might not evalate negative at infinity */
      //assert(a > d * d || evalPhiAtRay(sol, a, b, c, d, e) <= 0);
      return sol;
   }

   SCIPintervalSetBounds(&bounds, 0.0, SCIPinfinity(scip));

   /* SCIPintervalSolveUnivariateQuadExpressionPositiveAllScalar finds all x such that a x^2 + b x >= c and x in bounds.
    * it is known that if tsol is the root we are looking for, then gamma(zlp + t * ray) <= 0 between 0 and tsol, thus
    * tsol is the smallest t such that (A - D^2) t^2 + (B - 2 D*E) t + (C - E^2) >= 0
    */
   SCIPintervalSolveUnivariateQuadExpressionPositiveAllScalar(SCIP_INTERVAL_INFINITY, &result, a - d * d, b - 2.0 * d *
         e, -(c - e * e), bounds);

   /* it can still be empty because of our infinity, I guess... */
   sol = SCIPintervalIsEmpty(SCIP_INTERVAL_INFINITY, result) ? SCIPinfinity(scip) : SCIPintervalGetInf(result);

   /* check that solution is acceptable, ideally it should be <= 0, however when it is positive, we trigger a binary
    * search to make it negative. This binary search might return a solution point that is not at accurately 0 as the
    * one obtained from the function above. Thus, it might fail to satisfy the condition of case 4b in some cases, e.g.,
    * ex8_3_1, bchoco05, etc
    */
   if( evalPhiAtRay(sol, a, b, c, d, e) <= 1e-10 )
   {
#ifdef INTERCUT_MOREDEBUG
      printf("interval solution returned %g -> phival = %g, believe it\n", sol, evalPhiAtRay(sol, a, b, c, d, e));
      printf("don't do bin search\n");
#endif

      return sol;
   }
   else
   {
      /* perform a binary search to make it negative: this might correct a wrong infinity (e.g. crudeoil_lee1_05) */
#ifdef INTERCUT_MOREDEBUG
      printf("do bin search because phival is %g\n", evalPhiAtRay(sol, a, b, c, d, e));
#endif
      doBinarySearch(scip, a, b, c, d, e, &sol);
   }

   return sol;
}

/** adds cutcoef * (col - col*) to rowprep */
static
SCIP_RETCODE addColToCut(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ROWPREP*         rowprep,            /**< rowprep to store intersection cut */
   SCIP_Real             cutcoef,            /**< cut coefficient */
   SCIP_COL*             col                 /**< column to add to rowprep */
   )
{
   assert(col != NULL);

#ifdef DEBUG_INTERCUTS_NUMERICS
   SCIPinfoMessage(scip, NULL, "adding col %s to cut. %g <= col <= %g\n", SCIPvarGetName(SCIPcolGetVar(col)),
      SCIPvarGetLbLocal(SCIPcolGetVar(col)), SCIPvarGetUbLocal(SCIPcolGetVar(col)));
   SCIPinfoMessage(scip, NULL, "col is active at %s. Value %.15f\n", SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_LOWER ? "lower bound" :
      "upper bound" , SCIPcolGetPrimsol(col));
#endif

   SCIP_CALL( SCIPaddRowprepTerm(scip, rowprep, SCIPcolGetVar(col), cutcoef) );
   SCIPaddRowprepConstant(rowprep, -cutcoef * SCIPcolGetPrimsol(col) );

   return SCIP_OKAY;
}

/** adds cutcoef * (slack - slack*) to rowprep
  * row is lhs <= <coefs, vars> + constant <= rhs, thus slack is defined by
  * slack + <coefs.vars> + constant = side
  * If row (slack) is at upper, it means that <coefs,vars*> + constant = rhs, and so
  * slack* = side - rhs --> slack - slack* = rhs - <coefs, vars> - constant.
  * If row (slack) is at lower, then <coefs,vars*> + constant = lhs, and so
  * slack* = side - lhs --> slack - slack* = lhs - <coefs, vars> - constant.
  */
static
SCIP_RETCODE addRowToCut(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ROWPREP*         rowprep,            /**< rowprep to store intersection cut */
   SCIP_Real             cutcoef,            /**< cut coefficient */
   SCIP_ROW*             row,                /**< row, whose slack we are ading to rowprep */
   SCIP_Bool*            success             /**< if the row is nonbasic enough */
   )
{
   int i;
   SCIP_COL** rowcols;
   SCIP_Real* rowcoefs;
   int nnonz;

   assert(row != NULL);

   rowcols = SCIProwGetCols(row);
   rowcoefs = SCIProwGetVals(row);
   nnonz = SCIProwGetNLPNonz(row);

#ifdef DEBUG_INTERCUTS_NUMERICS
   SCIPinfoMessage(scip, NULL, "adding slack var row_%d to cut. %g <= row <= %g\n", SCIProwGetLPPos(row), SCIProwGetLhs(row), SCIProwGetRhs(row));
   SCIPinfoMessage(scip, NULL, "row is active at %s = %.15f Activity %.15f\n", SCIProwGetBasisStatus(row) == SCIP_BASESTAT_LOWER ? "lhs" :
   "rhs" , SCIProwGetBasisStatus(row) == SCIP_BASESTAT_LOWER ? SCIProwGetLhs(row) : SCIProwGetRhs(row),
   SCIPgetRowActivity(scip, row));
#endif

   if( SCIProwGetBasisStatus(row) == SCIP_BASESTAT_LOWER )
   {
      assert(!SCIPisInfinity(scip, -SCIProwGetLhs(row)));
      if( ! SCIPisFeasEQ(scip, SCIProwGetLhs(row), SCIPgetRowActivity(scip, row)) )
      {
         *success = FALSE;
         return SCIP_OKAY;
      }

      SCIPaddRowprepConstant(rowprep, SCIProwGetLhs(row) * cutcoef);
   }
   else
   {
      assert(!SCIPisInfinity(scip, SCIProwGetRhs(row)));
      if( ! SCIPisFeasEQ(scip, SCIProwGetRhs(row), SCIPgetRowActivity(scip, row)) )
      {
         *success = FALSE;
         return SCIP_OKAY;
      }

      SCIPaddRowprepConstant(rowprep, SCIProwGetRhs(row) * cutcoef);
   }

   for( i = 0; i < nnonz; i++ )
   {
      SCIP_CALL( SCIPaddRowprepTerm(scip, rowprep, SCIPcolGetVar(rowcols[i]), -rowcoefs[i] * cutcoef) );
   }

   SCIPaddRowprepConstant(rowprep, -SCIProwGetConstant(row) * cutcoef);

   return SCIP_OKAY;
}


/** get the tableau rows of the variables in vars */
static
SCIP_RETCODE getTableauRows(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variables in the minor */
   int*                  basicvarpos2tableaurow,/**< map between basic var and its tableau row */
   SCIP_HASHMAP*         tableau,            /**< map between var an its tableau row */
   SCIP_Real**           tableaurows         /**< buffer to store tableau row */
   )
{
   int v;
   int nrows;
   int ncols;

   nrows = SCIPgetNLPRows(scip);
   ncols = SCIPgetNLPCols(scip);

   /* check if we have the tableau row of the variable and if not compute it */
   for( v = 0; v < 4; ++v )
   {
      if( ! SCIPhashmapExists(tableau, (void*)vars[v]) )
      {
         SCIP_COL* col;

         /* get column of variable */
         col = SCIPvarGetCol(vars[v]);

         /* if variable is basic, then get its tableau row and insert it in the hashmap */
         /* TODO: if this gets too nasty, Antonia will fix it */
         if( SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_BASIC )
         {
            int lppos;
            SCIP_Real* densetableaurow;

            lppos = SCIPcolGetLPPos(col);
            SCIP_CALL( SCIPallocBufferArray(scip, &densetableaurow, ncols + nrows) );

            SCIP_CALL( SCIPgetLPBInvRow(scip, basicvarpos2tableaurow[lppos], &densetableaurow[ncols], NULL, NULL) );
            SCIP_CALL( SCIPgetLPBInvARow(scip, basicvarpos2tableaurow[lppos], &densetableaurow[ncols], densetableaurow, NULL, NULL) );

            /* insert tableau row in hashmap*/
            SCIP_CALL( SCIPhashmapInsert(tableau, (void*)vars[v], (void *)densetableaurow) );
         }
         else if( SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_ZERO )
         {
            return SCIP_OKAY; /* don't even bother */
            /* TODO: please Antonia, could you be so nice to make sure that we are releasing all the memory that we allocated if any */
         }
         else
         {
            /* if variable is non-basic, then ???? (= TODO antonia) */
            SCIP_CALL( SCIPhashmapInsert(tableau, (void*)vars[v], (void *)NULL) );
         }

      }

      /* get tableau row of var */
      tableaurows[v] = (SCIP_Real *)SCIPhashmapGetImage(tableau, (void*)vars[v]);
   }
   return SCIP_OKAY;
}

/** computes the cut coefs of the  non-basic (non-slack) variables (correspond to cols) and adds them to the
 * intersection cut
 */
static
SCIP_RETCODE addCols(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variables */
   SCIP_Real**           tableaurows,        /**< tableau rows corresponding to the variables in vars */
   SCIP_ROWPREP*         rowprep,            /**< store cut */
   SCIP_Bool*            success             /**< pointer to store whether the generation of cutcoefs was successful */
   )
{
   int i;
   int ncols;
   SCIP_COL** cols;

   *success = TRUE;

   /* loop over non-basic (non-slack) variables */
   cols = SCIPgetLPCols(scip);
   ncols = SCIPgetNLPCols(scip);
   for( i = 0; i < ncols; ++i )
   {
      SCIP_COL* col;
      SCIP_Real ray[4];
      SCIP_Real coefs[5];
      SCIP_Real factor;
      SCIP_Bool israynonzero;
      SCIP_Real cutcoef;
      SCIP_Real interpoint;
      int v;

      col = cols[i];

      /* set factor to store entries of ray as = [-BinvL, BinvU] */
      if( SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_LOWER )
         factor = -1.0;
      else if( SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_UPPER )
         factor = 1.0;
      else if( SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_ZERO )
      {
         *success = FALSE;
         return SCIP_OKAY;
      }
      else
         continue;

      /* build the ray */
      israynonzero = FALSE;
      for( v = 0; v < 4; ++v )
      {
         int index;

         index = i;

         if( tableaurows[v] != NULL )
            ray[v] = factor * (SCIPisZero(scip, tableaurows[v][index]) ? 0.0 : tableaurows[v][index]);
         else
         {
            if( col == SCIPvarGetCol(vars[v]) )
               ray[v] = factor;
            else
               ray[v] = 0.0;
         }

         israynonzero = israynonzero || (ray[v] != 0.0);
      }

      /* do nothing if ray is 0 */
      if( ! israynonzero )
         continue;

      /* compute the cut */
      computeRestrictionToRay(scip, ray, vars, coefs);

      /* compute intersection point */
      interpoint = computeRoot(scip, coefs);

      /* compute cut coef */
      cutcoef = SCIPisInfinity(scip, interpoint) ? 0.0 : 1.0 / interpoint;

      /* add var to cut: if variable is nonbasic at upper we have to flip sign of cutcoef */
      assert(SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_UPPER || SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_LOWER);
      SCIP_CALL( addColToCut(scip, rowprep, SCIPcolGetBasisStatus(col) == SCIP_BASESTAT_UPPER ? -cutcoef :
            cutcoef, col) );
   }

   return SCIP_OKAY;
}

/** computes the cut coefs of the non-basic slack variables (correspond to rows) and adds them to the
 * intersection cut
 */
static
SCIP_RETCODE addRows(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variables */
   SCIP_Real**           tableaurows,        /**< tableau rows corresponding to the variables in vars */
   SCIP_ROWPREP*         rowprep,            /**< store cut */
   SCIP_Bool*            success             /**< pointer to store whether the generation of cutcoefs was successful */
   )
{
   int i;
   int nrows;
   int ncols;
   SCIP_ROW** rows;

   nrows = SCIPgetNLPRows(scip);
   ncols = SCIPgetNLPCols(scip);

   *success = TRUE;

   /* loop over non-basic slack variables */
   rows = SCIPgetLPRows(scip);
   for( i = 0; i < nrows; ++i )
   {
      SCIP_ROW* row;
      SCIP_Real ray[4];
      SCIP_Real coefs[5];
      SCIP_Real factor;
      SCIP_Bool israynonzero;
      SCIP_Real cutcoef;
      SCIP_Real interpoint;
      int v;

      row = rows[i];

      /* set factor to store entries of ray as = [BinvL, -BinvU] */
      if( SCIProwGetBasisStatus(row) == SCIP_BASESTAT_LOWER )
         factor = 1.0;
      else if( SCIProwGetBasisStatus(row) == SCIP_BASESTAT_UPPER )
         factor = -1.0;
      else if( SCIProwGetBasisStatus(row) == SCIP_BASESTAT_ZERO )
      {
         *success = FALSE;
         return SCIP_OKAY;
      }
      else
         continue;

      /* build the ray */
      israynonzero = FALSE;
      for( v = 0; v < 4; ++v )
      {
         int index;

         index = ncols + i;

         if( tableaurows[v] != NULL )
            ray[v] = factor * (SCIPisZero(scip, tableaurows[v][index]) ? 0.0 : tableaurows[v][index]);
         else
         {
            /* TODO: We assume that slack variables can never occure in the minor. This is correct, right? */
            ray[v] = 0.0;
         }

         israynonzero = israynonzero || (ray[v] != 0.0);
      }

      /* do nothing if ray is 0 */
      if( ! israynonzero )
         continue;

      /* compute the cut */
      computeRestrictionToRay(scip, ray, vars, coefs);

      /* compute intersection point */
      interpoint = computeRoot(scip, coefs);

      /* compute cut coef */
      cutcoef = SCIPisInfinity(scip, interpoint) ? 0.0 : 1.0 / interpoint;

      /* add var to cut: if variable is nonbasic at upper we have to flip sign of cutcoef */
      assert(SCIProwGetBasisStatus(row) == SCIP_BASESTAT_LOWER || SCIProwGetBasisStatus(row) == SCIP_BASESTAT_UPPER);

      SCIP_CALL( addRowToCut(scip, rowprep, SCIProwGetBasisStatus(row) == SCIP_BASESTAT_UPPER ? cutcoef :
            -cutcoef, row, success) ); /* rows have flipper base status! */
   }

   return SCIP_OKAY;
}

/** separates cuts for stored principal minors */
static
SCIP_RETCODE separateDeterminant(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SEPA*            sepa,               /**< separator */
   SCIP_VAR*             xik,                /**< variable X_ik = x_i * x_k */
   SCIP_VAR*             xil,                /**< variable X_il = x_i * x_l */
   SCIP_VAR*             xjk,                /**< variable X_jk = x_j * x_k */
   SCIP_VAR*             xjl,                /**< variable X_jl = x_j * x_l */
   int*                  basicvarpos2tableaurow,/**< map between basic var and its tableau row */
   SCIP_HASHMAP*         tableau,            /**< map between var an its tableau row */
   SCIP_RESULT*          result              /**< pointer to store the result of the separation call */
   )
{
   SCIP_ROWPREP* rowprep;
   SCIP_VAR* vars[4] = {xik, xil, xjk, xjl};
   SCIP_Real* tableaurows[4];
   SCIP_Bool success;

   /* cut (in the nonbasic space) is of the form alpha^T x >= 1 */
   SCIP_CALL( SCIPcreateRowprep(scip, &rowprep, SCIP_SIDETYPE_LEFT, TRUE) );

   /* check if we have the tableau row of the variable and if not compute it */
   SCIP_CALL( getTableauRows(scip, vars, basicvarpos2tableaurow, tableau, tableaurows) );

   /* loop over each non-basic var; get the ray; compute cut coefficient */
   SCIP_CALL( addCols(scip, vars, tableaurows, rowprep, &success) );

   if( ! success )
      goto CLEANUP;

   /* loop over non-basic slack variables */
   SCIP_CALL( addRows(scip, vars, tableaurows, rowprep, &success) );

   if( ! success )
      goto CLEANUP;

   /* merge coefficients that belong to same variable */
   SCIPmergeRowprepTerms(scip, rowprep);

   SCIP_CALL( SCIPcleanupRowprep(scip, rowprep, NULL, SCIP_CONSEXPR_CUTMAXRANGE, INTERCUTS_MINVIOL, NULL, &success) );

   /* if cleanup was successfull, create row out of rowprep and add it */
   if( success )
   {
      SCIP_ROW* row;
      SCIP_Bool infeasible;

      /* create row */
      SCIP_CALL( SCIPgetRowprepRowSepa(scip, &row, rowprep, sepa) );

      assert(SCIPgetCutEfficacy(scip, NULL, row) > 0.0);

      /* add row */
      SCIP_CALL( SCIPaddRow(scip, row, FALSE, &infeasible) );

      if( infeasible )
         *result = SCIP_CUTOFF;
      else
         *result = SCIP_SEPARATED;

      SCIP_CALL( SCIPreleaseRow(scip, &row) );
   }

CLEANUP:
   SCIPfreeRowprep(scip, &rowprep);
   return SCIP_OKAY;
}


/** separates cuts for stored principal minors */
static
SCIP_RETCODE separatePoint(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SEPA*            sepa,               /**< separator */
   SCIP_RESULT*          result              /**< pointer to store the result of the separation call */
   )
{
   SCIP_SEPADATA* sepadata;
   SCIP_HASHMAP* tableau;
   int* basicvarpos2tableaurow; /* map between basic var and its tableau row */
   int nrows;
   int ncols;
   int i;

   assert(sepa != NULL);
   assert(result != NULL);

   *result = SCIP_DIDNOTRUN;

   sepadata = SCIPsepaGetData(sepa);
   assert(sepadata != NULL);

   /* check whether there are some minors available */
   if( sepadata->nminors == 0 )
      return SCIP_OKAY;

   *result = SCIP_DIDNOTFIND;

   nrows = SCIPgetNLPRows(scip);
   ncols = SCIPgetNLPCols(scip);

   /* allocate memory */
   SCIP_CALL( SCIPallocBufferArray(scip, &basicvarpos2tableaurow, ncols) );
   SCIP_CALL( SCIPhashmapCreate(&tableau, SCIPblkmem(scip), SCIPgetNVars(scip)) );

   /* construct basicvar to tableau row map */
   SCIP_CALL( constructBasicVars2TableauRowMap(scip, basicvarpos2tableaurow) );

   /* loop over the minors and if they are violated build cut */
   for( i = 0; i < sepadata->nminors && (*result != SCIP_CUTOFF); ++i )
   {
      SCIP_VAR* auxvarxik;
      SCIP_VAR* auxvarxil;
      SCIP_VAR* auxvarxjk;
      SCIP_VAR* auxvarxjl;
      SCIP_Real solxik;
      SCIP_Real solxil;
      SCIP_Real solxjk;
      SCIP_Real solxjl;
      SCIP_Real det;

      /* get variables of the i-th minor */
      SCIP_CALL( getMinorVars(sepadata, i, &auxvarxik, &auxvarxil, &auxvarxjk, &auxvarxjl) );

      /* get current solution values */
      solxik = SCIPvarGetLPSol(auxvarxik);
      solxil = SCIPvarGetLPSol(auxvarxil);
      solxjk = SCIPvarGetLPSol(auxvarxjk);
      solxjl = SCIPvarGetLPSol(auxvarxjl);

      det = solxik * solxjl - solxil * solxjk;

      if( SCIPisFeasPositive(scip, det) )
      {
         printf("separate xik xjl - xil xjk <= 0; det is %g\n", det);
         SCIP_CALL( separateDeterminant(scip, sepa, auxvarxik, auxvarxil, auxvarxjk, auxvarxjl, basicvarpos2tableaurow,
                  tableau, result) );
      }
      else if( SCIPisFeasNegative(scip, det) )
      {
         printf("separate xil xjk - xik xjl <= 0; det is %g\n", det);
         SCIP_CALL( separateDeterminant(scip, sepa, auxvarxil, auxvarxik, auxvarxjl, auxvarxjk, basicvarpos2tableaurow,
                  tableau, result) );
      }
      else
         continue;
   }

   /* free memory */
   SCIPhashmapFree(&tableau);
   SCIPfreeBufferArray(scip, &basicvarpos2tableaurow);

   return SCIP_OKAY;
}

/*
 * Callback methods of separator
 */

/** copy method for separator plugins (called when SCIP copies plugins) */
static
SCIP_DECL_SEPACOPY(sepaCopyMinor)
{  /*lint --e{715}*/
   assert(scip != NULL);
   assert(sepa != NULL);
   assert(strcmp(SCIPsepaGetName(sepa), SEPA_NAME) == 0);

   /* call inclusion method of constraint handler */
   SCIP_CALL( SCIPincludeSepaInterminor(scip) );

   return SCIP_OKAY;
}


/** destructor of separator to free user data (called when SCIP is exiting) */
static
SCIP_DECL_SEPAFREE(sepaFreeMinor)
{  /*lint --e{715}*/
   SCIP_SEPADATA* sepadata;

   sepadata = SCIPsepaGetData(sepa);
   assert(sepadata != NULL);
   assert(sepadata->minors == NULL);
   assert(sepadata->nminors == 0);
   assert(sepadata->minorssize == 0);

   /* free separator data */
   SCIPfreeBlockMemory(scip, &sepadata);
   SCIPsepaSetData(sepa, NULL);

   return SCIP_OKAY;
}


/** initialization method of separator (called after problem was transformed) */
static
SCIP_DECL_SEPAINIT(sepaInitMinor)
{  /*lint --e{715}*/
   SCIP_SEPADATA* sepadata;

   /* get separator data */
   sepadata = SCIPsepaGetData(sepa);
   assert(sepadata != NULL);
   assert(sepadata->randnumgen == NULL);

   /* create random number generator */
   SCIP_CALL( SCIPcreateRandom(scip, &sepadata->randnumgen, DEFAULT_RANDSEED, TRUE) );

   return SCIP_OKAY;
}


/** deinitialization method of separator (called before transformed problem is freed) */
static
SCIP_DECL_SEPAEXIT(sepaExitMinor)
{  /*lint --e{715}*/
   SCIP_SEPADATA* sepadata;

   /* get separator data */
   sepadata = SCIPsepaGetData(sepa);
   assert(sepadata != NULL);
   assert(sepadata->randnumgen != NULL);

   /* free random number generator */
   SCIPfreeRandom(scip, &sepadata->randnumgen);

   return SCIP_OKAY;
}


/** solving process initialization method of separator (called when branch and bound process is about to begin) */
static
SCIP_DECL_SEPAINITSOL(sepaInitsolMinor)
{  /*lint --e{715}*/
   return SCIP_OKAY;
}


/** solving process deinitialization method of separator (called before branch and bound process data is freed) */
static
SCIP_DECL_SEPAEXITSOL(sepaExitsolMinor)
{  /*lint --e{715}*/
   SCIP_SEPADATA* sepadata;

   sepadata = SCIPsepaGetData(sepa);
   assert(sepadata != NULL);

   /* clear separation data */
   SCIP_CALL( sepadataClear(scip, sepadata) );

   return SCIP_OKAY;
}


/** LP solution separation method of separator */
static
SCIP_DECL_SEPAEXECLP(sepaExeclpMinor)
{  /*lint --e{715}*/
   SCIP_SEPADATA* sepadata;
   int ncalls;
   int depth;

   /* need routine to compute eigenvalues/eigenvectors */
   if( !SCIPisIpoptAvailableIpopt() )
      return SCIP_OKAY;

   sepadata = SCIPsepaGetData(sepa);
   assert(sepadata != NULL);
   depth = SCIPgetDepth(scip);
   ncalls = SCIPsepaGetNCallsAtNode(sepa);

   /* only call the separator a given number of times at each node */
   if( (depth == 0 && sepadata->maxroundsroot >= 0 && ncalls >= sepadata->maxroundsroot)
      || (depth > 0 && sepadata->maxrounds >= 0 && ncalls >= sepadata->maxrounds) )
   {
      SCIPdebugMsg(scip, "reached round limit for node\n");
      return SCIP_OKAY;
   }

   /* try to detect minors */
   SCIP_CALL( detectMinors(scip, sepadata) );

   /* call separation method */
   SCIP_CALL( separatePoint(scip, sepa, result) );

   return SCIP_OKAY;
}


/*
 * separator specific interface methods
 */

/** creates the minor separator and includes it in SCIP */
SCIP_RETCODE SCIPincludeSepaInterminor(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_SEPADATA* sepadata = NULL;
   SCIP_SEPA* sepa = NULL;

   /* create minor separator data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &sepadata) );
   BMSclearMemory(sepadata);

   /* include separator */
   SCIP_CALL( SCIPincludeSepaBasic(scip, &sepa, SEPA_NAME, SEPA_DESC, SEPA_PRIORITY, SEPA_FREQ, SEPA_MAXBOUNDDIST,
         SEPA_USESSUBSCIP, SEPA_DELAY,
         sepaExeclpMinor, NULL,
         sepadata) );

   assert(sepa != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetSepaCopy(scip, sepa, sepaCopyMinor) );
   SCIP_CALL( SCIPsetSepaFree(scip, sepa, sepaFreeMinor) );
   SCIP_CALL( SCIPsetSepaInit(scip, sepa, sepaInitMinor) );
   SCIP_CALL( SCIPsetSepaExit(scip, sepa, sepaExitMinor) );
   SCIP_CALL( SCIPsetSepaInitsol(scip, sepa, sepaInitsolMinor) );
   SCIP_CALL( SCIPsetSepaExitsol(scip, sepa, sepaExitsolMinor) );

   /* add minor separator parameters */
   SCIP_CALL( SCIPaddIntParam(scip,
         "separating/" SEPA_NAME "/maxminorsconst",
         "constant for the maximum number of minors, i.e., max(const, fac * # quadratic terms)",
         &sepadata->maxminorsconst, FALSE, DEFAULT_MAXMINORSCONST, 0, INT_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddRealParam(scip,
         "separating/" SEPA_NAME "/maxminorsfac",
         "factor for the maximum number of minors, i.e., max(const, fac * # quadratic terms)",
         &sepadata->maxminorsfac, FALSE, DEFAULT_MAXMINORSFAC, 0.0, SCIP_REAL_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddRealParam(scip,
         "separating/" SEPA_NAME "/mincutviol",
         "minimum required violation of a cut",
         &sepadata->mincutviol, FALSE, DEFAULT_MINCUTVIOL, 0.0, SCIP_REAL_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddIntParam(scip,
         "separating/" SEPA_NAME "/maxrounds",
         "maximal number of separation rounds per node (-1: unlimited)",
         &sepadata->maxrounds, FALSE, DEFAULT_MAXROUNDS, -1, INT_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddIntParam(scip,
         "separating/" SEPA_NAME "/maxroundsroot",
         "maximal number of separation rounds in the root node (-1: unlimited)",
         &sepadata->maxroundsroot, FALSE, DEFAULT_MAXROUNDSROOT, -1, INT_MAX, NULL, NULL) );

   SCIP_CALL( SCIPaddBoolParam(scip,
         "separating/" SEPA_NAME "/ignorepackingconss",
         "whether to ignore circle packing constraints during minor detection",
         &sepadata->ignorepackingconss, FALSE, DEFAULT_IGNOREPACKINGCONSS, NULL, NULL) );

   return SCIP_OKAY;
}
