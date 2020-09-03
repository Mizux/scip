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
/*  along with SCIP; see the file COPYING. If not visit scipopt.org.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   cutsel_default.c
 * @ingroup DEFPLUGINS_CUTSEL
 * @brief  default cut selector
 * @author Leona Gottwald
 * @author Mark Turner
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <assert.h>

#include "scip/scip_cutsel.h"
#include "scip/scip_cut.h"
#include "scip/scip_lp.h"
#include "scip/scip_randnumgen.h"
#include "scip/cutsel_default.h"


/* TODO: change name to something meaningful, unlike wsum */
#define CUTSEL_NAME              "default"
#define CUTSEL_DESC              "TODO"
#define CUTSEL_PRIORITY                 0

#define RANDSEED                  0x5EED
#define GOODSCORE                 0.9
#define BADSCORE                  0.0

#define DEFAULT_EFFICACYWEIGHT    0.6 /**< weight of efficacy in score calculation */
#define DEFAULT_DIRCUTOFFDISTWEIGHT  0.5 /**< weight of directed cutoff distance in score calculation */
#define DEFAULT_OBJPARALWEIGHT    0.1 /**< weight of objective parallelism in score calculation */
#define DEFAULT_INTSUPPORTWEIGHT  0.1 /**< weight of integral support in cut score calculation */
#define DEFAULT_MINORTHO          0.90 /**< minimal orthogonality for a cut to enter the LP */
#define DEFAULT_MINORTHOROOT      0.90 /**< minimal orthogonality for a cut to enter the LP in the root node */

/*
 * Data structures
 */

/* TODO: fill in the necessary cut selector data */

/** cut selector data */
struct SCIP_CutselData
{
   SCIP_RANDNUMGEN*      randnumgen;         /**< random generator for tiebreaking */
   SCIP_Real             goodscore;          /**< threshold for score of cut relative to best score to be considered good,
                                              *   so that less strict filtering is applied */
   SCIP_Real             badscore;           /**< threshold for score of cut relative to best score to be discarded */
   SCIP_Real             objparalweight;     /**< weight of objective parallelism in cut score calculation */
   SCIP_Real             efficacyweight;     /**< weight of efficacy in cut score calculation */
   SCIP_Real             dircutoffdistweight;/**< weight of directed cutoff distance in cut score calculation */
   SCIP_Real             intsupportweight;   /**< weight of integral support in cut score calculation */
   SCIP_Real             goodmaxparall;      /**< maximum parallelism for good cuts */
   SCIP_Real             maxparall;          /**< maximum parallelism for non-good cuts */
   SCIP_Real             minortho;           /**< minimal orthogonality for a cut to enter the LP */
   SCIP_Real             minorthoroot;       /**< minimal orthogonality for a cut to enter the LP in the root node */
   //TODO: maybe this information should be asked from some global info
   int                   maxsepacuts;        /**< maximal number of cuts separated per separation round */
   int                   maxsepacutsroot;    /**< maximal number of cuts separated per separation round in root node */
};


/*
 * Local methods
 */

/** move the cut with the highest score to the first position in the array; there must be at least one cut */
static
void selectBestCut(
   SCIP_ROW**            cuts,               /**< array with cuts to perform selection algorithm */
   SCIP_Real*            scores,             /**< array with scores of cuts to perform selection algorithm */
   int                   ncuts               /**< number of cuts in given array */
   )
{
   int i;
   int bestpos;
   SCIP_Real bestscore;

   assert(ncuts > 0);
   assert(cuts != NULL);
   assert(scores != NULL);

   bestscore = scores[0];
   bestpos = 0;

   for( i = 1; i < ncuts; ++i )
   {
      if( scores[i] > bestscore )
      {
         bestpos = i;
         bestscore = scores[i];
      }
   }

   SCIPswapPointers((void**) &cuts[bestpos], (void**) &cuts[0]);
   SCIPswapReals(&scores[bestpos], &scores[0]);
}

/** filters the given array of cuts to enforce a maximum parallelism constraints
 *  for the given cut; moves filtered cuts to the end of the array and returns number of selected cuts */
static
int filterWithParallelism(
   SCIP_ROW*             cut,                /**< cut to filter orthogonality with */
   SCIP_ROW**            cuts,               /**< array with cuts to perform selection algorithm */
   SCIP_Real*            scores,             /**< array with scores of cuts to perform selection algorithm */
   int                   ncuts,              /**< number of cuts in given array */
   SCIP_Real             goodscore,          /**< threshold for the score to be considered a good cut */
   SCIP_Real             goodmaxparall,      /**< maximal parallelism for good cuts */
   SCIP_Real             maxparall           /**< maximal parallelism for all cuts that are not good */
   )
{
   int i;

   assert( cut != NULL );
   assert( ncuts == 0 || cuts != NULL );
   assert( ncuts == 0 || scores != NULL );

   for( i = ncuts - 1; i >= 0; --i )
   {
      SCIP_Real thisparall;
      SCIP_Real thismaxparall;

      thisparall = SCIProwGetParallelism(cut, cuts[i], 'e');
      thismaxparall = scores[i] >= goodscore ? goodmaxparall : maxparall;

      if( thisparall > thismaxparall )
      {
         --ncuts;
         SCIPswapPointers((void**) &cuts[i], (void**) &cuts[ncuts]);
         SCIPswapReals(&scores[i], &scores[ncuts]);
      }
   }

   return ncuts;
}


/*
 * Callback methods of cut selector
 */

/* TODO: Implement all necessary cut selector methods. The methods with an #if 0 ... #else #define ... are optional */


/** copy method for cut selector plugin (called when SCIP copies plugins) */
#if 0
static
SCIP_DECL_CUTSELCOPY(cutselCopyDefault)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of default cut selector not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define cutselCopyDefault NULL
#endif

/** destructor of cut selector to free user data (called when SCIP is exiting) */
static
SCIP_DECL_CUTSELFREE(cutselFreeDefault)
{  /*lint --e{715}*/
   SCIP_CUTSELDATA* cutseldata;

   cutseldata = SCIPcutselGetData(cutsel);

   //TODO free randnumber generator or in EXIT?

   SCIPfreeBlockMemory(scip, &cutseldata);

   return SCIP_OKAY;
}


/** initialization method of cut selector (called after problem was transformed) */
#if 0
static
SCIP_DECL_CUTSELINIT(cutselInitDefault)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of default cut selector not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define cutselInitDefault NULL
#endif


/** deinitialization method of cut selector (called before transformed problem is freed) */
#if 0
static
SCIP_DECL_CUTSELEXIT(cutselExitDefault)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of default cut selector not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/
   //TODO free randnumber generator or in FREE??

   return SCIP_OKAY;
}
#else
#define cutselExitDefault NULL
#endif


/** solving process initialization method of cut selector (called when branch and bound process is about to begin) */
#if 0
static
SCIP_DECL_CUTSELINITSOL(cutselInitsolDefault)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of default cut selector not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define cutselInitsolDefault NULL
#endif


/** solving process deinitialization method of cut selector (called before branch and bound process data is freed) */
#if 0
static
SCIP_DECL_CUTSELEXITSOL(cutselExitsolDefault)
{  /*lint --e{715}*/
   SCIPerrorMessage("method of default cut selector not implemented yet\n");
   SCIPABORT(); /*lint --e{527}*/

   return SCIP_OKAY;
}
#else
#define cutselExitsolDefault NULL
#endif


/** cut selection method of cut selector */
static
SCIP_DECL_CUTSELSELECT(cutselSelectDefault)
{  /*lint --e{715}*/
   SCIP_CUTSELDATA* cutseldata;
   SCIP_Real goodmaxparall;
   SCIP_Real maxparall;

   cutseldata = SCIPcutselGetData(cutsel);

   if( root )
   {
      maxparall = 1.0 - cutseldata->minorthoroot;
      goodmaxparall = MAX(0.5, 1.0 - cutseldata->minorthoroot);
   }
   else
   {
      maxparall = 1.0 - cutseldata->minortho;
      goodmaxparall = MAX(0.5, 1.0 - cutseldata->minortho);
   }

   SCIP_CALL( SCIPselectCutsDefault(scip, cuts, cutseldata->randnumgen, cutseldata->goodscore, cutseldata->badscore,
            goodmaxparall, maxparall, cutseldata->dircutoffdistweight, cutseldata->efficacyweight,
            cutseldata->objparalweight, cutseldata->intsupportweight, ncuts, nforcedcuts, maxnselectedcuts, nselectedcuts) );

   return SCIP_OKAY;
}


/*
 * cut selector specific interface methods
 */

/** creates the default cut selector and includes it in SCIP */
SCIP_RETCODE SCIPincludeCutselDefault(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CUTSELDATA* cutseldata;
   SCIP_CUTSEL* cutsel;

   /* create default cut selector data */
   SCIP_CALL( SCIPallocBlockMemory(scip, &cutseldata) );
   BMSclearMemory(cutseldata);
   SCIP_CALL( SCIPcreateRandom(scip, &(cutseldata)->randnumgen, RANDSEED, TRUE) );
   cutseldata->goodscore = GOODSCORE;
   cutseldata->badscore  = BADSCORE;



   /* include cut selector */
#if 0
   /* use SCIPincludeCutsel() if you want to set all callbacks explicitly and realize (by getting compiler errors) when
    * new callbacks are added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeCutsel(scip, CUTSEL_NAME, CUTSEL_DESC, CUTSEL_PRIORITY,
         cutselCopyDefault, cutselFreeDefault, cutselInitDefault, cutselExitDefault, cutselInitsolDefault, cutselExitsolDefault, cutselSelectDefault,
         cutseldata) );
#else
   /* use SCIPincludeCutselBasic() plus setter functions if you want to set callbacks one-by-one and your code should
    * compile independent of new callbacks being added in future SCIP versions
    */
   SCIP_CALL( SCIPincludeCutselBasic(scip, &cutsel, CUTSEL_NAME, CUTSEL_DESC, CUTSEL_PRIORITY, cutselSelectDefault,
            cutseldata) );

   assert(cutsel != NULL);

   /* set non fundamental callbacks via setter functions */
   SCIP_CALL( SCIPsetCutselCopy(scip, cutsel, cutselCopyDefault) );

   //SCIP_CALL( SCIPsetCutselFree(scip, cutsel, cutselFreeDefault) );
   //SCIP_CALL( SCIPsetCutselInit(scip, cutsel, cutselInitDefault) );
   //SCIP_CALL( SCIPsetCutselExit(scip, cutsel, cutselExitDefault) );
   //SCIP_CALL( SCIPsetCutselInitsol(scip, cutsel, cutselInitsolDefault) );
   //SCIP_CALL( SCIPsetCutselExitsol(scip, cutsel, cutselExitsolDefault) );
#endif

   /* add default cut selector parameters */
   SCIP_CALL( SCIPaddRealParam(scip,
            "cutselectors/" CUTSEL_NAME "/efficacyweight",
            "weight of efficacy in cut score calculation",
            &cutseldata->efficacyweight, FALSE, DEFAULT_EFFICACYWEIGHT, 0.0, SCIP_INVALID/10.0, NULL, NULL) );

   SCIP_CALL( SCIPaddRealParam(scip,
         "cutselectors/" CUTSEL_NAME "/dircutoffdistweight",
         "weight of directed cutoff distance in cut score calculation",
         &cutseldata->dircutoffdistweight, FALSE, DEFAULT_DIRCUTOFFDISTWEIGHT, 0.0, SCIP_INVALID/10.0, NULL, NULL) );

   SCIP_CALL( SCIPaddRealParam(scip,
         "cutselectors/" CUTSEL_NAME "/objparalweight",
         "weight of objective parallelism in cut score calculation",
         &cutseldata->objparalweight, FALSE, DEFAULT_OBJPARALWEIGHT, 0.0, SCIP_INVALID/10.0, NULL, NULL) );

   SCIP_CALL( SCIPaddRealParam(scip,
         "cutselectors/" CUTSEL_NAME "/intsupportweight",
         "weight of integral support in cut score calculation",
         &cutseldata->intsupportweight, FALSE, DEFAULT_INTSUPPORTWEIGHT, 0.0, SCIP_INVALID/10.0, NULL, NULL) );

   SCIP_CALL( SCIPaddRealParam(scip,
         "cutselectors/" CUTSEL_NAME "/minortho",
         "minimal orthogonality for a cut to enter the LP",
         &cutseldata->minortho, FALSE, DEFAULT_MINORTHO, 0.0, 1.0, NULL, NULL) );

   SCIP_CALL( SCIPaddRealParam(scip,
         "cutselectors/" CUTSEL_NAME "/minorthoroot",
         "minimal orthogonality for a cut to enter the LP in the root node",
         &cutseldata->minorthoroot, FALSE, DEFAULT_MINORTHOROOT, 0.0, 1.0, NULL, NULL) );

   return SCIP_OKAY;
}

/** perform a cut selection algorithm for the given array of cuts;
 *  this is the selection method of the default cut selector which does a weighted sum of the
 *  efficacy, parallelism, directed cutoff distance, and the integral support.
 *  The input array is partitioned s.t the selected cuts comes first and the remaining
 *  ones are the end.
 */
SCIP_RETCODE SCIPselectCutsDefault(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_ROW**            cuts,               /**< array with cuts to perform selection algorithm */
   SCIP_RANDNUMGEN*      randnumgen,         /**< random number generator for tie-breaking, or NULL */
   SCIP_Real             goodscorefac,       /**< factor of best score among the given cuts to consider a cut good
                                              *   and filter with less strict settings of the maximum parallelism */
   SCIP_Real             badscorefac,        /**< factor of best score among the given cuts to consider a cut bad
                                              *   and discard it regardless of its parallelism to other cuts */
   SCIP_Real             goodmaxparall,      /**< maximum parallelism for good cuts */
   SCIP_Real             maxparall,          /**< maximum parallelism for non-good cuts */
   SCIP_Real             dircutoffdistweight,/**< weight of directed cutoff distance in cut score calculation */
   SCIP_Real             efficacyweight,     /**< weight of efficacy in cut score calculation */
   SCIP_Real             objparalweight,     /**< weight of objective parallelism in cut score calculation */
   SCIP_Real             intsupportweight,   /**< weight of integral support in cut score calculation */
   int                   ncuts,              /**< number of cuts in given array */
   int                   nforcedcuts,        /**< number of forced cuts at start of given array */
   int                   maxselectedcuts,    /**< maximal number of cuts to select */
   int*                  nselectedcuts       /**< pointer to return number of selected cuts */
   )
{
   int i;
   SCIP_Real* scores;
   SCIP_Real goodscore;
   SCIP_Real badscore;
   SCIP_SOL* sol;

   assert( nselectedcuts != NULL );

   /* if all cuts are forced cuts, no selection is required */
   if( nforcedcuts >= MIN(ncuts, maxselectedcuts) )
   {
      *nselectedcuts = nforcedcuts;
      return SCIP_OKAY;
   }
   *nselectedcuts = 0;

   SCIP_CALL( SCIPallocBufferArray(scip, &scores, ncuts) );

   sol = SCIPgetBestSol(scip);

   goodscore = 0.0;

   /* if there is an incumbent and the factor is not 0.0, compute directed cutoff distances for the incumbent */
   if( sol != NULL && dircutoffdistweight > 0.0 )
   {
      for( i = 0; i < ncuts; ++i )
      {
         SCIP_Real objparallelism;
         SCIP_Real intsupport;
         SCIP_Real efficacy;

         intsupport = intsupportweight != 0.0 ?  intsupportweight * SCIPgetRowNumIntCols(scip, cuts[i]) / (SCIP_Real)
            SCIProwGetNNonz(cuts[i]) : 0.0;

         objparallelism = objparalweight != 0.0 ? objparalweight * SCIPgetRowObjParallelism(scip, cuts[i]) : 0.0;

         efficacy = SCIPgetCutEfficacy(scip, NULL, cuts[i]);

         if( SCIProwIsLocal(cuts[i]) )
         {
            scores[i] = dircutoffdistweight * efficacy;
         }
         else
         {
            scores[i] = SCIPgetCutLPSolCutoffDistance(scip, sol, cuts[i]);
            scores[i] = dircutoffdistweight * MAX(scores[i], efficacy);
         }

         efficacy *= efficacyweight;
         scores[i] += objparallelism + intsupport + efficacy;

         /* add small term to prefer global pool cuts */
         scores[i] += SCIProwIsInGlobalCutpool(cuts[i]) ? 1e-4 : 0.0;

         if( randnumgen != NULL )
         {
            scores[i] += SCIPrandomGetReal(randnumgen, 0.0, 1e-6);
         }

         goodscore = MAX(goodscore, scores[i]);
      }
   }
   else
   {
      /* in case there is no solution add the directed cutoff distance weight to the efficacy weight
       * since the efficacy underestimates the directed cuttoff distance
       */
      efficacyweight += dircutoffdistweight;
      for( i = 0; i < ncuts; ++i )
      {
         SCIP_Real objparallelism;
         SCIP_Real intsupport;
         SCIP_Real efficacy;

         intsupport = intsupportweight > 0.0 ?  intsupportweight * SCIPgetRowNumIntCols(scip, cuts[i]) / (SCIP_Real)
            SCIProwGetNNonz(cuts[i]) : 0.0;

         objparallelism = objparalweight > 0.0 ? objparalweight * SCIPgetRowObjParallelism(scip, cuts[i]) : 0.0;

         efficacy = efficacyweight > 0.0 ?  efficacyweight * SCIPgetCutEfficacy(scip, NULL, cuts[i]) : 0.0;

         scores[i] = objparallelism + intsupport + efficacy;

         /* add small term to prefer global pool cuts */
         scores[i] += SCIProwIsInGlobalCutpool(cuts[i]) ? 1e-4 : 0.0;

         if( randnumgen != NULL )
         {
            scores[i] += SCIPrandomGetReal(randnumgen, 0.0, 1e-6);
         }

         goodscore = MAX(goodscore, scores[i]);
      }
   }

   /* compute values for filtering cuts */
   badscore = goodscore * badscorefac;
   goodscore *= goodscorefac;

   /* perform cut selection algorithm for the cuts */
   {
      int nnonforcedcuts;
      SCIP_ROW** nonforcedcuts;
      SCIP_Real* nonforcedscores;

      /* adjust pointers to the beginning of the non-forced cuts */
      nnonforcedcuts = ncuts - nforcedcuts;
      nonforcedcuts = cuts + nforcedcuts;
      nonforcedscores = scores + nforcedcuts;

      /* select the forced cuts first */
      *nselectedcuts = nforcedcuts;
      for( i = 0; i < nforcedcuts && nnonforcedcuts > 0; ++i )
      {
         nnonforcedcuts = filterWithParallelism(cuts[i], nonforcedcuts, nonforcedscores, nnonforcedcuts, goodscore, goodmaxparall, maxparall);
      }

      /* now greedily select the remaining cuts */
      while( nnonforcedcuts > 0 )
      {
         SCIP_ROW* selectedcut;

         selectBestCut(nonforcedcuts, nonforcedscores, nnonforcedcuts);
         selectedcut = nonforcedcuts[0];

         /* if the best cut of the remaining cuts is considered bad, we discard it and all remaining cuts */
         if( nonforcedscores[0] < badscore )
            goto TERMINATE;

         ++(*nselectedcuts);

         /* if the maximal number of cuts was selected, we can stop here */
         if( *nselectedcuts == maxselectedcuts )
            goto TERMINATE;

         /* move the pointers to the next position and filter the remaining cuts to enforce the maximum parallelism constraint */
         ++nonforcedcuts;
         ++nonforcedscores;
         --nnonforcedcuts;

         nnonforcedcuts = filterWithParallelism(selectedcut, nonforcedcuts, nonforcedscores, nnonforcedcuts, goodscore, goodmaxparall, maxparall);
      }
   }

  TERMINATE:
   SCIPfreeBufferArray(scip, &scores);

   return SCIP_OKAY;
}
