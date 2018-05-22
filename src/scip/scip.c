/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2018 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   scip.c
 * @brief  SCIP callable library
 * @author Tobias Achterberg
 * @author Timo Berthold
 * @author Gerald Gamrath
 * @author Stefan Heinz
 * @author Gregor Hendel
 * @author Thorsten Koch
 * @author Alexander Martin
 * @author Marc Pfetsch
 * @author Michael Winkler
 * @author Kati Wolter
 *
 * @todo check all SCIPcheckStage() calls, use bit flags instead of the SCIP_Bool parameters
 * @todo check all SCIP_STAGE_* switches, and include the new stages TRANSFORMED and INITSOLVE
 * @todo When making an interface change to SCIPcreateProb(), add an indication whether it is known that the objective
 *       function is integral or whether this is not known. This avoids a manual call of SCIPsetObjIntegral(). Moreover,
 *       then the detection could be performed incrementally, whenever a variable is added.
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#if defined(_WIN32) || defined(_WIN64)
#else
#include <strings.h> /*lint --e{766}*/
#endif

#ifdef WITH_ZLIB
#include <zlib.h>
#endif

#include "scip/def.h"
#include "scip/retcode.h"
#include "scip/set.h"
#include "scip/paramset.h"
#include "scip/stat.h"
#include "scip/clock.h"
#include "scip/visual.h"
#include "scip/interrupt.h"
#include "scip/mem.h"
#include "scip/misc.h"
#include "scip/history.h"
#include "scip/event.h"
#include "scip/lp.h"
#include "scip/nlp.h"
#include "scip/var.h"
#include "scip/implics.h"
#include "scip/prob.h"
#include "scip/sol.h"
#include "scip/primal.h"
#include "scip/reopt.h"
#include "scip/tree.h"
#include "scip/pricestore.h"
#include "scip/sepastore.h"
#include "scip/conflictstore.h"
#include "scip/syncstore.h"
#include "scip/cutpool.h"
#include "scip/solve.h"
#include "scip/scipbuildflags.h"
#include "scip/scipgithash.h"
#include "scip/cuts.h"
#include "scip/scip.h"
#include "lpi/lpi.h"

#include "scip/scipcoreplugins.h"
#include "scip/branch.h"
#include "scip/conflict.h"
#include "scip/cons.h"
#include "scip/dialog.h"
#include "scip/disp.h"
#include "scip/table.h"
#include "scip/heur.h"
#include "scip/concsolver.h"
#include "scip/compr.h"
#include "scip/nodesel.h"
#include "scip/reader.h"
#include "scip/presol.h"
#include "scip/pricer.h"
#include "scip/relax.h"
#include "scip/sepa.h"
#include "scip/prop.h"
#include "nlpi/nlpi.h"
#include "nlpi/exprinterpret.h"
#include "scip/debug.h"
#include "scip/dialog_default.h"
#include "scip/message_default.h"
#include "scip/syncstore.h"
#include "scip/concurrent.h"
#include "scip/benders.h"
#include "scip/benderscut.h"
#include "xml/xml.h"

/* We include the linear constraint handler to be able to copy a (multi)aggregation of variables (to a linear constraint).
 * The better way would be to handle the distinction between original and transformed variables via a flag 'isoriginal'
 * in the variable data structure. This would allow to have (multi)aggregated variables in the original problem.
 *
 * A second reason for including the linear constraint handler is for copying cuts to linear constraints.
 */
#include "scip/cons_linear.h"

/* We need to include the branching and the heurtistics for reoptimization after creating the reoptimization because we
 * do not want to use these plugins by default if reoptimization is disabled. */
#include "scip/branch_nodereopt.h"
#include "scip/heur_reoptsols.h"
#include "scip/heur_trivialnegation.h"
#include "scip/heur_ofins.h"

/* In debug mode, we include the SCIP's structure in scip.c, such that no one can access
 * this structure except the interface methods in scip.c.
 * In optimized mode, the structure is included in scip.h, because some of the methods
 * are implemented as defines for performance reasons (e.g. the numerical comparisons)
 */
#ifndef NDEBUG
#include "scip/struct_scip.h"
#endif

/*
 * miscellaneous methods
 */


/*
 * message output methods
 */



/*
 * SCIP copy methods
 */



/*
 * parameter settings
 */





/*
 * SCIP user functionality methods: managing plugins
 */







#undef SCIPgetSepaMinEfficacy





/* new callback/method setter methods */

/* new callback/method setter methods */







/** creates a statistics table and includes it in SCIP */
SCIP_RETCODE SCIPincludeTable(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           name,               /**< name of statistics table */
   const char*           desc,               /**< description of statistics table */
   SCIP_Bool             active,             /**< should the table be activated by default? */
   SCIP_DECL_TABLECOPY   ((*tablecopy)),     /**< copy method of statistics table or NULL if you don't want to copy your plugin into sub-SCIPs */
   SCIP_DECL_TABLEFREE   ((*tablefree)),     /**< destructor of statistics table */
   SCIP_DECL_TABLEINIT   ((*tableinit)),     /**< initialize statistics table */
   SCIP_DECL_TABLEEXIT   ((*tableexit)),     /**< deinitialize statistics table */
   SCIP_DECL_TABLEINITSOL ((*tableinitsol)), /**< solving process initialization method of statistics table */
   SCIP_DECL_TABLEEXITSOL ((*tableexitsol)), /**< solving process deinitialization method of statistics table */
   SCIP_DECL_TABLEOUTPUT ((*tableoutput)),   /**< output method */
   SCIP_TABLEDATA*       tabledata,          /**< statistics table data */
   int                   position,           /**< position of statistics table */
   SCIP_STAGE            earlieststage       /**< output of the statistics table is only printed from this stage onwards */
   )
{
   SCIP_TABLE* table;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPincludeTable", TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   /* check whether statistics table is already present */
   if( SCIPfindTable(scip, name) != NULL )
   {
      SCIPerrorMessage("statistics table <%s> already included.\n", name);
      return SCIP_INVALIDDATA;
   }

   SCIP_CALL( SCIPtableCreate(&table, scip->set, scip->messagehdlr, scip->mem->setmem,
         name, desc, active, tablecopy,
         tablefree, tableinit, tableexit, tableinitsol, tableexitsol, tableoutput, tabledata,
         position, earlieststage) );
   SCIP_CALL( SCIPsetIncludeTable(scip->set, table) );

   return SCIP_OKAY;
}

/** returns the statistics table of the given name, or NULL if not existing */
SCIP_TABLE* SCIPfindTable(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           name                /**< name of statistics table */
   )
{
   assert(scip != NULL);
   assert(scip->set != NULL);
   assert(name != NULL);

   return SCIPsetFindTable(scip->set, name);
}

/** returns the array of currently available statistics tables */
SCIP_TABLE** SCIPgetTables(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);
   assert(scip->set != NULL);

   return scip->set->tables;
}

/** returns the number of currently available statistics tables */
int SCIPgetNTables(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);
   assert(scip->set != NULL);

   return scip->set->ntables;
}

/** method to call, when the priority of an NLPI was changed */
static
SCIP_DECL_PARAMCHGD(paramChgdNlpiPriority)
{  /*lint --e{715}*/
   SCIP_PARAMDATA* paramdata;

   paramdata = SCIPparamGetData(param);
   assert(paramdata != NULL);

   /* use SCIPsetSetPriorityNlpi() to mark the nlpis unsorted */
   SCIP_CALL( SCIPsetNlpiPriority(scip, (SCIP_NLPI*)paramdata, SCIPparamGetInt(param)) );

   return SCIP_OKAY;
}



/*
 * user interactive dialog methods
 */


/*
 * global problem methods
 */





/*
 * local subproblem methods
 */




/*
 * solve methods
 */

/** checks solution for feasibility in original problem without adding it to the solution store; to improve the
 *  performance we use the following order when checking for violations:
 *
 *  1. variable bounds
 *  2. constraint handlers with positive or zero priority that don't need constraints (e.g. integral constraint handler)
 *  3. original constraints
 *  4. constraint handlers with negative priority that don't need constraints (e.g. Benders' decomposition constraint handler)
 */
static
SCIP_RETCODE checkSolOrig(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Bool*            feasible,           /**< stores whether given solution is feasible */
   SCIP_Bool             printreason,        /**< Should the reason for the violation be printed? */
   SCIP_Bool             completely,         /**< Should all violations be checked? */
   SCIP_Bool             checkbounds,        /**< Should the bounds of the variables be checked? */
   SCIP_Bool             checkintegrality,   /**< Has integrality to be checked? */
   SCIP_Bool             checklprows,        /**< Do constraints represented by rows in the current LP have to be checked? */
   SCIP_Bool             checkmodifiable     /**< have modifiable constraint to be checked? */
   )
{
   SCIP_RESULT result;
   int v;
   int c;
   int h;

   assert(scip != NULL);
   assert(sol != NULL);
   assert(feasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "checkSolOrig", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   *feasible = TRUE;

   SCIPsolResetViolations(sol);

   if( !printreason )
      completely = FALSE;

   /* check bounds */
   if( checkbounds )
   {
      for( v = 0; v < scip->origprob->nvars; ++v )
      {
         SCIP_VAR* var;
         SCIP_Real solval;
         SCIP_Real lb;
         SCIP_Real ub;

         var = scip->origprob->vars[v];
         solval = SCIPsolGetVal(sol, scip->set, scip->stat, var);

         lb = SCIPvarGetLbOriginal(var);
         ub = SCIPvarGetUbOriginal(var);

         SCIPupdateSolBoundViolation(scip, sol, lb - solval, SCIPrelDiff(lb, solval));
         SCIPupdateSolBoundViolation(scip, sol, solval - ub, SCIPrelDiff(solval, ub));

         if( SCIPsetIsFeasLT(scip->set, solval, lb) || SCIPsetIsFeasGT(scip->set, solval, ub) )
         {
            *feasible = FALSE;

            if( printreason )
            {
               SCIPmessagePrintInfo(scip->messagehdlr, "solution violates original bounds of variable <%s> [%g,%g] solution value <%g>\n",
                  SCIPvarGetName(var), lb, ub, solval);
            }

            if( !completely )
               return SCIP_OKAY;
         }
      }
   }

   /* call constraint handlers with positive or zero check priority that don't need constraints */
   for( h = 0; h < scip->set->nconshdlrs; ++h )
   {
      if( SCIPconshdlrGetCheckPriority(scip->set->conshdlrs[h]) >= 0 )
      {
         if( !SCIPconshdlrNeedsCons(scip->set->conshdlrs[h]) )
         {
            SCIP_CALL( SCIPconshdlrCheck(scip->set->conshdlrs[h], scip->mem->probmem, scip->set, scip->stat, sol,
                  checkintegrality, checklprows, printreason, completely, &result) );

            if( result != SCIP_FEASIBLE )
            {
               *feasible = FALSE;

               if( !completely )
                  return SCIP_OKAY;
            }
         }
      }
      /* constraint handlers are sorted by priority, so we can break when reaching the first one with negative priority */
      else
         break;
   }

   /* check original constraints
    *
    * in general modifiable constraints can not be checked, because the variables to fulfill them might be missing in
    * the original problem; however, if the solution comes from a heuristic during presolving modifiable constraints
    * have to be checked;
    */
   for( c = 0; c < scip->origprob->nconss; ++c )
   {
      if( SCIPconsIsChecked(scip->origprob->conss[c]) && (checkmodifiable || !SCIPconsIsModifiable(scip->origprob->conss[c])) )
      {
         /* check solution */
         SCIP_CALL( SCIPconsCheck(scip->origprob->conss[c], scip->set, sol,
               checkintegrality, checklprows, printreason, &result) );

         if( result != SCIP_FEASIBLE )
         {
            *feasible = FALSE;

            if( !completely )
               return SCIP_OKAY;
         }
      }
   }

   /* call constraint handlers with negative check priority that don't need constraints;
    * continue with the first constraint handler with negative priority which caused us to break in the above loop */
   for( ; h < scip->set->nconshdlrs; ++h )
   {
      assert(SCIPconshdlrGetCheckPriority(scip->set->conshdlrs[h]) < 0);
      if( !SCIPconshdlrNeedsCons(scip->set->conshdlrs[h]) )
      {
         SCIP_CALL( SCIPconshdlrCheck(scip->set->conshdlrs[h], scip->mem->probmem, scip->set, scip->stat, sol,
               checkintegrality, checklprows, printreason, completely, &result) );

         if( result != SCIP_FEASIBLE )
         {
            *feasible = FALSE;

            if( !completely )
               return SCIP_OKAY;
         }
      }
   }

   return SCIP_OKAY;
}

/** update integrality violation of a solution */
void SCIPupdateSolIntegralityViolation(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Real             absviol             /**< absolute violation */
   )
{
   if( SCIPprimalUpdateViolations(scip->origprimal) )
      SCIPsolUpdateIntegralityViolation(sol, absviol);
}

/** update bound violation of a solution */
void SCIPupdateSolBoundViolation(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Real             absviol,            /**< absolute violation */
   SCIP_Real             relviol             /**< relative violation */
   )
{
   if( SCIPprimalUpdateViolations(scip->origprimal) )
      SCIPsolUpdateBoundViolation(sol, absviol, relviol);
}

/** update LP row violation of a solution */
void SCIPupdateSolLPRowViolation(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Real             absviol,            /**< absolute violation */
   SCIP_Real             relviol             /**< relative violation */
   )
{
   if( SCIPprimalUpdateViolations(scip->origprimal) )
      SCIPsolUpdateLPRowViolation(sol, absviol, relviol);
}

/** update constraint violation of a solution */
void SCIPupdateSolConsViolation(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Real             absviol,            /**< absolute violation */
   SCIP_Real             relviol             /**< relative violation */
   )
{
   if( SCIPprimalUpdateViolations(scip->origprimal) )
      SCIPsolUpdateConsViolation(sol, absviol, relviol);
}

/** update LP row and constraint violations of a solution */
void SCIPupdateSolLPConsViolation(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Real             absviol,            /**< absolute violation */
   SCIP_Real             relviol             /**< relative violation */
   )
{
   if( SCIPprimalUpdateViolations(scip->origprimal) )
      SCIPsolUpdateLPConsViolation(sol, absviol, relviol);
}

/** allow violation updates */
void SCIPactivateSolViolationUpdates(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIPprimalSetUpdateViolations(scip->origprimal, TRUE);
}

/** disallow violation updates */
void SCIPdeactivateSolViolationUpdates(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIPprimalSetUpdateViolations(scip->origprimal, FALSE);
}

/** calculates number of nonzeros in problem */
static
SCIP_RETCODE calcNonZeros(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Longint*         nchecknonzeros,     /**< pointer to store number of non-zeros in all check constraints */
   SCIP_Longint*         nactivenonzeros,    /**< pointer to store number of non-zeros in all active constraints */
   SCIP_Bool*            approxchecknonzeros,/**< pointer to store if the number of non-zeros in all check constraints
                                              *   is only a lowerbound
                                              */
   SCIP_Bool*            approxactivenonzeros/**< pointer to store if the number of non-zeros in all active constraints
                                              *   is only a lowerbound
                                              */
   )
{
   SCIP_CONS** conss;
   SCIP_Bool success;
   SCIP_Bool ischeck;
   int nconss;
   int nvars;
   int c;
   int h;

   *nchecknonzeros = 0LL;
   *nactivenonzeros = 0LL;
   *approxchecknonzeros = FALSE;
   *approxactivenonzeros = FALSE;

   /* computes number of non-zeros over all active constraints */
   for( h = scip->set->nconshdlrs - 1; h >= 0; --h )
   {
      nconss = SCIPconshdlrGetNActiveConss(scip->set->conshdlrs[h]);

      if( nconss > 0 )
      {
         conss = SCIPconshdlrGetConss(scip->set->conshdlrs[h]);

         /* calculate all active constraints */
         for( c = nconss - 1; c >= 0; --c )
         {
            SCIP_CALL( SCIPconsGetNVars(conss[c], scip->set, &nvars, &success) );
            ischeck = SCIPconsIsChecked(conss[c]);

            if( !success )
            {
               *approxactivenonzeros = TRUE;
               if( ischeck )
                  *approxchecknonzeros = TRUE;
            }
            else
            {
               *nactivenonzeros += nvars;
               if( ischeck )
                  *nchecknonzeros += nvars;
            }
         }
      }

      /* add nonzeros on inactive check constraints */
      nconss = SCIPconshdlrGetNCheckConss(scip->set->conshdlrs[h]);
      if( nconss > 0 )
      {
         conss = SCIPconshdlrGetCheckConss(scip->set->conshdlrs[h]);

         for( c = nconss - 1; c >= 0; --c )
         {
            if( !SCIPconsIsActive(conss[c]) )
            {
               SCIP_CALL( SCIPconsGetNVars(conss[c], scip->set, &nvars, &success) );

               if( !success )
                  *approxchecknonzeros = TRUE;
               else
                  *nchecknonzeros += nvars;
            }
         }
      }
   }

   return SCIP_OKAY;
}


/** initializes solving data structures and transforms problem
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *       - \ref SCIP_STAGE_FREE
 *
 *  @post When calling this method in the \ref SCIP_STAGE_PROBLEM stage, the \SCIP stage is changed to \ref
 *        SCIP_STAGE_TRANSFORMED; otherwise, the stage is not changed
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPtransformProb(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Longint oldnsolsfound;
   int nfeassols;
   int ncandsols;
   int h;
   int s;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtransformProb", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE) );

   /* check, if the problem was already transformed */
   if( scip->set->stage >= SCIP_STAGE_TRANSFORMED )
      return SCIP_OKAY;

   assert(scip->stat->status == SCIP_STATUS_UNKNOWN);

   /* check, if a node selector exists */
   if( SCIPsetGetNodesel(scip->set, scip->stat) == NULL )
   {
      SCIPerrorMessage("no node selector available\n");
      return SCIP_PLUGINNOTFOUND;
   }

   /* call garbage collector on original problem and parameter settings memory spaces */
   BMSgarbagecollectBlockMemory(scip->mem->setmem);
   BMSgarbagecollectBlockMemory(scip->mem->probmem);

   /* remember number of constraints */
   SCIPprobMarkNConss(scip->origprob);

   /* switch stage to TRANSFORMING */
   scip->set->stage = SCIP_STAGE_TRANSFORMING;

   /* mark statistics before solving */
   SCIPstatMark(scip->stat);

   /* init solve data structures */
   SCIP_CALL( SCIPeventfilterCreate(&scip->eventfilter, scip->mem->probmem) );
   SCIP_CALL( SCIPeventqueueCreate(&scip->eventqueue) );
   SCIP_CALL( SCIPbranchcandCreate(&scip->branchcand) );
   SCIP_CALL( SCIPlpCreate(&scip->lp, scip->set, scip->messagehdlr, scip->stat, SCIPprobGetName(scip->origprob)) );
   SCIP_CALL( SCIPprimalCreate(&scip->primal) );
   SCIP_CALL( SCIPtreeCreate(&scip->tree, scip->mem->probmem, scip->set, SCIPsetGetNodesel(scip->set, scip->stat)) );
   SCIP_CALL( SCIPrelaxationCreate(&scip->relaxation, scip->mem->probmem, scip->set, scip->stat, scip->primal, scip->tree) );
   SCIP_CALL( SCIPconflictCreate(&scip->conflict, scip->mem->probmem, scip->set) );
   SCIP_CALL( SCIPcliquetableCreate(&scip->cliquetable, scip->set, scip->mem->probmem) );

   /* copy problem in solve memory */
   SCIP_CALL( SCIPprobTransform(scip->origprob, scip->mem->probmem, scip->set, scip->stat, scip->primal, scip->tree,
         scip->reopt, scip->lp, scip->branchcand, scip->eventfilter, scip->eventqueue, scip->conflictstore,
         &scip->transprob) );

   /* switch stage to TRANSFORMED */
   scip->set->stage = SCIP_STAGE_TRANSFORMED;

   /* check, whether objective value is always integral by inspecting the problem, if it is the case adjust the
    * cutoff bound if primal solution is already known
    */
   SCIP_CALL( SCIPprobCheckObjIntegral(scip->transprob, scip->origprob, scip->mem->probmem, scip->set, scip->stat, scip->primal,
	 scip->tree, scip->reopt, scip->lp, scip->eventqueue) );

   /* if possible, scale objective function such that it becomes integral with gcd 1 */
   SCIP_CALL( SCIPprobScaleObj(scip->transprob, scip->origprob, scip->mem->probmem, scip->set, scip->stat, scip->primal,
	 scip->tree, scip->reopt, scip->lp, scip->eventqueue) );

   /* check solution of solution candidate storage */
   nfeassols = 0;
   ncandsols = scip->origprimal->nsols;
   oldnsolsfound = 0;

   /* update upper bound and cutoff bound due to objective limit in primal data */
   SCIP_CALL( SCIPprimalUpdateObjlimit(scip->primal, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
         scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp) );

   if( !scip->set->reopt_enable && scip->set->nactivebenders == 0 )
   {
      oldnsolsfound = scip->primal->nsolsfound;
      for( s = scip->origprimal->nsols - 1; s >= 0; --s )
      {
         SCIP_Bool feasible;
         SCIP_SOL* sol;

         sol =  scip->origprimal->sols[s];

         /* recompute objective function, since the objective might have changed in the meantime */
         SCIPsolRecomputeObj(sol, scip->set, scip->stat, scip->origprob);

         /* SCIPprimalTrySol() can only be called on transformed solutions; therefore check solutions in original problem
          * including modifiable constraints
          */
         SCIP_CALL( checkSolOrig(scip, sol, &feasible,
               (scip->set->disp_verblevel >= SCIP_VERBLEVEL_HIGH ? scip->set->misc_printreason : FALSE),
               FALSE, TRUE, TRUE, TRUE, TRUE) );

         if( feasible )
         {
            SCIP_Real abssolobj;

            abssolobj = REALABS(SCIPsolGetObj(sol, scip->set, scip->transprob, scip->origprob));

            /* we do not want to add solutions with objective value +infinity */
            if( !SCIPisInfinity(scip, abssolobj) )
            {
               SCIP_SOL* bestsol = SCIPgetBestSol(scip);
               SCIP_Bool stored;

               /* add primal solution to solution storage by copying it */
               SCIP_CALL( SCIPprimalAddSol(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat, scip->origprob, scip->transprob,
                     scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter, sol, &stored) );

               if( stored )
               {
                  nfeassols++;

                  if( bestsol != SCIPgetBestSol(scip) )
                     SCIPstoreSolutionGap(scip);
               }
            }
         }

         SCIP_CALL( SCIPsolFree(&sol, scip->mem->probmem, scip->origprimal) );
         scip->origprimal->nsols--;
      }
   }

   assert(scip->origprimal->nsols == 0);

   scip->stat->nexternalsolsfound += scip->primal->nsolsfound - oldnsolsfound;

   if( nfeassols > 0 )
   {
      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
         "%d/%d feasible solution%s given by solution candidate storage, new primal bound %.6e\n\n",
         nfeassols, ncandsols, (nfeassols > 1 ? "s" : ""), SCIPgetSolOrigObj(scip, SCIPgetBestSol(scip)));
   }
   else if( ncandsols > 0 && !scip->set->reopt_enable )
   {
      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
         "all %d solutions given by solution candidate storage are infeasible\n\n", ncandsols);
   }

   /* print transformed problem statistics */
   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
      "transformed problem has %d variables (%d bin, %d int, %d impl, %d cont) and %d constraints\n",
      scip->transprob->nvars, scip->transprob->nbinvars, scip->transprob->nintvars, scip->transprob->nimplvars,
      scip->transprob->ncontvars, scip->transprob->nconss);

   for( h = 0; h < scip->set->nconshdlrs; ++h )
   {
      int nactiveconss;

      nactiveconss = SCIPconshdlrGetNActiveConss(scip->set->conshdlrs[h]);
      if( nactiveconss > 0 )
      {
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
            "%7d constraints of type <%s>\n", nactiveconss, SCIPconshdlrGetName(scip->set->conshdlrs[h]));
      }
   }
   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL, "\n");

   {
      SCIP_Real maxnonzeros;
      SCIP_Longint nchecknonzeros;
      SCIP_Longint nactivenonzeros;
      SCIP_Bool approxchecknonzeros;
      SCIP_Bool approxactivenonzeros;

      /* determine number of non-zeros */
      maxnonzeros = (SCIP_Real)SCIPgetNConss(scip) * SCIPgetNVars(scip);
      maxnonzeros = MAX(maxnonzeros, 1.0);
      SCIP_CALL( calcNonZeros(scip, &nchecknonzeros, &nactivenonzeros, &approxchecknonzeros, &approxactivenonzeros) );
      scip->stat->nnz = nactivenonzeros;
      scip->stat->avgnnz = (SCIPgetNConss(scip) == 0 ? 0.0 : (SCIP_Real) nactivenonzeros / ((SCIP_Real) SCIPgetNConss(scip)));

      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
         "original problem has %s%" SCIP_LONGINT_FORMAT " active (%g%%) nonzeros and %s%" SCIP_LONGINT_FORMAT " (%g%%) check nonzeros\n",
         approxactivenonzeros ? "more than " : "", nactivenonzeros, nactivenonzeros/maxnonzeros * 100,
         approxchecknonzeros ? "more than " : "", nchecknonzeros, nchecknonzeros/maxnonzeros * 100);
      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL, "\n");
   }

   /* call initialization methods of plugins */
   SCIP_CALL( SCIPsetInitPlugins(scip->set, scip->mem->probmem, scip->stat) );

   /* in case the permutation seed is different to 0, permute the transformed problem */
   if( scip->set->random_permutationseed > 0 )
   {
      SCIP_Bool permuteconss;
      SCIP_Bool permutevars;
      int permutationseed;

      permuteconss = scip->set->random_permuteconss;
      permutevars = scip->set->random_permutevars;
      permutationseed = scip->set->random_permutationseed;

      SCIP_CALL( SCIPpermuteProb(scip, (unsigned int)permutationseed, permuteconss, permutevars, permutevars, permutevars, permutevars) );
   }

   if( scip->set->misc_estimexternmem )
   {
      if( scip->set->limit_memory < SCIP_MEM_NOLIMIT )
      {
         SCIP_Longint memused = SCIPgetMemUsed(scip);

         /* if the memory limit is set, we take 1% as the minimum external memory storage */
         scip->stat->externmemestim = MAX(memused, (SCIP_Longint) (0.01 * scip->set->limit_memory * 1048576.0));
      }
      else
         scip->stat->externmemestim = SCIPgetMemUsed(scip);
      SCIPdebugMsg(scip, "external memory usage estimated to %" SCIP_LONGINT_FORMAT " byte\n", scip->stat->externmemestim);
   }

   return SCIP_OKAY;
}

/** initializes presolving */
static
SCIP_RETCODE initPresolve(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
#ifndef NDEBUG
   size_t nusedbuffers;
   size_t nusedcleanbuffers;
#endif

   assert(scip != NULL);
   assert(scip->mem != NULL);
   assert(scip->set != NULL);
   assert(scip->stat != NULL);
   assert(scip->transprob != NULL);
   assert(scip->set->stage == SCIP_STAGE_TRANSFORMED);

   /* retransform all existing solutions to original problem space, because the transformed problem space may
    * get modified in presolving and the solutions may become invalid for the transformed problem
    */
   SCIP_CALL( SCIPprimalRetransformSolutions(scip->primal, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
         scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp) );

   /* reset statistics for presolving and current branch and bound run */
   SCIPstatResetPresolving(scip->stat, scip->set, scip->transprob, scip->origprob);

   /* increase number of branch and bound runs */
   scip->stat->nruns++;

   /* remember problem size of previous run */
   scip->stat->prevrunnvars = scip->transprob->nvars;

   /* switch stage to INITPRESOLVE */
   scip->set->stage = SCIP_STAGE_INITPRESOLVE;

   /* create temporary presolving root node */
   SCIP_CALL( SCIPtreeCreatePresolvingRoot(scip->tree, scip->reopt, scip->mem->probmem, scip->set, scip->messagehdlr,
         scip->stat, scip->transprob, scip->origprob, scip->primal, scip->lp, scip->branchcand, scip->conflict,
         scip->conflictstore, scip->eventfilter, scip->eventqueue, scip->cliquetable) );

   /* GCG wants to perform presolving during the reading process of a file reader;
    * hence the number of used buffers does not need to be zero, however, it should not
    * change by calling SCIPsetInitprePlugins()
    */
#ifndef NDEBUG
   nusedbuffers = BMSgetNUsedBufferMemory(SCIPbuffer(scip));
   nusedcleanbuffers = BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip));
#endif

   /* inform plugins that the presolving is abound to begin */
   SCIP_CALL( SCIPsetInitprePlugins(scip->set, scip->mem->probmem, scip->stat) );
   assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
   assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

   /* delete the variables from the problems that were marked to be deleted */
   SCIP_CALL( SCIPprobPerformVarDeletions(scip->transprob, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->cliquetable, scip->lp, scip->branchcand) );

   /* switch stage to PRESOLVING */
   scip->set->stage = SCIP_STAGE_PRESOLVING;

   return SCIP_OKAY;
}

/** deinitializes presolving */
static
SCIP_RETCODE exitPresolve(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             solved,             /**< is problem already solved? */
   SCIP_Bool*            infeasible          /**< pointer to store if the clique clean up detects an infeasibility */
   )
{
   SCIP_VAR** vars;
   int nvars;
   int v;
#ifndef NDEBUG
   size_t nusedbuffers;
   size_t nusedcleanbuffers;
#endif

   assert(scip != NULL);
   assert(scip->mem != NULL);
   assert(scip->set != NULL);
   assert(scip->stat != NULL);
   assert(scip->transprob != NULL);
   assert(scip->set->stage == SCIP_STAGE_PRESOLVING);
   assert(infeasible != NULL);

   *infeasible = FALSE;

   /* switch stage to EXITPRESOLVE */
   scip->set->stage = SCIP_STAGE_EXITPRESOLVE;

   if( !solved )
   {
      /* flatten all variables */
      vars = SCIPgetFixedVars(scip);
      nvars = SCIPgetNFixedVars(scip);
      assert(nvars == 0 || vars != NULL);

      for( v = nvars - 1; v >= 0; --v )
      {
	 SCIP_VAR* var;
#ifndef NDEBUG
	 SCIP_VAR** multvars;
	 int i;
#endif
	 var = vars[v]; /*lint !e613*/
	 assert(var != NULL);

	 if( SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR )
	 {
	    /* flattens aggregation graph of multi-aggregated variable in order to avoid exponential recursion later-on */
	    SCIP_CALL( SCIPvarFlattenAggregationGraph(var, scip->mem->probmem, scip->set) );

#ifndef NDEBUG
	    multvars = SCIPvarGetMultaggrVars(var);
	    for( i = SCIPvarGetMultaggrNVars(var) - 1; i >= 0; --i)
	       assert(SCIPvarGetStatus(multvars[i]) != SCIP_VARSTATUS_MULTAGGR);
#endif
	 }
      }
   }

   /* exitPresolve() might be called during the reading process of a file reader;
    * hence the number of used buffers does not need to be zero, however, it should not
    * change by calling SCIPsetExitprePlugins() or SCIPprobExitPresolve()
    */
#ifndef NDEBUG
   nusedbuffers = BMSgetNUsedBufferMemory(SCIPbuffer(scip));
   nusedcleanbuffers = BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip));
#endif

   /* inform plugins that the presolving is finished, and perform final modifications */
   SCIP_CALL( SCIPsetExitprePlugins(scip->set, scip->mem->probmem, scip->stat) );
   assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
   assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

   /* remove empty and single variable cliques from the clique table, and convert all two variable cliques
    * into implications
    * delete the variables from the problems that were marked to be deleted
    */
   if( !solved )
   {
      int nlocalbdchgs = 0;

      SCIP_CALL( SCIPprobPerformVarDeletions(scip->transprob, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
            scip->cliquetable, scip->lp, scip->branchcand) );

      SCIP_CALL( SCIPcliquetableCleanup(scip->cliquetable, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
            scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, &nlocalbdchgs,
            infeasible) );

      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
         "clique table cleanup detected %d bound changes%s\n", nlocalbdchgs, *infeasible ? " and infeasibility" : "");
   }

   /* exit presolving */
   SCIP_CALL( SCIPprobExitPresolve(scip->transprob,  scip->set) );
   assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
   assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

   if( !solved )
   {
      /* check, whether objective value is always integral by inspecting the problem, if it is the case adjust the
       * cutoff bound if primal solution is already known
       */
      SCIP_CALL( SCIPprobCheckObjIntegral(scip->transprob, scip->origprob, scip->mem->probmem, scip->set, scip->stat, scip->primal,
	    scip->tree, scip->reopt, scip->lp, scip->eventqueue) );

      /* if possible, scale objective function such that it becomes integral with gcd 1 */
      SCIP_CALL( SCIPprobScaleObj(scip->transprob, scip->origprob, scip->mem->probmem, scip->set, scip->stat, scip->primal,
            scip->tree, scip->reopt, scip->lp, scip->eventqueue) );

      scip->stat->lastlowerbound = SCIPprobInternObjval(scip->transprob, scip->origprob, scip->set, scip->transprob->dualbound);

      /* we need to update the primal dual integral here to update the last{upper/dual}bound values after a restart */
      if( scip->set->misc_calcintegral )
      {
         SCIPstatUpdatePrimalDualIntegral(scip->stat, scip->set, scip->transprob, scip->origprob, SCIPgetUpperbound(scip), SCIPgetLowerbound(scip) );
      }
   }

   /* free temporary presolving root node */
   SCIP_CALL( SCIPtreeFreePresolvingRoot(scip->tree, scip->reopt, scip->mem->probmem, scip->set, scip->messagehdlr,
         scip->stat, scip->transprob, scip->origprob, scip->primal, scip->lp, scip->branchcand, scip->conflict,
         scip->conflictstore, scip->eventfilter, scip->eventqueue, scip->cliquetable) );

   /* switch stage to PRESOLVED */
   scip->set->stage = SCIP_STAGE_PRESOLVED;

   return SCIP_OKAY;
}

/** applies one round of presolving with the given presolving timing
 *
 *  This method will always be called with presoltiming fast first. It iterates over all presolvers, propagators, and
 *  constraint handlers and calls their presolving callbacks with timing fast.  If enough reductions are found, it
 *  returns and the next presolving round will be started (again with timing fast).  If the fast presolving does not
 *  find enough reductions, this methods calls itself recursively with presoltiming medium.  Again, it calls the
 *  presolving callbacks of all presolvers, propagators, and constraint handlers with timing medium.  If enough
 *  reductions are found, it returns and the next presolving round will be started (with timing fast).  Otherwise, it is
 *  called recursively with presoltiming exhaustive. In exhaustive presolving, presolvers, propagators, and constraint
 *  handlers are called w.r.t. their priority, but this time, we stop as soon as enough reductions were found and do not
 *  necessarily call all presolving methods. If we stop, we return and another presolving round is started with timing
 *  fast.
 *
 *  @todo check if we want to do the following (currently disabled):
 *  In order to avoid calling the same expensive presolving methods again and again (which is possibly ineffective
 *  for the current instance), we continue the loop for exhaustive presolving where we stopped it the last time.  The
 *  {presol/prop/cons}start pointers are used to this end: they provide the plugins to start the loop with in the
 *  current presolving round (if we reach exhaustive presolving), and are updated in this case to the next ones to be
 *  called in the next round. In case we reach the end of the loop in exhaustive presolving, we call the method again
 *  with exhaustive timing, now starting with the first presolving steps in the loop until we reach the ones we started
 *  the last call with.  This way, we won't stop until all exhaustive presolvers were called without finding enough
 *  reductions (in sum).
 */
static
SCIP_RETCODE presolveRound(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_PRESOLTIMING*    timing,             /**< pointer to current presolving timing */
   SCIP_Bool*            unbounded,          /**< pointer to store whether presolving detected unboundedness */
   SCIP_Bool*            infeasible,         /**< pointer to store whether presolving detected infeasibility */
   SCIP_Bool             lastround,          /**< is this the last presolving round due to a presolving round limit? */
   int*                  presolstart,        /**< pointer to get the presolver to start exhaustive presolving with in
                                              *   the current round and store the one to start with in the next round */
   int                   presolend,          /**< last presolver to treat in exhaustive presolving */
   int*                  propstart,          /**< pointer to get the propagator to start exhaustive presolving with in
                                              *   the current round and store the one to start with in the next round */
   int                   propend,            /**< last propagator to treat in exhaustive presolving */
   int*                  consstart,          /**< pointer to get the constraint handler to start exhaustive presolving with in
                                              *   the current round and store the one to start with in the next round */
   int                   consend             /**< last constraint handler to treat in exhaustive presolving */
   )
{
   SCIP_RESULT result;
   SCIP_EVENT event;
   SCIP_Bool aborted;
   SCIP_Bool lastranpresol;
#if 0
   int oldpresolstart = 0;
   int oldpropstart = 0;
   int oldconsstart = 0;
#endif
   int priopresol;
   int prioprop;
   int i;
   int j;
   int k;
#ifndef NDEBUG
   size_t nusedbuffers;
   size_t nusedcleanbuffers;
#endif

   assert(scip != NULL);
   assert(scip->set != NULL);
   assert(unbounded != NULL);
   assert(infeasible != NULL);
   assert(presolstart != NULL);
   assert(propstart != NULL);
   assert(consstart != NULL);

   assert((presolend == scip->set->npresols && propend == scip->set->nprops && consend == scip->set->nconshdlrs)
      || (*presolstart == 0 && *propstart == 0 && *consstart == 0));

   *unbounded = FALSE;
   *infeasible = FALSE;
   aborted = FALSE;

   assert( scip->set->propspresolsorted );

   /* GCG wants to perform presolving during the reading process of a file reader;
    * hence the number of used buffers does not need to be zero, however, it should not
    * change by calling the presolving callbacks
    */
#ifndef NDEBUG
   nusedbuffers = BMSgetNUsedBufferMemory(SCIPbuffer(scip));
   nusedcleanbuffers = BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip));
#endif


   if( *timing == SCIP_PRESOLTIMING_EXHAUSTIVE )
   {
      /* In exhaustive presolving, we continue the loop where we stopped last time to avoid calling the same
       * (possibly ineffective) presolving step again and again. If we reach the end of the arrays of presolvers,
       * propagators, and constraint handlers without having made enough reductions, we start again from the beginning
       */
      i = *presolstart;
      j = *propstart;
      k = *consstart;
#if 0
      oldpresolstart = i;
      oldpropstart = j;
      oldconsstart = k;
#endif
      if( i >= presolend && j >= propend && k >= consend )
         return SCIP_OKAY;

      if( i == 0 && j == 0 && k == 0 )
         ++(scip->stat->npresolroundsext);
   }
   else
   {
      /* in fast and medium presolving, we always iterate over all presolvers, propagators, and constraint handlers */
      assert(presolend == scip->set->npresols);
      assert(propend == scip->set->nprops);
      assert(consend == scip->set->nconshdlrs);

      i = 0;
      j = 0;
      k = 0;

      if( *timing == SCIP_PRESOLTIMING_FAST )
         ++(scip->stat->npresolroundsfast);
      if( *timing == SCIP_PRESOLTIMING_MEDIUM )
         ++(scip->stat->npresolroundsmed);
   }

   SCIPdebugMsg(scip, "starting presolving round %d (%d/%d/%d), timing = %u\n",
      scip->stat->npresolrounds, scip->stat->npresolroundsfast, scip->stat->npresolroundsmed,
      scip->stat->npresolroundsext, *timing);

   /* call included presolvers with nonnegative priority */
   while( !(*unbounded) && !(*infeasible) && !aborted && (i < presolend || j < propend) )
   {
      if( i < presolend )
         priopresol = SCIPpresolGetPriority(scip->set->presols[i]);
      else
         priopresol = -1;

      if( j < propend )
         prioprop = SCIPpropGetPresolPriority(scip->set->props_presol[j]);
      else
         prioprop = -1;

      /* call next propagator */
      if( prioprop >= priopresol )
      {
         /* only presolving methods which have non-negative priority will be called before constraint handlers */
         if( prioprop < 0 )
            break;

         SCIPdebugMsg(scip, "executing presolving of propagator <%s>\n", SCIPpropGetName(scip->set->props_presol[j]));
         SCIP_CALL( SCIPpropPresol(scip->set->props_presol[j], scip->set, *timing, scip->stat->npresolrounds,
               &scip->stat->npresolfixedvars, &scip->stat->npresolaggrvars, &scip->stat->npresolchgvartypes,
               &scip->stat->npresolchgbds, &scip->stat->npresoladdholes, &scip->stat->npresoldelconss,
               &scip->stat->npresoladdconss, &scip->stat->npresolupgdconss, &scip->stat->npresolchgcoefs,
               &scip->stat->npresolchgsides, &result) );
         assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
         assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

         lastranpresol = FALSE;
         ++j;
      }
      /* call next presolver */
      else
      {
         /* only presolving methods which have non-negative priority will be called before constraint handlers */
         if( priopresol < 0 )
            break;

         SCIPdebugMsg(scip, "executing presolver <%s>\n", SCIPpresolGetName(scip->set->presols[i]));
         SCIP_CALL( SCIPpresolExec(scip->set->presols[i], scip->set, *timing, scip->stat->npresolrounds,
               &scip->stat->npresolfixedvars, &scip->stat->npresolaggrvars, &scip->stat->npresolchgvartypes,
               &scip->stat->npresolchgbds, &scip->stat->npresoladdholes, &scip->stat->npresoldelconss,
               &scip->stat->npresoladdconss, &scip->stat->npresolupgdconss, &scip->stat->npresolchgcoefs,
               &scip->stat->npresolchgsides, &result) );
         assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
         assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

         lastranpresol = TRUE;
         ++i;
      }

      if( result == SCIP_CUTOFF )
      {
         *infeasible = TRUE;

         if( lastranpresol )
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "presolver <%s> detected infeasibility\n", SCIPpresolGetName(scip->set->presols[i-1]));
         else
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "propagator <%s> detected infeasibility\n", SCIPpropGetName(scip->set->props_presol[j-1]));
      }
      else if( result == SCIP_UNBOUNDED )
      {
         *unbounded = TRUE;

         if( lastranpresol )
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "presolver <%s> detected unboundedness (or infeasibility)\n", SCIPpresolGetName(scip->set->presols[i-1]));
         else
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "propagator <%s> detected  unboundedness (or infeasibility)\n", SCIPpropGetName(scip->set->props_presol[j-1]));
      }

      /* delete the variables from the problems that were marked to be deleted */
      SCIP_CALL( SCIPprobPerformVarDeletions(scip->transprob, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->cliquetable, scip->lp,
            scip->branchcand) );

      SCIPdebugMsg(scip, "presolving callback returned result <%d>\n", result);

      /* if we work off the exhaustive presolvers, we stop immediately if a reduction was found */
      if( (*timing == SCIP_PRESOLTIMING_EXHAUSTIVE) && !lastround && !SCIPisPresolveFinished(scip) )
      {
         assert(*consstart == 0);

         if( lastranpresol )
         {
            *presolstart = i + 1;
            *propstart = j;
         }
         else
         {
            *presolstart = i;
            *propstart = j + 1;
         }
         aborted = TRUE;

         break;
      }
   }

   /* call presolve methods of constraint handlers */
   while( k < consend && !(*unbounded) && !(*infeasible) && !aborted )
   {
      SCIPdebugMsg(scip, "executing presolve method of constraint handler <%s>\n",
         SCIPconshdlrGetName(scip->set->conshdlrs[k]));
      SCIP_CALL( SCIPconshdlrPresolve(scip->set->conshdlrs[k], scip->mem->probmem, scip->set, scip->stat,
            *timing, scip->stat->npresolrounds,
            &scip->stat->npresolfixedvars, &scip->stat->npresolaggrvars, &scip->stat->npresolchgvartypes,
            &scip->stat->npresolchgbds, &scip->stat->npresoladdholes, &scip->stat->npresoldelconss,
            &scip->stat->npresoladdconss, &scip->stat->npresolupgdconss, &scip->stat->npresolchgcoefs,
            &scip->stat->npresolchgsides, &result) );
      assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
      assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

      ++k;

      if( result == SCIP_CUTOFF )
      {
         *infeasible = TRUE;
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
            "constraint handler <%s> detected infeasibility\n", SCIPconshdlrGetName(scip->set->conshdlrs[k-1]));
      }
      else if( result == SCIP_UNBOUNDED )
      {
         *unbounded = TRUE;
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
            "constraint handler <%s> detected unboundedness (or infeasibility)\n",
            SCIPconshdlrGetName(scip->set->conshdlrs[k-1]));
      }

      /* delete the variables from the problems that were marked to be deleted */
      SCIP_CALL( SCIPprobPerformVarDeletions(scip->transprob, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->cliquetable, scip->lp,
            scip->branchcand) );

      SCIPdebugMsg(scip, "presolving callback returned with result <%d>\n", result);

      /* if we work off the exhaustive presolvers, we stop immediately if a reduction was found */
      if( (*timing == SCIP_PRESOLTIMING_EXHAUSTIVE) && !lastround && !SCIPisPresolveFinished(scip) )
      {
         *presolstart = i;
         *propstart = j;
         *consstart = k + 1;
         aborted = TRUE;

         break;
      }
   }

   assert( scip->set->propspresolsorted );

   /* call included presolvers with negative priority */
   while( !(*unbounded) && !(*infeasible) && !aborted && (i < presolend || j < propend) )
   {
      if( i < scip->set->npresols )
         priopresol = SCIPpresolGetPriority(scip->set->presols[i]);
      else
         priopresol = -INT_MAX;

      if( j < scip->set->nprops )
         prioprop = SCIPpropGetPresolPriority(scip->set->props_presol[j]);
      else
         prioprop = -INT_MAX;

      /* choose presolving */
      if( prioprop >= priopresol )
      {
         assert(prioprop <= 0);

         SCIPdebugMsg(scip, "executing presolving of propagator <%s>\n", SCIPpropGetName(scip->set->props_presol[j]));
         SCIP_CALL( SCIPpropPresol(scip->set->props_presol[j], scip->set, *timing, scip->stat->npresolrounds,
               &scip->stat->npresolfixedvars, &scip->stat->npresolaggrvars, &scip->stat->npresolchgvartypes,
               &scip->stat->npresolchgbds, &scip->stat->npresoladdholes, &scip->stat->npresoldelconss,
               &scip->stat->npresoladdconss, &scip->stat->npresolupgdconss, &scip->stat->npresolchgcoefs,
               &scip->stat->npresolchgsides, &result) );
         assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
         assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

         lastranpresol = FALSE;
         ++j;
      }
      else
      {
         assert(priopresol < 0);

         SCIPdebugMsg(scip, "executing presolver <%s>\n", SCIPpresolGetName(scip->set->presols[i]));
         SCIP_CALL( SCIPpresolExec(scip->set->presols[i], scip->set, *timing, scip->stat->npresolrounds,
               &scip->stat->npresolfixedvars, &scip->stat->npresolaggrvars, &scip->stat->npresolchgvartypes,
               &scip->stat->npresolchgbds, &scip->stat->npresoladdholes, &scip->stat->npresoldelconss,
               &scip->stat->npresoladdconss, &scip->stat->npresolupgdconss, &scip->stat->npresolchgcoefs,
               &scip->stat->npresolchgsides, &result) );
         assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
         assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

         lastranpresol = TRUE;
         ++i;
      }

      if( result == SCIP_CUTOFF )
      {
         *infeasible = TRUE;

         if( lastranpresol )
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "presolver <%s> detected infeasibility\n", SCIPpresolGetName(scip->set->presols[i-1]));
         else
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "propagator <%s> detected infeasibility\n", SCIPpropGetName(scip->set->props_presol[j-1]));
      }
      else if( result == SCIP_UNBOUNDED )
      {
         *unbounded = TRUE;

         if( lastranpresol )
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "presolver <%s> detected unboundedness (or infeasibility)\n", SCIPpresolGetName(scip->set->presols[i-1]));
         else
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
               "propagator <%s> detected  unboundedness (or infeasibility)\n", SCIPpropGetName(scip->set->props_presol[j-1]));
      }

      /* delete the variables from the problems that were marked to be deleted */
      SCIP_CALL( SCIPprobPerformVarDeletions(scip->transprob, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->cliquetable, scip->lp,
            scip->branchcand) );

      SCIPdebugMsg(scip, "presolving callback return with result <%d>\n", result);

      /* if we work off the exhaustive presolvers, we stop immediately if a reduction was found */
      if( (*timing == SCIP_PRESOLTIMING_EXHAUSTIVE) && !lastround && !SCIPisPresolveFinished(scip) )
      {
         assert(k == consend);

         if( lastranpresol )
         {
            *presolstart = i + 1;
            *propstart = j;
         }
         else
         {
            *presolstart = i;
            *propstart = j + 1;
         }
         *consstart = k;

         break;
      }
   }

   /* remove empty and single variable cliques from the clique table */
   if( !(*unbounded) && !(*infeasible) )
   {
      int nlocalbdchgs = 0;

      SCIP_CALL( SCIPcliquetableCleanup(scip->cliquetable, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
            scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, &nlocalbdchgs,
            infeasible) );

      if( nlocalbdchgs > 0 || *infeasible )
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
            "clique table cleanup detected %d bound changes%s\n", nlocalbdchgs, *infeasible ? " and infeasibility" : "");

      scip->stat->npresolfixedvars += nlocalbdchgs;

      if( !*infeasible && scip->set->nheurs > 0 )
      {
         /* call primal heuristics that are applicable during presolving */
         SCIP_Bool foundsol;

         SCIPdebugMsg(scip, "calling primal heuristics during presolving\n");

         /* call primal heuristics */
         SCIP_CALL( SCIPprimalHeuristics(scip->set, scip->stat, scip->transprob, scip->primal, NULL, NULL, NULL,
               SCIP_HEURTIMING_DURINGPRESOLLOOP, FALSE, &foundsol, unbounded) );

         /* output a message, if a solution was found */
         if( foundsol )
         {
            SCIP_SOL* sol;

            assert(SCIPgetNSols(scip) > 0);
            sol = SCIPgetBestSol(scip);
            assert(sol != NULL);
            assert(SCIPgetSolOrigObj(scip,sol) != SCIP_INVALID); /*lint !e777*/

            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
               "feasible solution found by %s heuristic after %.1f seconds, objective value %.6e\n",
               SCIPgetSolvingTime(scip), SCIPheurGetName(SCIPsolGetHeur(sol)), SCIPgetSolOrigObj(scip, sol));
         }
      }
   }

   if( !(*unbounded) && !(*infeasible) )
   {
      /* call more expensive presolvers */
      if( (SCIPisPresolveFinished(scip) || lastround) )
      {
         if( *timing != SCIP_PRESOLTIMING_FINAL )
         {
            assert((*timing == SCIP_PRESOLTIMING_FAST) || (*timing == SCIP_PRESOLTIMING_MEDIUM) || (*timing == SCIP_PRESOLTIMING_EXHAUSTIVE));

            SCIPdebugMsg(scip, "not enough reductions in %s presolving, running %s presolving now...\n",
               *timing == SCIP_PRESOLTIMING_FAST ? "fast" : *timing == SCIP_PRESOLTIMING_MEDIUM ? "medium" : "exhaustive",
               *timing == SCIP_PRESOLTIMING_FAST ? "medium" : *timing == SCIP_PRESOLTIMING_MEDIUM ? "exhaustive" : "final");

            /* increase timing */
            *timing = ((*timing == SCIP_PRESOLTIMING_FAST) ? SCIP_PRESOLTIMING_MEDIUM : (*timing == SCIP_PRESOLTIMING_MEDIUM) ? SCIP_PRESOLTIMING_EXHAUSTIVE : SCIP_PRESOLTIMING_FINAL);

            /* computational experiments showed that always starting the loop of exhaustive presolvers from the beginning
             * performs better than continuing from the last processed presolver. Therefore, we start from 0, but keep
             * the mechanisms to possibly change this back later.
             * @todo try starting from the last processed exhaustive presolver
             */
            *presolstart = 0;
            *propstart = 0;
            *consstart = 0;

            SCIP_CALL( presolveRound(scip, timing, unbounded, infeasible, lastround, presolstart, presolend,
                  propstart, propend, consstart, consend) );
         }
#if 0
         /* run remaining exhaustive presolvers (if we did not start from the beginning anyway) */
         else if( (oldpresolstart > 0 || oldpropstart > 0 || oldconsstart > 0) && presolend == scip->set->npresols
            && propend == scip->set->nprops && consend == scip->set->nconshdlrs )
         {
            int newpresolstart = 0;
            int newpropstart = 0;
            int newconsstart = 0;

            SCIPdebugMsg(scip, "reached end of exhaustive presolving loop, starting from the beginning...\n");

            SCIP_CALL( presolveRound(scip, timing, unbounded, infeasible, lastround, &newpresolstart,
                  oldpresolstart, &newpropstart, oldpropstart, &newconsstart, oldconsstart) );

            *presolstart = newpresolstart;
            *propstart = newpropstart;
            *consstart = newconsstart;
         }
#endif
      }
   }

   /* issue PRESOLVEROUND event */
   SCIP_CALL( SCIPeventChgType(&event, SCIP_EVENTTYPE_PRESOLVEROUND) );
   SCIP_CALL( SCIPeventProcess(&event, scip->set, NULL, NULL, NULL, scip->eventfilter) );

   return SCIP_OKAY;
}


/** loops through the included presolvers and constraint's presolve methods, until changes are too few */
static
SCIP_RETCODE presolve(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool*            unbounded,          /**< pointer to store whether presolving detected unboundedness */
   SCIP_Bool*            infeasible          /**< pointer to store whether presolving detected infeasibility */
   )
{
   SCIP_PRESOLTIMING presoltiming;
   SCIP_Bool finished;
   SCIP_Bool stopped;
   SCIP_Bool lastround;
   int presolstart = 0;
   int propstart = 0;
   int consstart = 0;
#ifndef NDEBUG
   size_t nusedbuffers;
   size_t nusedcleanbuffers;
#endif

   assert(scip != NULL);
   assert(scip->mem != NULL);
   assert(scip->primal != NULL);
   assert(scip->set != NULL);
   assert(scip->stat != NULL);
   assert(scip->transprob != NULL);
   assert(scip->set->stage == SCIP_STAGE_TRANSFORMED || scip->set->stage == SCIP_STAGE_PRESOLVING);
   assert(unbounded != NULL);
   assert(infeasible != NULL);

   *unbounded = FALSE;

   /* GCG wants to perform presolving during the reading process of a file reader;
    * hence the number of used buffers does not need to be zero, however, it should
    * be the same again after presolve is finished
    */
#ifndef NDEBUG
   nusedbuffers = BMSgetNUsedBufferMemory(SCIPbuffer(scip));
   nusedcleanbuffers = BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip));
#endif


   /* switch status to unknown */
   scip->stat->status = SCIP_STATUS_UNKNOWN;

   /* update upper bound and cutoff bound due to objective limit in primal data */
   SCIP_CALL( SCIPprimalUpdateObjlimit(scip->primal, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
         scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp) );

   /* start presolving timer */
   SCIPclockStart(scip->stat->presolvingtime, scip->set);
   SCIPclockStart(scip->stat->presolvingtimeoverall, scip->set);

   /* initialize presolving */
   if( scip->set->stage == SCIP_STAGE_TRANSFORMED )
   {
      SCIP_CALL( initPresolve(scip) );
   }
   assert(scip->set->stage == SCIP_STAGE_PRESOLVING);

   /* call primal heuristics that are applicable before presolving */
   if( scip->set->nheurs > 0 )
   {
      SCIP_Bool foundsol;

      SCIPdebugMsg(scip, "calling primal heuristics before presolving\n");

      /* call primal heuristics */
      SCIP_CALL( SCIPprimalHeuristics(scip->set, scip->stat, scip->transprob, scip->primal, NULL, NULL, NULL,
            SCIP_HEURTIMING_BEFOREPRESOL, FALSE, &foundsol, unbounded) );

      /* output a message, if a solution was found */
      if( foundsol )
      {
         SCIP_SOL* sol;

         assert(SCIPgetNSols(scip) > 0);
         sol = SCIPgetBestSol(scip);
         assert(sol != NULL);
         assert(SCIPgetSolOrigObj(scip,sol) != SCIP_INVALID);  /*lint !e777*/

         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
            "feasible solution found by %s heuristic after %.1f seconds, objective value %.6e\n",
            SCIPheurGetName(SCIPsolGetHeur(sol)), SCIPgetSolvingTime(scip), SCIPgetSolOrigObj(scip, sol));
      }
   }

   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH, "presolving:\n");

   *infeasible = FALSE;
   *unbounded = (*unbounded) || (SCIPgetNSols(scip) > 0 && SCIPisInfinity(scip, -SCIPgetSolOrigObj(scip, SCIPgetBestSol(scip))));

   finished = (scip->set->presol_maxrounds != -1 && scip->stat->npresolrounds >= scip->set->presol_maxrounds)
         || (*unbounded) || (scip->set->reopt_enable && scip->stat->nreoptruns >= 1) || scip->set->nactivebenders > 0;
   stopped = SCIPsolveIsStopped(scip->set, scip->stat, TRUE);

   /* perform presolving rounds */
   while( !finished && !stopped )
   {
      /* store current number of reductions */
      scip->stat->lastnpresolfixedvars = scip->stat->npresolfixedvars;
      scip->stat->lastnpresolaggrvars = scip->stat->npresolaggrvars;
      scip->stat->lastnpresolchgvartypes = scip->stat->npresolchgvartypes;
      scip->stat->lastnpresolchgbds = scip->stat->npresolchgbds;
      scip->stat->lastnpresoladdholes = scip->stat->npresoladdholes;
      scip->stat->lastnpresoldelconss = scip->stat->npresoldelconss;
      scip->stat->lastnpresoladdconss = scip->stat->npresoladdconss;
      scip->stat->lastnpresolupgdconss = scip->stat->npresolupgdconss;
      scip->stat->lastnpresolchgcoefs = scip->stat->npresolchgcoefs;
      scip->stat->lastnpresolchgsides = scip->stat->npresolchgsides;
#ifdef SCIP_DISABLED_CODE
      scip->stat->lastnpresolimplications = scip->stat->nimplications;
      scip->stat->lastnpresolcliques = SCIPcliquetableGetNCliques(scip->cliquetable);
#endif

      /* set presolving flag */
      scip->stat->performpresol = TRUE;

      /* sort propagators */
      SCIPsetSortPropsPresol(scip->set);

      /* sort presolvers by priority */
      SCIPsetSortPresols(scip->set);

      /* check if this will be the last presolving round (in that case, we want to run all presolvers) */
      lastround = (scip->set->presol_maxrounds == -1 ? FALSE : (scip->stat->npresolrounds + 1 >= scip->set->presol_maxrounds));

      presoltiming = SCIP_PRESOLTIMING_FAST;

      /* perform the presolving round by calling the presolvers, propagators, and constraint handlers */
      assert(!(*unbounded));
      assert(!(*infeasible));
      SCIP_CALL( presolveRound(scip, &presoltiming, unbounded, infeasible, lastround,
            &presolstart, scip->set->npresols, &propstart, scip->set->nprops, &consstart, scip->set->nconshdlrs) );

      /* check, if we should abort presolving due to not enough changes in the last round */
      finished = SCIPisPresolveFinished(scip) || presoltiming == SCIP_PRESOLTIMING_FINAL;

      SCIPdebugMsg(scip, "presolving round %d returned with unbounded = %u, infeasible = %u, finished = %u\n", scip->stat->npresolrounds, *unbounded, *infeasible, finished);

      /* check whether problem is infeasible or unbounded */
      finished = finished || *unbounded || *infeasible;

      /* increase round number */
      scip->stat->npresolrounds++;

      if( !finished )
      {
         /* print presolving statistics */
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
            "(round %d, %-11s %d del vars, %d del conss, %d add conss, %d chg bounds, %d chg sides, %d chg coeffs, %d upgd conss, %d impls, %d clqs\n",
            scip->stat->npresolrounds, ( presoltiming == SCIP_PRESOLTIMING_FAST ? "fast)" :
               (presoltiming == SCIP_PRESOLTIMING_MEDIUM ? "medium)" :
                  (presoltiming == SCIP_PRESOLTIMING_EXHAUSTIVE ?"exhaustive)" :
                     "final)")) ),
            scip->stat->npresolfixedvars + scip->stat->npresolaggrvars,
            scip->stat->npresoldelconss, scip->stat->npresoladdconss,
            scip->stat->npresolchgbds, scip->stat->npresolchgsides,
            scip->stat->npresolchgcoefs, scip->stat->npresolupgdconss,
            scip->stat->nimplications, SCIPcliquetableGetNCliques(scip->cliquetable));
      }

      /* abort if time limit was reached or user interrupted */
      stopped = SCIPsolveIsStopped(scip->set, scip->stat, TRUE);
   }

   if( *infeasible || *unbounded )
   {
      /* first change status of scip, so that all plugins in their exitpre callbacks can ask SCIP for the correct status */
      if( *infeasible )
      {
         /* switch status to OPTIMAL */
         if( scip->primal->nlimsolsfound > 0 )
         {
            scip->stat->status = SCIP_STATUS_OPTIMAL;
         }
         else /* switch status to INFEASIBLE */
            scip->stat->status = SCIP_STATUS_INFEASIBLE;
      }
      else if( scip->primal->nsols >= 1 ) /* switch status to UNBOUNDED */
         scip->stat->status = SCIP_STATUS_UNBOUNDED;
      else /* switch status to INFORUNBD */
         scip->stat->status = SCIP_STATUS_INFORUNBD;
   }

   /* deinitialize presolving */
   if( finished && (!stopped || *unbounded || *infeasible) )
   {
      SCIP_Real maxnonzeros;
      SCIP_Longint nchecknonzeros;
      SCIP_Longint nactivenonzeros;
      SCIP_Bool approxchecknonzeros;
      SCIP_Bool approxactivenonzeros;
      SCIP_Bool infeas;

      SCIP_CALL( exitPresolve(scip, *unbounded || *infeasible, &infeas) );
      *infeasible = *infeasible || infeas;

      assert(scip->set->stage == SCIP_STAGE_PRESOLVED);

      /* resort variables if we are not already done */
      if( !(*infeasible) && !(*unbounded) )
      {
         /* (Re)Sort the variables, which appear in the four categories (binary, integer, implicit, continuous) after
          * presolve with respect to their original index (within their categories). Adjust the problem index afterwards
          * which is supposed to reflect the position in the variable array. This additional (re)sorting is supposed to
          * get more robust against the order presolving fixed variables. (We also reobtain a possible block structure
          * induced by the user model)
          */
         SCIPprobResortVars(scip->transprob);
      }

      /* determine number of non-zeros */
      maxnonzeros = (SCIP_Real)SCIPgetNConss(scip) * SCIPgetNVars(scip);
      maxnonzeros = MAX(maxnonzeros, 1.0);
      SCIP_CALL( calcNonZeros(scip, &nchecknonzeros, &nactivenonzeros, &approxchecknonzeros, &approxactivenonzeros) );
      scip->stat->nnz = nactivenonzeros;

      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL, "\n");
      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL,
         "presolved problem has %s%" SCIP_LONGINT_FORMAT " active (%g%%) nonzeros and %s%" SCIP_LONGINT_FORMAT " (%g%%) check nonzeros\n",
         approxactivenonzeros ? "more than " : "", nactivenonzeros, nactivenonzeros/maxnonzeros * 100,
         approxchecknonzeros ? "more than " : "", nchecknonzeros, nchecknonzeros/maxnonzeros * 100);
      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_FULL, "\n");
   }
   assert(BMSgetNUsedBufferMemory(SCIPbuffer(scip)) == nusedbuffers);
   assert(BMSgetNUsedBufferMemory(SCIPcleanbuffer(scip)) == nusedcleanbuffers);

   /* stop presolving time */
   SCIPclockStop(scip->stat->presolvingtime, scip->set);
   SCIPclockStop(scip->stat->presolvingtimeoverall, scip->set);

   /* print presolving statistics */
   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL,
      "presolving (%d rounds: %d fast, %d medium, %d exhaustive):\n", scip->stat->npresolrounds,
      scip->stat->npresolroundsfast, scip->stat->npresolroundsmed, scip->stat->npresolroundsext);
   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL,
      " %d deleted vars, %d deleted constraints, %d added constraints, %d tightened bounds, %d added holes, %d changed sides, %d changed coefficients\n",
      scip->stat->npresolfixedvars + scip->stat->npresolaggrvars, scip->stat->npresoldelconss, scip->stat->npresoladdconss,
      scip->stat->npresolchgbds, scip->stat->npresoladdholes, scip->stat->npresolchgsides, scip->stat->npresolchgcoefs);
   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL,
      " %d implications, %d cliques\n", scip->stat->nimplications, SCIPcliquetableGetNCliques(scip->cliquetable));

   /* remember number of constraints */
   SCIPprobMarkNConss(scip->transprob);

   return SCIP_OKAY;
}

/** tries to transform original solutions to the transformed problem space */
static
SCIP_RETCODE transformSols(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_SOL** sols;
   SCIP_SOL** scipsols;
   SCIP_SOL* sol;
   SCIP_Real* solvals;
   SCIP_Bool* solvalset;
   SCIP_Bool added;
   SCIP_Longint oldnsolsfound;
   int nsols;
   int ntransvars;
   int naddedsols;
   int s;

   nsols = SCIPgetNSols(scip);
   oldnsolsfound = scip->primal->nsolsfound;

   /* no solution to transform */
   if( nsols == 0 )
      return SCIP_OKAY;

   SCIPdebugMsg(scip, "try to transfer %d original solutions into the transformed problem space\n", nsols);

   ntransvars = scip->transprob->nvars;
   naddedsols = 0;

   /* It might happen, that the added transferred solution does not equal the corresponding original one, which might
    * result in the array of solutions being changed.  Thus we temporarily copy the array and traverse it in reverse
    * order to ensure that the regarded solution in the copied array was not already freed when new solutions were added
    * and the worst solutions were freed.
    */
   scipsols = SCIPgetSols(scip);
   SCIP_CALL( SCIPduplicateBufferArray(scip, &sols, scipsols, nsols) );
   SCIP_CALL( SCIPallocBufferArray(scip, &solvals, ntransvars) );
   SCIP_CALL( SCIPallocBufferArray(scip, &solvalset, ntransvars) );

   for( s = nsols-1; s >= 0; --s )
   {
      sol = sols[s];

      /* it might happen that a transferred original solution has a better objective than its original counterpart
       * (e.g., because multi-aggregated variables get another value, but the solution is still feasible);
       * in this case, it might happen that the solution is not an original one and we just skip this solution
       */
      if( !SCIPsolIsOriginal(sol) )
         continue;

      SCIP_CALL( SCIPprimalTransformSol(scip->primal, sol, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
            scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter, solvals,
            solvalset, ntransvars, &added) );

      if( added )
         ++naddedsols;
   }

   if( naddedsols > 0 )
   {
      SCIPverbMessage(scip, SCIP_VERBLEVEL_HIGH, NULL,
         "transformed %d/%d original solutions to the transformed problem space\n",
         naddedsols, nsols);

      scip->stat->nexternalsolsfound += scip->primal->nsolsfound - oldnsolsfound;
   }

   SCIPfreeBufferArray(scip, &solvalset);
   SCIPfreeBufferArray(scip, &solvals);
   SCIPfreeBufferArray(scip, &sols);

   return SCIP_OKAY;
}

/** initializes solution process data structures */
static
SCIP_RETCODE initSolve(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             solved              /**< is problem already solved? */
   )
{
   assert(scip != NULL);
   assert(scip->mem != NULL);
   assert(scip->set != NULL);
   assert(scip->stat != NULL);
   assert(scip->nlp == NULL);
   assert(scip->set->stage == SCIP_STAGE_PRESOLVED);

   /**@todo check whether other methodscan be skipped if problem has been solved */
   /* if problem has been solved, several time consuming tasks must not be performed */
   if( !solved )
   {
      /* reset statistics for current branch and bound run */
      SCIPstatResetCurrentRun(scip->stat, scip->set, scip->transprob, scip->origprob, solved);
      SCIPstatEnforceLPUpdates(scip->stat);

      /* LP is empty anyway; mark empty LP to be solved and update validsollp counter */
      SCIP_CALL( SCIPlpReset(scip->lp, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->eventfilter) );

      /* update upper bound and cutoff bound due to objective limit in primal data */
      SCIP_CALL( SCIPprimalUpdateObjlimit(scip->primal, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
	    scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp) );
   }

   /* switch stage to INITSOLVE */
   scip->set->stage = SCIP_STAGE_INITSOLVE;

   /* initialize NLP if there are nonlinearities */
   if( scip->transprob->nlpenabled && !scip->set->nlp_disable )
   {
      SCIPdebugMsg(scip, "constructing empty NLP\n");

      SCIP_CALL( SCIPnlpCreate(&scip->nlp, scip->mem->probmem, scip->set, scip->stat, SCIPprobGetName(scip->transprob), scip->transprob->nvars) );
      assert(scip->nlp != NULL);

      SCIP_CALL( SCIPnlpAddVars(scip->nlp, scip->mem->probmem, scip->set, scip->transprob->nvars, scip->transprob->vars) );
   }

   /* possibly create visualization output file */
   SCIP_CALL( SCIPvisualInit(scip->stat->visual, scip->mem->probmem, scip->set, scip->messagehdlr) );

   /* initialize solution process data structures */
   SCIP_CALL( SCIPpricestoreCreate(&scip->pricestore) );
   SCIP_CALL( SCIPsepastoreCreate(&scip->sepastore, scip->mem->probmem, scip->set) );
   SCIP_CALL( SCIPsepastoreCreate(&scip->sepastoreprobing, scip->mem->probmem, scip->set) );
   SCIP_CALL( SCIPcutpoolCreate(&scip->cutpool, scip->mem->probmem, scip->set, scip->set->sepa_cutagelimit, TRUE) );
   SCIP_CALL( SCIPcutpoolCreate(&scip->delayedcutpool, scip->mem->probmem, scip->set, scip->set->sepa_cutagelimit, FALSE) );
   SCIP_CALL( SCIPtreeCreateRoot(scip->tree, scip->reopt, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
         scip->lp) );

   /* update dual bound of the root node if a valid dual bound is at hand */
   if( scip->transprob->dualbound < SCIP_INVALID )
   {
      SCIP_Real internobjval = SCIPprobInternObjval(scip->transprob, scip->origprob, scip->set, scip->transprob->dualbound);

      scip->stat->lastlowerbound = internobjval;

      SCIPnodeUpdateLowerbound(SCIPtreeGetRootNode(scip->tree), scip->stat, scip->set, scip->tree, scip->transprob,
         scip->origprob, internobjval);
   }

   /* try to transform original solutions to the transformed problem space */
   if( scip->set->misc_transorigsols )
   {
      SCIP_CALL( transformSols(scip) );
   }

   /* inform the transformed problem that the branch and bound process starts now */
   SCIP_CALL( SCIPprobInitSolve(scip->transprob, scip->set) );

   /* inform plugins that the branch and bound process starts now */
   SCIP_CALL( SCIPsetInitsolPlugins(scip->set, scip->mem->probmem, scip->stat) );

   /* remember number of constraints */
   SCIPprobMarkNConss(scip->transprob);

   /* if all variables are known, calculate a trivial primal bound by setting all variables to their worst bound */
   if( scip->set->nactivepricers == 0 )
   {
      SCIP_VAR* var;
      SCIP_Real obj;
      SCIP_Real objbound;
      SCIP_Real bd;
      int v;

      objbound = 0.0;
      for( v = 0; v < scip->transprob->nvars && !SCIPsetIsInfinity(scip->set, objbound); ++v )
      {
         var = scip->transprob->vars[v];
         obj = SCIPvarGetObj(var);
         if( !SCIPsetIsZero(scip->set, obj) )
         {
            bd = SCIPvarGetWorstBoundGlobal(var);
            if( SCIPsetIsInfinity(scip->set, REALABS(bd)) )
               objbound = SCIPsetInfinity(scip->set);
            else
               objbound += obj * bd;
         }
      }

      /* adjust primal bound, such that solution with worst bound may be found */
      if( objbound + SCIPsetCutoffbounddelta(scip->set) != objbound ) /*lint !e777*/
         objbound += SCIPsetCutoffbounddelta(scip->set);
      /* if objbound is very large, adding the cutoffbounddelta may not change the number; in this case, we are using
       * SCIPnextafter to ensure that the cutoffbound is really larger than the best possible solution value
       */
      else
         objbound = SCIPnextafter(objbound, SCIP_REAL_MAX);

      /* update cutoff bound */
      if( !SCIPsetIsInfinity(scip->set, objbound) && SCIPsetIsLT(scip->set, objbound, scip->primal->cutoffbound) )
      {
         /* adjust cutoff bound */
         SCIP_CALL( SCIPprimalSetCutoffbound(scip->primal, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
               scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, objbound, FALSE) );
      }
   }

   /* switch stage to SOLVING */
   scip->set->stage = SCIP_STAGE_SOLVING;

   return SCIP_OKAY;
}

/** frees solution process data structures */
static
SCIP_RETCODE freeSolve(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             restart             /**< was this free solve call triggered by a restart? */
   )
{
   assert(scip != NULL);
   assert(scip->mem != NULL);
   assert(scip->set != NULL);
   assert(scip->stat != NULL);
   assert(scip->set->stage == SCIP_STAGE_SOLVING || scip->set->stage == SCIP_STAGE_SOLVED);

   /* mark that we are currently restarting */
   if( restart )
   {
      scip->stat->inrestart = TRUE;

      /* copy the current dual bound into the problem data structure such that it can be used initialize the new search
       * tree
       */
      SCIPprobUpdateDualbound(scip->transprob, SCIPgetDualbound(scip));
   }

   /* remove focus from the current focus node */
   if( SCIPtreeGetFocusNode(scip->tree) != NULL )
   {
      SCIP_NODE* node = NULL;
      SCIP_Bool cutoff;

      SCIP_CALL( SCIPnodeFocus(&node, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat, scip->transprob,
            scip->origprob, scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->conflict,
            scip->conflictstore, scip->eventfilter, scip->eventqueue, scip->cliquetable, &cutoff, FALSE, TRUE) );
      assert(!cutoff);
   }

   /* switch stage to EXITSOLVE */
   scip->set->stage = SCIP_STAGE_EXITSOLVE;

   /* cleanup the conflict storage */
   SCIP_CALL( SCIPconflictstoreClean(scip->conflictstore, scip->mem->probmem, scip->set, scip->stat, scip->reopt) );

   /* inform plugins that the branch and bound process is finished */
   SCIP_CALL( SCIPsetExitsolPlugins(scip->set, scip->mem->probmem, scip->stat, restart) );

   /* free the NLP, if there is one, and reset the flags indicating nonlinearity */
   if( scip->nlp != NULL )
   {
      SCIP_CALL( SCIPnlpFree(&scip->nlp, scip->mem->probmem, scip->set, scip->eventqueue, scip->lp) );
   }
   scip->transprob->nlpenabled = FALSE;

   /* clear the LP, and flush the changes to clear the LP of the solver */
   SCIP_CALL( SCIPlpReset(scip->lp, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->eventfilter) );
   SCIPlpInvalidateRootObjval(scip->lp);

   /* resets the debug environment */
   SCIP_CALL( SCIPdebugReset(scip->set) ); /*lint !e506 !e774*/

   /* clear all row references in internal data structures */
   SCIP_CALL( SCIPcutpoolClear(scip->cutpool, scip->mem->probmem, scip->set, scip->lp) );
   SCIP_CALL( SCIPcutpoolClear(scip->delayedcutpool, scip->mem->probmem, scip->set, scip->lp) );

   /* we have to clear the tree prior to the problem deinitialization, because the rows stored in the forks and
    * subroots have to be released
    */
   SCIP_CALL( SCIPtreeClear(scip->tree, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->lp) );

   /* deinitialize transformed problem */
   SCIP_CALL( SCIPprobExitSolve(scip->transprob, scip->mem->probmem, scip->set, scip->eventqueue, scip->lp, restart) );

   /* free solution process data structures */
   SCIP_CALL( SCIPcutpoolFree(&scip->cutpool, scip->mem->probmem, scip->set, scip->lp) );
   SCIP_CALL( SCIPcutpoolFree(&scip->delayedcutpool, scip->mem->probmem, scip->set, scip->lp) );
   SCIP_CALL( SCIPsepastoreFree(&scip->sepastoreprobing, scip->mem->probmem) );
   SCIP_CALL( SCIPsepastoreFree(&scip->sepastore, scip->mem->probmem) );
   SCIP_CALL( SCIPpricestoreFree(&scip->pricestore) );

   /* possibly close visualization output file */
   SCIPvisualExit(scip->stat->visual, scip->set, scip->messagehdlr);

   /* reset statistics for current branch and bound run */
   if( scip->stat->status == SCIP_STATUS_INFEASIBLE || scip->stat->status == SCIP_STATUS_OPTIMAL || scip->stat->status == SCIP_STATUS_UNBOUNDED || scip->stat->status == SCIP_STATUS_INFORUNBD )
      SCIPstatResetCurrentRun(scip->stat, scip->set, scip->transprob, scip->origprob, TRUE);
   else
      SCIPstatResetCurrentRun(scip->stat, scip->set, scip->transprob, scip->origprob, FALSE);

   /* switch stage to TRANSFORMED */
   scip->set->stage = SCIP_STAGE_TRANSFORMED;

   /* restart finished */
   assert( ! restart || scip->stat->inrestart );
   scip->stat->inrestart = FALSE;

   return SCIP_OKAY;
}

/** frees solution process data structures when reoptimization is used
 *
 *  in contrast to a freeSolve() this method will preserve the transformed problem such that another presolving round
 *  after changing the problem (modifying the objective function) is not necessary.
 */
static
SCIP_RETCODE freeReoptSolve(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);
   assert(scip->mem != NULL);
   assert(scip->set != NULL);
   assert(scip->stat != NULL);
   assert(scip->set->stage == SCIP_STAGE_SOLVING || scip->set->stage == SCIP_STAGE_SOLVED);

   /* remove focus from the current focus node */
   if( SCIPtreeGetFocusNode(scip->tree) != NULL )
   {
      SCIP_NODE* node = NULL;
      SCIP_Bool cutoff;

      SCIP_CALL( SCIPnodeFocus(&node, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat, scip->transprob,
            scip->origprob, scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->conflict,
            scip->conflictstore, scip->eventfilter, scip->eventqueue, scip->cliquetable, &cutoff, FALSE, TRUE) );
      assert(!cutoff);
   }

   /* deinitialize conflict store */
   SCIP_CALL( SCIPconflictstoreClear(scip->conflictstore, scip->mem->probmem, scip->set, scip->stat, scip->reopt) );

   /* invalidate the dual bound */
   SCIPprobInvalidateDualbound(scip->transprob);

   /* switch stage to EXITSOLVE */
   scip->set->stage = SCIP_STAGE_EXITSOLVE;

   /* inform plugins that the branch and bound process is finished */
   SCIP_CALL( SCIPsetExitsolPlugins(scip->set, scip->mem->probmem, scip->stat, FALSE) );

   /* call exit methods of plugins */
   SCIP_CALL( SCIPsetExitPlugins(scip->set, scip->mem->probmem, scip->stat) );

   /* free the NLP, if there is one, and reset the flags indicating nonlinearity */
   if( scip->nlp != NULL )
   {
      SCIP_CALL( SCIPnlpFree(&scip->nlp, scip->mem->probmem, scip->set, scip->eventqueue, scip->lp) );
   }
   scip->transprob->nlpenabled = FALSE;

   /* clear the LP, and flush the changes to clear the LP of the solver */
   SCIP_CALL( SCIPlpReset(scip->lp, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->eventfilter) );
   SCIPlpInvalidateRootObjval(scip->lp);

   /* resets the debug environment */
   SCIP_CALL( SCIPdebugReset(scip->set) ); /*lint !e506 !e774*/

   /* clear all row references in internal data structures */
   SCIP_CALL( SCIPcutpoolClear(scip->cutpool, scip->mem->probmem, scip->set, scip->lp) );
   SCIP_CALL( SCIPcutpoolClear(scip->delayedcutpool, scip->mem->probmem, scip->set, scip->lp) );

   /* we have to clear the tree prior to the problem deinitialization, because the rows stored in the forks and
    * subroots have to be released
    */
   SCIP_CALL( SCIPtreeClear(scip->tree, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->lp) );

   /* deinitialize transformed problem */
   SCIP_CALL( SCIPprobExitSolve(scip->transprob, scip->mem->probmem, scip->set, scip->eventqueue, scip->lp, FALSE) );

   /* free solution process data structures */
   SCIP_CALL( SCIPrelaxationFree(&scip->relaxation) );

   SCIP_CALL( SCIPcutpoolFree(&scip->cutpool, scip->mem->probmem, scip->set, scip->lp) );
   SCIP_CALL( SCIPcutpoolFree(&scip->delayedcutpool, scip->mem->probmem, scip->set, scip->lp) );
   SCIP_CALL( SCIPsepastoreFree(&scip->sepastoreprobing, scip->mem->probmem) );
   SCIP_CALL( SCIPsepastoreFree(&scip->sepastore, scip->mem->probmem) );
   SCIP_CALL( SCIPpricestoreFree(&scip->pricestore) );

   /* possibly close visualization output file */
   SCIPvisualExit(scip->stat->visual, scip->set, scip->messagehdlr);

   /* reset statistics for current branch and bound run */
   SCIPstatResetCurrentRun(scip->stat, scip->set, scip->transprob, scip->origprob, FALSE);

   /* switch stage to PRESOLVED */
   scip->set->stage = SCIP_STAGE_PRESOLVED;

   /* restart finished */
   scip->stat->inrestart = FALSE;

   /* reset solving specific paramters */
   if( scip->set->reopt_enable )
   {
      assert(scip->reopt != NULL);
      SCIP_CALL( SCIPreoptReset(scip->reopt, scip->set, scip->mem->probmem) );
   }

   /* free the debug solution which might live in transformed primal data structure */
   SCIP_CALL( SCIPprimalClear(&scip->primal, scip->mem->probmem) );

   if( scip->set->misc_resetstat )
   {
      /* reset statistics to the point before the problem was transformed */
      SCIPstatReset(scip->stat, scip->set, scip->transprob, scip->origprob);
   }
   else
   {
      /* even if statistics are not completely reset, a partial reset of the primal-dual integral is necessary */
      SCIPstatResetPrimalDualIntegral(scip->stat, scip->set, TRUE);
   }

   /* reset objective limit */
   SCIP_CALL( SCIPsetObjlimit(scip, SCIP_INVALID) );

   return SCIP_OKAY;
}

/** free transformed problem */
static
SCIP_RETCODE freeTransform(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Bool reducedfree;

   assert(scip != NULL);
   assert(scip->mem != NULL);
   assert(scip->stat != NULL);
   assert(scip->set->stage == SCIP_STAGE_TRANSFORMED || scip->set->stage == SCIP_STAGE_PRESOLVING ||
         (scip->set->stage == SCIP_STAGE_PRESOLVED && scip->set->reopt_enable));

   /* If the following evaluates to true, SCIPfreeReoptSolve() has already called the exit-callbacks of the plugins.
    * We can skip calling some of the following methods. This can happen if a new objective function was
    * installed but the solve was not started.
    */
   reducedfree = (scip->set->stage == SCIP_STAGE_PRESOLVED && scip->set->reopt_enable);

   if( !reducedfree )
   {
      /* call exit methods of plugins */
      SCIP_CALL( SCIPsetExitPlugins(scip->set, scip->mem->probmem, scip->stat) );
   }

   /* copy best primal solutions to original solution candidate list */
   if( !scip->set->reopt_enable && scip->set->limit_maxorigsol > 0 && scip->set->misc_transsolsorig && scip->set->nactivebenders == 0 )
   {
      SCIP_Bool stored;
      SCIP_Bool hasinfval;
      int maxsols;
      int nsols;
      int s;

      assert(scip->origprimal->nsols == 0);

      nsols = scip->primal->nsols;
      maxsols = scip->set->limit_maxorigsol;
      stored = TRUE;
      s = 0;

      /* iterate over all solutions as long as the original solution candidate store size limit is not reached */
      while( s < nsols && scip->origprimal->nsols < maxsols )
      {
         SCIP_SOL* sol;

         sol = scip->primal->sols[s];
         assert(sol != NULL);

         if( !SCIPsolIsOriginal(sol) )
         {
            /* retransform solution into the original problem space */
            SCIP_CALL( SCIPsolRetransform(sol, scip->set, scip->stat, scip->origprob, scip->transprob, &hasinfval) );
         }
         else
            hasinfval = FALSE;

         /* removing infinite fixings is turned off by the corresponding parameter */
         if( !scip->set->misc_finitesolstore )
            hasinfval = FALSE;

         if( !hasinfval )
         {
            /* add solution to original candidate solution storage */
            SCIP_CALL( SCIPprimalAddOrigSol(scip->origprimal, scip->mem->probmem, scip->set, scip->stat, scip->origprob, sol, &stored) );
         }
         else
         {
            SCIP_SOL* newsol;
            SCIP_Bool success;

            SCIP_CALL( SCIPcreateFiniteSolCopy(scip, &newsol, sol, &success) );

            /* infinite fixing could be removed */
            if( newsol != NULL )
            {
               /* add solution to original candidate solution storage; we must not use SCIPprimalAddOrigSolFree()
                * because we want to create a copy of the solution in the origprimal solution store, but newsol was
                * created in the (transformed) primal
                */
               SCIP_CALL( SCIPprimalAddOrigSol(scip->origprimal, scip->mem->probmem, scip->set, scip->stat, scip->origprob, newsol, &stored) );

               /* free solution in (transformed) primal where it was created */
               SCIP_CALL( SCIPsolFree(&newsol, scip->mem->probmem, scip->primal) );
            }
         }
         ++s;
      }

      if( scip->origprimal->nsols > 1 )
      {
         SCIPverbMessage(scip, SCIP_VERBLEVEL_FULL, NULL,
            "stored the %d best primal solutions in the original solution candidate list\n", scip->origprimal->nsols);
      }
      else if( scip->origprimal->nsols == 1 )
      {
         SCIPverbMessage(scip, SCIP_VERBLEVEL_FULL, NULL,
            "stored the best primal solution in the original solution candidate list\n");
      }
   }

   /* switch stage to FREETRANS */
   scip->set->stage = SCIP_STAGE_FREETRANS;

   /* reset solving specific paramters */
   if( scip->set->reopt_enable )
   {
      assert(scip->reopt != NULL);
      SCIP_CALL( SCIPreoptReset(scip->reopt, scip->set, scip->mem->probmem) );
   }

   /* @todo if a variable was removed from the problem during solving, its locks were not reduced;
    *       we might want to remove locks also in that case
    */
   /* remove var locks set to avoid dual reductions */
   if( scip->set->reopt_enable || !scip->set->misc_allowdualreds )
   {
      int v;

      /* unlock all variables */
      for(v = 0; v < scip->transprob->nvars; v++)
      {
         SCIP_CALL( SCIPaddVarLocksType(scip, scip->transprob->vars[v], SCIP_LOCKTYPE_MODEL, -1, -1) );
      }
   }

   if( !reducedfree )
   {
      /* clear the conflict store
       *
       * since the conflict store can contain transformed constraints we need to remove them. the store will be finally
       * freed in SCIPfreeProb().
       */
      SCIP_CALL( SCIPconflictstoreClear(scip->conflictstore, scip->mem->probmem, scip->set, scip->stat, scip->reopt) );

   }

   /* free transformed problem data structures */
   SCIP_CALL( SCIPprobFree(&scip->transprob, scip->messagehdlr, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->lp) );
   SCIP_CALL( SCIPcliquetableFree(&scip->cliquetable, scip->mem->probmem) );
   SCIP_CALL( SCIPconflictFree(&scip->conflict, scip->mem->probmem) );

   if( !reducedfree )
   {
      SCIP_CALL( SCIPrelaxationFree(&scip->relaxation) );
   }
   SCIP_CALL( SCIPtreeFree(&scip->tree, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->lp) );

   /* free the debug solution which might live in transformed primal data structure */
   SCIP_CALL( SCIPdebugFreeSol(scip->set) ); /*lint !e506 !e774*/
   SCIP_CALL( SCIPprimalFree(&scip->primal, scip->mem->probmem) );

   SCIP_CALL( SCIPlpFree(&scip->lp, scip->mem->probmem, scip->set, scip->eventqueue, scip->eventfilter) );
   SCIP_CALL( SCIPbranchcandFree(&scip->branchcand) );
   SCIP_CALL( SCIPeventfilterFree(&scip->eventfilter, scip->mem->probmem, scip->set) );
   SCIP_CALL( SCIPeventqueueFree(&scip->eventqueue) );

   if( scip->set->misc_resetstat && !reducedfree )
   {
      /* reset statistics to the point before the problem was transformed */
      SCIPstatReset(scip->stat, scip->set, scip->transprob, scip->origprob);
   }
   else
   {
      /* even if statistics are not completely reset, a partial reset of the primal-dual integral is necessary */
      SCIPstatResetPrimalDualIntegral(scip->stat, scip->set, TRUE);
   }

   /* switch stage to PROBLEM */
   scip->set->stage = SCIP_STAGE_PROBLEM;

   /* reset objective limit */
   SCIP_CALL( SCIPsetObjlimit(scip, SCIP_INVALID) );

   /* reset original variable's local and global bounds to their original values */
   SCIP_CALL( SCIPprobResetBounds(scip->origprob, scip->mem->probmem, scip->set, scip->stat) );

   return SCIP_OKAY;
}

/** displays most relevant statistics after problem was solved */
static
SCIP_RETCODE displayRelevantStats(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   /* display most relevant statistics */
   if( scip->set->disp_verblevel >= SCIP_VERBLEVEL_NORMAL && scip->set->disp_relevantstats )
   {
      SCIP_Bool objlimitreached = FALSE;

      if( SCIPgetStage(scip) == SCIP_STAGE_SOLVED && scip->primal->nlimsolsfound == 0
         && !SCIPisInfinity(scip, SCIPgetPrimalbound(scip)) )
         objlimitreached = TRUE;

      SCIPmessagePrintInfo(scip->messagehdlr, "\n");
      SCIPmessagePrintInfo(scip->messagehdlr, "SCIP Status        : ");
      SCIP_CALL( SCIPprintStage(scip, NULL) );
      SCIPmessagePrintInfo(scip->messagehdlr, "\n");
      if( scip->set->reopt_enable )
         SCIPmessagePrintInfo(scip->messagehdlr, "Solving Time (sec) : %.2f (over %d runs: %.2f)\n", SCIPclockGetTime(scip->stat->solvingtime), scip->stat->nreoptruns, SCIPclockGetTime(scip->stat->solvingtimeoverall));
      else
         SCIPmessagePrintInfo(scip->messagehdlr, "Solving Time (sec) : %.2f\n", SCIPclockGetTime(scip->stat->solvingtime));
      if( scip->stat->nruns > 1 )
         SCIPmessagePrintInfo(scip->messagehdlr, "Solving Nodes      : %" SCIP_LONGINT_FORMAT " (total of %" SCIP_LONGINT_FORMAT " nodes in %d runs)\n",
            scip->stat->nnodes, scip->stat->ntotalnodes, scip->stat->nruns);
      else if( scip->set->reopt_enable )
      {
         SCIP_BRANCHRULE* branchrule;

         branchrule = SCIPfindBranchrule(scip, "nodereopt");
         assert(branchrule != NULL);

         SCIPmessagePrintInfo(scip->messagehdlr, "Solving Nodes      : %" SCIP_LONGINT_FORMAT " (%" SCIP_LONGINT_FORMAT " reactivated)\n", scip->stat->nnodes, SCIPbranchruleGetNChildren(branchrule));
      }
      else
         SCIPmessagePrintInfo(scip->messagehdlr, "Solving Nodes      : %" SCIP_LONGINT_FORMAT "\n", scip->stat->nnodes);
      if( scip->set->stage >= SCIP_STAGE_TRANSFORMED && scip->set->stage <= SCIP_STAGE_EXITSOLVE )
      {
         if( objlimitreached )
         {
            SCIPmessagePrintInfo(scip->messagehdlr, "Primal Bound       : %+.14e (%" SCIP_LONGINT_FORMAT " solutions)\n",
               SCIPinfinity(scip), scip->primal->nsolsfound);
         }
         else
         {
            char limsolstring[SCIP_MAXSTRLEN];
            if( scip->primal->nsolsfound != scip->primal->nlimsolsfound )
               (void) SCIPsnprintf(limsolstring, SCIP_MAXSTRLEN, ", %" SCIP_LONGINT_FORMAT " respecting the objective limit", scip->primal->nlimsolsfound);
            else
               (void) SCIPsnprintf(limsolstring, SCIP_MAXSTRLEN,"");

            SCIPmessagePrintInfo(scip->messagehdlr, "Primal Bound       : %+.14e (%" SCIP_LONGINT_FORMAT " solutions%s)\n",
               SCIPgetPrimalbound(scip), scip->primal->nsolsfound, limsolstring);
         }
      }
      if( scip->set->stage >= SCIP_STAGE_SOLVING && scip->set->stage <= SCIP_STAGE_SOLVED )
      {
         if( objlimitreached )
         {
            SCIPmessagePrintInfo(scip->messagehdlr, "Dual Bound         : %+.14e\n", SCIPinfinity(scip));
         }
         else
         {
            SCIPmessagePrintInfo(scip->messagehdlr, "Dual Bound         : %+.14e\n", SCIPgetDualbound(scip));
         }
         SCIPmessagePrintInfo(scip->messagehdlr, "Gap                : ");
         if( SCIPsetIsInfinity(scip->set, SCIPgetGap(scip)) )
            SCIPmessagePrintInfo(scip->messagehdlr, "infinite\n");
         else
            SCIPmessagePrintInfo(scip->messagehdlr, "%.2f %%\n", 100.0*SCIPgetGap(scip));
      }

      /* check solution for feasibility in original problem */
      if( scip->set->stage >= SCIP_STAGE_TRANSFORMED )
      {
         SCIP_SOL* sol;

         sol = SCIPgetBestSol(scip);
         if( sol != NULL )
         {
            SCIP_Real checkfeastolfac;
            SCIP_Real oldfeastol;
            SCIP_Bool dispallviols;
            SCIP_Bool feasible;

            oldfeastol = SCIPfeastol(scip);
            SCIP_CALL( SCIPgetRealParam(scip, "numerics/checkfeastolfac", &checkfeastolfac) );
            SCIP_CALL( SCIPgetBoolParam(scip, "display/allviols", &dispallviols) );

            /* scale feasibility tolerance by set->num_checkfeastolfac */
            if( !SCIPisEQ(scip, checkfeastolfac, 1.0) )
            {
               SCIP_CALL( SCIPchgFeastol(scip, oldfeastol * checkfeastolfac) );
            }

            SCIP_CALL( SCIPcheckSolOrig(scip, sol, &feasible, TRUE, dispallviols) );

            /* restore old feasibilty tolerance */
            if( !SCIPisEQ(scip, checkfeastolfac, 1.0) )
            {
               SCIP_CALL( SCIPchgFeastol(scip, oldfeastol) );
            }

            if( !feasible )
            {
               SCIPmessagePrintInfo(scip->messagehdlr, "best solution is not feasible in original problem\n");
            }
         }
      }
   }

   return SCIP_OKAY;
}

/** calls compression based on the reoptimization structure after the presolving */
static
SCIP_RETCODE compressReoptTree(
   SCIP*                 scip                /**< global SCIP settings */
   )
{
   SCIP_RESULT result;
   int c;
   int noldnodes;
   int nnewnodes;

   result = SCIP_DIDNOTFIND;

   noldnodes = SCIPreoptGetNNodes(scip->reopt, scip->tree->root);

   /* do not run if there exists only the root node */
   if( noldnodes <= 1 )
      return SCIP_OKAY;

   /* do not run a tree compression if the problem contains (implicit) integer variables */
   if( scip->transprob->nintvars > 0 || scip->transprob->nimplvars > 0 )
      return SCIP_OKAY;

   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
         "tree compression:\n");
   SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
         "  given tree has %d nodes.\n", noldnodes);

   /* sort compressions by priority */
   SCIPsetSortComprs(scip->set);

   for(c = 0; c < scip->set->ncomprs; c++)
   {
      assert(result == SCIP_DIDNOTFIND || result == SCIP_DIDNOTRUN);

      /* call tree compression technique */
      SCIP_CALL( SCIPcomprExec(scip->set->comprs[c], scip->set, scip->reopt, &result) );

      if( result == SCIP_SUCCESS )
      {
         nnewnodes = SCIPreoptGetNNodes(scip->reopt, scip->tree->root);
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
               "  <%s> compressed the search tree to %d nodes (rate %g).\n", SCIPcomprGetName(scip->set->comprs[c]),
               nnewnodes, ((SCIP_Real)nnewnodes)/noldnodes);

         break;
      }
   }

   if( result != SCIP_SUCCESS )
   {
      assert(result == SCIP_DIDNOTFIND || result == SCIP_DIDNOTRUN);
      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
            "  search tree could not be compressed.\n");
   }

   return SCIP_OKAY;
}

/* prepare all plugins and data structures for a reoptimization run */
static
SCIP_RETCODE prepareReoptimization(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Bool reoptrestart;

   assert(scip != NULL);
   assert(scip->set->reopt_enable);

   /* @ todo: we could check if the problem is feasible, eg, by backtracking */

   /* increase number of reopt_runs */
   ++scip->stat->nreoptruns;

   /* inform the reoptimization plugin that a new iteration starts */
   SCIP_CALL( SCIPreoptAddRun(scip->reopt, scip->set, scip->mem->probmem, scip->origprob->vars,
         scip->origprob->nvars, scip->set->limit_maxsol) );

   /* check whether we need to add globally valid constraints */
   if( scip->set->reopt_sepaglbinfsubtrees || scip->set->reopt_sepabestsol )
   {
      SCIP_CALL( SCIPreoptApplyGlbConss(scip, scip->reopt, scip->set, scip->stat, scip->mem->probmem) );
   }

   /* after presolving the problem the first time we remember all global bounds and active constraints. bounds and
    * constraints will be restored within SCIPreoptInstallBounds() and SCIPreoptResetActiveConss().
    */
   if( scip->stat->nreoptruns == 1 )
   {
      assert(scip->set->stage == SCIP_STAGE_PRESOLVED);

      SCIP_CALL( SCIPreoptSaveGlobalBounds(scip->reopt, scip->transprob, scip->mem->probmem) );

      SCIP_CALL( SCIPreoptSaveActiveConss(scip->reopt, scip->transprob, scip->mem->probmem) );
   }
   /* we are at least in the second run */
   else
   {
      assert(scip->transprob != NULL);

      SCIP_CALL( SCIPreoptMergeVarHistory(scip->reopt, scip->set, scip->stat, scip->origprob->vars, scip->origprob->nvars) );

      SCIP_CALL( SCIPrelaxationCreate(&scip->relaxation, scip->mem->probmem, scip->set, scip->stat, scip->primal,
            scip->tree) );

      /* mark statistics before solving */
      SCIPstatMark(scip->stat);

      SCIPbranchcandInvalidate(scip->branchcand);

      SCIP_CALL( SCIPreoptResetActiveConss(scip->reopt, scip->set, scip->stat) );

      /* check whether we want to restart the tree search */
      SCIP_CALL( SCIPreoptCheckRestart(scip->reopt, scip->set, scip->mem->probmem, NULL, scip->transprob->vars,
            scip->transprob->nvars, &reoptrestart) );

      /* call initialization methods of plugins */
      SCIP_CALL( SCIPsetInitPlugins(scip->set, scip->mem->probmem, scip->stat) );

      /* install globally valid lower and upper bounds */
      SCIP_CALL( SCIPreoptInstallBounds(scip->reopt, scip->set, scip->stat, scip->transprob, scip->lp, scip->branchcand,
            scip->eventqueue, scip->cliquetable, scip->mem->probmem) );

      /* check, whether objective value is always integral by inspecting the problem, if it is the case adjust the
       * cutoff bound if primal solution is already known
       */
      SCIP_CALL( SCIPprobCheckObjIntegral(scip->transprob, scip->origprob, scip->mem->probmem, scip->set, scip->stat,
            scip->primal, scip->tree, scip->reopt, scip->lp, scip->eventqueue) );

      /* if possible, scale objective function such that it becomes integral with gcd 1 */
      SCIP_CALL( SCIPprobScaleObj(scip->transprob, scip->origprob, scip->mem->probmem, scip->set, scip->stat, scip->primal,
            scip->tree, scip->reopt, scip->lp, scip->eventqueue) );

      SCIPlpRecomputeLocalAndGlobalPseudoObjval(scip->lp, scip->set, scip->transprob);
   }

   /* try to compress the search tree */
   if( scip->set->compr_enable )
   {
      SCIP_CALL( compressReoptTree(scip) );
   }

   return SCIP_OKAY;
}

/** transforms and presolves problem
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *
 *  @post After calling this method \SCIP reaches one of the following stages:
 *        - \ref SCIP_STAGE_PRESOLVING if the presolving process was interrupted
 *        - \ref SCIP_STAGE_PRESOLVED if the presolving process was finished and did not solve the problem
 *        - \ref SCIP_STAGE_SOLVED if the problem was solved during presolving
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPpresolve(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Bool unbounded;
   SCIP_Bool infeasible;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPpresolve", FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   /* start solving timer */
   SCIPclockStart(scip->stat->solvingtime, scip->set);
   SCIPclockStart(scip->stat->solvingtimeoverall, scip->set);

   /* capture the CTRL-C interrupt */
   if( scip->set->misc_catchctrlc )
      SCIPinterruptCapture(scip->interrupt);

   /* reset the user interrupt flag */
   scip->stat->userinterrupt = FALSE;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      /* initialize solving data structures and transform problem */
      SCIP_CALL( SCIPtransformProb(scip) );
      assert(scip->set->stage == SCIP_STAGE_TRANSFORMED);

      /*lint -fallthrough*/

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_PRESOLVING:
      /* presolve problem */
      SCIP_CALL( presolve(scip, &unbounded, &infeasible) );
      assert(scip->set->stage == SCIP_STAGE_PRESOLVED || scip->set->stage == SCIP_STAGE_PRESOLVING);

      if( infeasible || unbounded )
      {
         assert(scip->set->stage == SCIP_STAGE_PRESOLVED);

         /* initialize solving process data structures to be able to switch to SOLVED stage */
         SCIP_CALL( initSolve(scip, TRUE) );

         /* switch stage to SOLVED */
         scip->set->stage = SCIP_STAGE_SOLVED;

         /* print solution message */
         switch( scip->stat->status )/*lint --e{788}*/
         {
         case SCIP_STATUS_OPTIMAL:
            /* remove the root node from the tree, s.t. the lower bound is set to +infinity ???????????? (see initSolve())*/
            SCIP_CALL( SCIPtreeClear(scip->tree, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue, scip->lp) );
            break;

         case SCIP_STATUS_INFEASIBLE:
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL,
               "presolving detected infeasibility\n");
            break;

         case SCIP_STATUS_UNBOUNDED:
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL,
               "presolving detected unboundedness\n");
            break;

         case SCIP_STATUS_INFORUNBD:
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL,
               "presolving detected unboundedness (or infeasibility)\n");
            break;

         default:
            /* note that this is in an internal SCIP error since the status is corrupted */
            SCIPerrorMessage("invalid SCIP status <%d>\n", scip->stat->status);
            SCIPABORT();
            return SCIP_ERROR; /*lint !e527*/
         }
      }
      else if( scip->set->stage == SCIP_STAGE_PRESOLVED )
      {
         int h;

         /* print presolved problem statistics */
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL,
            "presolved problem has %d variables (%d bin, %d int, %d impl, %d cont) and %d constraints\n",
            scip->transprob->nvars, scip->transprob->nbinvars, scip->transprob->nintvars, scip->transprob->nimplvars,
            scip->transprob->ncontvars, scip->transprob->nconss);

         for( h = 0; h < scip->set->nconshdlrs; ++h )
         {
            int nactiveconss;

            nactiveconss = SCIPconshdlrGetNActiveConss(scip->set->conshdlrs[h]);
            if( nactiveconss > 0 )
            {
               SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
                  "%7d constraints of type <%s>\n", nactiveconss, SCIPconshdlrGetName(scip->set->conshdlrs[h]));
            }
         }

         if( SCIPprobIsObjIntegral(scip->transprob) )
         {
            SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
               "transformed objective value is always integral (scale: %.15g)\n", scip->transprob->objscale);
         }
      }
      else
      {
         assert(scip->set->stage == SCIP_STAGE_PRESOLVING);
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH, "presolving was interrupted.\n");
      }

      /* display timing statistics */
      SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_HIGH,
         "Presolving Time: %.2f\n", SCIPclockGetTime(scip->stat->presolvingtime));
      break;

   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVED:
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   /* release the CTRL-C interrupt */
   if( scip->set->misc_catchctrlc )
      SCIPinterruptRelease(scip->interrupt);

   /* stop solving timer */
   SCIPclockStop(scip->stat->solvingtime, scip->set);
   SCIPclockStop(scip->stat->solvingtimeoverall, scip->set);

   if( scip->set->stage == SCIP_STAGE_SOLVED )
   {
      /* display most relevant statistics */
      SCIP_CALL( displayRelevantStats(scip) );
   }

   return SCIP_OKAY;
}

/** transforms, presolves, and solves problem
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  @post After calling this method \SCIP reaches one of the following stages depending on if and when the solution
 *        process was interrupted:
 *        - \ref SCIP_STAGE_PRESOLVING if the solution process was interrupted during presolving
 *        - \ref SCIP_STAGE_SOLVING if the solution process was interrupted during the tree search
 *        - \ref SCIP_STAGE_SOLVED if the solving process was not interrupted
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPsolve(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Bool statsprinted = FALSE;
   SCIP_Bool restart;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPsolve", FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* if the stage is already SCIP_STAGE_SOLVED do nothing */
   if( scip->set->stage == SCIP_STAGE_SOLVED )
      return SCIP_OKAY;

   if( scip->stat->status == SCIP_STATUS_INFEASIBLE || scip->stat->status == SCIP_STATUS_OPTIMAL || scip->stat->status == SCIP_STATUS_UNBOUNDED || scip->stat->status == SCIP_STATUS_INFORUNBD )
   {
      SCIPwarningMessage(scip, "SCIPsolve() was called, but problem is already solved\n");
      return SCIP_OKAY;
   }

   /* check, if a node selector exists */
   if( SCIPsetGetNodesel(scip->set, scip->stat) == NULL )
   {
      SCIPerrorMessage("no node selector available\n");
      return SCIP_PLUGINNOTFOUND;
   }

   /* check, if an integrality constraint handler exists if there are integral variables */
   if( (SCIPgetNBinVars(scip) >= 0 || SCIPgetNIntVars(scip) >= 0) && SCIPfindConshdlr(scip, "integral") == NULL )
   {
      SCIPwarningMessage(scip, "integrality constraint handler not available\n");
   }

   /* initialize presolving flag (may be modified in SCIPpresolve()) */
   scip->stat->performpresol = FALSE;

   /* start solving timer */
   SCIPclockStart(scip->stat->solvingtime, scip->set);
   SCIPclockStart(scip->stat->solvingtimeoverall, scip->set);

   /* capture the CTRL-C interrupt */
   if( scip->set->misc_catchctrlc )
      SCIPinterruptCapture(scip->interrupt);

   /* reset the user interrupt flag */
   scip->stat->userinterrupt = FALSE;

   /* automatic restarting loop */
   restart = scip->stat->userrestart;

   do
   {
      if( restart )
      {
         /* free the solving process data in order to restart */
         assert(scip->set->stage == SCIP_STAGE_SOLVING);
         if( scip->stat->userrestart )
            SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL,
               "(run %d, node %" SCIP_LONGINT_FORMAT ") performing user restart\n",
               scip->stat->nruns, scip->stat->nnodes, scip->stat->nrootintfixingsrun);
         else
            SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL,
               "(run %d, node %" SCIP_LONGINT_FORMAT ") restarting after %d global fixings of integer variables\n",
               scip->stat->nruns, scip->stat->nnodes, scip->stat->nrootintfixingsrun);
         /* an extra blank line should be printed separately since the buffer message handler only handles up to one line
          * correctly */
         SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "\n");
         /* reset relaxation solution, so that the objective value is recomputed from scratch next time, using the new
          * fixings which may be produced during the presolving after the restart */
         SCIP_CALL( SCIPclearRelaxSolVals(scip) );

         SCIP_CALL( freeSolve(scip, TRUE) );
         assert(scip->set->stage == SCIP_STAGE_TRANSFORMED);
      }
      restart = FALSE;
      scip->stat->userrestart = FALSE;

      switch( scip->set->stage )
      {
      case SCIP_STAGE_PROBLEM:
      case SCIP_STAGE_TRANSFORMED:
      case SCIP_STAGE_PRESOLVING:
         /* initialize solving data structures, transform and problem */

         SCIP_CALL( SCIPpresolve(scip) );
         /* remember that we already printed the relevant statistics */
         if( scip->set->stage == SCIP_STAGE_SOLVED )
            statsprinted = TRUE;

         if( scip->set->stage == SCIP_STAGE_SOLVED || scip->set->stage == SCIP_STAGE_PRESOLVING )
            break;
         assert(scip->set->stage == SCIP_STAGE_PRESOLVED);

         /*lint -fallthrough*/

      case SCIP_STAGE_PRESOLVED:
         /* check if reoptimization is enabled and global constraints are saved */
         if( scip->set->reopt_enable )
         {
            SCIP_CALL( prepareReoptimization(scip) );
         }

         /* initialize solving process data structures */
         SCIP_CALL( initSolve(scip, FALSE) );
         assert(scip->set->stage == SCIP_STAGE_SOLVING);
         SCIPmessagePrintVerbInfo(scip->messagehdlr, scip->set->disp_verblevel, SCIP_VERBLEVEL_NORMAL, "\n");

         /*lint -fallthrough*/

      case SCIP_STAGE_SOLVING:
         /* reset display */
         SCIPstatResetDisplay(scip->stat);

         /* continue solution process */
         SCIP_CALL( SCIPsolveCIP(scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat, scip->mem, scip->origprob, scip->transprob,
               scip->primal, scip->tree, scip->reopt, scip->lp, scip->relaxation, scip->pricestore, scip->sepastore,
               scip->cutpool, scip->delayedcutpool, scip->branchcand, scip->conflict, scip->conflictstore,
               scip->eventfilter, scip->eventqueue, scip->cliquetable, &restart) );

         /* detect, whether problem is solved */
         if( SCIPtreeGetNNodes(scip->tree) == 0 && SCIPtreeGetCurrentNode(scip->tree) == NULL )
         {
            assert(scip->stat->status == SCIP_STATUS_OPTIMAL
               || scip->stat->status == SCIP_STATUS_INFEASIBLE
               || scip->stat->status == SCIP_STATUS_UNBOUNDED
               || scip->stat->status == SCIP_STATUS_INFORUNBD);
            assert(!restart);

            /* tree is empty, and no current node exists -> problem is solved */
            scip->set->stage = SCIP_STAGE_SOLVED;
         }
         break;

      case SCIP_STAGE_SOLVED:
         assert(scip->stat->status == SCIP_STATUS_OPTIMAL
            || scip->stat->status == SCIP_STATUS_INFEASIBLE
            || scip->stat->status == SCIP_STATUS_UNBOUNDED
            || scip->stat->status == SCIP_STATUS_INFORUNBD);

         break;

      default:
         SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
         return SCIP_INVALIDCALL;
      }  /*lint !e788*/
   }
   while( restart && !SCIPsolveIsStopped(scip->set, scip->stat, TRUE) );

   /* we have to store all unprocessed nodes if reoptimization is enabled */
   if( scip->set->reopt_enable && scip->set->stage != SCIP_STAGE_PRESOLVING
    && SCIPsolveIsStopped(scip->set, scip->stat, TRUE) )
   {
      /* save unprocessed nodes */
      if( SCIPgetNNodesLeft(scip) > 0 )
      {
         SCIP_NODE** leaves;
         SCIP_NODE** children;
         SCIP_NODE** siblings;
         int nleaves;
         int nchildren;
         int nsiblings;

         /* get all open leave nodes */
         SCIP_CALL( SCIPgetLeaves(scip, &leaves, &nleaves) );

         /* get all open children nodes */
         SCIP_CALL( SCIPgetChildren(scip, &children, &nchildren) );

         /* get all open sibling nodes */
         SCIP_CALL( SCIPgetSiblings(scip, &siblings, &nsiblings) );

         /* add all open node to the reoptimization tree */
         SCIP_CALL( SCIPreoptSaveOpenNodes(scip->reopt, scip->set, scip->lp, scip->mem->probmem, leaves, nleaves,
               children, nchildren, siblings, nsiblings) );
      }
   }

   /* release the CTRL-C interrupt */
   if( scip->set->misc_catchctrlc )
      SCIPinterruptRelease(scip->interrupt);

   if( scip->set->reopt_enable )
   {
      /* save found solutions */
      int nsols;
      int s;

      nsols = scip->set->reopt_savesols == -1 ? INT_MAX : MAX(scip->set->reopt_savesols, 1);
      nsols = MIN(scip->primal->nsols, nsols);

      for( s = 0; s < nsols; s++ )
      {
         SCIP_SOL* sol;
         SCIP_Bool added;

         sol = scip->primal->sols[s];
         assert(sol != NULL);

         if( !SCIPsolIsOriginal(sol) )
         {
            SCIP_Bool hasinfval;

            /* retransform solution into the original problem space */
            SCIP_CALL( SCIPsolRetransform(sol, scip->set, scip->stat, scip->origprob, scip->transprob, &hasinfval) );
         }

         if( SCIPsolGetNodenum(sol) > 0 || SCIPsolGetHeur(sol) != NULL || (s == 0 && scip->set->reopt_sepabestsol) )
         {
            /* if the best solution should be separated, we must not store it in the solution tree */
            if( s == 0 && scip->set->reopt_sepabestsol )
            {
               SCIP_CALL( SCIPreoptAddOptSol(scip->reopt, sol, scip->mem->probmem, scip->set, scip->stat, scip->origprimal,
                     scip->origprob->vars, scip->origprob->nvars) );
            }
            /* add solution to solution tree */
            else
            {
               SCIPdebugMsg(scip, "try to add solution to the solution tree:\n");
               SCIPdebug( SCIP_CALL( SCIPsolPrint(sol, scip->set, scip->messagehdlr, scip->stat, scip->origprob, \
                     scip->transprob, NULL, FALSE, FALSE) ); );

               SCIP_CALL( SCIPreoptAddSol(scip->reopt, scip->set, scip->stat, scip->origprimal, scip->mem->probmem,
                     sol, s == 0, &added, scip->origprob->vars, scip->origprob->nvars, scip->stat->nreoptruns) );
            }
         }
      }

      SCIPdebugMsg(scip, "-> saved %d solution.\n", nsols);

      /* store variable history */
      if( scip->set->reopt_storevarhistory )
      {
         SCIP_CALL( SCIPreoptUpdateVarHistory(scip->reopt, scip->set, scip->stat, scip->mem->probmem,
               scip->origprob->vars, scip->origprob->nvars) );
      }
   }

   /* stop solving timer */
   SCIPclockStop(scip->stat->solvingtime, scip->set);
   SCIPclockStop(scip->stat->solvingtimeoverall, scip->set);

   /* decrease time limit during reoptimization */
   if( scip->set->reopt_enable && scip->set->reopt_commontimelimit )
   {
      SCIP_Real timelimit;
      SCIP_Real usedtime;

      SCIP_CALL( SCIPgetRealParam(scip, "limits/time", &timelimit) );
      usedtime = SCIPgetSolvingTime(scip);
      timelimit = timelimit - usedtime;
      timelimit = MAX(0, timelimit);

      SCIP_CALL( SCIPsetRealParam(scip, "limits/time", timelimit) );
   }

   if( !statsprinted )
   {
      /* display most relevant statistics */
      SCIP_CALL( displayRelevantStats(scip) );
   }

   return SCIP_OKAY;
}

/** transforms, presolves, and solves problem using the configured concurrent solvers
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  @post After calling this method \SCIP reaches one of the following stages depending on if and when the solution
 *        process was interrupted:
 *        - \ref SCIP_STAGE_PRESOLVING if the solution process was interrupted during presolving
 *        - \ref SCIP_STAGE_SOLVING if the solution process was interrupted during the tree search
 *        - \ref SCIP_STAGE_SOLVED if the solving process was not interrupted
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 *
 *  @deprecated Please use SCIPsolveConcurrent() instead.
 */
SCIP_RETCODE SCIPsolveParallel(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsolveParallel", FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPsolveConcurrent(scip);
}

/** transforms, presolves, and solves problem using the configured concurrent solvers
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  @post After calling this method \SCIP reaches one of the following stages depending on if and when the solution
 *        process was interrupted:
 *        - \ref SCIP_STAGE_PRESOLVING if the solution process was interrupted during presolving
 *        - \ref SCIP_STAGE_SOLVING if the solution process was interrupted during the tree search
 *        - \ref SCIP_STAGE_SOLVED if the solving process was not interrupted
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPsolveConcurrent(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
#ifdef TPI_NONE
   SCIPinfoMessage(scip, NULL, "SCIP was compiled without task processing interface. Parallel solve not possible\n");
   return SCIP_OKAY;
#else
   SCIP_RETCODE     retcode;
   int              i;
   SCIP_RANDNUMGEN* rndgen;
   int              minnthreads;
   int              maxnthreads;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPsolveConcurrent", FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPsetIntParam(scip, "timing/clocktype", SCIP_CLOCKTYPE_WALL) );

   minnthreads = scip->set->parallel_minnthreads;
   maxnthreads = scip->set->parallel_maxnthreads;

   if( minnthreads > maxnthreads )
   {
      SCIPerrorMessage("minimum number of threads greater than maximum number of threads\n");
      return SCIP_INVALIDDATA;
   }
   if( scip->concurrent == NULL )
   {
      int                   nconcsolvertypes;
      SCIP_CONCSOLVERTYPE** concsolvertypes;
      SCIP_Longint          nthreads;
      SCIP_Real             memorylimit;
      int*                  solvertypes;
      SCIP_Longint*         weights;
      SCIP_Real*            prios;
      int                   ncandsolvertypes;
      SCIP_Real             prefpriosum;

      /* check if concurrent solve is configured to presolve the problem
       * before setting up the concurrent solvers
       */
      if( scip->set->concurrent_presolvebefore )
      {
         /* if yes, then presolve the problem */
         SCIP_CALL( SCIPpresolve(scip) );
      }
      else
      {
         SCIP_Bool infeas;

         /* if not, transform the problem and switch stage to presolved */
         SCIP_CALL( SCIPtransformProb(scip) );
         SCIP_CALL( initPresolve(scip) );
         SCIP_CALL( exitPresolve(scip, TRUE, &infeas) );
         assert(!infeas);
      }

      /* the presolving must have run into a limit, so we stop here */
      if( scip->set->stage < SCIP_STAGE_PRESOLVED )
      {
         SCIP_CALL( displayRelevantStats(scip) );
         return SCIP_OKAY;
      }

      nthreads = INT_MAX;
      /* substract the memory already used by the main SCIP and the estimated memory usage of external software */
      memorylimit = scip->set->limit_memory;
      if( memorylimit < SCIP_MEM_NOLIMIT )
      {
         memorylimit -= SCIPgetMemUsed(scip)/1048576.0;
         memorylimit -= SCIPgetMemExternEstim(scip)/1048576.0;
         /* estimate maximum number of copies that be created based on memory limit */
         nthreads = MAX(1, memorylimit / (4.0*SCIPgetMemExternEstim(scip)/1048576.0));
         SCIPverbMessage(scip, SCIP_VERBLEVEL_FULL, NULL, "estimated a maximum of %lli threads based on memory limit\n", nthreads);
      }
      nconcsolvertypes = SCIPgetNConcsolverTypes(scip);
      concsolvertypes = SCIPgetConcsolverTypes(scip);

      if( minnthreads > nthreads )
      {
         SCIP_CALL( initSolve(scip, TRUE) );
         scip->stat->status = SCIP_STATUS_MEMLIMIT;
         SCIPsyncstoreSetSolveIsStopped(SCIPgetSyncstore(scip), TRUE);
         SCIPwarningMessage(scip, "requested minimum number of threads could not be satisfied with given memory limit\n");
         SCIP_CALL( displayRelevantStats(scip) );
         return SCIP_OKAY;
      }

      if( nthreads == 1 )
      {
         SCIPwarningMessage(scip, "can only use 1 thread, doing sequential solve instead\n");
         SCIP_CALL( SCIPfreeConcurrent(scip) );
         return SCIPsolve(scip);
      }
      nthreads = MIN(nthreads, maxnthreads);
      SCIPverbMessage(scip, SCIP_VERBLEVEL_FULL, NULL, "using %lli threads for concurrent solve\n", nthreads);

      /* now set up nthreads many concurrent solvers that will be used for the concurrent solve
       * using the preferred priorities of each concurrent solver
       */
      prefpriosum = 0.0;
      for( i = 0; i < nconcsolvertypes; ++i )
         prefpriosum += SCIPconcsolverTypeGetPrefPrio(concsolvertypes[i]);

      ncandsolvertypes = 0;
      SCIP_CALL( SCIPallocBufferArray(scip, &solvertypes, nthreads + nconcsolvertypes) );
      SCIP_CALL( SCIPallocBufferArray(scip, &weights, nthreads + nconcsolvertypes) );
      SCIP_CALL( SCIPallocBufferArray(scip, &prios, nthreads + nconcsolvertypes) );
      for( i = 0; i < nconcsolvertypes; ++i )
      {
         int j;
         SCIP_Real prio;
         prio = nthreads * SCIPconcsolverTypeGetPrefPrio(concsolvertypes[i]) / prefpriosum;
         while( prio > 0.0 )
         {
            j = ncandsolvertypes++;
            assert(j < 2*nthreads);
            weights[j] = 1;
            solvertypes[j] = i;
            prios[j] = MIN(1.0, prio);
            prio = prio - 1.0;
         }
      }
      /* select nthreads many concurrent solver types to create instances
       * according to the preferred prioriteis the user has set
       * This basically corresponds to a knapsack problem
       * with unit weights and capacity nthreads, where the profits are
       * the unrounded fraction of the total number of threads to be used.
       */
      SCIPselectDownRealInt(prios, solvertypes, nthreads, ncandsolvertypes);

      SCIP_CALL( SCIPcreateRandom(scip, &rndgen, (unsigned) scip->set->concurrent_initseed, TRUE) );
      for( i = 0; i < nthreads; ++i )
      {
         SCIP_CONCSOLVER* concsolver;

         SCIP_CALL( SCIPconcsolverCreateInstance(scip->set, concsolvertypes[solvertypes[i]], &concsolver) );
         if( scip->set->concurrent_changeseeds && SCIPgetNConcurrentSolvers(scip) > 1 )
            SCIP_CALL( SCIPconcsolverInitSeeds(concsolver, SCIPrandomGetInt(rndgen, 0, INT_MAX)) );
      }
      SCIPfreeRandom(scip, &rndgen);
      SCIPfreeBufferArray(scip, &prios);
      SCIPfreeBufferArray(scip, &weights);
      SCIPfreeBufferArray(scip, &solvertypes);

      assert(SCIPgetNConcurrentSolvers(scip) == nthreads);

      SCIP_CALL( SCIPsyncstoreInit(scip) );
   }

   if( SCIPgetStage(scip) == SCIP_STAGE_PRESOLVED )
   {
      /* switch stage to solving */
      SCIP_CALL( initSolve(scip, TRUE) );
   }

   SCIPclockStart(scip->stat->solvingtime, scip->set);
   retcode = SCIPconcurrentSolve(scip);
   SCIPclockStop(scip->stat->solvingtime, scip->set);
   SCIP_CALL( displayRelevantStats(scip) );

   return retcode;
#endif
}

/** include specific heuristics and branching rules for reoptimization
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 */
SCIP_RETCODE SCIPenableReoptimization(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             enable              /**< enable reoptimization (TRUE) or disable it (FALSE) */
   )
{
   assert(scip != NULL);

   /* we want to skip if nothing has changed */
   if( (enable && scip->set->reopt_enable && scip->reopt != NULL)
    || (!enable && !scip->set->reopt_enable && scip->reopt == NULL) )
      return SCIP_OKAY;

   /* check stage and throw an error if we try to disable reoptimization during the solving process.
    *
    * @note the case that we will disable the reoptimization and have already performed presolving can only happen if
    *       we are try to solve a general MIP
    *
    * @note this fix is only for the bugfix release 3.2.1, in the next major release reoptimization can be used for
    *       general MIPs, too.
    */
   if( scip->set->stage > SCIP_STAGE_PROBLEM && !(!enable && scip->set->stage == SCIP_STAGE_PRESOLVED) )
   {
      SCIPerrorMessage("reoptimization cannot be %s after starting the (pre)solving process\n", enable ? "enabled" : "disabled");
      return SCIP_INVALIDCALL;
   }

   /* if the current stage is SCIP_STAGE_PROBLEM we have to include the heuristics and branching rule */
   if( scip->set->stage == SCIP_STAGE_PROBLEM || (!enable && scip->set->stage == SCIP_STAGE_PRESOLVED) )
   {
      /* initialize all reoptimization data structures */
      if( enable && scip->reopt == NULL )
      {
         /* set enable flag */
         scip->set->reopt_enable = enable;

         SCIP_CALL( SCIPreoptCreate(&scip->reopt, scip->set, scip->mem->probmem) );
         SCIP_CALL( SCIPsetSetReoptimizationParams(scip->set, scip->messagehdlr) );
      }
      /* disable all reoptimization plugins and free the structure if necessary */
      else if( (!enable && scip->reopt != NULL) || (!enable && scip->set->reopt_enable && scip->reopt == NULL) )
      {
         /* set enable flag */
         scip->set->reopt_enable = enable;

         if( scip->reopt != NULL )
         {
            SCIP_CALL( SCIPreoptFree(&(scip->reopt), scip->set, scip->origprimal, scip->mem->probmem) );
            assert(scip->reopt == NULL);
         }
         SCIP_CALL( SCIPsetSetReoptimizationParams(scip->set, scip->messagehdlr) );
      }
   }
   else
   {
      /* set enable flag */
      scip->set->reopt_enable = enable;
   }

   return SCIP_OKAY;
}

/** save bound change based on dual information in the reoptimization tree
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPaddReoptDualBndchg(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node,               /**< node of the search tree */
   SCIP_VAR*             var,                /**< variable whose bound changed */
   SCIP_Real             newbound,           /**< new bound of the variable */
   SCIP_Real             oldbound            /**< old bound of the variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddReoptDualBndchg", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert(SCIPsetIsFeasLT(scip->set, newbound, oldbound) || SCIPsetIsFeasGT(scip->set, newbound, oldbound));

   SCIP_CALL( SCIPreoptAddDualBndchg(scip->reopt, scip->set, scip->mem->probmem, node, var, newbound, oldbound) );

   return SCIP_OKAY;
}

/** returns the optimal solution of the last iteration or NULL of none exists */
SCIP_SOL* SCIPgetReoptLastOptSol(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_SOL* sol;

   assert(scip != NULL);

   sol = NULL;

   if( scip->set->reopt_enable && scip->stat->nreoptruns > 1 )
   {
      sol = SCIPreoptGetLastBestSol(scip->reopt);
   }

   return sol;
}

/** returns the objective coefficent of a given variable in a previous iteration
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetReoptOldObjCoef(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable */
   int                   run,                /**< number of the run */
   SCIP_Real*            objcoef             /**< pointer to store the objective coefficient */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(0 < run && run <= scip->stat->nreoptruns);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetReoptOldObjCoef", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( SCIPvarIsOriginal(var) )
      *objcoef = SCIPreoptGetOldObjCoef(scip->reopt, run, SCIPvarGetIndex(var));
   else
   {
      SCIP_VAR* origvar;
      SCIP_Real constant;
      SCIP_Real scalar;

      assert(SCIPvarIsActive(var));

      origvar = var;
      constant = 0.0;
      scalar = 1.0;

      SCIP_CALL( SCIPvarGetOrigvarSum(&origvar, &scalar, &constant) );
      assert(origvar != NULL);
      assert(SCIPvarIsOriginal(origvar));

      *objcoef = SCIPreoptGetOldObjCoef(scip->reopt, run, SCIPvarGetIndex(origvar));
   }
   return SCIP_OKAY;
}

/** frees branch and bound tree and all solution process data; statistics, presolving data and transformed problem is
 *  preserved
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  @post If this method is called in \SCIP stage \ref SCIP_STAGE_INIT or \ref SCIP_STAGE_PROBLEM, the stage of
 *        \SCIP is not changed; otherwise, the \SCIP stage is changed to \ref SCIP_STAGE_TRANSFORMED
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPfreeSolve(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             restart             /**< should certain data be preserved for improved restarting? */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPfreeSolve", TRUE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_INIT:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_PROBLEM:
      return SCIP_OKAY;

   case SCIP_STAGE_PRESOLVING:
   {
      SCIP_Bool infeasible;

      assert(scip->stat->status != SCIP_STATUS_INFEASIBLE);
      assert(scip->stat->status != SCIP_STATUS_INFORUNBD);
      assert(scip->stat->status != SCIP_STATUS_UNBOUNDED);
      assert(scip->stat->status != SCIP_STATUS_OPTIMAL);

      /* exit presolving */
      SCIP_CALL( exitPresolve(scip, FALSE, &infeasible) );
      assert(scip->set->stage == SCIP_STAGE_PRESOLVED);
   }

   /*lint -fallthrough*/
   case SCIP_STAGE_PRESOLVED:
      /* switch stage to TRANSFORMED */
      scip->set->stage = SCIP_STAGE_TRANSFORMED;
      return SCIP_OKAY;

   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
      /* free solution process data structures */
      SCIP_CALL( freeSolve(scip, restart) );
      assert(scip->set->stage == SCIP_STAGE_TRANSFORMED);
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** frees branch and bound tree and all solution process data; statistics, presolving data and transformed problem is
 *  preserved
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  @post If this method is called in \SCIP stage \ref SCIP_STAGE_INIT, \ref SCIP_STAGE_TRANSFORMED or \ref SCIP_STAGE_PROBLEM,
 *        the stage of \SCIP is not changed; otherwise, the \SCIP stage is changed to \ref SCIP_STAGE_PRESOLVED.
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPfreeReoptSolve(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPfreeReoptSolve", TRUE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_INIT:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_PROBLEM:
      return SCIP_OKAY;

   case SCIP_STAGE_PRESOLVING:
   {
      SCIP_Bool infeasible;

      assert(scip->stat->status != SCIP_STATUS_INFEASIBLE);
      assert(scip->stat->status != SCIP_STATUS_INFORUNBD);
      assert(scip->stat->status != SCIP_STATUS_UNBOUNDED);
      assert(scip->stat->status != SCIP_STATUS_OPTIMAL);

      /* exit presolving */
      SCIP_CALL( exitPresolve(scip, FALSE, &infeasible) );
      assert(scip->set->stage == SCIP_STAGE_PRESOLVED);

      return SCIP_OKAY;
   }

   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
      /* free solution process data structures */
      SCIP_CALL( freeReoptSolve(scip) );
      assert(scip->set->stage == SCIP_STAGE_PRESOLVED);
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** frees all solution process data including presolving and transformed problem, only original problem is kept
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  @post After calling this method \SCIP reaches one of the following stages:
 *        - \ref SCIP_STAGE_INIT if the method was called from \SCIP stage \ref SCIP_STAGE_INIT
 *        - \ref SCIP_STAGE_PROBLEM if the method was called from any other of the allowed stages
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPfreeTransform(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPfreeTransform", TRUE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_INIT:
   case SCIP_STAGE_PROBLEM:
      return SCIP_OKAY;

   case SCIP_STAGE_PRESOLVING:
   {
      SCIP_Bool infeasible;

      assert(scip->stat->status != SCIP_STATUS_INFEASIBLE);
      assert(scip->stat->status != SCIP_STATUS_INFORUNBD);
      assert(scip->stat->status != SCIP_STATUS_UNBOUNDED);
      assert(scip->stat->status != SCIP_STATUS_OPTIMAL);

      /* exit presolving */
      SCIP_CALL( exitPresolve(scip, FALSE, &infeasible) );
      assert(scip->set->stage == SCIP_STAGE_PRESOLVED);
   }

   /*lint -fallthrough*/
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
      /* the solve was already freed, we directly go to freeTransform() */
      if( !scip->set->reopt_enable || scip->set->stage != SCIP_STAGE_PRESOLVED )
      {
         /* free solution process data */
         SCIP_CALL( SCIPfreeSolve(scip, FALSE) );
         assert(scip->set->stage == SCIP_STAGE_TRANSFORMED);
      }
      /*lint -fallthrough*/

   case SCIP_STAGE_TRANSFORMED:
      /* free transformed problem data structures */
      SCIP_CALL( freeTransform(scip) );
      assert(scip->set->stage == SCIP_STAGE_PROBLEM);
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** informs \SCIP that the solving process should be interrupted as soon as possible (e.g., after the current node has
 *   been solved)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note the \SCIP stage does not get changed
 */
SCIP_RETCODE SCIPinterruptSolve(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPinterruptSolve", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   /* set the userinterrupt flag */
   scip->stat->userinterrupt = TRUE;

   return SCIP_OKAY;
}

/** informs SCIP that the solving process should be restarted as soon as possible (e.g., after the current node has
 *  been solved)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note the \SCIP stage does not get changed
 */
SCIP_RETCODE SCIPrestartSolve(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPrestartSolve", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   /* set the userrestart flag */
   scip->stat->userrestart = TRUE;

   return SCIP_OKAY;
}

/** returns whether reoptimization is enabled or not */
SCIP_Bool SCIPisReoptEnabled(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   return scip->set->reopt_enable;
}

/** returns the stored solutions corresponding to a given run */
SCIP_RETCODE SCIPgetReoptSolsRun(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   run,                /**< number of the run */
   SCIP_SOL**            sols,               /**< array to store solutions */
   int                   solssize,           /**< size of the array */
   int*                  nsols               /**< pointer to store number of solutions */
   )
{
   assert(scip != NULL);
   assert(sols != NULL);
   assert(solssize > 0);

   if( scip->set->reopt_enable )
   {
      assert(run > 0 && run <= scip->stat->nreoptruns);
      SCIP_CALL( SCIPreoptGetSolsRun(scip->reopt, run, sols, solssize, nsols) );
   }
   else
   {
      *nsols = 0;
   }

   return SCIP_OKAY;
}

/** mark all stored solutions as not updated */
void SCIPresetReoptSolMarks(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);
   assert(scip->set->reopt_enable);
   assert(scip->reopt != NULL);

   if( scip->set->reopt_enable )
   {
      assert(scip->reopt != NULL);
      SCIPreoptResetSolMarks(scip->reopt);
   }
}

/** check if the reoptimization process should be restarted
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcheckReoptRestart(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node,               /**< current node of the branch and bound tree (or NULL) */
   SCIP_Bool*            restart             /**< pointer to store of the reoptimitation process should be restarted */
   )
{
   assert(scip != NULL);
   assert(scip->set->reopt_enable);
   assert(scip->reopt != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPcheckReoptRestart", FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPreoptCheckRestart(scip->reopt, scip->set, scip->mem->probmem, node, scip->transprob->vars,
         scip->transprob->nvars, restart) );

   return SCIP_OKAY;
}


/** returns whether we are in the restarting phase
 *
 *  @return TRUE, if we are in the restarting phase; FALSE, otherwise
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Bool SCIPisInRestart(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPisInRestart", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   /* return the restart status */
   return scip->stat->inrestart;
}


/*
 * variable methods
 */

/** creates and captures problem variable; if variable is of integral type, fractional bounds are automatically rounded;
 *  an integer variable with bounds zero and one is automatically converted into a binary variable;
 *
 *  @warning When doing column generation and the original problem is a maximization problem, notice that SCIP will
 *           transform the problem into a minimization problem by multiplying the objective function by -1.  Thus, the
 *           original objective function value of variables created during the solving process has to be multiplied by
 *           -1, too.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note the variable gets captured, hence at one point you have to release it using the method SCIPreleaseVar()
 */
SCIP_RETCODE SCIPcreateVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            var,                /**< pointer to variable object */
   const char*           name,               /**< name of variable, or NULL for automatic name creation */
   SCIP_Real             lb,                 /**< lower bound of variable */
   SCIP_Real             ub,                 /**< upper bound of variable */
   SCIP_Real             obj,                /**< objective function value */
   SCIP_VARTYPE          vartype,            /**< type of variable */
   SCIP_Bool             initial,            /**< should var's column be present in the initial root LP? */
   SCIP_Bool             removable,          /**< is var's column removable from the LP (due to aging or cleanup)? */
   SCIP_DECL_VARDELORIG  ((*vardelorig)),    /**< frees user data of original variable, or NULL */
   SCIP_DECL_VARTRANS    ((*vartrans)),      /**< creates transformed user data by transforming original user data, or NULL */
   SCIP_DECL_VARDELTRANS ((*vardeltrans)),   /**< frees user data of transformed variable, or NULL */
   SCIP_DECL_VARCOPY     ((*varcopy)),       /**< copies variable data if wanted to subscip, or NULL */
   SCIP_VARDATA*         vardata             /**< user data for this specific variable */
   )
{
   assert(var != NULL);
   assert(lb <= ub);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateVar", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   /* forbid infinite objective function values */
   if( SCIPisInfinity(scip, REALABS(obj)) )
   {
      SCIPerrorMessage("invalid objective function value: value is infinite\n");
      return SCIP_INVALIDDATA;
   }

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      SCIP_CALL( SCIPvarCreateOriginal(var, scip->mem->probmem, scip->set, scip->stat,
            name, lb, ub, obj, vartype, initial, removable, vardelorig, vartrans, vardeltrans, varcopy, vardata) );
      break;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPvarCreateTransformed(var, scip->mem->probmem, scip->set, scip->stat,
            name, lb, ub, obj, vartype, initial, removable, vardelorig, vartrans, vardeltrans, varcopy, vardata) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** creates and captures problem variable with optional callbacks and variable data set to NULL, which can be set
 *  afterwards using SCIPvarSetDelorigData(), SCIPvarSetTransData(),
 *  SCIPvarSetDeltransData(), SCIPvarSetCopy(), and SCIPvarSetData(); sets variable flags initial=TRUE
 *  and removable = FALSE, which can be adjusted by using SCIPvarSetInitial() and SCIPvarSetRemovable(), resp.;
 *  if variable is of integral type, fractional bounds are automatically rounded;
 *  an integer variable with bounds zero and one is automatically converted into a binary variable;
 *
 *  @warning When doing column generation and the original problem is a maximization problem, notice that SCIP will
 *           transform the problem into a minimization problem by multiplying the objective function by -1.  Thus, the
 *           original objective function value of variables created during the solving process has to be multiplied by
 *           -1, too.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note the variable gets captured, hence at one point you have to release it using the method SCIPreleaseVar()
 */
SCIP_RETCODE SCIPcreateVarBasic(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            var,                /**< pointer to variable object */
   const char*           name,               /**< name of variable, or NULL for automatic name creation */
   SCIP_Real             lb,                 /**< lower bound of variable */
   SCIP_Real             ub,                 /**< upper bound of variable */
   SCIP_Real             obj,                /**< objective function value */
   SCIP_VARTYPE          vartype             /**< type of variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateVarBasic", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPcreateVar(scip, var, name, lb, ub, obj, vartype, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL) );

   return SCIP_OKAY;
}

/** outputs the variable name to the file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPwriteVarName(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file, or NULL for stdout */
   SCIP_VAR*             var,                /**< variable to output */
   SCIP_Bool             type                /**< should the variable type be also posted */
   )
{
   assert(scip != NULL);
   assert(var != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPwriteVarName", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   /* print variable name */
   if( SCIPvarIsNegated(var) )
   {
      SCIP_VAR* negatedvar;

      SCIP_CALL( SCIPgetNegatedVar(scip, var, &negatedvar) );
      SCIPinfoMessage(scip, file, "<~%s>", SCIPvarGetName(negatedvar));
   }
   else
   {
      SCIPinfoMessage(scip, file, "<%s>", SCIPvarGetName(var));
   }

   if( type )
   {
      /* print variable type */
      SCIPinfoMessage(scip, file, "[%c]",
         SCIPvarGetType(var) == SCIP_VARTYPE_BINARY ? SCIP_VARTYPE_BINARY_CHAR :
         SCIPvarGetType(var) == SCIP_VARTYPE_INTEGER ? SCIP_VARTYPE_INTEGER_CHAR :
         SCIPvarGetType(var) == SCIP_VARTYPE_IMPLINT ? SCIP_VARTYPE_IMPLINT_CHAR : SCIP_VARTYPE_CONTINUOUS_CHAR);
   }

   return SCIP_OKAY;
}

/** print the given list of variables to output stream separated by the given delimiter character;
 *
 *  i. e. the variables x1, x2, ..., xn with given delimiter ',' are written as: \<x1\>, \<x2\>, ..., \<xn\>;
 *
 *  the method SCIPparseVarsList() can parse such a string
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note The printing process is done via the message handler system.
 */
SCIP_RETCODE SCIPwriteVarsList(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file, or NULL for stdout */
   SCIP_VAR**            vars,               /**< variable array to output */
   int                   nvars,              /**< number of variables */
   SCIP_Bool             type,               /**< should the variable type be also posted */
   char                  delimiter           /**< character which is used for delimitation */
   )
{
   int v;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPwriteVarsList", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   for( v = 0; v < nvars; ++v )
   {
      if( v > 0 )
      {
         SCIPinfoMessage(scip, file, "%c", delimiter);
      }

      /* print variable name */
      SCIP_CALL( SCIPwriteVarName(scip, file, vars[v], type) );
   }

   return SCIP_OKAY;
}

/** print the given variables and coefficients as linear sum in the following form
 *  c1 \<x1\> + c2 \<x2\>   ... + cn \<xn\>
 *
 *  This string can be parsed by the method SCIPparseVarsLinearsum().
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note The printing process is done via the message handler system.
 */
SCIP_RETCODE SCIPwriteVarsLinearsum(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file, or NULL for stdout */
   SCIP_VAR**            vars,               /**< variable array to output */
   SCIP_Real*            vals,               /**< array of coefficients or NULL if all coefficients are 1.0 */
   int                   nvars,              /**< number of variables */
   SCIP_Bool             type                /**< should the variable type be also posted */
   )
{
   int v;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPwriteVarsLinearsum", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   for( v = 0; v < nvars; ++v )
   {
      if( vals != NULL )
      {
         if( vals[v] == 1.0 )
         {
            if( v > 0 )
               SCIPinfoMessage(scip, file, " +");
         }
         else if( vals[v] == -1.0 )
            SCIPinfoMessage(scip, file, " -");
         else
            SCIPinfoMessage(scip, file, " %+.15g", vals[v]);
      }
      else if( nvars > 0 )
         SCIPinfoMessage(scip, file, " +");

      /* print variable name */
      SCIP_CALL( SCIPwriteVarName(scip, file, vars[v], type) );
   }

   return SCIP_OKAY;
}

/** print the given monomials as polynomial in the following form
 *  c1 \<x11\>^e11 \<x12\>^e12 ... \<x1n\>^e1n + c2 \<x21\>^e21 \<x22\>^e22 ... + ... + cn \<xn1\>^en1 ...
 *
 *  This string can be parsed by the method SCIPparseVarsPolynomial().
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note The printing process is done via the message handler system.
 */
SCIP_RETCODE SCIPwriteVarsPolynomial(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file, or NULL for stdout */
   SCIP_VAR***           monomialvars,       /**< arrays with variables for each monomial */
   SCIP_Real**           monomialexps,       /**< arrays with variable exponents, or NULL if always 1.0 */
   SCIP_Real*            monomialcoefs,      /**< array with monomial coefficients */
   int*                  monomialnvars,      /**< array with number of variables for each monomial */
   int                   nmonomials,         /**< number of monomials */
   SCIP_Bool             type                /**< should the variable type be also posted */
   )
{
   int i;
   int v;

   assert(scip != NULL);
   assert(monomialvars  != NULL || nmonomials == 0);
   assert(monomialcoefs != NULL || nmonomials == 0);
   assert(monomialnvars != NULL || nmonomials == 0);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPwriteVarsPolynomial", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   if( nmonomials == 0 )
   {
      SCIPinfoMessage(scip, file, " 0 ");
      return SCIP_OKAY;
   }

   for( i = 0; i < nmonomials; ++i )
   {
      if( monomialcoefs[i] == 1.0 ) /*lint !e613*/
      {
         if( i > 0 )
            SCIPinfoMessage(scip, file, " +");
      }
      else if( monomialcoefs[i] == -1.0 ) /*lint !e613*/
         SCIPinfoMessage(scip, file, " -");
      else
         SCIPinfoMessage(scip, file, " %+.15g", monomialcoefs[i]); /*lint !e613*/

      assert(monomialvars[i] != NULL || monomialnvars[i] == 0); /*lint !e613*/

      for( v = 0; v < monomialnvars[i]; ++v ) /*lint !e613*/
      {
         SCIP_CALL( SCIPwriteVarName(scip, file, monomialvars[i][v], type) ); /*lint !e613*/
         if( monomialexps != NULL && monomialexps[i] != NULL && monomialexps[i][v] != 1.0 )
         {
            SCIPinfoMessage(scip, file, "^%.15g", monomialexps[i][v]);
         }
      }
   }

   return SCIP_OKAY;
}

/** parses variable information (in cip format) out of a string; if the parsing process was successful a variable is
 *  created and captured; if variable is of integral type, fractional bounds are automatically rounded; an integer
 *  variable with bounds zero and one is automatically converted into a binary variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPparseVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            var,                /**< pointer to store the problem variable */
   const char*           str,                /**< string to parse */
   SCIP_Bool             initial,            /**< should var's column be present in the initial root LP? */
   SCIP_Bool             removable,          /**< is var's column removable from the LP (due to aging or cleanup)? */
   SCIP_DECL_VARCOPY     ((*varcopy)),       /**< copies variable data if wanted to subscip, or NULL */
   SCIP_DECL_VARDELORIG  ((*vardelorig)),    /**< frees user data of original variable */
   SCIP_DECL_VARTRANS    ((*vartrans)),      /**< creates transformed user data by transforming original user data */
   SCIP_DECL_VARDELTRANS ((*vardeltrans)),   /**< frees user data of transformed variable */
   SCIP_VARDATA*         vardata,            /**< user data for this specific variable */
   char**                endptr,             /**< pointer to store the final string position if successful */
   SCIP_Bool*            success             /**< pointer store if the paring process was successful */
   )
{
   assert(var != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPparseVar", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      SCIP_CALL( SCIPvarParseOriginal(var, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
            str, initial, removable, varcopy, vardelorig, vartrans, vardeltrans, vardata, endptr, success) );
      break;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPvarParseTransformed(var, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
            str, initial, removable, varcopy, vardelorig, vartrans, vardeltrans, vardata, endptr, success) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** parses the given string for a variable name and stores the variable in the corresponding pointer if such a variable
 *  exits and returns the position where the parsing stopped
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPparseVarName(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           str,                /**< string to parse */
   SCIP_VAR**            var,                /**< pointer to store the problem variable, or NULL if it does not exit */
   char**                endptr              /**< pointer to store the final string position if successful */
   )
{
   char varname[SCIP_MAXSTRLEN];

   assert(str != NULL);
   assert(var != NULL);
   assert(endptr != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPparseVarName", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPstrCopySection(str, '<', '>', varname, SCIP_MAXSTRLEN, endptr);
   assert(*endptr != NULL);

   if( *varname == '\0' )
   {
      SCIPerrorMessage("invalid variable name string given: could not find '<'\n");
      return SCIP_INVALIDDATA;
   }

   /* check if we have a negated variable */
   if( *varname == '~' )
   {
      SCIPdebugMsg(scip, "parsed negated variable name <%s>\n", &varname[1]);

      /* search for the variable and ignore '~' */
      (*var) = SCIPfindVar(scip, &varname[1]);

      if( *var  != NULL )
      {
         SCIP_CALL( SCIPgetNegatedVar(scip, *var, var) );
      }
   }
   else
   {
      SCIPdebugMsg(scip, "parsed variable name <%s>\n", varname);

      /* search for the variable */
      (*var) = SCIPfindVar(scip, varname);
   }

   str = *endptr;

   /* skip additional variable type marker */
   if( *str == '[' && (str[1] == SCIP_VARTYPE_BINARY_CHAR || str[1] == SCIP_VARTYPE_INTEGER_CHAR ||
       str[1] == SCIP_VARTYPE_IMPLINT_CHAR || str[1] == SCIP_VARTYPE_CONTINUOUS_CHAR )  && str[2] == ']' )
      (*endptr) += 3;

   return SCIP_OKAY;
}

/** parse the given string as variable list (here ',' is the delimiter)) (\<x1\>, \<x2\>, ..., \<xn\>) (see
 *  SCIPwriteVarsList() ); if it was successful, the pointer success is set to TRUE
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note The pointer success in only set to FALSE in the case that a variable with a parsed variable name does not exist.
 *
 *  @note If the number of (parsed) variables is greater than the available slots in the variable array, nothing happens
 *        except that the required size is stored in the corresponding integer; the reason for this approach is that we
 *        cannot reallocate memory, since we do not know how the memory has been allocated (e.g., by a C++ 'new' or SCIP
 *        memory functions).
 */
SCIP_RETCODE SCIPparseVarsList(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           str,                /**< string to parse */
   SCIP_VAR**            vars,               /**< array to store the parsed variable */
   int*                  nvars,              /**< pointer to store number of parsed variables */
   int                   varssize,           /**< size of the variable array */
   int*                  requiredsize,       /**< pointer to store the required array size for the active variables */
   char**                endptr,             /**< pointer to store the final string position if successful */
   char                  delimiter,          /**< character which is used for delimitation */
   SCIP_Bool*            success             /**< pointer to store the whether the parsing was successful or not */
   )
{
   SCIP_VAR** tmpvars;
   SCIP_VAR* var;
   int ntmpvars = 0;
   int v;

   assert( nvars != NULL );
   assert( requiredsize != NULL );
   assert( endptr != NULL );
   assert( success != NULL );

   SCIP_CALL( SCIPcheckStage(scip, "SCIPparseVarsList", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   /* allocate buffer memory for temporary storing the parsed variables */
   SCIP_CALL( SCIPallocBufferArray(scip, &tmpvars, varssize) );

   (*success) = TRUE;

   do
   {
      *endptr = (char*)str;

      /* parse variable name */
      SCIP_CALL( SCIPparseVarName(scip, str, &var, endptr) );

      if( var == NULL )
      {
         SCIPdebugMsg(scip, "variable with name <%s> does not exist\n", SCIPvarGetName(var));
         (*success) = FALSE;
         break;
      }

      /* store the variable in the tmp array */
      if( ntmpvars < varssize )
         tmpvars[ntmpvars] = var;

      ntmpvars++;

      str = *endptr;

      while( isspace((unsigned char)*str) )
         str++;
   }
   while( *str == delimiter );

   *endptr = (char*)str;

   /* if all variable name searches were successful and the variable array has enough slots, copy the collected variables */
   if( (*success) && ntmpvars <= varssize )
   {
      for( v = 0; v < ntmpvars; ++v )
         vars[v] = tmpvars[v];

      (*nvars) = ntmpvars;
   }
   else
      (*nvars) = 0;

   (*requiredsize) = ntmpvars;

   /* free buffer arrays */
   SCIPfreeBufferArray(scip, &tmpvars);

   return SCIP_OKAY;
}

/** parse the given string as linear sum of variables and coefficients (c1 \<x1\> + c2 \<x2\> + ... + cn \<xn\>)
 *  (see SCIPwriteVarsLinearsum() ); if it was successful, the pointer success is set to TRUE
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note The pointer success in only set to FALSE in the case that a variable with a parsed variable name does not exist.
 *
 *  @note If the number of (parsed) variables is greater than the available slots in the variable array, nothing happens
 *        except that the required size is stored in the corresponding integer; the reason for this approach is that we
 *        cannot reallocate memory, since we do not know how the memory has been allocated (e.g., by a C++ 'new' or SCIP
 *        memory functions).
 */
SCIP_RETCODE SCIPparseVarsLinearsum(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           str,                /**< string to parse */
   SCIP_VAR**            vars,               /**< array to store the parsed variables */
   SCIP_Real*            vals,               /**< array to store the parsed coefficients */
   int*                  nvars,              /**< pointer to store number of parsed variables */
   int                   varssize,           /**< size of the variable array */
   int*                  requiredsize,       /**< pointer to store the required array size for the active variables */
   char**                endptr,             /**< pointer to store the final string position if successful */
   SCIP_Bool*            success             /**< pointer to store the whether the parsing was successful or not */
   )
{
   SCIP_VAR*** monomialvars;
   SCIP_Real** monomialexps;
   SCIP_Real*  monomialcoefs;
   int*        monomialnvars;
   int         nmonomials;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPparseVarsLinearsum", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert(scip != NULL);
   assert(str != NULL);
   assert(vars != NULL || varssize == 0);
   assert(vals != NULL || varssize == 0);
   assert(nvars != NULL);
   assert(requiredsize != NULL);
   assert(endptr != NULL);
   assert(success != NULL);

   *requiredsize = 0;

   SCIP_CALL( SCIPparseVarsPolynomial(scip, str, &monomialvars, &monomialexps, &monomialcoefs, &monomialnvars, &nmonomials, endptr, success) );

   if( !*success )
   {
      assert(nmonomials == 0); /* SCIPparseVarsPolynomial should have freed all buffers, so no need to call free here */
      return SCIP_OKAY;
   }

   /* check if linear sum is just "0" */
   if( nmonomials == 1 && monomialnvars[0] == 0 && monomialcoefs[0] == 0.0 )
   {
      *nvars = 0;
      *requiredsize = 0;

      SCIPfreeParseVarsPolynomialData(scip, &monomialvars, &monomialexps, &monomialcoefs, &monomialnvars, nmonomials);

      return SCIP_OKAY;
   }

   *nvars = nmonomials;
   *requiredsize = nmonomials;

   /* if we have enough slots in the variables array, copy variables over */
   if( varssize >= nmonomials )
   {
      int v;

      for( v = 0; v < nmonomials; ++v )
      {
         if( monomialnvars[v] == 0 )
         {
            SCIPerrorMessage("constant in linear sum\n");
            *success = FALSE;
            break;
         }
         if( monomialnvars[v] > 1 || monomialexps[v][0] != 1.0 )
         {
            SCIPerrorMessage("nonlinear monomial in linear sum\n");
            *success = FALSE;
            break;
         }
         assert(monomialnvars[v]   == 1);
         assert(monomialvars[v][0] != NULL);
         assert(monomialexps[v][0] == 1.0);

         vars[v] = monomialvars[v][0]; /*lint !e613*/
         vals[v] = monomialcoefs[v]; /*lint !e613*/
      }
   }

   SCIPfreeParseVarsPolynomialData(scip, &monomialvars, &monomialexps, &monomialcoefs, &monomialnvars, nmonomials);

   return SCIP_OKAY;
}

/** parse the given string as polynomial of variables and coefficients
 *  (c1 \<x11\>^e11 \<x12\>^e12 ... \<x1n\>^e1n + c2 \<x21\>^e21 \<x22\>^e22 ... + ... + cn \<xn1\>^en1 ...)
 *  (see SCIPwriteVarsPolynomial()); if it was successful, the pointer success is set to TRUE
 *
 *  The user has to call SCIPfreeParseVarsPolynomialData(scip, monomialvars, monomialexps,
 *  monomialcoefs, monomialnvars, *nmonomials) short after SCIPparseVarsPolynomial to free all the
 *  allocated memory again.  Do not keep the arrays created by SCIPparseVarsPolynomial around, since
 *  they use buffer memory that is intended for short term use only.
 *
 *  Parsing is stopped at the end of string (indicated by the \\0-character) or when no more monomials
 *  are recognized.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPparseVarsPolynomial(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           str,                /**< string to parse */
   SCIP_VAR****          monomialvars,       /**< pointer to store arrays with variables for each monomial */
   SCIP_Real***          monomialexps,       /**< pointer to store arrays with variable exponents */
   SCIP_Real**           monomialcoefs,      /**< pointer to store array with monomial coefficients */
   int**                 monomialnvars,      /**< pointer to store array with number of variables for each monomial */
   int*                  nmonomials,         /**< pointer to store number of parsed monomials */
   char**                endptr,             /**< pointer to store the final string position if successful */
   SCIP_Bool*            success             /**< pointer to store the whether the parsing was successful or not */
   )
{
   typedef enum
   {
      SCIPPARSEPOLYNOMIAL_STATE_BEGIN,       /* we are at the beginning of a monomial */
      SCIPPARSEPOLYNOMIAL_STATE_INTERMED,    /* we are in between the factors of a monomial */
      SCIPPARSEPOLYNOMIAL_STATE_COEF,        /* we parse the coefficient of a monomial */
      SCIPPARSEPOLYNOMIAL_STATE_VARS,        /* we parse monomial variables */
      SCIPPARSEPOLYNOMIAL_STATE_EXPONENT,    /* we parse the exponent of a variable */
      SCIPPARSEPOLYNOMIAL_STATE_END,         /* we are at the end the polynomial */
      SCIPPARSEPOLYNOMIAL_STATE_ERROR        /* a parsing error occured */
   } SCIPPARSEPOLYNOMIAL_STATES;

   SCIPPARSEPOLYNOMIAL_STATES state;
   int monomialssize;

   /* data of currently parsed monomial */
   int varssize;
   int nvars;
   SCIP_VAR** vars;
   SCIP_Real* exponents;
   SCIP_Real coef;

   assert(scip != NULL);
   assert(str != NULL);
   assert(monomialvars != NULL);
   assert(monomialexps != NULL);
   assert(monomialnvars != NULL);
   assert(monomialcoefs != NULL);
   assert(nmonomials != NULL);
   assert(endptr != NULL);
   assert(success != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPparseVarsPolynomial", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *success = FALSE;
   *nmonomials = 0;
   monomialssize = 0;
   *monomialvars = NULL;
   *monomialexps = NULL;
   *monomialcoefs = NULL;
   *monomialnvars = NULL;

   /* initialize state machine */
   state = SCIPPARSEPOLYNOMIAL_STATE_BEGIN;
   varssize = 0;
   nvars = 0;
   vars = NULL;
   exponents = NULL;
   coef = SCIP_INVALID;

   SCIPdebugMsg(scip, "parsing polynomial from '%s'\n", str);

   while( *str && state != SCIPPARSEPOLYNOMIAL_STATE_END && state != SCIPPARSEPOLYNOMIAL_STATE_ERROR )
   {
      /* skip white space */
      while( isspace((unsigned char)*str) )
         str++;

      assert(state != SCIPPARSEPOLYNOMIAL_STATE_END);

      switch( state )
      {
      case SCIPPARSEPOLYNOMIAL_STATE_BEGIN:
      {
         if( coef != SCIP_INVALID  ) /*lint !e777*/
         {
            SCIPdebugMsg(scip, "push monomial with coefficient <%g> and <%d> vars\n", coef, nvars);
            /* push previous monomial */
            if( monomialssize <= *nmonomials )
            {
               monomialssize = SCIPcalcMemGrowSize(scip, *nmonomials+1);

               SCIP_CALL( SCIPreallocBufferArray(scip, monomialvars,  monomialssize) );
               SCIP_CALL( SCIPreallocBufferArray(scip, monomialexps,  monomialssize) );
               SCIP_CALL( SCIPreallocBufferArray(scip, monomialnvars, monomialssize) );
               SCIP_CALL( SCIPreallocBufferArray(scip, monomialcoefs, monomialssize) );
            }

            if( nvars > 0 )
            {
               SCIP_CALL( SCIPduplicateBufferArray(scip, &(*monomialvars)[*nmonomials], vars, nvars) ); /*lint !e866*/
               SCIP_CALL( SCIPduplicateBufferArray(scip, &(*monomialexps)[*nmonomials], exponents, nvars) ); /*lint !e866*/
            }
            else
            {
               (*monomialvars)[*nmonomials] = NULL;
               (*monomialexps)[*nmonomials] = NULL;
            }
            (*monomialcoefs)[*nmonomials] = coef;
            (*monomialnvars)[*nmonomials] = nvars;
            ++*nmonomials;

            nvars = 0;
            coef = SCIP_INVALID;
         }

         if( *str == '<' )
         {
            /* there seem to come a variable at the beginning of a monomial
             * so assume the coefficient is 1.0
             */
            state = SCIPPARSEPOLYNOMIAL_STATE_VARS;
            coef = 1.0;
            break;
         }
         if( *str == '-' || *str == '+' || isdigit(*str) )
         {
            state = SCIPPARSEPOLYNOMIAL_STATE_COEF;
            break;
         }

         state = SCIPPARSEPOLYNOMIAL_STATE_END;

         break;
      }

      case SCIPPARSEPOLYNOMIAL_STATE_INTERMED:
      {
         if( *str == '<' )
         {
            /* there seem to come another variable */
            state = SCIPPARSEPOLYNOMIAL_STATE_VARS;
            break;
         }

         if( *str == '-' || *str == '+' || isdigit(*str) )
         {
            /* there seem to come a coefficient, which means the next monomial */
            state = SCIPPARSEPOLYNOMIAL_STATE_BEGIN;
            break;
         }

         /* since we cannot detect the symbols we stop parsing the polynomial */
         state = SCIPPARSEPOLYNOMIAL_STATE_END;
         break;
      }

      case SCIPPARSEPOLYNOMIAL_STATE_COEF:
      {
         if( *str == '+' && !isdigit(str[1]) )
         {
            /* only a plus sign, without number */
            coef =  1.0;
            ++str;
         }
         else if( *str == '-' && !isdigit(str[1]) )
         {
            /* only a minus sign, without number */
            coef = -1.0;
            ++str;
         }
         else if( SCIPstrToRealValue(str, &coef, endptr) )
         {
            str = *endptr;
         }
         else
         {
            SCIPerrorMessage("could not parse number in the beginning of '%s'\n", str);
            state = SCIPPARSEPOLYNOMIAL_STATE_ERROR;
            break;
         }

         /* after the coefficient we go into the intermediate state, i.e., expecting next variables */
         state = SCIPPARSEPOLYNOMIAL_STATE_INTERMED;

         break;
      }

      case SCIPPARSEPOLYNOMIAL_STATE_VARS:
      {
         SCIP_VAR* var;

         assert(*str == '<');

         /* parse variable name */
         SCIP_CALL( SCIPparseVarName(scip, str, &var, endptr) );

         /* check if variable name was parsed */
         if( *endptr == str )
         {
            state = SCIPPARSEPOLYNOMIAL_STATE_END;
            break;
         }

         if( var == NULL )
         {
            SCIPerrorMessage("did not find variable in the beginning of %s\n", str);
            state = SCIPPARSEPOLYNOMIAL_STATE_ERROR;
            break;
         }

         /* add variable to vars array */
         if( nvars + 1 > varssize )
         {
            varssize = SCIPcalcMemGrowSize(scip, nvars+1);
            SCIP_CALL( SCIPreallocBufferArray(scip, &vars,      varssize) );
            SCIP_CALL( SCIPreallocBufferArray(scip, &exponents, varssize) );
         }
         assert(vars != NULL);
         assert(exponents != NULL);

         vars[nvars] = var;
         exponents[nvars] = 1.0;
         ++nvars;

         str = *endptr;

         if( *str == '^' )
            state = SCIPPARSEPOLYNOMIAL_STATE_EXPONENT;
         else
            state = SCIPPARSEPOLYNOMIAL_STATE_INTERMED;

         break;
      }

      case SCIPPARSEPOLYNOMIAL_STATE_EXPONENT:
      {
         assert(*str == '^');
         assert(nvars > 0); /* we should be in a monomial that has already a variable */
         assert(exponents != NULL);
         ++str;

         if( !SCIPstrToRealValue(str, &exponents[nvars-1], endptr) )
         {
            SCIPerrorMessage("could not parse number in the beginning of '%s'\n", str);
            state = SCIPPARSEPOLYNOMIAL_STATE_ERROR;
            break;
         }
         str = *endptr;

         /* after the exponent we go into the intermediate state, i.e., expecting next variables */
         state = SCIPPARSEPOLYNOMIAL_STATE_INTERMED;
         break;
      }

      case SCIPPARSEPOLYNOMIAL_STATE_END:
      case SCIPPARSEPOLYNOMIAL_STATE_ERROR:
      default:
         SCIPerrorMessage("unexpected state\n");
         return SCIP_READERROR;
      }
   }

   /* set end pointer */
   *endptr = (char*)str;

   /* check state at end of string */
   switch( state )
   {
   case SCIPPARSEPOLYNOMIAL_STATE_BEGIN:
   case SCIPPARSEPOLYNOMIAL_STATE_END:
   case SCIPPARSEPOLYNOMIAL_STATE_INTERMED:
   {
      if( coef != SCIP_INVALID ) /*lint !e777*/
      {
         /* push last monomial */
         SCIPdebugMsg(scip, "push monomial with coefficient <%g> and <%d> vars\n", coef, nvars);
         if( monomialssize <= *nmonomials )
         {
            monomialssize = *nmonomials+1;
            SCIP_CALL( SCIPreallocBufferArray(scip, monomialvars,  monomialssize) );
            SCIP_CALL( SCIPreallocBufferArray(scip, monomialexps,  monomialssize) );
            SCIP_CALL( SCIPreallocBufferArray(scip, monomialnvars, monomialssize) );
            SCIP_CALL( SCIPreallocBufferArray(scip, monomialcoefs, monomialssize) );
         }

         if( nvars > 0 )
         {
            /* shrink vars and exponents array to needed size and take over ownership */
            SCIP_CALL( SCIPreallocBufferArray(scip, &vars,      nvars) );
            SCIP_CALL( SCIPreallocBufferArray(scip, &exponents, nvars) );
            (*monomialvars)[*nmonomials] = vars;
            (*monomialexps)[*nmonomials] = exponents;
            vars = NULL;
            exponents = NULL;
         }
         else
         {
            (*monomialvars)[*nmonomials] = NULL;
            (*monomialexps)[*nmonomials] = NULL;
         }
         (*monomialcoefs)[*nmonomials] = coef;
         (*monomialnvars)[*nmonomials] = nvars;
         ++*nmonomials;
      }

      *success = TRUE;
      break;
   }

   case SCIPPARSEPOLYNOMIAL_STATE_COEF:
   case SCIPPARSEPOLYNOMIAL_STATE_VARS:
   case SCIPPARSEPOLYNOMIAL_STATE_EXPONENT:
   {
      SCIPerrorMessage("unexpected parsing state at end of polynomial string\n");
   }
   /*lint -fallthrough*/
   case SCIPPARSEPOLYNOMIAL_STATE_ERROR:
      assert(!*success);
      break;
   }

   /* free memory to store current monomial, if still existing */
   SCIPfreeBufferArrayNull(scip, &vars);
   SCIPfreeBufferArrayNull(scip, &exponents);

   if( *success && *nmonomials > 0 )
   {
      /* shrink arrays to required size, so we do not need to keep monomialssize around */
      assert(*nmonomials <= monomialssize);
      SCIP_CALL( SCIPreallocBufferArray(scip, monomialvars,  *nmonomials) );
      SCIP_CALL( SCIPreallocBufferArray(scip, monomialexps,  *nmonomials) );
      SCIP_CALL( SCIPreallocBufferArray(scip, monomialnvars, *nmonomials) );
      SCIP_CALL( SCIPreallocBufferArray(scip, monomialcoefs, *nmonomials) );

      /* SCIPwriteVarsPolynomial(scip, NULL, *monomialvars, *monomialexps, *monomialcoefs, *monomialnvars, *nmonomials, FALSE); */
   }
   else
   {
      /* in case of error, cleanup all data here */
      SCIPfreeParseVarsPolynomialData(scip, monomialvars, monomialexps, monomialcoefs, monomialnvars, *nmonomials);
      *nmonomials = 0;
   }

   return SCIP_OKAY;
}

/** frees memory allocated when parsing a polynomial from a string
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
void SCIPfreeParseVarsPolynomialData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR****          monomialvars,       /**< pointer to store arrays with variables for each monomial */
   SCIP_Real***          monomialexps,       /**< pointer to store arrays with variable exponents */
   SCIP_Real**           monomialcoefs,      /**< pointer to store array with monomial coefficients */
   int**                 monomialnvars,      /**< pointer to store array with number of variables for each monomial */
   int                   nmonomials          /**< pointer to store number of parsed monomials */
   )
{
   int i;

   assert(scip != NULL);
   assert(monomialvars  != NULL);
   assert(monomialexps  != NULL);
   assert(monomialcoefs != NULL);
   assert(monomialnvars != NULL);
   assert((*monomialvars  != NULL) == (nmonomials > 0));
   assert((*monomialexps  != NULL) == (nmonomials > 0));
   assert((*monomialcoefs != NULL) == (nmonomials > 0));
   assert((*monomialnvars != NULL) == (nmonomials > 0));

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPfreeParseVarsPolynomialData", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( nmonomials == 0 )
      return;

   for( i = nmonomials - 1; i >= 0; --i )
   {
      SCIPfreeBufferArrayNull(scip, &(*monomialvars)[i]);
      SCIPfreeBufferArrayNull(scip, &(*monomialexps)[i]);
   }

   SCIPfreeBufferArray(scip, monomialvars);
   SCIPfreeBufferArray(scip, monomialexps);
   SCIPfreeBufferArray(scip, monomialcoefs);
   SCIPfreeBufferArray(scip, monomialnvars);
}

/** increases usage counter of variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPcaptureVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to capture */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcaptureVar", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );
   assert(var->scip == scip);

   SCIPvarCapture(var);

   return SCIP_OKAY;
}

/** decreases usage counter of variable, if the usage pointer reaches zero the variable gets freed
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note the pointer of the variable will be NULLed
 */
SCIP_RETCODE SCIPreleaseVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            var                 /**< pointer to variable */
   )
{
   assert(var != NULL);
   assert(*var != NULL);
   assert((*var)->scip == scip);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPreleaseVar", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      SCIP_CALL( SCIPvarRelease(var, scip->mem->probmem, scip->set, scip->eventqueue, scip->lp) );
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
   case SCIP_STAGE_FREETRANS:
      if( !SCIPvarIsTransformed(*var) && (*var)->nuses == 1 )
      {
         SCIPerrorMessage("cannot release last use of original variable while the transformed problem exists\n");
         return SCIP_INVALIDCALL;
      }
      SCIP_CALL( SCIPvarRelease(var, scip->mem->probmem, scip->set, scip->eventqueue, scip->lp) );
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** changes the name of a variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_PROBLEM
 *
 *  @note to get the current name of a variable, use SCIPvarGetName() from pub_var.h
 */
SCIP_RETCODE SCIPchgVarName(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable */
   const char*           name                /**< new name of constraint */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarName", FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );
   assert( var->scip == scip );

   if( SCIPgetStage(scip) != SCIP_STAGE_PROBLEM )
   {
      SCIPerrorMessage("variable names can only be changed in problem creation stage\n");
      SCIPABORT();
      return SCIP_INVALIDCALL; /*lint !e527*/
   }

   /* remove variable's name from the namespace if the variable was already added */
   if( SCIPvarGetProbindex(var) != -1 )
   {
      SCIP_CALL( SCIPprobRemoveVarName(scip->origprob, var) );
   }

   /* change variable name */
   SCIP_CALL( SCIPvarChgName(var, SCIPblkmem(scip), name) );

   /* add variable's name to the namespace if the variable was already added */
   if( SCIPvarGetProbindex(var) != -1 )
   {
      SCIP_CALL( SCIPprobAddVarName(scip->origprob, var) );
   }

   return SCIP_OKAY;
}

/** gets and captures transformed variable of a given variable; if the variable is not yet transformed,
 *  a new transformed variable for this variable is created
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPtransformVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get/create transformed variable for */
   SCIP_VAR**            transvar            /**< pointer to store the transformed variable */
   )
{
   assert(transvar != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtransformVar", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( SCIPvarIsTransformed(var) )
   {
      *transvar = var;
      SCIPvarCapture(*transvar);
   }
   else
   {
      SCIP_CALL( SCIPvarTransform(var, scip->mem->probmem, scip->set, scip->stat, scip->origprob->objsense, transvar) );
   }

   return SCIP_OKAY;
}

/** gets and captures transformed variables for an array of variables;
 *  if a variable of the array is not yet transformed, a new transformed variable for this variable is created;
 *  it is possible to call this method with vars == transvars
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPtransformVars(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   nvars,              /**< number of variables to get/create transformed variables for */
   SCIP_VAR**            vars,               /**< array with variables to get/create transformed variables for */
   SCIP_VAR**            transvars           /**< array to store the transformed variables */
   )
{
   int v;

   assert(nvars == 0 || vars != NULL);
   assert(nvars == 0 || transvars != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtransformVars", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   for( v = 0; v < nvars; ++v )
   {
      if( SCIPvarIsTransformed(vars[v]) )
      {
         transvars[v] = vars[v];
         SCIPvarCapture(transvars[v]);
      }
      else
      {
         SCIP_CALL( SCIPvarTransform(vars[v], scip->mem->probmem, scip->set, scip->stat, scip->origprob->objsense,
               &transvars[v]) );
      }
   }

   return SCIP_OKAY;
}

/** gets corresponding transformed variable of a given variable;
 *  returns NULL as transvar, if transformed variable is not yet existing
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPgetTransformedVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get transformed variable for */
   SCIP_VAR**            transvar            /**< pointer to store the transformed variable */
   )
{
   assert(transvar != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetTransformedVar", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   if( SCIPvarIsTransformed(var) )
      *transvar = var;
   else
   {
      SCIP_CALL( SCIPvarGetTransformed(var, scip->mem->probmem, scip->set, scip->stat, transvar) );
   }

   return SCIP_OKAY;
}

/** gets corresponding transformed variables for an array of variables;
 *  stores NULL in a transvars slot, if the transformed variable is not yet existing;
 *  it is possible to call this method with vars == transvars, but remember that variables that are not
 *  yet transformed will be replaced with NULL
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPgetTransformedVars(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   nvars,              /**< number of variables to get transformed variables for */
   SCIP_VAR**            vars,               /**< array with variables to get transformed variables for */
   SCIP_VAR**            transvars           /**< array to store the transformed variables */
   )
{
   int v;

   assert(nvars == 0 || vars != NULL);
   assert(nvars == 0 || transvars != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetTransformedVars", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   for( v = 0; v < nvars; ++v )
   {
      if( SCIPvarIsTransformed(vars[v]) )
         transvars[v] = vars[v];
      else
      {
         SCIP_CALL( SCIPvarGetTransformed(vars[v], scip->mem->probmem, scip->set, scip->stat, &transvars[v]) );
      }
   }

   return SCIP_OKAY;
}

/** gets negated variable x' = lb + ub - x of variable x; negated variable is created, if not yet existing;
 *  in difference to \ref SCIPcreateVar, the negated variable must not be released (unless captured explicitly)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPgetNegatedVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get negated variable for */
   SCIP_VAR**            negvar              /**< pointer to store the negated variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetNegatedVar", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );
   assert( var->scip == scip );

   SCIP_CALL( SCIPvarNegate(var, scip->mem->probmem, scip->set, scip->stat, negvar) );

   return SCIP_OKAY;
}

/** gets negated variables x' = lb + ub - x of variables x; negated variables are created, if not yet existing
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPgetNegatedVars(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   nvars,              /**< number of variables to get negated variables for */
   SCIP_VAR**            vars,               /**< array of variables to get negated variables for */
   SCIP_VAR**            negvars             /**< array to store the negated variables */
   )
{
   int v;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetNegatedVars", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   for( v = 0; v < nvars; ++v )
   {
      SCIP_CALL( SCIPvarNegate(vars[v], scip->mem->probmem, scip->set, scip->stat, &(negvars[v])) );
   }

   return SCIP_OKAY;
}

/** gets a binary variable that is equal to the given binary variable, and that is either active, fixed, or
 *  multi-aggregated, or the negated variable of an active, fixed, or multi-aggregated variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPgetBinvarRepresentative(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< binary variable to get binary representative for */
   SCIP_VAR**            repvar,             /**< pointer to store the binary representative */
   SCIP_Bool*            negated             /**< pointer to store whether the negation of an active variable was returned */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(repvar != NULL);
   assert(negated != NULL);
   assert(var->scip == scip);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetBinvarRepresentative", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   /* get the active representative of the given variable */
   *repvar = var;
   *negated = FALSE;
   SCIP_CALL( SCIPvarGetProbvarBinary(repvar, negated) );

   /* negate the representative, if it corresponds to the negation of the given variable */
   if( *negated )
   {
      SCIP_CALL( SCIPgetNegatedVar(scip, *repvar, repvar) );
   }

   return SCIP_OKAY;
}

/** gets binary variables that are equal to the given binary variables, and which are either active, fixed, or
 *  multi-aggregated, or the negated variables of active, fixed, or multi-aggregated variables
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPgetBinvarRepresentatives(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   nvars,              /**< number of binary variables to get representatives for */
   SCIP_VAR**            vars,               /**< binary variables to get binary representatives for */
   SCIP_VAR**            repvars,            /**< array to store the binary representatives */
   SCIP_Bool*            negated             /**< array to store whether the negation of an active variable was returned */
   )
{
   int v;

   assert(scip != NULL);
   assert(vars != NULL || nvars == 0);
   assert(repvars != NULL || nvars == 0);
   assert(negated != NULL || nvars == 0);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetBinvarRepresentatives", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   if( nvars == 0 )
      return SCIP_OKAY;

   /* get the active representative of the given variable */
   BMScopyMemoryArray(repvars, vars, nvars);
   BMSclearMemoryArray(negated, nvars);
   SCIP_CALL( SCIPvarsGetProbvarBinary(&repvars, &negated, nvars) );

   /* negate the representatives, if they correspond to the negation of the given variables */
   for( v = nvars - 1; v >= 0; --v )
      if( negated[v] )
      {
         SCIP_CALL( SCIPgetNegatedVar(scip, repvars[v], &(repvars[v])) );
      }

   return SCIP_OKAY;
}

/** flattens aggregation graph of multi-aggregated variable in order to avoid exponential recursion later on
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPflattenVarAggregationGraph(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   assert( scip != NULL );
   assert( var != NULL );
   SCIP_CALL( SCIPcheckStage(scip, "SCIPflattenVarAggregationGraph", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarFlattenAggregationGraph(var, scip->mem->probmem, scip->set) );
   return SCIP_OKAY;
}

/** Transforms a given linear sum of variables, that is a_1*x_1 + ... + a_n*x_n + c into a corresponding linear sum of
 *  active variables, that is b_1*y_1 + ... + b_m*y_m + d.
 *
 *  If the number of needed active variables is greater than the available slots in the variable array, nothing happens
 *  except that the required size is stored in the corresponding variable (requiredsize). Otherwise, the active variable
 *  representation is stored in the variable array, scalar array and constant.
 *
 *  The reason for this approach is that we cannot reallocate memory, since we do not know how the memory has been
 *  allocated (e.g., by a C++ 'new' or SCIP functions).
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note The resulting linear sum is stored into the given variable array, scalar array, and constant. That means the
 *        given entries are overwritten.
 *
 *  @note That method can be used to convert a single variables into variable space of active variables. Therefore call
 *        the method with the linear sum 1.0*x + 0.0.
 */
SCIP_RETCODE SCIPgetProbvarLinearSum(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variable array x_1, ..., x_n in the linear sum which will be
                                              *   overwritten by the variable array y_1, ..., y_m in the linear sum
                                              *   w.r.t. active variables */
   SCIP_Real*            scalars,            /**< scalars a_1, ..., a_n in linear sum which will be overwritten to the
                                              *   scalars b_1, ..., b_m in the linear sum of the active variables  */
   int*                  nvars,              /**< pointer to number of variables in the linear sum which will be
                                              *   overwritten by the number of variables in the linear sum corresponding
                                              *   to the active variables */
   int                   varssize,           /**< available slots in vars and scalars array which is needed to check if
                                              *   the array are large enough for the linear sum w.r.t. active
                                              *   variables */
   SCIP_Real*            constant,           /**< pointer to constant c in linear sum a_1*x_1 + ... + a_n*x_n + c which
                                              *   will chnage to constant d in the linear sum b_1*y_1 + ... + b_m*y_m +
                                              *   d w.r.t. the active variables */
   int*                  requiredsize,       /**< pointer to store the required array size for the linear sum w.r.t. the
                                              *   active variables */
   SCIP_Bool             mergemultiples      /**< should multiple occurrences of a var be replaced by a single coeff? */
   )
{
   assert( scip != NULL );
   assert( nvars != NULL );
   assert( vars != NULL || *nvars == 0 );
   assert( scalars != NULL || *nvars == 0 );
   assert( constant != NULL );
   assert( requiredsize != NULL );
   assert( *nvars <= varssize );

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetProbvarLinearSum", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );
   SCIP_CALL( SCIPvarGetActiveRepresentatives(scip->set, vars, scalars, nvars, varssize, constant, requiredsize, mergemultiples) );

   return SCIP_OKAY;
}

/** transforms given variable, scalar and constant to the corresponding active, fixed, or
 *  multi-aggregated variable, scalar and constant; if the variable resolves to a fixed variable,
 *  "scalar" will be 0.0 and the value of the sum will be stored in "constant"; a multi-aggregation
 *  with only one active variable (this can happen due to fixings after the multi-aggregation),
 *  is treated like an aggregation; if the multi-aggregation constant is infinite, "scalar" will be 0.0
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPgetProbvarSum(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            var,                /**< pointer to problem variable x in sum a*x + c */
   SCIP_Real*            scalar,             /**< pointer to scalar a in sum a*x + c */
   SCIP_Real*            constant            /**< pointer to constant c in sum a*x + c */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(scalar != NULL);
   assert(constant != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetProbvarSum", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );
   SCIP_CALL( SCIPvarGetProbvarSum(var, scip->set, scalar, constant) );

   return SCIP_OKAY;
}

/** return for given variables all their active counterparts; all active variables will be pairwise different
 *  @note It does not hold that the first output variable is the active variable for the first input variable.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPgetActiveVars(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variable array with given variables and as output all active
					      *   variables, if enough slots exist
					      */
   int*                  nvars,              /**< number of given variables, and as output number of active variables,
					      *   if enough slots exist
					      */
   int                   varssize,           /**< available slots in vars array */
   int*                  requiredsize        /**< pointer to store the required array size for the active variables */
   )
{
   assert(scip != NULL);
   assert(nvars != NULL);
   assert(vars != NULL || *nvars == 0);
   assert(varssize >= *nvars);
   assert(requiredsize != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetActiveVars", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );
   SCIP_CALL( SCIPvarsGetActiveVars(scip->set, vars, nvars, varssize, requiredsize) );

   return SCIP_OKAY;
}

/** returns the reduced costs of the variable in the current node's LP relaxation;
 *  the current node has to have a feasible LP.
 *
 *  returns SCIP_INVALID if the variable is active but not in the current LP;
 *  returns 0 if the variable has been aggregated out or fixed in presolving.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_SOLVING
 *
 *  @note The return value of this method should be used carefully if the dual feasibility check was explictely disabled.
 */
SCIP_Real SCIPgetVarRedcost(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get reduced costs, should be a column in current node LP */
   )
{
   assert( scip != NULL );
   assert( var != NULL );
   assert( var->scip == scip );

   switch( SCIPvarGetStatus(var) )
   {
   case SCIP_VARSTATUS_ORIGINAL:
      if( var->data.original.transvar == NULL )
         return SCIP_INVALID;
      return SCIPgetVarRedcost(scip, var->data.original.transvar);

   case SCIP_VARSTATUS_COLUMN:
      return SCIPgetColRedcost(scip, SCIPvarGetCol(var));

   case SCIP_VARSTATUS_LOOSE:
      return SCIP_INVALID;

   case SCIP_VARSTATUS_FIXED:
   case SCIP_VARSTATUS_AGGREGATED:
   case SCIP_VARSTATUS_MULTAGGR:
   case SCIP_VARSTATUS_NEGATED:
      return 0.0;

   default:
      SCIPerrorMessage("unknown variable status\n");
      SCIPABORT();
      return 0.0; /*lint !e527*/
   }
}

/** returns the implied reduced costs of the variable in the current node's LP relaxation;
 *  the current node has to have a feasible LP.
 *
 *  returns SCIP_INVALID if the variable is active but not in the current LP;
 *  returns 0 if the variable has been aggregated out or fixed in presolving.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_SOLVING
 *
 *  @note The return value of this method should be used carefully if the dual feasibility check was explictely disabled.
 */
SCIP_Real SCIPgetVarImplRedcost(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get reduced costs, should be a column in current node LP */
   SCIP_Bool             varfixing           /**< FALSE if for x == 0, TRUE for x == 1 */
   )
{
   assert( scip != NULL );
   assert( var != NULL );
   assert( var->scip == scip );

   switch( SCIPvarGetStatus(var) )
   {
   case SCIP_VARSTATUS_ORIGINAL:
      if( var->data.original.transvar == NULL )
         return SCIP_INVALID;
      return SCIPgetVarImplRedcost(scip, var->data.original.transvar, varfixing);

   case SCIP_VARSTATUS_COLUMN:
      return SCIPvarGetImplRedcost(var, scip->set, varfixing, scip->stat, scip->transprob, scip->lp);

   case SCIP_VARSTATUS_LOOSE:
      return SCIP_INVALID;

   case SCIP_VARSTATUS_FIXED:
   case SCIP_VARSTATUS_AGGREGATED:
   case SCIP_VARSTATUS_MULTAGGR:
   case SCIP_VARSTATUS_NEGATED:
      return 0.0;

   default:
      SCIPerrorMessage("unknown variable status\n");
      SCIPABORT();
      return 0.0; /*lint !e527*/
   }
}


/** returns the Farkas coefficient of the variable in the current node's LP relaxation;
 *  the current node has to have an infeasible LP.
 *
 *  returns SCIP_INVALID if the variable is active but not in the current LP;
 *  returns 0 if the variable has been aggregated out or fixed in presolving.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_SOLVING
 */
SCIP_Real SCIPgetVarFarkasCoef(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get reduced costs, should be a column in current node LP */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(var->scip == scip);

   switch( SCIPvarGetStatus(var) )
   {
   case SCIP_VARSTATUS_ORIGINAL:
      if( var->data.original.transvar == NULL )
         return SCIP_INVALID;
      return SCIPgetVarFarkasCoef(scip,var->data.original.transvar);

   case SCIP_VARSTATUS_COLUMN:
      return SCIPgetColFarkasCoef(scip,SCIPvarGetCol(var));

   case SCIP_VARSTATUS_LOOSE:
      return SCIP_INVALID;

   case SCIP_VARSTATUS_FIXED:
   case SCIP_VARSTATUS_AGGREGATED:
   case SCIP_VARSTATUS_MULTAGGR:
   case SCIP_VARSTATUS_NEGATED:
      return 0.0;

   default:
      SCIPerrorMessage("unknown variable status\n");
      SCIPABORT();
      return 0.0; /*lint !e527*/
   }
}

/** returns lower bound of variable directly before or after the bound change given by the bound change index
 *  was applied
 */
SCIP_Real SCIPgetVarLbAtIndex(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BDCHGIDX*        bdchgidx,           /**< bound change index representing time on path to current node */
   SCIP_Bool             after               /**< should the bound change with given index be included? */
   )
{
   SCIP_VARSTATUS varstatus;
   SCIP_BDCHGINFO* bdchginfo;
   assert(var != NULL);

   varstatus = SCIPvarGetStatus(var);

   /* get bounds of attached variables */
   switch( varstatus )
   {
   case SCIP_VARSTATUS_ORIGINAL:
      assert(var->data.original.transvar != NULL);
      return SCIPgetVarLbAtIndex(scip, var->data.original.transvar, bdchgidx, after);

   case SCIP_VARSTATUS_COLUMN:
   case SCIP_VARSTATUS_LOOSE:
      if( bdchgidx == NULL )
         return SCIPvarGetLbLocal(var);
      else
      {
         bdchginfo = SCIPvarGetLbchgInfo(var, bdchgidx, after);
         if( bdchginfo != NULL )
            return SCIPbdchginfoGetNewbound(bdchginfo);
         else
            return var->glbdom.lb;
      }

   case SCIP_VARSTATUS_FIXED:
      return var->glbdom.lb;

   case SCIP_VARSTATUS_AGGREGATED: /* x = a*y + c  ->  y = (x-c)/a */
      assert(var->data.aggregate.var != NULL);
      if( var->data.aggregate.scalar > 0.0 )
      {
         SCIP_Real lb;

         lb = SCIPgetVarLbAtIndex(scip, var->data.aggregate.var, bdchgidx, after);

         /* a > 0 -> get lower bound of y */
         if( SCIPisInfinity(scip, -lb) )
            return -SCIPinfinity(scip);
         else if( SCIPisInfinity(scip, lb) )
            return SCIPinfinity(scip);
         else
            return var->data.aggregate.scalar * lb + var->data.aggregate.constant;
      }
      else if( var->data.aggregate.scalar < 0.0 )
      {
         SCIP_Real ub;

         ub = SCIPgetVarUbAtIndex(scip, var->data.aggregate.var, bdchgidx, after);

         /* a < 0 -> get upper bound of y */
         if( SCIPisInfinity(scip, -ub) )
            return SCIPinfinity(scip);
         else if( SCIPisInfinity(scip, ub) )
            return -SCIPinfinity(scip);
         else
            return var->data.aggregate.scalar * ub + var->data.aggregate.constant;
      }
      else
      {
         SCIPerrorMessage("scalar is zero in aggregation\n");
         SCIPABORT();
         return SCIP_INVALID; /*lint !e527*/
      }

   case SCIP_VARSTATUS_MULTAGGR:
      /* handle multi-aggregated variables depending on one variable only (possibly caused by SCIPvarFlattenAggregationGraph()) */
      if ( var->data.multaggr.nvars == 1 )
      {
         assert(var->data.multaggr.vars != NULL);
         assert(var->data.multaggr.scalars != NULL);
         assert(var->data.multaggr.vars[0] != NULL);

         if( var->data.multaggr.scalars[0] > 0.0 )
         {
            SCIP_Real lb;

            lb = SCIPgetVarLbAtIndex(scip, var->data.multaggr.vars[0], bdchgidx, after);

            /* a > 0 -> get lower bound of y */
            if( SCIPisInfinity(scip, -lb) )
               return -SCIPinfinity(scip);
            else if( SCIPisInfinity(scip, lb) )
               return SCIPinfinity(scip);
            else
               return var->data.multaggr.scalars[0] * lb + var->data.multaggr.constant;
         }
         else if( var->data.multaggr.scalars[0] < 0.0 )
         {
            SCIP_Real ub;

            ub = SCIPgetVarUbAtIndex(scip, var->data.multaggr.vars[0], bdchgidx, after);

            /* a < 0 -> get upper bound of y */
            if( SCIPisInfinity(scip, -ub) )
               return SCIPinfinity(scip);
            else if( SCIPisInfinity(scip, ub) )
               return -SCIPinfinity(scip);
            else
               return var->data.multaggr.scalars[0] * ub + var->data.multaggr.constant;
         }
         else
         {
            SCIPerrorMessage("scalar is zero in multi-aggregation\n");
            SCIPABORT();
            return SCIP_INVALID; /*lint !e527*/
         }
      }
      SCIPerrorMessage("cannot get the bounds of a multi-aggregated variable.\n");
      SCIPABORT();
      return SCIP_INVALID; /*lint !e527*/

   case SCIP_VARSTATUS_NEGATED: /* x' = offset - x  ->  x = offset - x' */
      assert(var->negatedvar != NULL);
      assert(SCIPvarGetStatus(var->negatedvar) != SCIP_VARSTATUS_NEGATED);
      assert(var->negatedvar->negatedvar == var);
      return var->data.negate.constant - SCIPgetVarUbAtIndex(scip, var->negatedvar, bdchgidx, after);

   default:
      SCIPerrorMessage("unknown variable status\n");
      SCIPABORT();
      return SCIP_INVALID; /*lint !e527*/
   }
}

/** returns upper bound of variable directly before or after the bound change given by the bound change index
 *  was applied
 */
SCIP_Real SCIPgetVarUbAtIndex(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BDCHGIDX*        bdchgidx,           /**< bound change index representing time on path to current node */
   SCIP_Bool             after               /**< should the bound change with given index be included? */
   )
{
   SCIP_VARSTATUS varstatus;
   SCIP_BDCHGINFO* bdchginfo;
   assert(var != NULL);

   varstatus = SCIPvarGetStatus(var);

   /* get bounds of attached variables */
   switch( varstatus )
   {
   case SCIP_VARSTATUS_ORIGINAL:
      assert(var->data.original.transvar != NULL);
      return SCIPgetVarUbAtIndex(scip, var->data.original.transvar, bdchgidx, after);

   case SCIP_VARSTATUS_COLUMN:
   case SCIP_VARSTATUS_LOOSE:
      if( bdchgidx == NULL )
         return SCIPvarGetUbLocal(var);
      else
      {
         bdchginfo = SCIPvarGetUbchgInfo(var, bdchgidx, after);
         if( bdchginfo != NULL )
            return SCIPbdchginfoGetNewbound(bdchginfo);
         else
            return var->glbdom.ub;
      }

   case SCIP_VARSTATUS_FIXED:
      return var->glbdom.ub;

   case SCIP_VARSTATUS_AGGREGATED: /* x = a*y + c  ->  y = (x-c)/a */
      assert(var->data.aggregate.var != NULL);
      if( var->data.aggregate.scalar > 0.0 )
      {
         SCIP_Real ub;

         ub = SCIPgetVarUbAtIndex(scip, var->data.aggregate.var, bdchgidx, after);

         /* a > 0 -> get lower bound of y */
         if( SCIPisInfinity(scip, -ub) )
            return -SCIPinfinity(scip);
         else if( SCIPisInfinity(scip, ub) )
            return SCIPinfinity(scip);
         else
            return var->data.aggregate.scalar * ub + var->data.aggregate.constant;
      }
      else if( var->data.aggregate.scalar < 0.0 )
      {
         SCIP_Real lb;

         lb = SCIPgetVarLbAtIndex(scip, var->data.aggregate.var, bdchgidx, after);

         /* a < 0 -> get upper bound of y */
         if ( SCIPisInfinity(scip, -lb) )
            return SCIPinfinity(scip);
         else if ( SCIPisInfinity(scip, lb) )
            return -SCIPinfinity(scip);
         else
            return var->data.aggregate.scalar * lb + var->data.aggregate.constant;
      }
      else
      {
         SCIPerrorMessage("scalar is zero in aggregation\n");
         SCIPABORT();
         return SCIP_INVALID; /*lint !e527*/
      }

   case SCIP_VARSTATUS_MULTAGGR:
      /* handle multi-aggregated variables depending on one variable only (possibly caused by SCIPvarFlattenAggregationGraph()) */
      if ( var->data.multaggr.nvars == 1 )
      {
         assert(var->data.multaggr.vars != NULL);
         assert(var->data.multaggr.scalars != NULL);
         assert(var->data.multaggr.vars[0] != NULL);

         if( var->data.multaggr.scalars[0] > 0.0 )
         {
            SCIP_Real ub;

            ub = SCIPgetVarUbAtIndex(scip, var->data.multaggr.vars[0], bdchgidx, after);

            /* a > 0 -> get lower bound of y */
            if ( SCIPisInfinity(scip, -ub) )
               return -SCIPinfinity(scip);
            else if ( SCIPisInfinity(scip, ub) )
               return SCIPinfinity(scip);
            else
               return var->data.multaggr.scalars[0] * ub + var->data.multaggr.constant;
         }
         else if( var->data.multaggr.scalars[0] < 0.0 )
         {
            SCIP_Real lb;

            lb = SCIPgetVarLbAtIndex(scip, var->data.multaggr.vars[0], bdchgidx, after);

            /* a < 0 -> get upper bound of y */
            if ( SCIPisInfinity(scip, -lb) )
               return SCIPinfinity(scip);
            else if ( SCIPisInfinity(scip, lb) )
               return -SCIPinfinity(scip);
            else
               return var->data.multaggr.scalars[0] * lb + var->data.multaggr.constant;
         }
         else
         {
            SCIPerrorMessage("scalar is zero in multi-aggregation\n");
            SCIPABORT();
            return SCIP_INVALID; /*lint !e527*/
         }
      }
      SCIPerrorMessage("cannot get the bounds of a multiple aggregated variable.\n");
      SCIPABORT();
      return SCIP_INVALID; /*lint !e527*/

   case SCIP_VARSTATUS_NEGATED: /* x' = offset - x  ->  x = offset - x' */
      assert(var->negatedvar != NULL);
      assert(SCIPvarGetStatus(var->negatedvar) != SCIP_VARSTATUS_NEGATED);
      assert(var->negatedvar->negatedvar == var);
      return var->data.negate.constant - SCIPgetVarLbAtIndex(scip, var->negatedvar, bdchgidx, after);

   default:
      SCIPerrorMessage("unknown variable status\n");
      SCIPABORT();
      return SCIP_INVALID; /*lint !e527*/
   }
}

/** returns lower or upper bound of variable directly before or after the bound change given by the bound change index
 *  was applied
 */
SCIP_Real SCIPgetVarBdAtIndex(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BOUNDTYPE        boundtype,          /**< type of bound: lower or upper bound */
   SCIP_BDCHGIDX*        bdchgidx,           /**< bound change index representing time on path to current node */
   SCIP_Bool             after               /**< should the bound change with given index be included? */
   )
{
   if( boundtype == SCIP_BOUNDTYPE_LOWER )
      return SCIPgetVarLbAtIndex(scip, var, bdchgidx, after);
   else
   {
      assert(boundtype == SCIP_BOUNDTYPE_UPPER);
      return SCIPgetVarUbAtIndex(scip, var, bdchgidx, after);
   }
}

/** returns whether the binary variable was fixed at the time given by the bound change index */
SCIP_Bool SCIPgetVarWasFixedAtIndex(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BDCHGIDX*        bdchgidx,           /**< bound change index representing time on path to current node */
   SCIP_Bool             after               /**< should the bound change with given index be included? */
   )
{
   assert(var != NULL);
   assert(SCIPvarIsBinary(var));

   /* check the current bounds first in order to decide at which bound change information we have to look
    * (which is expensive because we have to follow the aggregation tree to the active variable)
    */
   return ((SCIPvarGetLbLocal(var) > 0.5 && SCIPgetVarLbAtIndex(scip, var, bdchgidx, after) > 0.5)
      || (SCIPvarGetUbLocal(var) < 0.5 && SCIPgetVarUbAtIndex(scip, var, bdchgidx, after) < 0.5));
}

/** gets solution value for variable in current node
 *
 *  @return solution value for variable in current node
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_Real SCIPgetVarSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get solution value for */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );
   assert( var->scip == scip );

   return SCIPvarGetSol(var, SCIPtreeHasCurrentNodeLP(scip->tree));
}

/** gets solution values of multiple variables in current node
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetVarSols(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   nvars,              /**< number of variables to get solution value for */
   SCIP_VAR**            vars,               /**< array with variables to get value for */
   SCIP_Real*            vals                /**< array to store solution values of variables */
   )
{
   int v;

   assert(nvars == 0 || vars != NULL);
   assert(nvars == 0 || vals != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarSols", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( SCIPtreeHasCurrentNodeLP(scip->tree) )
   {
      for( v = 0; v < nvars; ++v )
         vals[v] = SCIPvarGetLPSol(vars[v]);
   }
   else
   {
      for( v = 0; v < nvars; ++v )
         vals[v] = SCIPvarGetPseudoSol(vars[v]);
   }

   return SCIP_OKAY;
}

/** sets the solution value of all variables in the global relaxation solution to zero
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPclearRelaxSolVals(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_VAR** vars;
   int nvars;
   int v;

   assert(scip != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPclearRelaxSolVals", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   /* the relaxation solution is already cleared */
   if( SCIPrelaxationIsSolZero(scip->relaxation) )
      return SCIP_OKAY;

   SCIP_CALL( SCIPgetVarsData(scip, &vars, &nvars, NULL, NULL, NULL, NULL) );

   for( v = 0; v < nvars; v++ )
   {
      SCIP_CALL( SCIPvarSetRelaxSol(vars[v], scip->set, scip->relaxation, 0.0, FALSE) );
   }

   SCIPrelaxationSetSolObj(scip->relaxation, 0.0);
   SCIPrelaxationSetSolZero(scip->relaxation, TRUE);

   return SCIP_OKAY;
}

/** sets the value of the given variable in the global relaxation solution;
 *  this solution can be filled by the relaxation handlers  and can be used by heuristics and for separation;
 *  You can use SCIPclearRelaxSolVals() to set all values to zero, initially;
 *  after setting all solution values, you have to call SCIPmarkRelaxSolValid()
 *  to inform SCIP that the stored solution is valid
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note This method incrementally updates the objective value of the relaxation solution. If the whole solution
 *        should be updated, using SCIPsetRelaxSolVals() instead or calling SCIPclearRelaxSolVals() before setting
 *        the first value to reset the solution and the objective value to 0 may help the numerics.
 */
SCIP_RETCODE SCIPsetRelaxSolVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to set value for */
   SCIP_Real             val                 /**< solution value of variable */
   )
{
   assert(scip != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetRelaxSolVal", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarSetRelaxSol(var, scip->set, scip->relaxation, val, TRUE) );

   if( val != 0.0 )
      SCIPrelaxationSetSolZero(scip->relaxation, FALSE);
   SCIPrelaxationSetSolValid(scip->relaxation, FALSE, FALSE);

   return SCIP_OKAY;
}

/** sets the values of the given variables in the global relaxation solution and informs SCIP about the validity
 *  and whether the solution can be enforced via linear cuts;
 *  this solution can be filled by the relaxation handlers  and can be used by heuristics and for separation;
 *  the solution is automatically cleared, s.t. all other variables get value 0.0
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPsetRelaxSolVals(
   SCIP*                 scip,               /**< SCIP data structure */
   int                   nvars,              /**< number of variables to set relaxation solution value for */
   SCIP_VAR**            vars,               /**< array with variables to set value for */
   SCIP_Real*            vals,               /**< array with solution values of variables */
   SCIP_Bool             includeslp          /**< does the relaxator contain all cuts in the LP? */
   )
{
   int v;

   assert(scip != NULL);
   assert(nvars == 0 || vars != NULL);
   assert(nvars == 0 || vals != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetRelaxSolVals", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPclearRelaxSolVals(scip) );

   for( v = 0; v < nvars; v++ )
   {
      SCIP_CALL( SCIPvarSetRelaxSol(vars[v], scip->set, scip->relaxation, vals[v], TRUE) );
   }

   SCIPrelaxationSetSolZero(scip->relaxation, FALSE);
   SCIPrelaxationSetSolValid(scip->relaxation, TRUE, includeslp);

   return SCIP_OKAY;
}

/** sets the values of the variables in the global relaxation solution to the values in the given primal solution
 *  and informs SCIP about the validity and whether the solution can be enforced via linear cuts;
 *  the relaxation solution can be filled by the relaxation handlers and might be used by heuristics and for separation
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPsetRelaxSolValsSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal relaxation solution */
   SCIP_Bool             includeslp          /**< does the relaxator contain all cuts in the LP? */
   )
{
   SCIP_VAR** vars;
   SCIP_Real* vals;
   int nvars;
   int v;

   assert(scip != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetRelaxSolValsSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPgetVarsData(scip, &vars, &nvars, NULL, NULL, NULL, NULL) );

   /* alloc buffer array for solution values of the variables and get the values */
   SCIP_CALL( SCIPallocBufferArray(scip, &vals, nvars) );
   SCIP_CALL( SCIPgetSolVals(scip, sol, nvars, vars, vals) );

   SCIP_CALL( SCIPclearRelaxSolVals(scip) );

   for( v = 0; v < nvars; v++ )
   {
      SCIP_CALL( SCIPvarSetRelaxSol(vars[v], scip->set, scip->relaxation, vals[v], FALSE) );
   }

   SCIPrelaxationSetSolObj(scip->relaxation, SCIPsolGetObj(sol, scip->set, scip->transprob, scip->origprob));

   SCIPrelaxationSetSolZero(scip->relaxation, FALSE);
   SCIPrelaxationSetSolValid(scip->relaxation, TRUE, includeslp);

   SCIPfreeBufferArray(scip, &vals);

   return SCIP_OKAY;
}

/** returns whether the relaxation solution is valid
 *
 *  @return TRUE, if the relaxation solution is valid; FALSE, otherwise
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_Bool SCIPisRelaxSolValid(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPisRelaxSolValid", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPrelaxationIsSolValid(scip->relaxation);
}

/** informs SCIP that the relaxation solution is valid and whether the relaxation can be enforced through linear cuts
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPmarkRelaxSolValid(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             includeslp          /**< does the relaxator contain all cuts in the LP? */
   )
{
   assert(scip != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPmarkRelaxSolValid", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPrelaxationSetSolValid(scip->relaxation, TRUE, includeslp);

   return SCIP_OKAY;
}

/** informs SCIP, that the relaxation solution is invalid
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPmarkRelaxSolInvalid(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPmarkRelaxSolInvalid", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPrelaxationSetSolValid(scip->relaxation, FALSE, FALSE);

   return SCIP_OKAY;
}

/** gets the relaxation solution value of the given variable
 *
 *  @return the relaxation solution value of the given variable
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_Real SCIPgetRelaxSolVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get value for */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(var->scip == scip);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetRelaxSolVal", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( !SCIPrelaxationIsSolValid(scip->relaxation) )
   {
      SCIPerrorMessage("Relaxation Solution is not valid!\n");
      SCIPABORT();
      return SCIP_INVALID; /*lint !e527*/
   }

   return SCIPvarGetRelaxSol(var, scip->set);
}

/** gets the relaxation solution objective value
 *
 *  @return the objective value of the relaxation solution
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_Real SCIPgetRelaxSolObj(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetRelaxSolObj", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( !SCIPrelaxationIsSolValid(scip->relaxation) )
   {
      SCIPerrorMessage("Relaxation Solution is not valid!\n");
      SCIPABORT();
      return SCIP_INVALID; /*lint !e527*/
   }

   return SCIPrelaxationGetSolObj(scip->relaxation);
}

/** determine which branching direction should be evaluated first by strong branching
 *
 *  @return TRUE iff strong branching should first evaluate the down child
 *
 */
SCIP_Bool SCIPisStrongbranchDownFirst(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to determine the branching direction on */
   )
{
   switch( scip->set->branch_firstsbchild )
   {
      case 'u':
         return FALSE;
      case 'd':
         return TRUE;
      case 'a':
         return (SCIPvarGetNLocksDown(var) > SCIPvarGetNLocksUp(var));
      default:
         assert(scip->set->branch_firstsbchild == 'h');
         return (SCIPgetVarAvgCutoffs(scip, var, SCIP_BRANCHDIR_DOWNWARDS) > SCIPgetVarAvgCutoffs(scip, var, SCIP_BRANCHDIR_UPWARDS));
   }
}

/** start strong branching - call before any strong branching
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note if propagation is enabled, strong branching is not done directly on the LP, but probing nodes are created
 *        which allow to perform propagation but also creates some overhead
 */
SCIP_RETCODE SCIPstartStrongbranch(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             enablepropagation   /**< should propagation be done before solving the strong branching LP? */
   )
{
   assert( scip != NULL );
   SCIP_CALL( SCIPcheckStage(scip, "SCIPstartStrongbranch", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert(!SCIPinProbing(scip));

   SCIPdebugMsg(scip, "starting strong branching mode%s: lpcount=%" SCIP_LONGINT_FORMAT "\n", enablepropagation ? " with propagation" : "", scip->stat->lpcount - scip->stat->nsbdivinglps);

   /* start probing mode to allow propagation before solving the strong branching LPs; if no propagation should be done,
    * start the strong branching mode in the LP interface
    */
   if( enablepropagation )
   {
      if( SCIPtreeProbing(scip->tree) )
      {
         SCIPerrorMessage("cannot start strong branching with propagation while in probing mode\n");
         return SCIP_INVALIDCALL;
      }

      if( scip->lp != NULL && SCIPlpDiving(scip->lp) )
      {
         SCIPerrorMessage("cannot start strong branching with propagation while in diving mode\n");
         return SCIP_INVALIDCALL;
      }

      /* other then in SCIPstartProbing(), we do not disable collecting variable statistics during strong branching;
       * we cannot disable it, because the pseudo costs would not be updated, otherwise,
       * and reliability branching would end up doing strong branching all the time
       */
      SCIP_CALL( SCIPtreeStartProbing(scip->tree, scip->mem->probmem, scip->set, scip->lp, scip->relaxation, scip->transprob, TRUE) );

      /* inform the LP that the current probing mode is used for strong branching */
      SCIPlpStartStrongbranchProbing(scip->lp);

   }
   else
   {
      SCIP_CALL( SCIPlpStartStrongbranch(scip->lp) );
   }

   /* reset local strong branching info */
   scip->stat->lastsblpsolstats[0] = scip->stat->lastsblpsolstats[1] = SCIP_LPSOLSTAT_NOTSOLVED;

   return SCIP_OKAY;
}

/** end strong branching - call after any strong branching
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPendStrongbranch(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert( scip != NULL );

   SCIP_CALL( SCIPcheckStage(scip, "SCIPendStrongbranch", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   /* depending on whether the strong branching mode was started with propagation enabled or not, we end the strong
    * branching probing mode or the LP strong branching mode
    */
   if( SCIPtreeProbing(scip->tree) )
   {
      SCIP_NODE* node;
      SCIP_DOMCHG* domchg;
      SCIP_VAR** boundchgvars;
      SCIP_Real* bounds;
      SCIP_BOUNDTYPE* boundtypes;
      int nboundchgs;
      int nbnds;
      int i;

      /* collect all bound changes deducted during probing, which were applied at the probing root and apply them to the
       * focusnode
       */
      node = SCIPgetCurrentNode(scip);
      assert(SCIPnodeGetType(node) == SCIP_NODETYPE_PROBINGNODE);
      assert(SCIPgetProbingDepth(scip) == 0);

      domchg = SCIPnodeGetDomchg(node);
      nboundchgs = SCIPdomchgGetNBoundchgs(domchg);

      SCIP_CALL( SCIPallocBufferArray(scip, &boundchgvars, nboundchgs) );
      SCIP_CALL( SCIPallocBufferArray(scip, &bounds, nboundchgs) );
      SCIP_CALL( SCIPallocBufferArray(scip, &boundtypes, nboundchgs) );

      for( i = 0, nbnds = 0; i < nboundchgs; ++i )
      {
         SCIP_BOUNDCHG* boundchg;

         boundchg = SCIPdomchgGetBoundchg(domchg, i);

         /* ignore redundant bound changes */
         if( SCIPboundchgIsRedundant(boundchg) )
            continue;

         boundchgvars[nbnds] = SCIPboundchgGetVar(boundchg);
         bounds[nbnds] = SCIPboundchgGetNewbound(boundchg);
         boundtypes[nbnds] = SCIPboundchgGetBoundtype(boundchg);
         ++nbnds;
      }

      SCIPdebugMsg(scip, "ending strong branching with probing: %d bound changes collected\n", nbnds);

      /* inform the LP that the probing mode is not used for strong branching anymore */
      SCIPlpEndStrongbranchProbing(scip->lp);

      /* switch back from probing to normal operation mode and restore variables and constraints to focus node */
      SCIP_CALL( SCIPtreeEndProbing(scip->tree, scip->reopt, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
         scip->transprob, scip->origprob, scip->lp, scip->relaxation, scip->primal,
         scip->branchcand, scip->eventqueue, scip->eventfilter, scip->cliquetable) );

      /* apply the collected bound changes */
      for( i = 0; i < nbnds; ++i )
      {
         if( boundtypes[i] == SCIP_BOUNDTYPE_LOWER )
         {
            SCIPdebugMsg(scip, "apply probing lower bound change <%s> >= %.9g\n", SCIPvarGetName(boundchgvars[i]), bounds[i]);
            SCIP_CALL( SCIPchgVarLb(scip, boundchgvars[i], bounds[i]) );
         }
         else
         {
            SCIPdebugMsg(scip, "apply probing upper bound change <%s> <= %.9g\n", SCIPvarGetName(boundchgvars[i]), bounds[i]);
            SCIP_CALL( SCIPchgVarUb(scip, boundchgvars[i], bounds[i]) );
         }
      }

      SCIPfreeBufferArray(scip, &boundtypes);
      SCIPfreeBufferArray(scip, &bounds);
      SCIPfreeBufferArray(scip, &boundchgvars);
   }
   else
   {
      SCIPdebugMsg(scip, "ending strong branching\n");

      SCIP_CALL( SCIPlpEndStrongbranch(scip->lp) );
   }

   return SCIP_OKAY;
}

/** analyze the strong branching for the given variable; that includes conflict analysis for infeasible branches and
 *  storing of root reduced cost information
 */
static
SCIP_RETCODE analyzeStrongbranch(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to analyze */
   SCIP_Bool*            downinf,            /**< pointer to store whether the downwards branch is infeasible, or NULL */
   SCIP_Bool*            upinf,              /**< pointer to store whether the upwards branch is infeasible, or NULL */
   SCIP_Bool*            downconflict,       /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible downwards branch, or NULL */
   SCIP_Bool*            upconflict          /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible upwards branch, or NULL */
   )
{
   SCIP_COL* col;
   SCIP_Bool downcutoff;
   SCIP_Bool upcutoff;

   col = SCIPvarGetCol(var);
   assert(col != NULL);

   downcutoff = col->sbdownvalid && SCIPsetIsGE(scip->set, col->sbdown, scip->lp->cutoffbound);
   upcutoff = col->sbupvalid && SCIPsetIsGE(scip->set, col->sbup, scip->lp->cutoffbound);

   if( downinf != NULL )
      *downinf = downcutoff;
   if( upinf != NULL )
      *upinf = upcutoff;

   /* analyze infeasible strong branching sub problems:
    * because the strong branching's bound change is necessary for infeasibility, it cannot be undone;
    * therefore, infeasible strong branchings on non-binary variables will not produce a valid conflict constraint
    */
   if( scip->set->conf_enable && scip->set->conf_usesb && scip->set->nconflicthdlrs > 0
      && SCIPvarIsBinary(var) && SCIPtreeGetCurrentDepth(scip->tree) > 0 )
   {
      if( (downcutoff && SCIPsetFeasCeil(scip->set, col->primsol-1.0) >= col->lb - 0.5)
         || (upcutoff && SCIPsetFeasFloor(scip->set, col->primsol+1.0) <= col->ub + 0.5) )
      {
         assert(downconflict != NULL);
         assert(upconflict   != NULL);
         SCIP_CALL( SCIPconflictAnalyzeStrongbranch(scip->conflict, scip->conflictstore, scip->mem->probmem, scip->set, scip->stat,
               scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, col, downconflict, upconflict) );
      }
   }

   /* the strong branching results can be used to strengthen the root reduced cost information which is used for example
    * to propagate against the cutoff bound
    *
    * @note Ignore the results if the LP solution of the down (up) branch LP is smaller which should not happened by
    *       theory but can arise due to numerical issues.
    */
   if( SCIPtreeGetCurrentDepth(scip->tree) == 0 && SCIPvarIsBinary(var) && SCIPlpIsDualReliable(scip->lp) )
   {
      SCIP_Real lpobjval;

      assert(SCIPgetLPSolstat(scip) == SCIP_LPSOLSTAT_OPTIMAL);

      lpobjval =  SCIPlpGetObjval(scip->lp, scip->set, scip->transprob);

      if( col->sbdownvalid && SCIPsetFeasCeil(scip->set, col->primsol-1.0) >= col->lb - 0.5 && lpobjval < col->sbdown )
         SCIPvarUpdateBestRootSol(var, scip->set, SCIPvarGetUbGlobal(var), -(col->sbdown - lpobjval), lpobjval);
      if( col->sbupvalid && SCIPsetFeasFloor(scip->set, col->primsol+1.0) <= col->ub + 0.5 && lpobjval < col->sbup )
         SCIPvarUpdateBestRootSol(var, scip->set, SCIPvarGetLbGlobal(var), col->sbup - lpobjval,  lpobjval);
   }

   return SCIP_OKAY;
}

/** gets strong branching information on column variable with fractional value
 *
 *  Before calling this method, the strong branching mode must have been activated by calling SCIPstartStrongbranch();
 *  after strong branching was done for all candidate variables, the strong branching mode must be ended by
 *  SCIPendStrongbranch(). Since this method does not apply domain propagation before strongbranching,
 *  propagation should not be enabled in the SCIPstartStrongbranch() call.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetVarStrongbranchFrac(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get strong branching values for */
   int                   itlim,              /**< iteration limit for strong branchings */
   SCIP_Real*            down,               /**< stores dual bound after branching column down */
   SCIP_Real*            up,                 /**< stores dual bound after branching column up */
   SCIP_Bool*            downvalid,          /**< stores whether the returned down value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Bool*            upvalid,            /**< stores whether the returned up value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Bool*            downinf,            /**< pointer to store whether the downwards branch is infeasible, or NULL */
   SCIP_Bool*            upinf,              /**< pointer to store whether the upwards branch is infeasible, or NULL */
   SCIP_Bool*            downconflict,       /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible downwards branch, or NULL */
   SCIP_Bool*            upconflict,         /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible upwards branch, or NULL */
   SCIP_Bool*            lperror             /**< pointer to store whether an unresolved LP error occurred or the
                                              *   solving process should be stopped (e.g., due to a time limit) */
   )
{
   SCIP_COL* col;

   assert(scip != NULL);
   assert(var != NULL);
   assert(lperror != NULL);
   assert(!SCIPtreeProbing(scip->tree)); /* we should not be in strong branching with propagation mode */
   assert(var->scip == scip);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarStrongbranchFrac", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( downvalid != NULL )
      *downvalid = FALSE;
   if( upvalid != NULL )
      *upvalid = FALSE;
   if( downinf != NULL )
      *downinf = FALSE;
   if( upinf != NULL )
      *upinf = FALSE;
   if( downconflict != NULL )
      *downconflict = FALSE;
   if( upconflict != NULL )
      *upconflict = FALSE;

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
   {
      SCIPerrorMessage("cannot get strong branching information on non-COLUMN variable <%s>\n", SCIPvarGetName(var));
      return SCIP_INVALIDDATA;
   }

   col = SCIPvarGetCol(var);
   assert(col != NULL);

   if( !SCIPcolIsInLP(col) )
   {
      SCIPerrorMessage("cannot get strong branching information on variable <%s> not in current LP\n", SCIPvarGetName(var));
      return SCIP_INVALIDDATA;
   }

   /* check if the solving process should be aborted */
   if( SCIPsolveIsStopped(scip->set, scip->stat, FALSE) )
   {
      /* mark this as if the LP failed */
      *lperror = TRUE;
      return SCIP_OKAY;
   }

   /* call strong branching for column with fractional value */
   SCIP_CALL( SCIPcolGetStrongbranch(col, FALSE, scip->set, scip->stat, scip->transprob, scip->lp, itlim,
         down, up, downvalid, upvalid, lperror) );

   /* check, if the branchings are infeasible; in exact solving mode, we cannot trust the strong branching enough to
    * declare the sub nodes infeasible
    */
   if( !(*lperror) && SCIPprobAllColsInLP(scip->transprob, scip->set, scip->lp) && !scip->set->misc_exactsolve )
   {
      SCIP_CALL( analyzeStrongbranch(scip, var, downinf, upinf, downconflict, upconflict) );
   }

   return SCIP_OKAY;
}

/** create, solve, and evaluate a single strong branching child (for strong branching with propagation) */
static
SCIP_RETCODE performStrongbranchWithPropagation(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get strong branching values for */
   SCIP_Bool             down,               /**< do we regard the down child? */
   SCIP_Bool             firstchild,         /**< is this the first of the two strong branching children? */
   SCIP_Bool             propagate,          /**< should domain propagation be performed? */
   SCIP_Real             newbound,           /**< new bound to apply at the strong branching child */
   int                   itlim,              /**< iteration limit for strong branchings */
   int                   maxproprounds,      /**< maximum number of propagation rounds (-1: no limit, -2: parameter
                                              *   settings) */
   SCIP_Real*            value,              /**< stores dual bound for strong branching child */
   SCIP_Bool*            valid,              /**< stores whether the returned value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Longint*         ndomreductions,     /**< pointer to store the number of domain reductions found, or NULL */
   SCIP_Bool*            conflict,           /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible strong branching child, or NULL */
   SCIP_Bool*            lperror,            /**< pointer to store whether an unresolved LP error occurred or the
                                              *   solving process should be stopped (e.g., due to a time limit) */
   SCIP_VAR**            vars,               /**< active problem variables */
   int                   nvars,              /**< number of active problem variables */
   SCIP_Real*            newlbs,             /**< array to store valid lower bounds for all active variables, or NULL */
   SCIP_Real*            newubs,             /**< array to store valid upper bounds for all active variables, or NULL */
   SCIP_Bool*            foundsol,           /**< pointer to store whether a primal solution was found during strong branching */
   SCIP_Bool*            cutoff              /**< pointer to store whether the strong branching child is infeasible */
   )
{
   SCIP_Longint ndomreds;

   assert(value != NULL);
   assert(foundsol != NULL);
   assert(cutoff != NULL);
   assert(valid != NULL ? !(*valid) : TRUE);

   *foundsol = FALSE;
   *cutoff = FALSE;

   /* check whether the strong branching child is already infeasible due to the bound change */
   if( down )
   {
      /* the down branch is infeasible due to the branching bound change; since this means that solval is not within the
       * bounds, this should only happen if previous strong branching calls on other variables detected bound changes which
       * are valid for and were already applied at the probing root
       */
      if( newbound < SCIPvarGetLbLocal(var) - 0.5 )
      {
         *value = SCIPinfinity(scip);

         if( valid != NULL )
            *valid = TRUE;

         /* bound changes are applied in SCIPendStrongbranch(), which can be seen as a conflict constraint */
         if( conflict != NULL )
            *conflict = TRUE;

         *cutoff = TRUE;

         return SCIP_OKAY;
      }
   }
   else
   {
      /* the up branch is infeasible due to the branching bound change; since this means that solval is not within the
       * bounds, this should only happen if previous strong branching calls on other variables detected bound changes which
       * are valid for and were already applied at the probing root
       */
      if( newbound > SCIPvarGetUbLocal(var) + 0.5 )
      {
         *value = SCIPinfinity(scip);

         if( valid != NULL )
            *valid = TRUE;

         /* bound changes are applied in SCIPendStrongbranch(), which can be seen as a conflict constraint */
         if( conflict != NULL )
            *conflict = TRUE;

         *cutoff = TRUE;

         return SCIP_OKAY;
      }
   }

   /* we need to ensure that we can create at least one new probing node without exceeding the maximal tree depth */
   if( SCIP_MAXTREEDEPTH > SCIPtreeGetProbingDepth(scip->tree) )
   {
      /* create a new probing node for the strong branching child and apply the new bound for the variable */
      SCIP_CALL( SCIPnewProbingNode(scip) );

      if( down )
      {
         assert(SCIPisGE(scip, newbound, SCIPvarGetLbLocal(var)));
         if( SCIPisLT(scip, newbound, SCIPvarGetUbLocal(var)) )
         {
            SCIP_CALL( SCIPchgVarUbProbing(scip, var, newbound) );
         }
      }
      else
      {
         assert(SCIPisLE(scip, newbound, SCIPvarGetUbLocal(var)));
         if( SCIPisGT(scip, newbound, SCIPvarGetLbLocal(var)) )
         {
            SCIP_CALL( SCIPchgVarLbProbing(scip, var, newbound) );
         }
      }
   }
   else
   {
      if( valid != NULL )
         *valid = FALSE;

      if( cutoff != NULL ) /*lint !e774*/
         *cutoff = FALSE;

      if( conflict != NULL )
         *conflict = FALSE;

      return SCIP_OKAY;
   }

   /* propagate domains at the probing node */
   if( propagate )
   {
      /* start time measuring */
      SCIPclockStart(scip->stat->strongpropclock, scip->set);

      ndomreds = 0;
      SCIP_CALL( SCIPpropagateProbing(scip, maxproprounds, cutoff, &ndomreds) );

      /* store number of domain reductions in strong branching */
      if( down )
         SCIPstatAdd(scip->stat, scip->set, nsbdowndomchgs, ndomreds);
      else
         SCIPstatAdd(scip->stat, scip->set, nsbupdomchgs, ndomreds);

      if( ndomreductions != NULL )
         *ndomreductions = ndomreds;

      /* stop time measuring */
      SCIPclockStop(scip->stat->strongpropclock, scip->set);

      if( *cutoff )
      {
         *value = SCIPinfinity(scip);

         if( valid != NULL )
            *valid = TRUE;

         SCIPdebugMsg(scip, "%s branch of var <%s> detected infeasible during propagation\n",
            down ? "down" : "up", SCIPvarGetName(var));
      }
   }

   /* if propagation did not already detect infeasibility, solve the probing LP */
   if( !(*cutoff) )
   {
      SCIP_CALL( SCIPsolveProbingLP(scip, itlim, lperror, cutoff) );
      assert(SCIPisLPRelax(scip));

      if( *cutoff )
      {
         assert(!(*lperror));

         *value = SCIPinfinity(scip);

         if( valid != NULL )
            *valid = TRUE;

         SCIPdebugMsg(scip, "%s branch of var <%s> detected infeasible in LP solving: status=%d\n",
            down ? "down" : "up", SCIPvarGetName(var), SCIPgetLPSolstat(scip));
      }
      else if( !(*lperror) )
      {
         /* save the lp solution status */
         scip->stat->lastsblpsolstats[down ? 0 : 1] = SCIPgetLPSolstat(scip);

         switch( SCIPgetLPSolstat(scip) )
         {
         case SCIP_LPSOLSTAT_OPTIMAL:
         {
            *value = SCIPgetLPObjval(scip);
            assert(SCIPisLT(scip, *value, SCIPgetCutoffbound(scip)));

            SCIPdebugMsg(scip, "probing LP solved to optimality, objective value: %16.9g\n", *value);

            if( valid != NULL )
               *valid = TRUE;

            /* check the strong branching LP solution for feasibility */
            SCIP_CALL( SCIPtryStrongbranchLPSol(scip, foundsol, cutoff) );
            break;
         }
         case SCIP_LPSOLSTAT_ITERLIMIT:
            ++scip->stat->nsbtimesiterlimhit;
            /*lint -fallthrough*/
         case SCIP_LPSOLSTAT_TIMELIMIT:
         {
            /* use LP value as estimate */
            SCIP_LPI* lpi;
            SCIP_Real objval;
            SCIP_Real looseobjval;

            SCIPdebugMsg(scip, "probing LP hit %s limit\n", SCIPgetLPSolstat(scip) == SCIP_LPSOLSTAT_ITERLIMIT ? "iteration" : "time");

            /* we access the LPI directly, because when a time limit was hit, we cannot access objective value and dual
             * feasibility using the SCIPlp... methods; we should try to avoid direct calls to the LPI, but this is rather
             * uncritical here, because we are immediately after the SCIPsolveProbingLP() call, because we access the LPI
             * read-only, and we check SCIPlpiWasSolved() first
             */
            SCIP_CALL( SCIPgetLPI(scip, &lpi) );

            if( SCIPlpiWasSolved(lpi) )
            {
               SCIP_CALL( SCIPlpiGetObjval(lpi, &objval) );
               looseobjval = SCIPlpGetLooseObjval(scip->lp, scip->set, scip->transprob);

               /* the infinity value in the LPI should not be smaller than SCIP's infinity value */
               assert(!SCIPlpiIsInfinity(lpi, objval) || SCIPisInfinity(scip, objval));

               /* we use SCIP's infinity value here because a value larger than this is counted as infeasible by SCIP */
               if( SCIPisInfinity(scip, objval) )
                  *value = SCIPinfinity(scip);
               else if( SCIPisInfinity(scip, -looseobjval) )
                  *value = -SCIPinfinity(scip);
               else
                  *value = objval + looseobjval;

               if( SCIPlpiIsDualFeasible(lpi) )
               {
                  if( valid != NULL )
                     *valid = TRUE;

                  if( SCIPisGE(scip, *value, SCIPgetCutoffbound(scip)) )
                     *cutoff = TRUE;
               }
            }
            break;
         }
         case SCIP_LPSOLSTAT_ERROR:
         case SCIP_LPSOLSTAT_UNBOUNDEDRAY:
            *lperror = TRUE;
            break;
         case SCIP_LPSOLSTAT_NOTSOLVED: /* should only be the case for *cutoff = TRUE or *lperror = TRUE */
         case SCIP_LPSOLSTAT_OBJLIMIT: /* in this case, *cutoff should be TRUE and we should not get here */
         case SCIP_LPSOLSTAT_INFEASIBLE: /* in this case, *cutoff should be TRUE and we should not get here */
         default:
            SCIPerrorMessage("invalid LP solution status <%d>\n", SCIPgetLPSolstat(scip));
            return SCIP_INVALIDDATA;
         }  /*lint !e788*/
      }

      /* If columns are missing in the LP, the cutoff flag may be wrong. Therefore, we need to set it and the valid pointer
       * to false here.
       */
      if( (*cutoff) && !SCIPallColsInLP(scip) )
      {
         *cutoff = FALSE;
      }

#ifndef NDEBUG
      if( *lperror )
      {
         SCIPdebugMsg(scip, "error during strong branching probing LP solving: status=%d\n", SCIPgetLPSolstat(scip));
      }
#endif
   }


   /* if the subproblem was feasible, we store the local bounds of the variables after propagation and (possibly)
    * conflict analysis
    * @todo do this after propagation? should be able to get valid bounds more often, but they might be weaker
    */
   if( !(*cutoff) && newlbs != NULL)
   {
      int v;

      assert(newubs != NULL);

      /* initialize the newlbs and newubs to the current local bounds */
      if( firstchild )
      {
         for( v = 0; v < nvars; ++v )
         {
            newlbs[v] = SCIPvarGetLbLocal(vars[v]);
            newubs[v] = SCIPvarGetUbLocal(vars[v]);
         }
      }
      /* update newlbs and newubs: take the weaker of the already stored bounds and the current local bounds */
      else
      {
         for( v = 0; v < nvars; ++v )
         {
            SCIP_Real lb = SCIPvarGetLbLocal(vars[v]);
            SCIP_Real ub = SCIPvarGetUbLocal(vars[v]);

            newlbs[v] = MIN(newlbs[v], lb);
            newubs[v] = MAX(newubs[v], ub);
         }
      }
   }

   /* revert all changes at the probing node */
   SCIP_CALL( SCIPbacktrackProbing(scip, 0) );

   return SCIP_OKAY;
}

/** gets strong branching information with previous domain propagation on column variable
 *
 *  Before calling this method, the strong branching mode must have been activated by calling SCIPstartStrongbranch();
 *  after strong branching was done for all candidate variables, the strong branching mode must be ended by
 *  SCIPendStrongbranch(). Since this method applies domain propagation before strongbranching, propagation has to be be
 *  enabled in the SCIPstartStrongbranch() call.
 *
 *  Before solving the strong branching LP, domain propagation can be performed. The number of propagation rounds
 *  can be specified by the parameter @p maxproprounds.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @warning When using this method, LP banching candidates and solution values must be copied beforehand, because
 *           they are updated w.r.t. the strong branching LP solution.
 */
SCIP_RETCODE SCIPgetVarStrongbranchWithPropagation(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get strong branching values for */
   SCIP_Real             solval,             /**< value of the variable in the current LP solution */
   SCIP_Real             lpobjval,           /**< LP objective value of the current LP solution */
   int                   itlim,              /**< iteration limit for strong branchings */
   int                   maxproprounds,      /**< maximum number of propagation rounds (-1: no limit, -2: parameter
                                              *   settings) */
   SCIP_Real*            down,               /**< stores dual bound after branching column down */
   SCIP_Real*            up,                 /**< stores dual bound after branching column up */
   SCIP_Bool*            downvalid,          /**< stores whether the returned down value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Bool*            upvalid,            /**< stores whether the returned up value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Longint*         ndomredsdown,       /**< pointer to store the number of domain reductions down, or NULL */
   SCIP_Longint*         ndomredsup,         /**< pointer to store the number of domain reductions up, or NULL */
   SCIP_Bool*            downinf,            /**< pointer to store whether the downwards branch is infeasible, or NULL */
   SCIP_Bool*            upinf,              /**< pointer to store whether the upwards branch is infeasible, or NULL */
   SCIP_Bool*            downconflict,       /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible downwards branch, or NULL */
   SCIP_Bool*            upconflict,         /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible upwards branch, or NULL */
   SCIP_Bool*            lperror,            /**< pointer to store whether an unresolved LP error occurred or the
                                              *   solving process should be stopped (e.g., due to a time limit) */
   SCIP_Real*            newlbs,             /**< array to store valid lower bounds for all active variables, or NULL */
   SCIP_Real*            newubs              /**< array to store valid upper bounds for all active variables, or NULL */
   )
{
   SCIP_COL* col;
   SCIP_VAR** vars;
   SCIP_Longint oldniters;
   SCIP_Real newub;
   SCIP_Real newlb;
   SCIP_Bool propagate;
   SCIP_Bool cutoff;
   SCIP_Bool downchild;
   SCIP_Bool firstchild;
   SCIP_Bool foundsol;
   SCIP_Bool downvalidlocal;
   SCIP_Bool upvalidlocal;
   SCIP_Bool allcolsinlp;
   SCIP_Bool enabledconflict;
   int oldnconflicts;
   int nvars;

   assert(scip != NULL);
   assert(var != NULL);
   assert(SCIPvarIsIntegral(var));
   assert(down != NULL);
   assert(up != NULL);
   assert(lperror != NULL);
   assert((newlbs != NULL) == (newubs != NULL));
   assert(SCIPinProbing(scip));
   assert(var->scip == scip);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarStrongbranchWithPropagation", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   /* check whether propagation should be performed */
   propagate = (maxproprounds != 0 && maxproprounds != -3);

   /* Check, if all existing columns are in LP.
    * If this is not the case, we may still return that the up and down dual bounds are valid, because the branching
    * rule should not apply them otherwise.
    * However, we must not set the downinf or upinf pointers to TRUE based on the dual bound, because we cannot
    * guarantee that this node can be cut off.
    */
   allcolsinlp = SCIPallColsInLP(scip);

   /* if maxproprounds is -2, change it to 0, which for the following calls means using the parameter settings */
   if( maxproprounds == -2 )
      maxproprounds = 0;

   *down = lpobjval;
   *up = lpobjval;
   if( downvalid != NULL )
      *downvalid = FALSE;
   if( upvalid != NULL )
      *upvalid = FALSE;
   if( downinf != NULL )
      *downinf = FALSE;
   if( upinf != NULL )
      *upinf = FALSE;
   if( downconflict != NULL )
      *downconflict = FALSE;
   if( upconflict != NULL )
      *upconflict = FALSE;
   if( ndomredsdown != NULL )
      *ndomredsdown = 0;
   if( ndomredsup != NULL )
      *ndomredsup = 0;

   *lperror = FALSE;

   vars = SCIPgetVars(scip);
   nvars = SCIPgetNVars(scip);

   scip->stat->lastsblpsolstats[0] = scip->stat->lastsblpsolstats[1] = SCIP_LPSOLSTAT_NOTSOLVED;

   /* check if the solving process should be aborted */
   if( SCIPsolveIsStopped(scip->set, scip->stat, FALSE) )
   {
      /* mark this as if the LP failed */
      *lperror = TRUE;
      return SCIP_OKAY;
   }

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
   {
      SCIPerrorMessage("cannot get strong branching information on non-COLUMN variable <%s>\n", SCIPvarGetName(var));
      return SCIP_INVALIDDATA;
   }

   col = SCIPvarGetCol(var);
   assert(col != NULL);

   if( !SCIPcolIsInLP(col) )
   {
      SCIPerrorMessage("cannot get strong branching information on variable <%s> not in current LP\n", SCIPvarGetName(var));
      return SCIP_INVALIDDATA;
   }

   newlb = SCIPfeasFloor(scip, solval + 1.0);
   newub = SCIPfeasCeil(scip, solval - 1.0);

   SCIPdebugMsg(scip, "strong branching on var <%s>: solval=%g, lb=%g, ub=%g\n", SCIPvarGetName(var), solval,
      SCIPvarGetLbLocal(var), SCIPvarGetUbLocal(var));

   /* the up branch is infeasible due to the branching bound change; since this means that solval is not within the
    * bounds, this should only happen if previous strong branching calls on other variables detected bound changes which
    * are valid for and were already applied at the probing root
    */
   if( newlb > SCIPvarGetUbLocal(var) + 0.5 )
   {
      *up = SCIPinfinity(scip);

      if( upinf != NULL )
         *upinf = TRUE;

      if( upvalid != NULL )
         *upvalid = TRUE;

      /* bound changes are applied in SCIPendStrongbranch(), which can be seen as a conflict constraint */
      if( upconflict != NULL )
         *upconflict = TRUE;

      SCIPcolSetStrongbranchData(col, scip->set, scip->stat, scip->lp, lpobjval, solval,
         *down, *up, FALSE, TRUE, 0LL, INT_MAX);

      /* we do not regard the down branch; its valid pointer stays set to FALSE */
      return SCIP_OKAY;
   }

   /* the down branch is infeasible due to the branching bound change; since this means that solval is not within the
    * bounds, this should only happen if previous strong branching calls on other variables detected bound changes which
    * are valid for and were already applied at the probing root
    */
   if( newub < SCIPvarGetLbLocal(var) - 0.5 )
   {
      *down = SCIPinfinity(scip);

      if( downinf != NULL )
         *downinf = TRUE;

      if( downvalid != NULL )
         *downvalid = TRUE;

      /* bound changes are applied in SCIPendStrongbranch(), which can be seen as a conflict constraint */
      if( downconflict != NULL )
         *downconflict = TRUE;

      SCIPcolSetStrongbranchData(col, scip->set, scip->stat, scip->lp, lpobjval, solval,
         *down, *up, TRUE, FALSE, 0LL, INT_MAX);

      /* we do not regard the up branch; its valid pointer stays set to FALSE */
      return SCIP_OKAY;
   }

   /* We now do strong branching by creating the two potential child nodes as probing nodes and solving them one after
    * the other. We will stop when the first child is detected infeasible, saving the effort we would need for the
    * second child. Since empirically, the up child tends to be infeasible more often, we do strongbranching first on
    * the up branch.
    */
   oldniters = scip->stat->nsbdivinglpiterations;
   firstchild = TRUE;
   cutoff = FALSE;

   /* switch conflict analysis according to usesb parameter */
   enabledconflict = scip->set->conf_enable;
   scip->set->conf_enable = (scip->set->conf_enable && scip->set->conf_usesb);

   /* @todo: decide the branch to look at first based on the cutoffs in previous calls? */
   downchild = SCIPisStrongbranchDownFirst(scip, var);

   downvalidlocal = FALSE;
   upvalidlocal = FALSE;

   do
   {
      oldnconflicts = SCIPconflictGetNConflicts(scip->conflict);

      if( downchild )
      {
         SCIP_CALL( performStrongbranchWithPropagation(scip, var, downchild, firstchild, propagate, newub, itlim, maxproprounds,
               down, &downvalidlocal, ndomredsdown, downconflict, lperror, vars, nvars, newlbs, newubs, &foundsol, &cutoff) );

         /* check whether a new solutions rendered the previous child infeasible */
         if( foundsol && !firstchild && allcolsinlp )
         {
            if( SCIPisGE(scip, *up, SCIPgetCutoffbound(scip)) )
            {
               if( upinf != NULL )
                  *upinf = TRUE;
            }
         }

         /* check for infeasibility */
         if( cutoff )
         {
            if( downinf != NULL )
               *downinf = TRUE;

            if( downconflict != NULL &&
               (SCIPvarGetLbLocal(var) > newub + 0.5 || SCIPconflictGetNConflicts(scip->conflict) > oldnconflicts) )
            {
               *downconflict = TRUE;
            }

            if( !scip->set->branch_forceall )
            {
               /* if this is the first call, we do not regard the up branch, its valid pointer is initially set to FALSE */
               break;
            }
         }
      }
      else
      {
         SCIP_CALL( performStrongbranchWithPropagation(scip, var, downchild, firstchild, propagate, newlb, itlim, maxproprounds,
               up, &upvalidlocal, ndomredsup, upconflict, lperror, vars, nvars, newlbs, newubs, &foundsol, &cutoff) );

         /* check whether a new solutions rendered the previous child infeasible */
         if( foundsol && !firstchild && allcolsinlp )
         {
            if( SCIPisGE(scip, *down, SCIPgetCutoffbound(scip)) )
            {
               if( downinf != NULL )
                  *downinf = TRUE;
            }
         }

         /* check for infeasibility */
         if( cutoff )
         {
            if( upinf != NULL )
               *upinf = TRUE;

            assert(upinf == NULL || (*upinf) == TRUE);

            if( upconflict != NULL &&
               (SCIPvarGetUbLocal(var) < newlb - 0.5 || SCIPconflictGetNConflicts(scip->conflict) > oldnconflicts) )
            {
               *upconflict = TRUE;
            }

            if( !scip->set->branch_forceall )
            {
               /* if this is the first call, we do not regard the down branch, its valid pointer is initially set to FALSE */
               break;
            }
         }
      }

      downchild = !downchild;
      firstchild = !firstchild;
   }
   while( !firstchild );


   /* set strong branching information in column */
   if( *lperror )
   {
      SCIPcolInvalidateStrongbranchData(col, scip->set, scip->stat, scip->lp);
   }
   else
   {
      SCIPcolSetStrongbranchData(col, scip->set, scip->stat, scip->lp, lpobjval, solval,
         *down, *up, downvalidlocal, upvalidlocal, scip->stat->nsbdivinglpiterations - oldniters, itlim);
   }

   if( downvalid != NULL )
      *downvalid = downvalidlocal;
   if( upvalid != NULL )
      *upvalid = upvalidlocal;

   scip->set->conf_enable = enabledconflict;

   return SCIP_OKAY;
}

/** gets strong branching information on column variable x with integral LP solution value (val); that is, the down branch
 *  is (val -1.0) and the up brach ins (val +1.0)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note If the integral LP solution value is the lower or upper bound of the variable, the corresponding branch will be
 *        marked as infeasible. That is, the valid pointer and the infeasible pointer are set to TRUE.
 */
SCIP_RETCODE SCIPgetVarStrongbranchInt(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get strong branching values for */
   int                   itlim,              /**< iteration limit for strong branchings */
   SCIP_Real*            down,               /**< stores dual bound after branching column down */
   SCIP_Real*            up,                 /**< stores dual bound after branching column up */
   SCIP_Bool*            downvalid,          /**< stores whether the returned down value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Bool*            upvalid,            /**< stores whether the returned up value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Bool*            downinf,            /**< pointer to store whether the downwards branch is infeasible, or NULL */
   SCIP_Bool*            upinf,              /**< pointer to store whether the upwards branch is infeasible, or NULL */
   SCIP_Bool*            downconflict,       /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible downwards branch, or NULL */
   SCIP_Bool*            upconflict,         /**< pointer to store whether a conflict constraint was created for an
                                              *   infeasible upwards branch, or NULL */
   SCIP_Bool*            lperror             /**< pointer to store whether an unresolved LP error occurred or the
                                              *   solving process should be stopped (e.g., due to a time limit) */
   )
{
   SCIP_COL* col;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarStrongbranchInt", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert(lperror != NULL);
   assert(var->scip == scip);

   if( downvalid != NULL )
      *downvalid = FALSE;
   if( upvalid != NULL )
      *upvalid = FALSE;
   if( downinf != NULL )
      *downinf = FALSE;
   if( upinf != NULL )
      *upinf = FALSE;
   if( downconflict != NULL )
      *downconflict = FALSE;
   if( upconflict != NULL )
      *upconflict = FALSE;

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
   {
      SCIPerrorMessage("cannot get strong branching information on non-COLUMN variable <%s>\n", SCIPvarGetName(var));
      return SCIP_INVALIDDATA;
   }

   col = SCIPvarGetCol(var);
   assert(col != NULL);

   if( !SCIPcolIsInLP(col) )
   {
      SCIPerrorMessage("cannot get strong branching information on variable <%s> not in current LP\n", SCIPvarGetName(var));
      return SCIP_INVALIDDATA;
   }

   /* check if the solving process should be aborted */
   if( SCIPsolveIsStopped(scip->set, scip->stat, FALSE) )
   {
      /* mark this as if the LP failed */
      *lperror = TRUE;
      return SCIP_OKAY;
   }

   /* call strong branching for column */
   SCIP_CALL( SCIPcolGetStrongbranch(col, TRUE, scip->set, scip->stat, scip->transprob, scip->lp, itlim,
         down, up, downvalid, upvalid, lperror) );

   /* check, if the branchings are infeasible; in exact solving mode, we cannot trust the strong branching enough to
    * declare the sub nodes infeasible
    */
   if( !(*lperror) && SCIPprobAllColsInLP(scip->transprob, scip->set, scip->lp) && !scip->set->misc_exactsolve )
   {
      SCIP_CALL( analyzeStrongbranch(scip, var, downinf, upinf, downconflict, upconflict) );
   }

   return SCIP_OKAY;
}

/** gets strong branching information on column variables with fractional values
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetVarsStrongbranchesFrac(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variables to get strong branching values for */
   int                   nvars,              /**< number of variables */
   int                   itlim,              /**< iteration limit for strong branchings */
   SCIP_Real*            down,               /**< stores dual bounds after branching variables down */
   SCIP_Real*            up,                 /**< stores dual bounds after branching variables up */
   SCIP_Bool*            downvalid,          /**< stores whether the returned down values are valid dual bounds, or NULL;
                                              *   otherwise, they can only be used as an estimate value */
   SCIP_Bool*            upvalid,            /**< stores whether the returned up values are valid dual bounds, or NULL;
                                              *   otherwise, they can only be used as an estimate value */
   SCIP_Bool*            downinf,            /**< array to store whether the downward branches are infeasible, or NULL */
   SCIP_Bool*            upinf,              /**< array to store whether the upward branches are infeasible, or NULL */
   SCIP_Bool*            downconflict,       /**< array to store whether conflict constraints were created for
                                              *   infeasible downward branches, or NULL */
   SCIP_Bool*            upconflict,         /**< array to store whether conflict constraints were created for
                                              *   infeasible upward branches, or NULL */
   SCIP_Bool*            lperror             /**< pointer to store whether an unresolved LP error occurred or the
                                              *   solving process should be stopped (e.g., due to a time limit) */
   )
{
   SCIP_COL** cols;
   int j;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarsStrongbranchesFrac", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert( lperror != NULL );
   assert( vars != NULL );

   /* set up data */
   cols = NULL;
   SCIP_CALL( SCIPallocBufferArray(scip, &cols, nvars) );
   assert(cols != NULL);
   for( j = 0; j < nvars; ++j )
   {
      SCIP_VAR* var;
      SCIP_COL* col;

      if( downvalid != NULL )
         downvalid[j] = FALSE;
      if( upvalid != NULL )
         upvalid[j] = FALSE;
      if( downinf != NULL )
         downinf[j] = FALSE;
      if( upinf != NULL )
         upinf[j] = FALSE;
      if( downconflict != NULL )
         downconflict[j] = FALSE;
      if( upconflict != NULL )
         upconflict[j] = FALSE;

      var = vars[j];
      assert( var != NULL );
      if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
      {
         SCIPerrorMessage("cannot get strong branching information on non-COLUMN variable <%s>\n", SCIPvarGetName(var));
         SCIPfreeBufferArray(scip, &cols);
         return SCIP_INVALIDDATA;
      }

      col = SCIPvarGetCol(var);
      assert(col != NULL);
      cols[j] = col;

      if( !SCIPcolIsInLP(col) )
      {
         SCIPerrorMessage("cannot get strong branching information on variable <%s> not in current LP\n", SCIPvarGetName(var));
         SCIPfreeBufferArray(scip, &cols);
         return SCIP_INVALIDDATA;
      }
   }

   /* check if the solving process should be aborted */
   if( SCIPsolveIsStopped(scip->set, scip->stat, FALSE) )
   {
      /* mark this as if the LP failed */
      *lperror = TRUE;
   }
   else
   {
      /* call strong branching for columns with fractional value */
      SCIP_CALL( SCIPcolGetStrongbranches(cols, nvars, FALSE, scip->set, scip->stat, scip->transprob, scip->lp, itlim,
            down, up, downvalid, upvalid, lperror) );

      /* check, if the branchings are infeasible; in exact solving mode, we cannot trust the strong branching enough to
       * declare the sub nodes infeasible
       */
      if( !(*lperror) && SCIPprobAllColsInLP(scip->transprob, scip->set, scip->lp) && !scip->set->misc_exactsolve )
      {
         for( j = 0; j < nvars; ++j )
         {
            SCIP_CALL( analyzeStrongbranch(scip, vars[j], (downinf != NULL) ? (&(downinf[j])) : NULL,
                  (upinf != NULL) ? (&(upinf[j])) : NULL, (downconflict != NULL) ? (&(downconflict[j])) : NULL,
                  (upconflict != NULL) ? (&(upconflict[j])) : NULL) );
         }
      }
   }
   SCIPfreeBufferArray(scip, &cols);

   return SCIP_OKAY;
}

/** gets strong branching information on column variables with integral values
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetVarsStrongbranchesInt(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variables to get strong branching values for */
   int                   nvars,              /**< number of variables */
   int                   itlim,              /**< iteration limit for strong branchings */
   SCIP_Real*            down,               /**< stores dual bounds after branching variables down */
   SCIP_Real*            up,                 /**< stores dual bounds after branching variables up */
   SCIP_Bool*            downvalid,          /**< stores whether the returned down values are valid dual bounds, or NULL;
                                              *   otherwise, they can only be used as an estimate value */
   SCIP_Bool*            upvalid,            /**< stores whether the returned up values are valid dual bounds, or NULL;
                                              *   otherwise, they can only be used as an estimate value */
   SCIP_Bool*            downinf,            /**< array to store whether the downward branches are infeasible, or NULL */
   SCIP_Bool*            upinf,              /**< array to store whether the upward branches are infeasible, or NULL */
   SCIP_Bool*            downconflict,       /**< array to store whether conflict constraints were created for
                                              *   infeasible downward branches, or NULL */
   SCIP_Bool*            upconflict,         /**< array to store whether conflict constraints were created for
                                              *   infeasible upward branches, or NULL */
   SCIP_Bool*            lperror             /**< pointer to store whether an unresolved LP error occurred or the
                                              *   solving process should be stopped (e.g., due to a time limit) */
   )
{
   SCIP_COL** cols;
   int j;

   assert(lperror != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarsStrongbranchesInt", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert( vars != NULL );

   /* set up data */
   cols = NULL;
   SCIP_CALL( SCIPallocBufferArray(scip, &cols, nvars) );
   assert(cols != NULL);
   for( j = 0; j < nvars; ++j )
   {
      SCIP_VAR* var;
      SCIP_COL* col;

      if( downvalid != NULL )
         downvalid[j] = FALSE;
      if( upvalid != NULL )
         upvalid[j] = FALSE;
      if( downinf != NULL )
         downinf[j] = FALSE;
      if( upinf != NULL )
         upinf[j] = FALSE;
      if( downconflict != NULL )
         downconflict[j] = FALSE;
      if( upconflict != NULL )
         upconflict[j] = FALSE;

      var = vars[j];
      assert( var != NULL );
      if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
      {
         SCIPerrorMessage("cannot get strong branching information on non-COLUMN variable <%s>\n", SCIPvarGetName(var));
         SCIPfreeBufferArray(scip, &cols);
         return SCIP_INVALIDDATA;
      }

      col = SCIPvarGetCol(var);
      assert(col != NULL);
      cols[j] = col;

      if( !SCIPcolIsInLP(col) )
      {
         SCIPerrorMessage("cannot get strong branching information on variable <%s> not in current LP\n", SCIPvarGetName(var));
         SCIPfreeBufferArray(scip, &cols);
         return SCIP_INVALIDDATA;
      }
   }

   /* check if the solving process should be aborted */
   if( SCIPsolveIsStopped(scip->set, scip->stat, FALSE) )
   {
      /* mark this as if the LP failed */
      *lperror = TRUE;
   }
   else
   {
      /* call strong branching for columns */
      SCIP_CALL( SCIPcolGetStrongbranches(cols, nvars, TRUE, scip->set, scip->stat, scip->transprob, scip->lp, itlim,
            down, up, downvalid, upvalid, lperror) );

      /* check, if the branchings are infeasible; in exact solving mode, we cannot trust the strong branching enough to
       * declare the sub nodes infeasible
       */
      if( !(*lperror) && SCIPprobAllColsInLP(scip->transprob, scip->set, scip->lp) && !scip->set->misc_exactsolve )
      {
         for( j = 0; j < nvars; ++j )
         {
            SCIP_CALL( analyzeStrongbranch(scip, vars[j], (downinf != NULL) ? (&(downinf[j])) : NULL,
                  (upinf != NULL) ? (&(upinf[j])) : NULL, (downconflict != NULL) ? (&(downconflict[j])) : NULL,
                  (upconflict != NULL) ? (&(upconflict[j])) : NULL) );

         }
      }
   }
   SCIPfreeBufferArray(scip, &cols);

   return SCIP_OKAY;
}

/** get LP solution status of last strong branching call (currently only works for strong branching with propagation) */
SCIP_LPSOLSTAT SCIPgetLastStrongbranchLPSolStat(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        branchdir           /**< branching direction for which LP solution status is requested */
   )
{
   assert(NULL != scip);
   assert(branchdir == SCIP_BRANCHDIR_DOWNWARDS || branchdir == SCIP_BRANCHDIR_UPWARDS);

   return scip->stat->lastsblpsolstats[branchdir == SCIP_BRANCHDIR_DOWNWARDS ? 0 : 1];
}

/** gets strong branching information on COLUMN variable of the last SCIPgetVarStrongbranch() call;
 *  returns values of SCIP_INVALID, if strong branching was not yet called on the given variable;
 *  keep in mind, that the returned old values may have nothing to do with the current LP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPgetVarStrongbranchLast(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to get last strong branching values for */
   SCIP_Real*            down,               /**< stores dual bound after branching column down */
   SCIP_Real*            up,                 /**< stores dual bound after branching column up */
   SCIP_Bool*            downvalid,          /**< stores whether the returned down value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Bool*            upvalid,            /**< stores whether the returned up value is a valid dual bound, or NULL;
                                              *   otherwise, it can only be used as an estimate value */
   SCIP_Real*            solval,             /**< stores LP solution value of variable at the last strong branching call, or NULL */
   SCIP_Real*            lpobjval            /**< stores LP objective value at last strong branching call, or NULL */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarStrongbranchLast", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
   {
      SCIPerrorMessage("cannot get strong branching information on non-COLUMN variable\n");
      return SCIP_INVALIDDATA;
   }

   SCIPcolGetStrongbranchLast(SCIPvarGetCol(var), down, up, downvalid, upvalid, solval, lpobjval);

   return SCIP_OKAY;
}

/** sets strong branching information for a column variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPsetVarStrongbranchData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to set last strong branching values for */
   SCIP_Real             lpobjval,           /**< objective value of the current LP */
   SCIP_Real             primsol,            /**< primal solution value of the column in the current LP */
   SCIP_Real             down,               /**< dual bound after branching column down */
   SCIP_Real             up,                 /**< dual bound after branching column up */
   SCIP_Bool             downvalid,          /**< is the returned down value a valid dual bound? */
   SCIP_Bool             upvalid,            /**< is the returned up value a valid dual bound? */
   SCIP_Longint          iter,               /**< total number of strong branching iterations */
   int                   itlim               /**< iteration limit applied to the strong branching call */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetVarStrongbranchData", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
   {
      SCIPerrorMessage("cannot set strong branching information on non-COLUMN variable\n");
      return SCIP_INVALIDDATA;
   }

   SCIPcolSetStrongbranchData(SCIPvarGetCol(var), scip->set, scip->stat, scip->lp, lpobjval, primsol,
      down, up, downvalid, upvalid, iter, itlim);

   return SCIP_OKAY;
}

/** rounds the current solution and tries it afterwards; if feasible, adds it to storage
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPtryStrongbranchLPSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool*            foundsol,           /**< stores whether solution was feasible and good enough to keep */
   SCIP_Bool*            cutoff              /**< stores whether solution was cutoff due to exceeding the cutoffbound */
   )
{
   assert(scip != NULL);
   assert(foundsol != NULL);
   assert(cutoff != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetVarStrongbranchData", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( scip->set->branch_checksbsol )
   {
      SCIP_SOL* sol;
      SCIP_Bool rounded = TRUE;
      SCIP_Real value = SCIPgetLPObjval(scip);
      SCIP_Longint oldnbestsolsfound = scip->primal->nbestsolsfound;

      /* start clock for strong branching solutions */
      SCIPclockStart(scip->stat->sbsoltime, scip->set);

      SCIP_CALL( SCIPcreateLPSol(scip, &sol, NULL) );

      /* try to round the strong branching solution */
      if( scip->set->branch_roundsbsol )
      {
         SCIP_CALL( SCIProundSol(scip, sol, &rounded) );
      }

      /* check the solution for feasibility if rounding worked well (or was not tried) */
      if( rounded )
      {
         SCIP_CALL( SCIPtrySolFree(scip, &sol, FALSE, FALSE, FALSE, TRUE, FALSE, foundsol) );
      }
      else
      {
         SCIP_CALL( SCIPfreeSol(scip, &sol) );
      }

      if( *foundsol )
      {
         SCIPdebugMsg(scip, "found new solution in strong branching\n");

         scip->stat->nsbsolsfound++;

         if( scip->primal->nbestsolsfound != oldnbestsolsfound )
         {
            scip->stat->nsbbestsolsfound++;
         }

         if( SCIPisGE(scip, value, SCIPgetCutoffbound(scip)) )
            *cutoff = TRUE;
      }

      /* stop clock for strong branching solutions */
      SCIPclockStop(scip->stat->sbsoltime, scip->set);
   }
   return SCIP_OKAY;
}


/** gets node number of the last node in current branch and bound run, where strong branching was used on the
 *  given variable, or -1 if strong branching was never applied to the variable in current run
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetVarStrongbranchNode(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get last strong branching node for */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarStrongbranchNode", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   assert( var->scip == scip );

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
      return -1;

   return SCIPcolGetStrongbranchNode(SCIPvarGetCol(var));
}

/** if strong branching was already applied on the variable at the current node, returns the number of LPs solved after
 *  the LP where the strong branching on this variable was applied;
 *  if strong branching was not yet applied on the variable at the current node, returns INT_MAX
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetVarStrongbranchLPAge(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get strong branching LP age for */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarStrongbranchLPAge", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   assert( var->scip == scip );

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
      return SCIP_LONGINT_MAX;

   return SCIPcolGetStrongbranchLPAge(SCIPvarGetCol(var), scip->stat);
}

/** gets number of times, strong branching was applied in current run on the given variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetVarNStrongbranchs(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get last strong branching node for */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarNStrongbranchs", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   assert( var->scip == scip );

   if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_COLUMN )
      return 0;

   return SCIPcolGetNStrongbranchs(SCIPvarGetCol(var));
}

/** adds given values to lock numbers of type @p locktype of variable for rounding
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPaddVarLocksType(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_LOCKTYPE         locktype,           /**< type of the variable locks */
   int                   nlocksdown,         /**< modification in number of rounding down locks */
   int                   nlocksup            /**< modification in number of rounding up locks */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarLocksType", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE) );

   assert( var->scip == scip );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      /*lint -fallthrough*/
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_EXITSOLVE:
   case SCIP_STAGE_FREETRANS:
      SCIP_CALL( SCIPvarAddLocks(var, scip->mem->probmem, scip->set, scip->eventqueue, locktype, nlocksdown, nlocksup) );
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** adds given values to lock numbers of variable for rounding
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note This method will always add variable locks of type model
 *
 *  @note It is recommented to use SCIPaddVarLocksType()
 */
SCIP_RETCODE SCIPaddVarLocks(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   int                   nlocksdown,         /**< modification in number of rounding down locks */
   int                   nlocksup            /**< modification in number of rounding up locks */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarLocks", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE) );

   SCIP_CALL( SCIPaddVarLocksType(scip, var, SCIP_LOCKTYPE_MODEL, nlocksdown, nlocksup) );

   return SCIP_OKAY;
}

/** add locks of variable with respect to the lock status of the constraint and its negation;
 *  this method should be called whenever the lock status of a variable in a constraint changes, for example if
 *  the coefficient of the variable changed its sign or if the left or right hand sides of the constraint were
 *  added or removed
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPlockVarCons(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_Bool             lockdown,           /**< should the rounding be locked in downwards direction? */
   SCIP_Bool             lockup              /**< should the rounding be locked in upwards direction? */
   )
{
   int nlocksdown[NLOCKTYPES];
   int nlocksup[NLOCKTYPES];
   int i;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPlockVarCons", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE) );

   assert( var->scip == scip );

   for( i = 0; i < NLOCKTYPES; i++ )
   {
      nlocksdown[i] = 0;
      nlocksup[i] = 0;

      if( SCIPconsIsLockedTypePos(cons, (SCIP_LOCKTYPE) i) )
      {
         if( lockdown )
            ++nlocksdown[i];
         if( lockup )
            ++nlocksup[i];
      }
      if( SCIPconsIsLockedTypeNeg(cons, (SCIP_LOCKTYPE) i) )
      {
         if( lockdown )
            ++nlocksup[i];
         if( lockup )
            ++nlocksdown[i];
      }
   }

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      /*lint -fallthrough*/
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_EXITSOLVE:
   case SCIP_STAGE_FREETRANS:
      for( i = 0; i < NLOCKTYPES; i++ )
      {
         if( nlocksdown[i] == 0 && nlocksup[i] == 0 )
            continue;

         SCIP_CALL( SCIPvarAddLocks(var, scip->mem->probmem, scip->set, scip->eventqueue, (SCIP_LOCKTYPE) i, nlocksdown[i], nlocksup[i]) );
      }
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** remove locks of type @p locktype of variable with respect to the lock status of the constraint and its negation;
 *  this method should be called whenever the lock status of a variable in a constraint changes, for example if
 *  the coefficient of the variable changed its sign or if the left or right hand sides of the constraint were
 *  added or removed
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPunlockVarCons(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_CONS*            cons,               /**< constraint */
   SCIP_Bool             lockdown,           /**< should the rounding be unlocked in downwards direction? */
   SCIP_Bool             lockup              /**< should the rounding be unlocked in upwards direction? */
   )
{
   int nlocksdown[NLOCKTYPES];
   int nlocksup[NLOCKTYPES];
   int i;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPunlockVarCons", FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE) );

   assert( var->scip == scip );

   for( i = 0; i < NLOCKTYPES; i++ )
   {
      nlocksdown[i] = 0;
      nlocksup[i] = 0;

      if( SCIPconsIsLockedTypePos(cons, (SCIP_LOCKTYPE) i) )
      {
         if( lockdown )
            ++nlocksdown[i];
         if( lockup )
            ++nlocksup[i];
      }
      if( SCIPconsIsLockedTypeNeg(cons, (SCIP_LOCKTYPE) i) )
      {
         if( lockdown )
            ++nlocksup[i];
         if( lockup )
            ++nlocksdown[i];
      }
   }
   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      /*lint -fallthrough*/
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_EXITSOLVE:
   case SCIP_STAGE_FREETRANS:
      for( i = 0; i < NLOCKTYPES; i++ )
      {
         if( nlocksdown[i] == 0 && nlocksup[i] == 0 )
            continue;

         SCIP_CALL( SCIPvarAddLocks(var, scip->mem->probmem, scip->set, scip->eventqueue, (SCIP_LOCKTYPE)  i, -nlocksdown[i], -nlocksup[i]) );
      }
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** changes variable's objective value
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 */
SCIP_RETCODE SCIPchgVarObj(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the objective value for */
   SCIP_Real             newobj              /**< new objective value */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarObj", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   /* forbid infinite objective values */
   if( SCIPisInfinity(scip, REALABS(newobj)) )
   {
      SCIPerrorMessage("invalid objective value: objective value is infinite\n");
      return SCIP_INVALIDDATA;
   }

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgObj(var, scip->mem->probmem, scip->set, scip->origprob, scip->primal, scip->lp, scip->eventqueue, newobj) );
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_PRESOLVED:
      SCIP_CALL( SCIPvarChgObj(var, scip->mem->probmem, scip->set,  scip->transprob, scip->primal, scip->lp, scip->eventqueue, newobj) );
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** adds value to variable's objective value
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 */
SCIP_RETCODE SCIPaddVarObj(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the objective value for */
   SCIP_Real             addobj              /**< additional objective value */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarObj", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarAddObj(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob, scip->primal,
            scip->tree, scip->reopt, scip->lp, scip->eventqueue, addobj) );
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
      SCIP_CALL( SCIPvarAddObj(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob, scip->primal,
            scip->tree, scip->reopt, scip->lp, scip->eventqueue, addobj) );
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** returns the adjusted (i.e. rounded, if the given variable is of integral type) lower bound value;
 *  does not change the bounds of the variable
 *
 *  @return adjusted lower bound for the given variable; the bound of the variable is not changed
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Real SCIPadjustedVarLb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to adjust the bound for */
   SCIP_Real             lb                  /**< lower bound value to adjust */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPadjustedVarLb", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIPvarAdjustLb(var, scip->set, &lb);

   return lb;
}

/** returns the adjusted (i.e. rounded, if the given variable is of integral type) upper bound value;
 *  does not change the bounds of the variable
 *
 *  @return adjusted upper bound for the given variable; the bound of the variable is not changed
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Real SCIPadjustedVarUb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to adjust the bound for */
   SCIP_Real             ub                  /**< upper bound value to adjust */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPadjustedVarUb", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIPvarAdjustUb(var, scip->set, &ub);

   return ub;
}

/** depending on SCIP's stage, changes lower bound of variable in the problem, in preprocessing, or in current node;
 *  if possible, adjusts bound to integral value; doesn't store any inference information in the bound change, such
 *  that in conflict analysis, this change is treated like a branching decision
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPchgVarLb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound            /**< new value for bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarLb", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPvarAdjustLb(var, scip->set, &newbound);

   /* ignore tightenings of lower bounds to +infinity during solving process */
   if( SCIPisInfinity(scip, newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore lower bound tightening for %s from %e to +infinity\n", SCIPvarGetName(var),
         SCIPvarGetLbLocal(var));
#endif
      return SCIP_OKAY;
   }

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgLbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgLbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_PRESOLVED:
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
               var, newbound, SCIP_BOUNDTYPE_LOWER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_Bool infeasible;

            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, &infeasible) );
            assert(!infeasible);
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
            scip->cliquetable, var, newbound,
            SCIP_BOUNDTYPE_LOWER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** depending on SCIP's stage, changes upper bound of variable in the problem, in preprocessing, or in current node;
 *  if possible, adjusts bound to integral value; doesn't store any inference information in the bound change, such
 *  that in conflict analysis, this change is treated like a branching decision
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPchgVarUb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound            /**< new value for bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarUb", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPvarAdjustUb(var, scip->set, &newbound);

   /* ignore tightenings of upper bounds to -infinity during solving process */
   if( SCIPisInfinity(scip, -newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore upper bound tightening for %s from %e to -infinity\n", SCIPvarGetName(var),
         SCIPvarGetUbLocal(var));
#endif
      return SCIP_OKAY;
   }

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgUbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgUbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_PRESOLVED:
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
               scip->cliquetable, var, newbound, SCIP_BOUNDTYPE_UPPER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_Bool infeasible;

            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, &infeasible) );
            assert(!infeasible);
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
            scip->cliquetable, var, newbound, SCIP_BOUNDTYPE_UPPER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** changes lower bound of variable in the given node; if possible, adjust bound to integral value; doesn't store any
 *  inference information in the bound change, such that in conflict analysis, this change is treated like a branching
 *  decision
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPchgVarLbNode(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node,               /**< node to change bound at, or NULL for current node */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound            /**< new value for bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarLbNode", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( node == NULL )
   {
      SCIP_CALL( SCIPchgVarLb(scip, var, newbound) );
   }
   else
   {
      SCIPvarAdjustLb(var, scip->set, &newbound);

      /* ignore tightenings of lower bounds to +infinity during solving process */
      if( SCIPisInfinity(scip, newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
      {
#ifndef NDEBUG
         SCIPwarningMessage(scip, "ignore lower bound tightening for %s from %e to +infinity\n", SCIPvarGetName(var),
            SCIPvarGetLbLocal(var));
#endif
         return SCIP_OKAY;
      }

      SCIP_CALL( SCIPnodeAddBoundchg(node, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
            scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
            SCIP_BOUNDTYPE_LOWER, FALSE) );
   }

   return SCIP_OKAY;
}

/** changes upper bound of variable in the given node; if possible, adjust bound to integral value; doesn't store any
 *  inference information in the bound change, such that in conflict analysis, this change is treated like a branching
 *  decision
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPchgVarUbNode(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node,               /**< node to change bound at, or NULL for current node */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound            /**< new value for bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarUbNode", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( node == NULL )
   {
      SCIP_CALL( SCIPchgVarUb(scip, var, newbound) );
   }
   else
   {
      SCIPvarAdjustUb(var, scip->set, &newbound);

      /* ignore tightenings of upper bounds to -infinity during solving process */
      if( SCIPisInfinity(scip, -newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
      {
#ifndef NDEBUG
         SCIPwarningMessage(scip, "ignore upper bound tightening for %s from %e to -infinity\n", SCIPvarGetName(var),
            SCIPvarGetUbLocal(var));
#endif
         return SCIP_OKAY;
      }

      SCIP_CALL( SCIPnodeAddBoundchg(node, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
            scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
            SCIP_BOUNDTYPE_UPPER, FALSE) );
   }

   return SCIP_OKAY;
}

/** changes global lower bound of variable; if possible, adjust bound to integral value; also tightens the local bound,
 *  if the global bound is better than the local bound
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPchgVarLbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound            /**< new value for bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarLbGlobal", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPvarAdjustLb(var, scip->set, &newbound);

   /* ignore tightenings of lower bounds to +infinity during solving process */
   if( SCIPisInfinity(scip, newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore lower bound tightening for %s from %e to +infinity\n", SCIPvarGetName(var),
         SCIPvarGetLbLocal(var));
#endif
      return SCIP_OKAY;
   }

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgLbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgLbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_TRANSFORMING:
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_LOWER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_Bool infeasible;

            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, &infeasible) );
            assert(!infeasible);
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
            scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
            SCIP_BOUNDTYPE_LOWER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** changes global upper bound of variable; if possible, adjust bound to integral value; also tightens the local bound,
 *  if the global bound is better than the local bound
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPchgVarUbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound            /**< new value for bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarUbGlobal", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPvarAdjustUb(var, scip->set, &newbound);

   /* ignore tightenings of upper bounds to -infinity during solving process */
   if( SCIPisInfinity(scip, -newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore upper bound tightening for %s from %e to -infinity\n", SCIPvarGetName(var),
         SCIPvarGetUbLocal(var));
#endif
      return SCIP_OKAY;
   }

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgUbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgUbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_TRANSFORMING:
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_UPPER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_Bool infeasible;

            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, &infeasible) );
            assert(!infeasible);
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
            scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
            SCIP_BOUNDTYPE_UPPER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** changes lazy lower bound of the variable, this is only possible if the variable is not in the LP yet
 *
 *  lazy bounds are bounds, that are enforced by constraints and the objective function; hence, these bounds do not need
 *  to be put into the LP explicitly.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note lazy bounds are useful for branch-and-price since the corresponding variable bounds are not part of the LP
 */
SCIP_RETCODE SCIPchgVarLbLazy(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             lazylb              /**< the lazy lower bound to be set */
   )
{
   assert(scip != NULL);
   assert(var != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarLbLazy", FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarChgLbLazy(var, scip->set, lazylb) );

   return SCIP_OKAY;
}

/** changes lazy upper bound of the variable, this is only possible if the variable is not in the LP yet
 *
 *  lazy bounds are bounds, that are enforced by constraints and the objective function; hence, these bounds do not need
 *  to be put into the LP explicitly.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note lazy bounds are useful for branch-and-price since the corresponding variable bounds are not part of the LP
 */
SCIP_RETCODE SCIPchgVarUbLazy(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             lazyub              /**< the lazy lower bound to be set */
   )
{
   assert(scip != NULL);
   assert(var != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarUbLazy", FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarChgUbLazy(var, scip->set, lazyub) );

   return SCIP_OKAY;
}

/** changes lower bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current bound; if possible, adjusts bound to integral value;
 *  doesn't store any inference information in the bound change, such that in conflict analysis, this change
 *  is treated like a branching decision
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPtightenVarLb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the new domain is empty */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);
   /** @todo if needed provide pending local/global bound changes that will be flushed after leaving diving mode (as in struct_tree.h) */
   assert(!SCIPinDive(scip));

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtightenVarLb", FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustLb(var, scip->set, &newbound);

   /* ignore tightenings of lower bounds to +infinity during solving process */
   if( SCIPisInfinity(scip, newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore lower bound tightening for %s from %e to +infinity\n", SCIPvarGetName(var),
         SCIPvarGetLbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPcomputeVarLbLocal(scip, var);
   ub = SCIPcomputeVarUbLocal(scip, var);
   assert(SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasGT(scip->set, newbound, ub) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MIN(newbound, ub);

   if( (force && SCIPsetIsLE(scip->set, newbound, lb)) || (!force && !SCIPsetIsLbBetter(scip->set, newbound, lb, ub)) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgLbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgLbOriginal(var, scip->set, newbound) );
      break;
   case SCIP_STAGE_TRANSFORMED:
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;
   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_LOWER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
            var, newbound, SCIP_BOUNDTYPE_LOWER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** changes upper bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current bound; if possible, adjusts bound to integral value;
 *  doesn't store any inference information in the bound change, such that in conflict analysis, this change
 *  is treated like a branching decision
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPtightenVarUb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the new domain is empty */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);
   /** @todo if needed provide pending local/global bound changes that will be flushed after leaving diving mode (as in struct_tree.h) */
   assert(!SCIPinDive(scip));

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtightenVarUb", FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustUb(var, scip->set, &newbound);

   /* ignore tightenings of upper bounds to -infinity during solving process */
   if( SCIPisInfinity(scip, -newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore upper bound tightening for %s from %e to -infinity\n", SCIPvarGetName(var),
         SCIPvarGetUbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPcomputeVarLbLocal(scip, var);
   ub = SCIPcomputeVarUbLocal(scip, var);
   assert(SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasLT(scip->set, newbound, lb) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MAX(newbound, lb);

   if( (force && SCIPsetIsGE(scip->set, newbound, ub)) || (!force && !SCIPsetIsUbBetter(scip->set, newbound, lb, ub)) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgUbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgUbOriginal(var, scip->set, newbound) );
      break;
   case SCIP_STAGE_TRANSFORMED:
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;
   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_UPPER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
            scip->cliquetable, var, newbound, SCIP_BOUNDTYPE_UPPER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** fixes variable in preprocessing or in the current node, if the new bound is tighter (w.r.t. bound strengthening
 *  epsilon) than the current bound; if possible, adjusts bound to integral value; the given inference constraint is
 *  stored, such that the conflict analysis is able to find out the reason for the deduction of the bound change
 *
 *  @note In presolving stage when not in probing mode the variable will be fixed directly, otherwise this method
 *        changes first the lowerbound by calling SCIPinferVarLbCons and second the upperbound by calling
 *        SCIPinferVarUbCons
 *
 *  @note If SCIP is in presolving stage, it can happen that the internal variable array (which get be accessed via
 *        SCIPgetVars()) gets resorted.
 *
 *  @note During presolving, an integer variable which bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPinferVarFixCons(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             fixedval,           /**< new value for fixation */
   SCIP_CONS*            infercons,          /**< constraint that deduced the bound change */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the bound change is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferVarFixCons", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( tightened != NULL )
      *tightened = FALSE;

   /* in presolving case we take the shortcut to directly fix the variables */
   if( SCIPgetStage(scip) == SCIP_STAGE_PRESOLVING && SCIPtreeGetCurrentDepth(scip->tree) == 0 )
   {
      SCIP_Bool fixed;

      SCIP_CALL( SCIPvarFix(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
            scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
            fixedval, infeasible, &fixed) );

      if( tightened != NULL )
	 *tightened = fixed;
   }
   /* otherwise we use the lb and ub methods */
   else
   {
      SCIP_Bool lbtightened;

      SCIP_CALL( SCIPinferVarLbCons(scip, var, fixedval, infercons, inferinfo, force, infeasible, &lbtightened) );

      if( ! (*infeasible) )
      {
	 SCIP_CALL( SCIPinferVarUbCons(scip, var, fixedval, infercons, inferinfo, force, infeasible, tightened) );

	 if( tightened != NULL )
	    *tightened |= lbtightened;
      }
   }

   return SCIP_OKAY;
}

/** changes lower bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current bound; if possible, adjusts bound to integral value;
 *  the given inference constraint is stored, such that the conflict analysis is able to find out the reason
 *  for the deduction of the bound change
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPinferVarLbCons(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_CONS*            infercons,          /**< constraint that deduced the bound change */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the bound change is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferVarLbCons", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustLb(var, scip->set, &newbound);

   /* ignore tightenings of lower bounds to +infinity during solving process */
   if( SCIPisInfinity(scip, newbound)  && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore lower bound tightening for %s from %e to +infinity\n", SCIPvarGetName(var),
         SCIPvarGetLbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPvarGetLbLocal(var);
   ub = SCIPvarGetUbLocal(var);
   assert(SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasGT(scip->set, newbound, ub) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MIN(newbound, ub);

   if( (force && SCIPsetIsLE(scip->set, newbound, lb)) || (!force && !SCIPsetIsLbBetter(scip->set, newbound, lb, ub)) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgLbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgLbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_LOWER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
            scip->cliquetable, var, newbound, SCIP_BOUNDTYPE_LOWER, infercons, NULL, inferinfo, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** changes upper bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current bound; if possible, adjusts bound to integral value;
 *  the given inference constraint is stored, such that the conflict analysis is able to find out the reason
 *  for the deduction of the bound change
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPinferVarUbCons(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_CONS*            infercons,          /**< constraint that deduced the bound change */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the bound change is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferVarUbCons", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustUb(var, scip->set, &newbound);

   /* ignore tightenings of upper bounds to -infinity during solving process */
   if( SCIPisInfinity(scip, -newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore upper bound tightening for %s from %e to -infinity\n", SCIPvarGetName(var),
         SCIPvarGetUbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPvarGetLbLocal(var);
   ub = SCIPvarGetUbLocal(var);
   assert(SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasLT(scip->set, newbound, lb) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MAX(newbound, lb);

   if( (force && SCIPsetIsGE(scip->set, newbound, ub)) || (!force && !SCIPsetIsUbBetter(scip->set, newbound, lb, ub)) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgUbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgUbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_UPPER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
            scip->cliquetable, var, newbound, SCIP_BOUNDTYPE_UPPER, infercons, NULL, inferinfo, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** depending on SCIP's stage, fixes binary variable in the problem, in preprocessing, or in current node;
 *  the given inference constraint is stored, such that the conflict analysis is able to find out the reason for the
 *  deduction of the fixing
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPinferBinvarCons(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< binary variable to fix */
   SCIP_Bool             fixedval,           /**< value to fix binary variable to */
   SCIP_CONS*            infercons,          /**< constraint that deduced the fixing */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the fixing is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the fixing tightened the local bounds, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(SCIPvarIsBinary(var));
   assert(fixedval == TRUE || fixedval == FALSE);
   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferBinvarCons", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   /* get current bounds */
   lb = SCIPvarGetLbLocal(var);
   ub = SCIPvarGetUbLocal(var);
   assert(SCIPsetIsEQ(scip->set, lb, 0.0) || SCIPsetIsEQ(scip->set, lb, 1.0));
   assert(SCIPsetIsEQ(scip->set, ub, 0.0) || SCIPsetIsEQ(scip->set, ub, 1.0));
   assert(SCIPsetIsLE(scip->set, lb, ub));

   /* check, if variable is already fixed */
   if( (lb > 0.5) || (ub < 0.5) )
   {
      *infeasible = (fixedval == (lb < 0.5));

      return SCIP_OKAY;
   }

   /* apply the fixing */
   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      if( fixedval == TRUE )
      {
         SCIP_CALL( SCIPchgVarLb(scip, var, 1.0) );
      }
      else
      {
         SCIP_CALL( SCIPchgVarUb(scip, var, 0.0) );
      }
      break;

   case SCIP_STAGE_PRESOLVING:
      if( SCIPtreeGetCurrentDepth(scip->tree) == 0 )
      {
         SCIP_Bool fixed;

         SCIP_CALL( SCIPvarFix(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
               (SCIP_Real)fixedval, infeasible, &fixed) );
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      if( fixedval == TRUE )
      {
         SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
               scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
               scip->cliquetable, var, 1.0, SCIP_BOUNDTYPE_LOWER, infercons, NULL, inferinfo, FALSE) );
      }
      else
      {
         SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
               scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
               scip->cliquetable, var, 0.0, SCIP_BOUNDTYPE_UPPER, infercons, NULL, inferinfo, FALSE) );
      }
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** fixes variable in preprocessing or in the current node, if the new bound is tighter (w.r.t. bound strengthening
 *  epsilon) than the current bound; if possible, adjusts bound to integral value; the given inference constraint is
 *  stored, such that the conflict analysis is able to find out the reason for the deduction of the bound change
 *
 *  @note In presolving stage when not in probing mode the variable will be fixed directly, otherwise this method
 *        changes first the lowerbound by calling SCIPinferVarLbProp and second the upperbound by calling
 *        SCIPinferVarUbProp
 *
 *  @note If SCIP is in presolving stage, it can happen that the internal variable array (which get be accessed via
 *        SCIPgetVars()) gets resorted.
 *
 *  @note During presolving, an integer variable which bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPinferVarFixProp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             fixedval,           /**< new value for fixation */
   SCIP_PROP*            inferprop,          /**< propagator that deduced the bound change */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the bound change is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferVarFixProp", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( tightened != NULL )
      *tightened = FALSE;

   /* in presolving case we take the shortcut to directly fix the variables */
   if( SCIPgetStage(scip) == SCIP_STAGE_PRESOLVING && SCIPtreeGetCurrentDepth(scip->tree) == 0 )
   {
      SCIP_Bool fixed;

      SCIP_CALL( SCIPvarFix(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
            scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
            fixedval, infeasible, &fixed) );

      if( tightened != NULL )
	 *tightened = fixed;
   }
   /* otherwise we use the lb and ub methods */
   else
   {
      SCIP_Bool lbtightened;

      SCIP_CALL( SCIPinferVarLbProp(scip, var, fixedval, inferprop, inferinfo, force, infeasible, &lbtightened) );

      if( ! (*infeasible) )
      {
	 SCIP_CALL( SCIPinferVarUbProp(scip, var, fixedval, inferprop, inferinfo, force, infeasible, tightened) );

	 if( tightened != NULL )
	    *tightened |= lbtightened;
      }
   }

   return SCIP_OKAY;
}

/** changes lower bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current bound; if possible, adjusts bound to integral value;
 *  the given inference propagator is stored, such that the conflict analysis is able to find out the reason
 *  for the deduction of the bound change
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPinferVarLbProp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_PROP*            inferprop,          /**< propagator that deduced the bound change */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the bound change is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferVarLbProp", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustLb(var, scip->set, &newbound);

   /* ignore tightenings of lower bounds to +infinity during solving process */
   if( SCIPisInfinity(scip, newbound)  && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore lower bound tightening for %s from %e to +infinity\n", SCIPvarGetName(var),
         SCIPvarGetLbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPvarGetLbLocal(var);
   ub = SCIPvarGetUbLocal(var);
   assert(SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasGT(scip->set, newbound, ub) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MIN(newbound, ub);

   if( (!force && !SCIPsetIsLbBetter(scip->set, newbound, lb, ub))
      || SCIPsetIsLE(scip->set, newbound, lb) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgLbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgLbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_LOWER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
            scip->cliquetable, var, newbound, SCIP_BOUNDTYPE_LOWER, NULL, inferprop, inferinfo, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** changes upper bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current bound; if possible, adjusts bound to integral value;
 *  the given inference propagator is stored, such that the conflict analysis is able to find out the reason
 *  for the deduction of the bound change
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPinferVarUbProp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_PROP*            inferprop,          /**< propagator that deduced the bound change */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the bound change is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferVarUbProp", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustUb(var, scip->set, &newbound);

   /* ignore tightenings of upper bounds to -infinity during solving process */
   if( SCIPisInfinity(scip, -newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore upper bound tightening for %s from %e to -infinity\n", SCIPvarGetName(var),
         SCIPvarGetUbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPvarGetLbLocal(var);
   ub = SCIPvarGetUbLocal(var);
   assert(SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasLT(scip->set, newbound, lb) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MAX(newbound, lb);

   if( (!force && !SCIPsetIsUbBetter(scip->set, newbound, lb, ub))
      || SCIPsetIsGE(scip->set, newbound, ub) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgUbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgUbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_UPPER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
            scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue,
            scip->cliquetable, var, newbound, SCIP_BOUNDTYPE_UPPER, NULL, inferprop, inferinfo, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** depending on SCIP's stage, fixes binary variable in the problem, in preprocessing, or in current node;
 *  the given inference propagator is stored, such that the conflict analysis is able to find out the reason for the
 *  deduction of the fixing
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPinferBinvarProp(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< binary variable to fix */
   SCIP_Bool             fixedval,           /**< value to fix binary variable to */
   SCIP_PROP*            inferprop,          /**< propagator that deduced the fixing */
   int                   inferinfo,          /**< user information for inference to help resolving the conflict */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the fixing is infeasible */
   SCIP_Bool*            tightened           /**< pointer to store whether the fixing tightened the local bounds, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(SCIPvarIsBinary(var));
   assert(fixedval == TRUE || fixedval == FALSE);
   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPinferBinvarProp", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   /* get current bounds */
   lb = SCIPvarGetLbLocal(var);
   ub = SCIPvarGetUbLocal(var);
   assert(SCIPsetIsEQ(scip->set, lb, 0.0) || SCIPsetIsEQ(scip->set, lb, 1.0));
   assert(SCIPsetIsEQ(scip->set, ub, 0.0) || SCIPsetIsEQ(scip->set, ub, 1.0));
   assert(SCIPsetIsLE(scip->set, lb, ub));

   /* check, if variable is already fixed */
   if( (lb > 0.5) || (ub < 0.5) )
   {
      *infeasible = (fixedval == (lb < 0.5));

      return SCIP_OKAY;
   }

   /* apply the fixing */
   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      if( fixedval == TRUE )
      {
         SCIP_CALL( SCIPchgVarLb(scip, var, 1.0) );
      }
      else
      {
         SCIP_CALL( SCIPchgVarUb(scip, var, 0.0) );
      }
      break;

   case SCIP_STAGE_PRESOLVING:
      if( SCIPtreeGetCurrentDepth(scip->tree) == 0 )
      {
         SCIP_Bool fixed;

         SCIP_CALL( SCIPvarFix(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
               (SCIP_Real)fixedval, infeasible, &fixed) );
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      if( fixedval == TRUE )
      {
         SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
               scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, 1.0,
               SCIP_BOUNDTYPE_LOWER, NULL, inferprop, inferinfo, FALSE) );
      }
      else
      {
         SCIP_CALL( SCIPnodeAddBoundinfer(SCIPtreeGetCurrentNode(scip->tree), scip->mem->probmem, scip->set, scip->stat,
               scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, 0.0,
               SCIP_BOUNDTYPE_UPPER, NULL, inferprop, inferinfo, FALSE) );
      }
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** changes global lower bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current global bound; if possible, adjusts bound to integral value;
 *  also tightens the local bound, if the global bound is better than the local bound
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPtightenVarLbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the new domain is empty */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtightenVarLbGlobal", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustLb(var, scip->set, &newbound);

   /* ignore tightenings of lower bounds to +infinity during solving process */
   if( SCIPisInfinity(scip, newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore lower bound tightening for %s from %e to +infinity\n", SCIPvarGetName(var),
         SCIPvarGetLbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPvarGetLbGlobal(var);
   ub = SCIPvarGetUbGlobal(var);
   assert(scip->set->stage == SCIP_STAGE_PROBLEM || SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasGT(scip->set, newbound, ub) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MIN(newbound, ub);

   /* bound changes of less than epsilon are ignored by SCIPvarChgLb or raise an assert in SCIPnodeAddBoundinfer,
    * so don't apply them even if force is set
    */
   if( SCIPsetIsEQ(scip->set, lb, newbound) || (!force && !SCIPsetIsLbBetter(scip->set, newbound, lb, ub)) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgLbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgLbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_TRANSFORMING:
      SCIP_CALL( SCIPvarChgLbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_LOWER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
            scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
            SCIP_BOUNDTYPE_LOWER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   /* coverity: unreachable code */
   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/** changes global upper bound of variable in preprocessing or in the current node, if the new bound is tighter
 *  (w.r.t. bound strengthening epsilon) than the current global bound; if possible, adjusts bound to integral value;
 *  also tightens the local bound, if the global bound is better than the local bound
 *
 *  @warning If SCIP is in presolving stage, it can happen that the internal variable array (which can be accessed via
 *           SCIPgetVars()) gets resorted.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note During presolving, an integer variable whose bound changes to {0,1} is upgraded to a binary variable.
 */
SCIP_RETCODE SCIPtightenVarUbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_Real             newbound,           /**< new value for bound */
   SCIP_Bool             force,              /**< force tightening even if below bound strengthening tolerance */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the new domain is empty */
   SCIP_Bool*            tightened           /**< pointer to store whether the bound was tightened, or NULL */
   )
{
   SCIP_Real lb;
   SCIP_Real ub;

   assert(infeasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtightenVarUbGlobal", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( tightened != NULL )
      *tightened = FALSE;

   SCIPvarAdjustUb(var, scip->set, &newbound);

   /* ignore tightenings of upper bounds to -infinity during solving process */
   if( SCIPisInfinity(scip, -newbound) && SCIPgetStage(scip) == SCIP_STAGE_SOLVING )
   {
#ifndef NDEBUG
      SCIPwarningMessage(scip, "ignore upper bound tightening for %s from %e to -infinity\n", SCIPvarGetName(var),
         SCIPvarGetUbLocal(var));
#endif
      return SCIP_OKAY;
   }

   /* get current bounds */
   lb = SCIPvarGetLbGlobal(var);
   ub = SCIPvarGetUbGlobal(var);
   assert(scip->set->stage == SCIP_STAGE_PROBLEM || SCIPsetIsLE(scip->set, lb, ub));

   if( SCIPsetIsFeasLT(scip->set, newbound, lb) )
   {
      *infeasible = TRUE;
      return SCIP_OKAY;
   }
   newbound = MAX(newbound, lb);

   /* bound changes of less than epsilon are ignored by SCIPvarChgUb or raise an assert in SCIPnodeAddBoundinfer,
    * so don't apply them even if force is set
    */
   if( SCIPsetIsEQ(scip->set, ub, newbound) || (!force && !SCIPsetIsUbBetter(scip->set, newbound, lb, ub)) )
      return SCIP_OKAY;

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      SCIP_CALL( SCIPvarChgUbLocal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, newbound) );
      SCIP_CALL( SCIPvarChgUbOriginal(var, scip->set, newbound) );
      break;

   case SCIP_STAGE_TRANSFORMING:
      SCIP_CALL( SCIPvarChgUbGlobal(var, scip->mem->probmem, scip->set, scip->stat, scip->lp,
            scip->branchcand, scip->eventqueue, scip->cliquetable, newbound) );
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPinProbing(scip) )
      {
         assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);
         assert(scip->tree->root == SCIPtreeGetCurrentNode(scip->tree));

         SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
               scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
               SCIP_BOUNDTYPE_UPPER, FALSE) );

         if( (SCIP_VARTYPE)var->vartype == SCIP_VARTYPE_INTEGER && SCIPvarIsBinary(var) )
         {
            SCIP_CALL( SCIPchgVarType(scip, var, SCIP_VARTYPE_BINARY, infeasible) );
            assert(!(*infeasible));
         }
         break;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPnodeAddBoundchg(scip->tree->root, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
            scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable, var, newbound,
            SCIP_BOUNDTYPE_UPPER, FALSE) );
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   /* coverity: unreachable code */
   if( tightened != NULL )
      *tightened = TRUE;

   return SCIP_OKAY;
}

/* some simple variable functions implemented as defines */
#undef SCIPcomputeVarLbGlobal
#undef SCIPcomputeVarUbGlobal
#undef SCIPcomputeVarLbLocal
#undef SCIPcomputeVarUbLocal

/** for a multi-aggregated variable, returns the global lower bound computed by adding the global bounds from all aggregation variables
 *
 *  This global bound may be tighter than the one given by SCIPvarGetLbGlobal, since the latter is not updated if bounds of aggregation variables are changing
 *  calling this function for a non-multi-aggregated variable results in a call to SCIPvarGetLbGlobal.
 *
 *  @return the global lower bound computed by adding the global bounds from all aggregation variables
 */
SCIP_Real SCIPcomputeVarLbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(scip != NULL);
   assert(var != NULL);

   if( SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR )
      return SCIPvarGetMultaggrLbGlobal(var, scip->set);
   else
      return SCIPvarGetLbGlobal(var);
}

/** for a multi-aggregated variable, returns the global upper bound computed by adding the global bounds from all aggregation variables
 *
 *  This global bound may be tighter than the one given by SCIPvarGetUbGlobal, since the latter is not updated if bounds of aggregation variables are changing
 *  calling this function for a non-multi-aggregated variable results in a call to SCIPvarGetUbGlobal
 *
 *  @return the global upper bound computed by adding the global bounds from all aggregation variables
 */
SCIP_Real SCIPcomputeVarUbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(scip != NULL);
   assert(var != NULL);

   if( SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR )
      return SCIPvarGetMultaggrUbGlobal(var, scip->set);
   else
      return SCIPvarGetUbGlobal(var);
}

/** for a multi-aggregated variable, returns the local lower bound computed by adding the local bounds from all aggregation variables
 *
 *  This local bound may be tighter than the one given by SCIPvarGetLbLocal, since the latter is not updated if bounds of aggregation variables are changing
 *  calling this function for a non-multi-aggregated variable results in a call to SCIPvarGetLbLocal.
 *
 *  @return the local lower bound computed by adding the global bounds from all aggregation variables
 */
SCIP_Real SCIPcomputeVarLbLocal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(scip != NULL);
   assert(var != NULL);

   if( SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR )
      return SCIPvarGetMultaggrLbLocal(var, scip->set);
   else
      return SCIPvarGetLbLocal(var);
}

/** for a multi-aggregated variable, returns the local upper bound computed by adding the local bounds from all aggregation variables
 *
 *  This local bound may be tighter than the one given by SCIPvarGetUbLocal, since the latter is not updated if bounds of aggregation variables are changing
 *  calling this function for a non-multi-aggregated variable results in a call to SCIPvarGetUbLocal.
 *
 *  @return the local upper bound computed by adding the global bounds from all aggregation variables
 */
SCIP_Real SCIPcomputeVarUbLocal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(scip != NULL);
   assert(var != NULL);

   if( SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR )
      return SCIPvarGetMultaggrUbLocal(var, scip->set);
   else
      return SCIPvarGetUbLocal(var);
}

/** for a multi-aggregated variable, gives the global lower bound computed by adding the global bounds from all
 *  aggregation variables, this global bound may be tighter than the one given by SCIPvarGetLbGlobal, since the latter is
 *  not updated if bounds of aggregation variables are changing
 *
 *  calling this function for a non-multi-aggregated variable is not allowed
 */
SCIP_Real SCIPgetVarMultaggrLbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR);
   return SCIPvarGetMultaggrLbGlobal(var, scip->set);
}

/** for a multi-aggregated variable, gives the global upper bound computed by adding the global bounds from all
 *  aggregation variables, this upper bound may be tighter than the one given by SCIPvarGetUbGlobal, since the latter is
 *  not updated if bounds of aggregation variables are changing
 *
 *  calling this function for a non-multi-aggregated variable is not allowed
 */
SCIP_Real SCIPgetVarMultaggrUbGlobal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR);
   return SCIPvarGetMultaggrUbGlobal(var, scip->set);
}

/** for a multi-aggregated variable, gives the local lower bound computed by adding the local bounds from all
 *  aggregation variables, this lower bound may be tighter than the one given by SCIPvarGetLbLocal, since the latter is
 *  not updated if bounds of aggregation variables are changing
 *
 *  calling this function for a non-multi-aggregated variable is not allowed
 */
SCIP_Real SCIPgetVarMultaggrLbLocal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR);
   return SCIPvarGetMultaggrLbLocal(var, scip->set);
}

/** for a multi-aggregated variable, gives the local upper bound computed by adding the local bounds from all
 *  aggregation variables, this upper bound may be tighter than the one given by SCIPvarGetUbLocal, since the latter is
 *  not updated if bounds of aggregation variables are changing
 *
 *  calling this function for a non-multi-aggregated variable is not allowed
 */
SCIP_Real SCIPgetVarMultaggrUbLocal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to compute the bound for */
   )
{
   assert(SCIPvarGetStatus(var) == SCIP_VARSTATUS_MULTAGGR);
   return SCIPvarGetMultaggrUbLocal(var, scip->set);
}

/** returns solution value and index of variable lower bound that is closest to the variable's value in the given primal
 *  solution or current LP solution if no primal solution is given; returns an index of -1 if no variable lower bound is
 *  available
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetVarClosestVlb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< active problem variable */
   SCIP_SOL*             sol,                /**< primal solution, or NULL for LP solution */
   SCIP_Real*            closestvlb,         /**< pointer to store the value of the closest variable lower bound */
   int*                  closestvlbidx       /**< pointer to store the index of the closest variable lower bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarClosestVlb", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPvarGetClosestVlb(var, sol, scip->set, scip->stat, closestvlb, closestvlbidx);

   return SCIP_OKAY;
}

/** returns solution value and index of variable upper bound that is closest to the variable's value in the given primal solution;
 *  or current LP solution if no primal solution is given; returns an index of -1 if no variable upper bound is available
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetVarClosestVub(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< active problem variable */
   SCIP_SOL*             sol,                /**< primal solution, or NULL for LP solution */
   SCIP_Real*            closestvub,         /**< pointer to store the value of the closest variable lower bound */
   int*                  closestvubidx       /**< pointer to store the index of the closest variable lower bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetVarClosestVub", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPvarGetClosestVub(var, sol, scip->set, scip->stat, closestvub, closestvubidx);

   return SCIP_OKAY;
}

/** informs variable x about a globally valid variable lower bound x >= b*z + d with integer variable z;
 *  if z is binary, the corresponding valid implication for z is also added;
 *  if z is non-continuous and 1/b not too small, the corresponding valid upper/lower bound
 *  z <= (x-d)/b or z >= (x-d)/b (depending on the sign of of b) is added, too;
 *  improves the global bounds of the variable and the vlb variable if possible
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPaddVarVlb(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_VAR*             vlbvar,             /**< variable z    in x >= b*z + d */
   SCIP_Real             vlbcoef,            /**< coefficient b in x >= b*z + d */
   SCIP_Real             vlbconstant,        /**< constant d    in x >= b*z + d */
   SCIP_Bool*            infeasible,         /**< pointer to store whether an infeasibility was detected */
   int*                  nbdchgs             /**< pointer to store the number of performed bound changes, or NULL */
   )
{
   int nlocalbdchgs;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarVlb", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarAddVlb(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob, scip->tree,
         scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, vlbvar, vlbcoef, vlbconstant,
         TRUE, infeasible, &nlocalbdchgs) );

   *nbdchgs = nlocalbdchgs;

   /* if x is not continuous we add a variable bound for z; do not add it if cofficient would be too small or we already
    * detected infeasibility
    */
   if( !(*infeasible) && SCIPvarGetType(var) != SCIP_VARTYPE_CONTINUOUS && !SCIPisZero(scip, 1.0/vlbcoef) )
   {
      if( vlbcoef > 0.0 )
      {
         /* if b > 0, we have a variable upper bound: x >= b*z + d  =>  z <= (x-d)/b */
         SCIP_CALL( SCIPvarAddVub(vlbvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var, 1.0/vlbcoef,
               -vlbconstant/vlbcoef, TRUE, infeasible, &nlocalbdchgs) );
      }
      else
      {
         /* if b < 0, we have a variable lower bound: x >= b*z + d  =>  z >= (x-d)/b */
         SCIP_CALL( SCIPvarAddVlb(vlbvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var, 1.0/vlbcoef,
               -vlbconstant/vlbcoef, TRUE, infeasible, &nlocalbdchgs) );
      }
      *nbdchgs += nlocalbdchgs;
   }

   return SCIP_OKAY;
}

/** informs variable x about a globally valid variable upper bound x <= b*z + d with integer variable z;
 *  if z is binary, the corresponding valid implication for z is also added;
 *  if z is non-continuous and 1/b not too small, the corresponding valid lower/upper bound
 *  z >= (x-d)/b or z <= (x-d)/b (depending on the sign of of b) is added, too;
 *  improves the global bounds of the variable and the vlb variable if possible
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPaddVarVub(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_VAR*             vubvar,             /**< variable z    in x <= b*z + d */
   SCIP_Real             vubcoef,            /**< coefficient b in x <= b*z + d */
   SCIP_Real             vubconstant,        /**< constant d    in x <= b*z + d */
   SCIP_Bool*            infeasible,         /**< pointer to store whether an infeasibility was detected */
   int*                  nbdchgs             /**< pointer to store the number of performed bound changes, or NULL */
   )
{
   int nlocalbdchgs;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarVub", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarAddVub(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob, scip->tree,
         scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, vubvar, vubcoef, vubconstant, TRUE,
         infeasible, &nlocalbdchgs) );

   *nbdchgs = nlocalbdchgs;

   /* if x is not continuous we add a variable bound for z; do not add it if cofficient would be too small or we already
    * detected infeasibility
    */
   if( !(*infeasible) && SCIPvarGetType(var) != SCIP_VARTYPE_CONTINUOUS && !SCIPisZero(scip, 1.0/vubcoef) )
   {
      if( vubcoef > 0.0 )
      {
         /* if b < 0, we have a variable lower bound: x >= b*z + d  =>  z >= (x-d)/b */
         SCIP_CALL( SCIPvarAddVlb(vubvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var, 1.0/vubcoef,
               -vubconstant/vubcoef, TRUE, infeasible, &nlocalbdchgs) );
      }
      else
      {
         /* if b > 0, we have a variable upper bound: x >= b*z + d  =>  z <= (x-d)/b */
         SCIP_CALL( SCIPvarAddVub(vubvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var, 1.0/vubcoef,
               -vubconstant/vubcoef, TRUE, infeasible, &nlocalbdchgs) );
      }
      *nbdchgs += nlocalbdchgs;
   }

   return SCIP_OKAY;
}

/** informs binary variable x about a globally valid implication:  x == 0 or x == 1  ==>  y <= b  or  y >= b;
 *  also adds the corresponding implication or variable bound to the implied variable;
 *  if the implication is conflicting, the variable is fixed to the opposite value;
 *  if the variable is already fixed to the given value, the implication is performed immediately;
 *  if the implication is redundant with respect to the variables' global bounds, it is ignored
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPaddVarImplication(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Bool             varfixing,          /**< FALSE if y should be added in implications for x == 0, TRUE for x == 1 */
   SCIP_VAR*             implvar,            /**< variable y in implication y <= b or y >= b */
   SCIP_BOUNDTYPE        impltype,           /**< type       of implication y <= b (SCIP_BOUNDTYPE_UPPER)
                                              *                          or y >= b (SCIP_BOUNDTYPE_LOWER) */
   SCIP_Real             implbound,          /**< bound b    in implication y <= b or y >= b */
   SCIP_Bool*            infeasible,         /**< pointer to store whether an infeasibility was detected */
   int*                  nbdchgs             /**< pointer to store the number of performed bound changes, or NULL */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarImplication", FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if ( nbdchgs != NULL )
      *nbdchgs = 0;

   if( !SCIPvarIsBinary(var) )
   {
      SCIPerrorMessage("can't add implication for nonbinary variable\n");
      return SCIP_INVALIDDATA;
   }

   /* transform implication containing two binary variables to clique */
   if( SCIPvarIsBinary(implvar) )
   {
      SCIP_VAR* vars[2];
      SCIP_Bool vals[2];

      assert(SCIPisFeasEQ(scip, implbound, 1.0) || SCIPisFeasZero(scip, implbound));
      assert((impltype == SCIP_BOUNDTYPE_UPPER) == SCIPisFeasZero(scip, implbound));

      vars[0] = var;
      vars[1] = implvar;
      vals[0] = varfixing;
      vals[1] = (impltype == SCIP_BOUNDTYPE_UPPER);

      SCIP_CALL( SCIPaddClique(scip, vars, vals, 2, FALSE, infeasible, nbdchgs) );

      return SCIP_OKAY;
   }

   /* the implication graph can only handle 'real' binary (SCIP_VARTYPE_BINARY) variables, therefore we transform the
    * implication in variable bounds, (lowerbound of y will be abbreviated by lby, upperbound equivlaent) the follwing
    * four cases are:
    *
    * 1. (x >= 1 => y >= b) => y >= (b - lby) * x + lby
    * 2. (x >= 1 => y <= b) => y <= (b - uby) * x + uby
    * 3. (x <= 0 => y >= b) => y >= (lby - b) * x + b
    * 4. (x <= 0 => y <= b) => y <= (uby - b) * x + b
    */
   if( SCIPvarGetType(var) != SCIP_VARTYPE_BINARY )
   {
      SCIP_Real lby;
      SCIP_Real uby;

      lby = SCIPvarGetLbGlobal(implvar);
      uby = SCIPvarGetUbGlobal(implvar);

      if( varfixing == TRUE )
      {
         if( impltype == SCIP_BOUNDTYPE_LOWER )
         {
            /* we return if the lower bound is infinity */
            if( SCIPisInfinity(scip, -lby) )
               return SCIP_OKAY;

            SCIP_CALL( SCIPvarAddVlb(implvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
                  scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var,
                  implbound - lby, lby, TRUE, infeasible, nbdchgs) );
         }
         else
         {
            /* we return if the upper bound is infinity */
            if( SCIPisInfinity(scip, uby) )
               return SCIP_OKAY;

            SCIP_CALL( SCIPvarAddVub(implvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
                  scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var,
                  implbound - uby, uby, TRUE, infeasible, nbdchgs) );
         }
      }
      else
      {
         if( impltype == SCIP_BOUNDTYPE_LOWER )
         {
            /* we return if the lower bound is infinity */
            if( SCIPisInfinity(scip, -lby) )
               return SCIP_OKAY;

            SCIP_CALL( SCIPvarAddVlb(implvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
                  scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var,
                  lby - implbound, implbound, TRUE, infeasible, nbdchgs) );
         }
         else
         {
            /* we return if the upper bound is infinity */
            if( SCIPisInfinity(scip, uby) )
               return SCIP_OKAY;

            SCIP_CALL( SCIPvarAddVub(implvar, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
                  scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, var,
                  uby - implbound, implbound, TRUE, infeasible, nbdchgs) );
         }
      }
   }
   else
   {
      SCIP_CALL( SCIPvarAddImplic(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
            scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventqueue, varfixing, implvar, impltype,
            implbound, TRUE, infeasible, nbdchgs) );
   }

   return SCIP_OKAY;
}

/** adds a clique information to SCIP, stating that at most one of the given binary variables can be set to 1;
 *  if a variable appears twice in the same clique, the corresponding implications are performed
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPaddClique(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< binary variables in the clique from which at most one can be set to 1 */
   SCIP_Bool*            values,             /**< values of the variables in the clique; NULL to use TRUE for all vars */
   int                   nvars,              /**< number of variables in the clique */
   SCIP_Bool             isequation,         /**< is the clique an equation or an inequality? */
   SCIP_Bool*            infeasible,         /**< pointer to store whether an infeasibility was detected */
   int*                  nbdchgs             /**< pointer to store the number of performed bound changes, or NULL */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddClique", FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   if( nbdchgs != NULL )
      *nbdchgs = 0;

   if( nvars > 1 )
   {
      /* add the clique to the clique table */
      SCIP_CALL( SCIPcliquetableAdd(scip->cliquetable, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
            scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, vars, values, nvars, isequation,
            infeasible, nbdchgs) );
   }

   return SCIP_OKAY;
}

/** relabels the given labels in-place in an increasing fashion: the first seen label is 0, the next label 1, etc...
 *
 *  @note every label equal to -1 is treated as a previously unseen, unique label and gets a new ordered label.
 */
static
SCIP_RETCODE relabelOrderConsistent(
   SCIP*const            scip,               /**< SCIP data structure */
   int*                  labels,             /**< current labels that will be overwritten */
   int const             nlabels,            /**< number of variables in the clique */
   int*                  nclasses            /**< pointer to store the total number of distinct labels */
   )
{
   SCIP_HASHMAP* classidx2newlabel;

   int classidx;
   int i;

   SCIP_CALL( SCIPhashmapCreate(&classidx2newlabel, SCIPblkmem(scip), nlabels) );

   classidx = 0;

   /* loop over labels to create local class indices that obey the variable order */
   for( i = 0; i < nlabels; ++i )
   {
      int currentlabel = labels[i];
      int localclassidx;

      /* labels equal to -1 are stored as singleton classes */
      if( currentlabel == -1 )
      {
         ++classidx;
         localclassidx = classidx;
      }
      else
      {
         assert(currentlabel >= 0);
         /* look up the class index image in the hash map; if it is not stored yet, new class index is created and stored */
         if( !SCIPhashmapExists(classidx2newlabel, (void*)(size_t)currentlabel) )
         {
            ++classidx;
            localclassidx = classidx;
            SCIP_CALL( SCIPhashmapInsert(classidx2newlabel, (void*)(size_t)currentlabel, (void *)(size_t)classidx) );
         }
         else
         {
            localclassidx = (int)(size_t)SCIPhashmapGetImage(classidx2newlabel, (void*)(size_t)currentlabel);
         }
      }
      assert(localclassidx - 1 >= 0);
      assert(localclassidx - 1 <= i);

      /* indices start with zero, but we have an offset of 1 because we cannot store 0 in a hashmap */
      labels[i] = localclassidx - 1;
   }

   assert(classidx > 0);
   assert(classidx <= nlabels);
   *nclasses = classidx;

   SCIPhashmapFree(&classidx2newlabel);

   return SCIP_OKAY;
}

/** sort the variables w.r.t. the given labels; thereby ensure the current order of the variables with the same label. */
static
SCIP_RETCODE labelSortStable(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< variable array */
   int*                  classlabels,        /**< array that contains a class label for every variable */
   SCIP_VAR**            sortedvars,         /**< array to store variables after stable sorting */
   int*                  sortedindices,      /**< array to store indices of sorted variables in the original vars array */
   int*                  classesstartposs,   /**< starting position array for each label class (must have size nclasses + 1) */
   int                   nvars,              /**< size of the vars arrays */
   int                   nclasses            /**< number of label classes */
   )
{
   SCIP_VAR*** varpointers;
   int** indexpointers;
   int* classcount;

   int nextpos;
   int c;
   int v;

   assert(scip != NULL);
   assert(vars != NULL);
   assert(sortedindices != NULL);
   assert(classesstartposs != NULL);

   assert(nvars == 0 || vars != NULL);

   if( nvars == 0 )
      return SCIP_OKAY;

   assert(classlabels != NULL);
   assert(nclasses > 0);

   /* we first count all class cardinalities and allocate temporary memory for a bucket sort */
   SCIP_CALL( SCIPallocBufferArray(scip, &classcount, nclasses) );
   BMSclearMemoryArray(classcount, nclasses);

   /* first we count for each class the number of elements */
   for( v = nvars - 1; v >= 0; --v )
   {
      assert(0 <= classlabels[v] && classlabels[v] < nclasses);
      ++(classcount[classlabels[v]]);
   }

#ifndef NDEBUG
   BMSclearMemoryArray(sortedvars, nvars);
   BMSclearMemoryArray(sortedindices, nvars);
#endif
   SCIP_CALL( SCIPallocBufferArray(scip, &varpointers, nclasses) );
   SCIP_CALL( SCIPallocBufferArray(scip, &indexpointers, nclasses) );

   nextpos = 0;
   /* now we initialize all start pointers for each class, so they will be ordered */
   for( c = 0; c < nclasses; ++c )
   {
      /* to reach the goal that all variables of each class will be standing next to each other we will initialize the
       * starting pointers for each class by adding the cardinality of each class to the last class starting pointer
       * e.g. class1 has 4 elements and class2 has 3 elements then the starting pointer for class1 will be the pointer
       *      to sortedvars[0], the starting pointer to class2 will be the pointer to sortedvars[4] and to class3 it will be
       *      the pointer to sortedvars[7]
       */
      varpointers[c] = (SCIP_VAR**) (sortedvars + nextpos);
      indexpointers[c] = (int*) (sortedindices + nextpos);
      classesstartposs[c] = nextpos;
      assert(classcount[c] > 0);
      nextpos += classcount[c];
      assert(nextpos > 0);
   }
   assert(nextpos == nvars);
   classesstartposs[c] = nextpos;

   /* now we copy all variables to the right order */
   for( v = 0; v < nvars; ++v )
   {
      /* copy variable itself to the right position */
      *(varpointers[classlabels[v]]) = vars[v];  /*lint !e613*/
      ++(varpointers[classlabels[v]]);

      /* copy index */
      *(indexpointers[classlabels[v]]) = v;
      ++(indexpointers[classlabels[v]]);
   }

/* in debug mode, we ensure the correctness of the mapping */
#ifndef NDEBUG
   for( v = 0; v < nvars; ++v )
   {
      assert(sortedvars[v] != NULL);
      assert(sortedindices[v] >= 0);

      /* assert that the sorted indices map back to the correct variable in the original order */
      assert(vars[sortedindices[v]] == sortedvars[v]);
   }
#endif

   /* free temporary memory */
   SCIPfreeBufferArray(scip, &indexpointers);
   SCIPfreeBufferArray(scip, &varpointers);
   SCIPfreeBufferArray(scip, &classcount);

   return SCIP_OKAY;
}


/* calculate clique partition for a maximal amount of comparisons on variables due to expensive algorithm
 * @todo: check for a good value, maybe it's better to check parts of variables
 */
#define MAXNCLIQUEVARSCOMP 1000000

/** calculates a partition of the given set of binary variables into cliques;
 *  afterwards the output array contains one value for each variable, such that two variables got the same value iff they
 *  were assigned to the same clique;
 *  the first variable is always assigned to clique 0, and a variable can only be assigned to clique i if at least one of
 *  the preceding variables was assigned to clique i-1;
 *  for each clique at most 1 variables can be set to TRUE in a feasible solution;
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
static
SCIP_RETCODE calcCliquePartitionGreedy(
   SCIP*const            scip,               /**< SCIP data structure */
   SCIP_VAR**const       vars,               /**< binary variables in the clique from which at most one can be set to 1 */
   SCIP_Bool*const       values,             /**< clique value (TRUE or FALSE) for each variable in the clique */
   int const             nvars,              /**< number of variables in the array */
   int*const             cliquepartition,    /**< array of length nvars to store the clique partition */
   int*const             ncliques            /**< pointer to store the number of cliques actually contained in the partition */
   )
{
   SCIP_VAR** cliquevars;
   SCIP_Bool* cliquevalues;
   int i;
   int maxncliquevarscomp;
   int ncliquevars;



   /* allocate temporary memory for storing the variables of the current clique */
   SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &cliquevars, nvars) );
   SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &cliquevalues, nvars) );

   /* initialize the cliquepartition array with -1 */
   for( i = nvars - 1; i >= 0; --i )
      cliquepartition[i] = -1;

   maxncliquevarscomp = (int) MIN(nvars * (SCIP_Longint)nvars, MAXNCLIQUEVARSCOMP);
   /* calculate the clique partition */
   *ncliques = 0;
   for( i = 0; i < nvars; ++i )
   {
      if( cliquepartition[i] == -1 )
      {
         int j;

         /* variable starts a new clique */
         cliquepartition[i] = *ncliques;
         cliquevars[0] = vars[i];
         cliquevalues[0] = values[i];
         ncliquevars = 1;

         /* if variable is not active (multi-aggregated or fixed), it cannot be in any clique */
         if( SCIPvarIsActive(vars[i]) && SCIPvarGetNCliques(vars[i], values[i]) > 0 )
         {
            /* greedily fill up the clique */
            for( j = i+1; j < nvars; ++j )
            {
               /* if variable is not active (multi-aggregated or fixed), it cannot be in any clique */
               if( cliquepartition[j] == -1 && SCIPvarIsActive(vars[j]) )
               {
                  int k;

                  /* check if every variable in the current clique can be extended by tmpvars[j] */
                  for( k = ncliquevars - 1; k >= 0; --k )
                  {
                     if( !SCIPvarsHaveCommonClique(vars[j], values[j], cliquevars[k], cliquevalues[k], FALSE) )
                        break;
                  }

                  if( k == -1 )
                  {
                     /* put the variable into the same clique */
                     cliquepartition[j] = cliquepartition[i];
                     cliquevars[ncliquevars] = vars[j];
                     cliquevalues[ncliquevars] = values[j];
                     ++ncliquevars;
                  }
               }
            }
         }

         /* this clique is finished */
         ++(*ncliques);
      }
      assert(cliquepartition[i] >= 0 && cliquepartition[i] < i+1);

      /* break if we reached the maximal number of comparisons */
      if( i * nvars > maxncliquevarscomp )
         break;
   }
   /* if we had to many variables fill up the cliquepartition and put each variable in a separate clique */
   for( ; i < nvars; ++i )
   {
      if( cliquepartition[i] == -1 )
      {
         cliquepartition[i] = *ncliques;
         ++(*ncliques);
      }
   }

   SCIPsetFreeBufferArray(scip->set, &cliquevalues);
   SCIPsetFreeBufferArray(scip->set, &cliquevars);

   return SCIP_OKAY;
}

/** calculates a partition of the given set of binary variables into cliques; takes into account independent clique components
 *
 *  The algorithm performs the following steps:
 *  - recomputes connected components of the clique table, if necessary
 *  - computes a clique partition for every connected component greedily.
 *  - relabels the resulting clique partition such that it satisfies the description below
 *
 *  afterwards the output array contains one value for each variable, such that two variables got the same value iff they
 *  were assigned to the same clique;
 *  the first variable is always assigned to clique 0, and a variable can only be assigned to clique i if at least one of
 *  the preceding variables was assigned to clique i-1;
 *  for each clique at most 1 variables can be set to TRUE in a feasible solution;
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcalcCliquePartition(
   SCIP*const            scip,               /**< SCIP data structure */
   SCIP_VAR**const       vars,               /**< binary variables in the clique from which at most one can be set to 1 */
   int const             nvars,              /**< number of variables in the clique */
   int*const             cliquepartition,    /**< array of length nvars to store the clique partition */
   int*const             ncliques            /**< pointer to store the number of cliques actually contained in the partition */
   )
{
   SCIP_VAR** tmpvars;

   SCIP_VAR** sortedtmpvars;
   SCIP_Bool* tmpvalues;
   SCIP_Bool* sortedtmpvalues;
   int* componentlabels;
   int* sortedindices;
   int* componentstartposs;
   int i;
   int c;

   int ncomponents;

   assert(scip != NULL);
   assert(nvars == 0 || vars != NULL);
   assert(nvars == 0 || cliquepartition != NULL);
   assert(ncliques != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPcalcCliquePartition", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( nvars == 0 )
   {
      *ncliques = 0;
      return SCIP_OKAY;
   }

   /* early abort if no cliques are present */
   if( SCIPgetNCliques(scip) == 0 )
   {
      for( i = 0; i < nvars; ++i )
         cliquepartition[i] = i;

      *ncliques = nvars;

      return SCIP_OKAY;
   }


   SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &tmpvalues, nvars) );
   SCIP_CALL( SCIPsetDuplicateBufferArray(scip->set, &tmpvars, vars, nvars) );
   SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &componentlabels, nvars) );
   SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &sortedindices, nvars) );

   /* initialize the tmpvalues array */
   for( i = nvars - 1; i >= 0; --i )
   {
      tmpvalues[i] = TRUE;
      cliquepartition[i] = -1;
   }

   /* get corresponding active problem variables */
   SCIP_CALL( SCIPvarsGetProbvarBinary(&tmpvars, &tmpvalues, nvars) );

   ncomponents = -1;

   /* update clique components if necessary */
   if( SCIPcliquetableNeedsComponentUpdate(scip->cliquetable) )
   {
      SCIP_VAR** allvars;
      int nallbinvars;
      int nallintvars;
      int nallimplvars;

      SCIP_CALL( SCIPgetVarsData(scip, &allvars, NULL, &nallbinvars, &nallintvars, &nallimplvars, NULL) );

      SCIP_CALL( SCIPcliquetableComputeCliqueComponents(scip->cliquetable, scip->set, SCIPblkmem(scip), allvars, nallbinvars, nallintvars, nallimplvars) );
   }

   assert(!SCIPcliquetableNeedsComponentUpdate(scip->cliquetable));

   /* store the global clique component labels */
   for( i = 0; i < nvars; ++i )
   {
      if( SCIPvarIsActive(tmpvars[i]) )
         componentlabels[i] = SCIPcliquetableGetVarComponentIdx(scip->cliquetable, tmpvars[i]);
      else
         componentlabels[i] = -1;
   }

   /* relabel component labels order consistent as prerequisite for a stable sort */
   SCIP_CALL( relabelOrderConsistent(scip, componentlabels, nvars, &ncomponents) );
   assert(ncomponents >= 1);
   assert(ncomponents <= nvars);

   /* allocate storage array for the starting positions of the components */
   SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &componentstartposs, ncomponents + 1) );

   /* stable sort the variables w.r.t. the component labels so that we can restrict the quadratic algorithm to the components */
   if( ncomponents > 1 )
   {
      SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &sortedtmpvars, nvars) );
      SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &sortedtmpvalues, nvars) );
      SCIP_CALL( labelSortStable(scip, tmpvars, componentlabels, sortedtmpvars, sortedindices, componentstartposs, nvars, ncomponents) );

      /* reassign the tmpvalues with respect to the sorting */
      for( i = 0; i < nvars; ++i )
      {
         assert(tmpvars[sortedindices[i]] == sortedtmpvars[i]);
         sortedtmpvalues[i] = tmpvalues[sortedindices[i]];
      }
   }
   else
   {
      /* if we have only one large connected component, skip the stable sorting and prepare the data differently */
      sortedtmpvars = tmpvars;
      sortedtmpvalues = tmpvalues;
      componentstartposs[0] = 0;
      componentstartposs[1] = nvars;

      /* sorted indices are the identity */
      for( i = 0; i < nvars; ++i )
         sortedindices[i] = i;
   }

   *ncliques = 0;
   /* calculate a greedy clique partition for each connected component */
   for( c = 0; c < ncomponents; ++c )
   {
      int* localcliquepartition;
      int nlocalcliques;
      int ncomponentvars;
      int l;

      /* extract the number of variables in this connected component */
      ncomponentvars = componentstartposs[c + 1] - componentstartposs[c];
      nlocalcliques = 0;

      /* allocate necessary memory to hold the intermediate component clique partition */
      SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &localcliquepartition, ncomponentvars) );

      /* call greedy clique algorithm for all component variables */
      SCIP_CALL( calcCliquePartitionGreedy(scip, &(sortedtmpvars[componentstartposs[c]]), &(sortedtmpvalues[componentstartposs[c]]),
            ncomponentvars, localcliquepartition, &nlocalcliques) );

      assert(nlocalcliques >= 1);
      assert(nlocalcliques <= ncomponentvars);

      /* store the obtained clique partition with an offset of ncliques for the original variables */
      for( l = componentstartposs[c]; l < componentstartposs[c + 1]; ++l )
      {
         int origvaridx = sortedindices[l];
         assert(cliquepartition[origvaridx] == -1);
         assert(localcliquepartition[l - componentstartposs[c]] <= l - componentstartposs[c]);
         cliquepartition[origvaridx] = localcliquepartition[l - componentstartposs[c]] + (*ncliques);
      }
      *ncliques += nlocalcliques;

      /* free the local clique partition */
      SCIPsetFreeBufferArray(scip->set, &localcliquepartition);
   }

   /* except in the two trivial cases, we have to ensure the order consistency of the partition indices */
   if( ncomponents > 1 && ncomponents < nvars )
   {
      int partitionsize;
      SCIP_CALL( relabelOrderConsistent(scip, cliquepartition, nvars, &partitionsize) );

      assert(partitionsize == *ncliques);
   }

   if( ncomponents > 1 )
   {
      SCIPsetFreeBufferArray(scip->set, &sortedtmpvalues);
      SCIPsetFreeBufferArray(scip->set, &sortedtmpvars);
   }

   /* use the greedy algorithm as a whole to verify the result on small number of variables */
#ifdef SCIP_DISABLED_CODE
   {
      int* debugcliquepartition;
      int ndebugcliques;

      SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &debugcliquepartition, nvars) );

      /* call greedy clique algorithm for all component variables */
      SCIP_CALL( calcCliquePartitionGreedy(scip, tmpvars, tmpvalues, nvars, debugcliquepartition, &ndebugcliques) );

      /* loop and compare the traditional greedy clique with  */
      for( i = 0; i < nvars; ++i )
         assert(i * nvars > MAXNCLIQUEVARSCOMP || cliquepartition[i] == debugcliquepartition[i]);

      SCIPsetFreeBufferArray(scip->set, &debugcliquepartition);
   }
#endif

   /* free temporary memory */
   SCIPsetFreeBufferArray(scip->set, &componentstartposs);
   SCIPsetFreeBufferArray(scip->set, &sortedindices);
   SCIPsetFreeBufferArray(scip->set, &componentlabels);
   SCIPsetFreeBufferArray(scip->set, &tmpvars);
   SCIPsetFreeBufferArray(scip->set, &tmpvalues);

   return SCIP_OKAY;
}

/** calculates a partition of the given set of binary variables into negated cliques;
 *  afterwards the output array contains one value for each variable, such that two variables got the same value iff they
 *  were assigned to the same negated clique;
 *  the first variable is always assigned to clique 0 and a variable can only be assigned to clique i if at least one of
 *  the preceding variables was assigned to clique i-1;
 *  for each clique with n_c variables at least n_c-1 variables can be set to TRUE in a feasible solution;
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcalcNegatedCliquePartition(
   SCIP*const            scip,               /**< SCIP data structure */
   SCIP_VAR**const       vars,               /**< binary variables in the clique from which at most one can be set to 1 */
   int const             nvars,              /**< number of variables in the clique */
   int*const             cliquepartition,    /**< array of length nvars to store the clique partition */
   int*const             ncliques            /**< pointer to store the number of cliques actually contained in the partition */
   )
{
   SCIP_VAR** negvars;
   int v;

   assert(scip != NULL);
   assert(cliquepartition != NULL || nvars == 0);
   assert(ncliques != NULL);

   if( nvars == 0 )
   {
      *ncliques = 0;
      return SCIP_OKAY;
   }
   assert(vars != NULL);

   /* allocate temporary memory */
   SCIP_CALL( SCIPsetAllocBufferArray(scip->set, &negvars, nvars) );

   /* get all negated variables */
   for( v = nvars - 1; v >= 0; --v )
   {
      SCIP_CALL( SCIPgetNegatedVar(scip, vars[v], &(negvars[v])) );
   }

   /* calculate cliques on negated variables, which are "negated" cliques on normal variables array */
   SCIP_CALL( SCIPcalcCliquePartition( scip, negvars, nvars, cliquepartition, ncliques) );

   /* free temporary memory */
   SCIPsetFreeBufferArray(scip->set, &negvars);

   return SCIP_OKAY;
}


/** force SCIP to clean up all cliques; cliques do not get automatically cleaned up after presolving. Use
 *  this method to prevent inactive variables in cliques when retrieved via SCIPgetCliques()
 *
 *  @return SCIP_OKAY if everything worked, otherwise a suitable error code is passed
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPcleanupCliques(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool*            infeasible          /**< pointer to store if cleanup detected infeasibility */
   )
{
   int nlocalbdchgs;
   SCIP_Bool globalinfeasibility;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPcleanupCliques", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   globalinfeasibility = FALSE;
   nlocalbdchgs = 0;
   SCIP_CALL( SCIPcliquetableCleanup(scip->cliquetable, scip->mem->probmem, scip->set, scip->stat, scip->transprob,
         scip->origprob, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, &nlocalbdchgs,
         &globalinfeasibility) );

   if( infeasible != NULL )
      *infeasible = globalinfeasibility;

   if( globalinfeasibility )
      scip->stat->status = SCIP_STATUS_INFEASIBLE;

   return SCIP_OKAY;
}

/** gets the number of cliques in the clique table
 *
 *  @return number of cliques in the clique table
 *
 *  @note cliques do not get automatically cleaned up after presolving. Use SCIPcleanupCliques()
 *  to prevent inactive variables in cliques when retrieved via SCIPgetCliques(). This might reduce the number of cliques
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetNCliques(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNCliques", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPcliquetableGetNCliques(scip->cliquetable);
}

/** gets the number of cliques created so far by the cliquetable
 *
 *  @return number of cliques created so far by the cliquetable
 *
 *  @note cliques do not get automatically cleaned up after presolving. Use SCIPcleanupCliques()
 *  to prevent inactive variables in cliques when retrieved via SCIPgetCliques(). This might reduce the number of cliques
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetNCliquesCreated(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNCliquesCreated", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPcliquetableGetNCliquesCreated(scip->cliquetable);
}

/** gets the array of cliques in the clique table
 *
 *  @return array of cliques in the clique table
 *
 *  @note cliques do not get automatically cleaned up after presolving. Use SCIPcleanupCliques()
 *  to prevent inactive variables in cliques when retrieved via SCIPgetCliques(). This might reduce the number of cliques
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_CLIQUE** SCIPgetCliques(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetCliques", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPcliquetableGetCliques(scip->cliquetable);
}

/** returns whether there is a clique that contains both given variable/value pairs;
 *  the variables must be active binary variables;
 *  if regardimplics is FALSE, only the cliques in the clique table are looked at;
 *  if regardimplics is TRUE, both the cliques and the implications of the implication graph are regarded
 *
 *  @return TRUE, if there is a clique that contains both variable/clique pairs; FALSE, otherwise
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *
 *  @note a variable with it's negated variable are NOT! in a clique
 *  @note a variable with itself are in a clique
 */
SCIP_Bool SCIPhaveVarsCommonClique(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var1,               /**< first variable */
   SCIP_Bool             value1,             /**< value of first variable */
   SCIP_VAR*             var2,               /**< second variable */
   SCIP_Bool             value2,             /**< value of second variable */
   SCIP_Bool             regardimplics       /**< should the implication graph also be searched for a clique? */
   )
{
   assert(scip != NULL);
   assert(var1 != NULL);
   assert(var2 != NULL);
   assert(SCIPvarIsActive(var1));
   assert(SCIPvarIsActive(var2));
   assert(SCIPvarIsBinary(var1));
   assert(SCIPvarIsBinary(var2));

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPhaveVarsCommonClique", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   /* if both variables together have more cliques then actual cliques exist, then they have a common clique (in debug
    * mode we check this for correctness), otherwise we need to call the pairwise comparison method for these variables
    */
#ifndef NDEBUG
   assert((SCIPvarGetNCliques(var1, value1) + SCIPvarGetNCliques(var2, value2) > SCIPcliquetableGetNCliques(scip->cliquetable)) ? SCIPvarsHaveCommonClique(var1, value1, var2, value2, FALSE) : TRUE);
#endif

   return (SCIPvarGetNCliques(var1, value1) + SCIPvarGetNCliques(var2, value2) > SCIPcliquetableGetNCliques(scip->cliquetable)
      || SCIPvarsHaveCommonClique(var1, value1, var2, value2, regardimplics));
}


/** writes the clique graph to a gml file
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *
 *  @note there can be duplicated arcs in the output file
 *
 *  If @p writenodeweights is true, only nodes corresponding to variables that have a fractional value and only edges
 *  between such nodes are written.
 */
SCIP_RETCODE SCIPwriteCliqueGraph(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           fname,              /**< name of file */
   SCIP_Bool             writenodeweights    /**< should we write weights of nodes? */
   )
{
   FILE* gmlfile;
   SCIP_HASHMAP* nodehashmap;
   SCIP_CLIQUE** cliques;
   SCIP_VAR** clqvars;
   SCIP_VAR** allvars;
   SCIP_Bool* clqvalues;
   char nodename[SCIP_MAXSTRLEN];
   int nallvars;
   int nbinvars;
   int nintvars;
   int nimplvars;
   int ncliques;
   int c;
   int v1;
   int v2;
   int id1;
   int id2;

   assert(scip != NULL);
   assert(fname != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPwriteCliqueGraph", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   /* get all active variables */
   SCIP_CALL( SCIPgetVarsData(scip, &allvars, &nallvars, &nbinvars, &nintvars, &nimplvars, NULL) );

   /* no possible variables for cliques exist */
   if( nbinvars + nimplvars == 0 )
      return SCIP_OKAY;

   ncliques = SCIPgetNCliques(scip);

   /* no cliques and do not wont to check for binary implications */
   if( ncliques == 0 )
      return SCIP_OKAY;

   /* open gml file */
   gmlfile = fopen(fname, "w");

   if( gmlfile == NULL )
   {
      SCIPerrorMessage("cannot open graph file <%s>\n", fname);
      SCIPABORT();
      return SCIP_INVALIDDATA; /*lint !e527*/
   }

   /* create the hash map */
   SCIP_CALL_FINALLY( SCIPhashmapCreate(&nodehashmap, SCIPblkmem(scip), nbinvars+nimplvars), fclose(gmlfile) );

   /* write starting of gml file */
   SCIPgmlWriteOpening(gmlfile, TRUE);

   cliques = SCIPgetCliques(scip);

   /* write nodes and arcs for all cliques */
   for( c = ncliques - 1; c >= 0; --c )
   {
      clqvalues = SCIPcliqueGetValues(cliques[c]);
      clqvars = SCIPcliqueGetVars(cliques[c]);

      for( v1 = SCIPcliqueGetNVars(cliques[c]) - 1; v1 >= 0; --v1 )
      {
	 id1 = clqvalues[v1] ? SCIPvarGetProbindex(clqvars[v1]) : (nallvars + SCIPvarGetProbindex(clqvars[v1]));

	 /* if corresponding node was not added yet, add it */
	 if( !SCIPhashmapExists(nodehashmap, (void*)(size_t)id1) )
	 {
            assert(id1 >= 0);
	    SCIP_CALL_FINALLY( SCIPhashmapInsert(nodehashmap, (void*)(size_t)id1, (void*)(size_t) 1), fclose(gmlfile) );

	    (void) SCIPsnprintf(nodename, SCIP_MAXSTRLEN, "%s%s", (id1 >= nallvars ? "~" : ""), SCIPvarGetName(clqvars[v1]));

            /* write new gml node for new variable */
            if ( writenodeweights )
            {
               if ( ! SCIPisFeasIntegral(scip, SCIPgetSolVal(scip, NULL, clqvars[v1])) )
                  SCIPgmlWriteNodeWeight(gmlfile, (unsigned int)id1, nodename, NULL, NULL, NULL, SCIPgetSolVal(scip, NULL, clqvars[v1]));
            }
            else
            {
               SCIPgmlWriteNode(gmlfile, (unsigned int)id1, nodename, NULL, NULL, NULL);
            }
	 }

	 for( v2 = SCIPcliqueGetNVars(cliques[c]) - 1; v2 >= 0; --v2 )
	 {
	    if( v1 == v2 )
	       continue;

	    id2 = clqvalues[v2] ? SCIPvarGetProbindex(clqvars[v2]) : (nallvars + SCIPvarGetProbindex(clqvars[v2]));

	    /* if corresponding node was not added yet, add it */
	    if( !SCIPhashmapExists(nodehashmap, (void*)(size_t)id2) )
	    {
               assert(id2 >= 0);
	       SCIP_CALL_FINALLY( SCIPhashmapInsert(nodehashmap, (void*)(size_t)id2, (void*)(size_t) 1), fclose(gmlfile) );

	       (void) SCIPsnprintf(nodename, SCIP_MAXSTRLEN, "%s%s", (id2 >= nallvars ? "~" : ""), SCIPvarGetName(clqvars[v2]));

	       /* write new gml node for new variable */
               if ( writenodeweights )
               {
                  if ( ! SCIPisFeasIntegral(scip, SCIPgetSolVal(scip, NULL, clqvars[v2])) )
                     SCIPgmlWriteNodeWeight(gmlfile, (unsigned int)id2, nodename, NULL, NULL, NULL, SCIPgetSolVal(scip, NULL, clqvars[v2]));
               }
               else
               {
                  SCIPgmlWriteNode(gmlfile, (unsigned int)id2, nodename, NULL, NULL, NULL);
               }
            }

	    /* write gml arc between resultant and operand */
            if ( ! writenodeweights || ! SCIPisFeasIntegral(scip, SCIPgetSolVal(scip, NULL, clqvars[v2])) )
               SCIPgmlWriteArc(gmlfile, (unsigned int)id1, (unsigned int)id2, NULL, NULL);
	 }
      }
   }

   /* free the hash map */
   SCIPhashmapFree(&nodehashmap);

   SCIPgmlWriteClosing(gmlfile);
   fclose(gmlfile);

   return SCIP_OKAY;
}

/** Removes (irrelevant) variable from all its global structures, i.e. cliques, implications and variable bounds.
 *  This is an advanced method which should be used with care.
 *
 *  @return SCIP_OKAY if everything worked, otherwise a suitable error code is passed
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPremoveVarFromGlobalStructures(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to remove from global structures */
   )
{
   assert(scip != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPremoveVarFromGlobalStructures", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   /* mark the variable as deletable from global structures - This is necessary for the delayed clean up of cliques */
   SCIPvarMarkDeleteGlobalStructures(var);

   /* remove variable from all its cliques, implications, and variable bounds */
   SCIP_CALL( SCIPvarRemoveCliquesImplicsVbs(var, SCIPblkmem(scip), scip->cliquetable, scip->set, TRUE, FALSE, TRUE) );

   return SCIP_OKAY;
}

/** sets the branch factor of the variable; this value can be used in the branching methods to scale the score
 *  values of the variables; higher factor leads to a higher probability that this variable is chosen for branching
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPchgVarBranchFactor(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             branchfactor        /**< factor to weigh variable's branching score with */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarBranchFactor", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarChgBranchFactor(var, scip->set, branchfactor) );

   return SCIP_OKAY;
}

/** scales the branch factor of the variable with the given value
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPscaleVarBranchFactor(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             scale               /**< factor to scale variable's branching factor with */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPscaleVarBranchFactor", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarChgBranchFactor(var, scip->set, scale * SCIPvarGetBranchFactor(var)) );

   return SCIP_OKAY;
}

/** adds the given value to the branch factor of the variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPaddVarBranchFactor(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             addfactor           /**< value to add to the branch factor of the variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarBranchFactor", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPvarChgBranchFactor(var, scip->set, addfactor + SCIPvarGetBranchFactor(var)) );

   return SCIP_OKAY;
}

/** sets the branch priority of the variable; variables with higher branch priority are always preferred to variables
 *  with lower priority in selection of branching variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 * @note the default branching priority is 0
 */
SCIP_RETCODE SCIPchgVarBranchPriority(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   int                   branchpriority      /**< branch priority of the variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarBranchPriority", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   if( SCIPisTransformed(scip)  )
   {
      assert(scip->branchcand != NULL);

      /* inform the pseudo branch candidates that the branch priority changes and change the branch priority */
      SCIP_CALL( SCIPbranchcandUpdateVarBranchPriority(scip->branchcand, scip->set, var, branchpriority) );
   }
   else
   {
      /* change the branching priority of the variable */
      SCIP_CALL( SCIPvarChgBranchPriority(var, branchpriority) );
   }

   return SCIP_OKAY;
}

/** changes the branch priority of the variable to the given value, if it is larger than the current priority
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPupdateVarBranchPriority(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   int                   branchpriority      /**< new branch priority of the variable, if it is larger than current priority */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPupdateVarBranchPriority", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   if( branchpriority > SCIPvarGetBranchPriority(var) )
   {
      SCIP_CALL( SCIPvarChgBranchPriority(var, branchpriority) );
   }

   return SCIP_OKAY;
}

/** adds the given value to the branch priority of the variable
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPaddVarBranchPriority(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   int                   addpriority         /**< value to add to the branch priority of the variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddVarBranchPriority", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   SCIP_CALL( SCIPvarChgBranchPriority(var, addpriority + SCIPvarGetBranchPriority(var)) );

   return SCIP_OKAY;
}

/** sets the branch direction of the variable (-1: prefer downwards branch, 0: automatic selection, +1: prefer upwards
 *  branch)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPchgVarBranchDirection(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        branchdirection     /**< preferred branch direction of the variable (downwards, upwards, auto) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarBranchDirection", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   SCIP_CALL( SCIPvarChgBranchDirection(var, branchdirection) );

   return SCIP_OKAY;
}

/** tightens the variable bounds due a new variable type */
static
SCIP_RETCODE tightenBounds(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_VARTYPE          vartype,            /**< new type of variable */
   SCIP_Bool*            infeasible          /**< pointer to store whether an infeasibility was detected (, due to
                                              *   integrality condition of the new variable type) */
   )
{
   assert(scip != NULL);
   assert(SCIPgetStage(scip) == SCIP_STAGE_PROBLEM || SCIPgetStage(scip) == SCIP_STAGE_PRESOLVING);
   assert(scip->set->stage == SCIP_STAGE_PROBLEM || SCIPvarIsTransformed(var));
   assert(var->scip == scip);

   *infeasible = FALSE;

   /* adjusts bounds if the variable type changed form continuous to non-continuous (integral) */
   if( SCIPvarGetType(var) == SCIP_VARTYPE_CONTINUOUS && vartype != SCIP_VARTYPE_CONTINUOUS )
   {
      SCIP_Bool tightened;

      /* we adjust variable bounds to integers first, since otherwise a later bound tightening with a fractional old
       * bound may give an assert because SCIP expects non-continuous variables to have non-fractional bounds
       *
       * we adjust bounds with a fractionality within [eps,feastol] only if the resulting bound change is a bound
       * tightening, because relaxing bounds may not be allowed
       */
      if( !SCIPisFeasIntegral(scip, SCIPvarGetLbGlobal(var)) ||
         (!SCIPisIntegral(scip, SCIPvarGetLbGlobal(var)) && SCIPvarGetLbGlobal(var) < SCIPfeasCeil(scip, SCIPvarGetLbGlobal(var)))
        )
      {
         SCIP_CALL( SCIPtightenVarLbGlobal(scip, var, SCIPfeasCeil(scip, SCIPvarGetLbGlobal(var)), TRUE, infeasible, &tightened) );
         if( *infeasible )
            return SCIP_OKAY;

         /* the only reason for not applying a forced boundchange is when the new bound is reduced because the variables upper bound is below the new bound
          * in a concrete case, lb == ub == 100.99999001; even though within feastol of 101, the lower bound cannot be tighented to 101 due to the upper bound
          */
         assert(tightened || SCIPisFeasLE(scip, SCIPvarGetUbGlobal(var), SCIPfeasCeil(scip, SCIPvarGetLbGlobal(var))));
      }
      if( !SCIPisFeasIntegral(scip, SCIPvarGetUbGlobal(var)) ||
         (!SCIPisIntegral(scip, SCIPvarGetUbGlobal(var)) && SCIPvarGetUbGlobal(var) > SCIPfeasFloor(scip, SCIPvarGetUbGlobal(var)))
        )
      {
         SCIP_CALL( SCIPtightenVarUbGlobal(scip, var, SCIPfeasFloor(scip, SCIPvarGetUbGlobal(var)), TRUE, infeasible, &tightened) );
         if( *infeasible )
            return SCIP_OKAY;

         assert(tightened || SCIPisFeasGE(scip, SCIPvarGetLbGlobal(var), SCIPfeasFloor(scip, SCIPvarGetUbGlobal(var))));
      }
   }

   return SCIP_OKAY;
}

/** changes type of variable in the problem;
 *
 *  @warning This type change might change the variable array returned from SCIPgetVars() and SCIPgetVarsData();
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_PRESOLVING
 *
 *  @note If SCIP is already beyond the SCIP_STAGE_PROBLEM and a original variable is passed, the variable type of the
 *        corresponding transformed variable is changed; the type of the original variable does not change
 *
 *  @note If the type changes from a continuous variable to a non-continuous variable the bounds of the variable get
 *        adjusted w.r.t. to integrality information
 */
SCIP_RETCODE SCIPchgVarType(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to change the bound for */
   SCIP_VARTYPE          vartype,            /**< new type of variable */
   SCIP_Bool*            infeasible          /**< pointer to store whether an infeasibility was detected (, due to
                                              *   integrality condition of the new variable type) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPchgVarType", FALSE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(var != NULL);
   assert(var->scip == scip);

   if( SCIPvarIsNegated(var) )
   {
      SCIPdebugMsg(scip, "upgrading type of negated variable <%s> from %d to %d\n", SCIPvarGetName(var), SCIPvarGetType(var), vartype);
      var = SCIPvarGetNegationVar(var);
   }
#ifndef NDEBUG
   else
   {
      if( SCIPgetStage(scip) > SCIP_STAGE_PROBLEM )
      {
         SCIPdebugMsg(scip, "upgrading type of variable <%s> from %d to %d\n", SCIPvarGetName(var), SCIPvarGetType(var), vartype);
      }
   }
#endif

   /* change variable type */
   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      assert(!SCIPvarIsTransformed(var));

      /* first adjust the variable due to new integrality information */
      SCIP_CALL( tightenBounds(scip, var, vartype, infeasible) );

      /* second change variable type */
      if( SCIPvarGetProbindex(var) >= 0 )
      {
         SCIP_CALL( SCIPprobChgVarType(scip->origprob, scip->mem->probmem, scip->set, scip->branchcand, scip->cliquetable, var, vartype) );
      }
      else
      {
         SCIP_CALL( SCIPvarChgType(var, vartype) );
      }
      break;

   case SCIP_STAGE_PRESOLVING:
      if( !SCIPvarIsTransformed(var) )
      {
         SCIP_VAR* transvar;

         SCIP_CALL( SCIPgetTransformedVar(scip, var, &transvar) );
         assert(transvar != NULL);

         /* recall method with transformed variable */
         SCIP_CALL( SCIPchgVarType(scip, transvar, vartype, infeasible) );
         return SCIP_OKAY;
      }

      /* first adjust the variable due to new integrality information */
      SCIP_CALL( tightenBounds(scip, var, vartype, infeasible) );

      /* second change variable type */
      if( SCIPvarGetProbindex(var) >= 0 )
      {
         SCIP_CALL( SCIPprobChgVarType(scip->transprob, scip->mem->probmem, scip->set, scip->branchcand, scip->cliquetable, var, vartype) );
      }
      else
      {
         SCIP_CALL( SCIPvarChgType(var, vartype) );
      }
      break;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** in problem creation and solving stage, both bounds of the variable are set to the given value;
 *  in presolving stage, the variable is converted into a fixed variable, and bounds are changed respectively;
 *  conversion into a fixed variable changes the vars array returned from SCIPgetVars() and SCIPgetVarsData(),
 *  and also renders arrays returned from the SCIPvarGetImpl...() methods invalid
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPfixVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable to fix */
   SCIP_Real             fixedval,           /**< value to fix variable to */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the fixing is infeasible */
   SCIP_Bool*            fixed               /**< pointer to store whether the fixing was performed (variable was unfixed) */
   )
{
   assert(var != NULL);
   assert(infeasible != NULL);
   assert(fixed != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPfixVar", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   *fixed = FALSE;

   /* in the problem creation stage, modify the bounds as requested, independently from the current bounds */
   if( scip->set->stage != SCIP_STAGE_PROBLEM )
   {
      if( (SCIPvarGetType(var) != SCIP_VARTYPE_CONTINUOUS && !SCIPsetIsFeasIntegral(scip->set, fixedval))
         || SCIPsetIsFeasLT(scip->set, fixedval, SCIPvarGetLbLocal(var))
         || SCIPsetIsFeasGT(scip->set, fixedval, SCIPvarGetUbLocal(var)) )
      {
         *infeasible = TRUE;
         return SCIP_OKAY;
      }
      else if( SCIPvarGetStatus(var) == SCIP_VARSTATUS_FIXED )
      {
         *infeasible = !SCIPsetIsFeasEQ(scip->set, fixedval, SCIPvarGetLbLocal(var));
         return SCIP_OKAY;
      }
   }
   else
      assert(SCIPvarGetStatus(var) == SCIP_VARSTATUS_ORIGINAL);

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      /* in the problem creation stage, modify the bounds as requested, independently from the current bounds;
       * we have to make sure, that the order of the bound changes does not intermediately produce an invalid
       * interval lb > ub
       */
      if( fixedval <= SCIPvarGetLbLocal(var) )
      {
         SCIP_CALL( SCIPchgVarLb(scip, var, fixedval) );
         SCIP_CALL( SCIPchgVarUb(scip, var, fixedval) );
         *fixed = TRUE;
      }
      else
      {
         SCIP_CALL( SCIPchgVarUb(scip, var, fixedval) );
         SCIP_CALL( SCIPchgVarLb(scip, var, fixedval) );
         *fixed = TRUE;
      }
      return SCIP_OKAY;

   case SCIP_STAGE_PRESOLVING:
      if( SCIPtreeGetCurrentDepth(scip->tree) == 0 )
      {
         SCIP_CALL( SCIPvarFix(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
               fixedval, infeasible, fixed) );
         return SCIP_OKAY;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_SOLVING:
      if( SCIPsetIsFeasGT(scip->set, fixedval, SCIPvarGetLbLocal(var)) )
      {
         SCIP_CALL( SCIPchgVarLb(scip, var, fixedval) );
         *fixed = TRUE;
      }
      if( SCIPsetIsFeasLT(scip->set, fixedval, SCIPvarGetUbLocal(var)) )
      {
         SCIP_CALL( SCIPchgVarUb(scip, var, fixedval) );
         *fixed = TRUE;
      }
      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** From a given equality a*x + b*y == c, aggregates one of the variables and removes it from the set of
 *  active problem variables. This changes the vars array returned from SCIPgetVars() and SCIPgetVarsData(),
 *  and also renders the arrays returned from the SCIPvarGetImpl...() methods for the two variables invalid.
 *  In the first step, the equality is transformed into an equality with active problem variables
 *  a'*x' + b'*y' == c'. If x' == y', this leads to the detection of redundancy if a' == -b' and c' == 0,
 *  of infeasibility, if a' == -b' and c' != 0, or to a variable fixing x' == c'/(a'+b') (and possible
 *  infeasibility) otherwise.
 *  In the second step, the variable to be aggregated is chosen among x' and y', prefering a less strict variable
 *  type as aggregation variable (i.e. continuous variables are preferred over implicit integers, implicit integers
 *  over integers, and integers over binaries). If none of the variables is continuous, it is tried to find an integer
 *  aggregation (i.e. integral coefficients a'' and b'', such that a''*x' + b''*y' == c''). This can lead to
 *  the detection of infeasibility (e.g. if c'' is fractional), or to a rejection of the aggregation (denoted by
 *  aggregated == FALSE), if the resulting integer coefficients are too large and thus numerically instable.
 *
 *  The output flags have the following meaning:
 *  - infeasible: the problem is infeasible
 *  - redundant:  the equality can be deleted from the constraint set
 *  - aggregated: the aggregation was successfully performed (the variables were not aggregated before)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_PRESOLVING
 */
SCIP_RETCODE SCIPaggregateVars(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             varx,               /**< variable x in equality a*x + b*y == c */
   SCIP_VAR*             vary,               /**< variable y in equality a*x + b*y == c */
   SCIP_Real             scalarx,            /**< multiplier a in equality a*x + b*y == c */
   SCIP_Real             scalary,            /**< multiplier b in equality a*x + b*y == c */
   SCIP_Real             rhs,                /**< right hand side c in equality a*x + b*y == c */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the aggregation is infeasible */
   SCIP_Bool*            redundant,          /**< pointer to store whether the equality is (now) redundant */
   SCIP_Bool*            aggregated          /**< pointer to store whether the aggregation was successful */
   )
{
   SCIP_Real constantx;
   SCIP_Real constanty;

   assert(infeasible != NULL);
   assert(redundant != NULL);
   assert(aggregated != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPaggregateVars", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   *infeasible = FALSE;
   *redundant = FALSE;
   *aggregated = FALSE;

   if( SCIPtreeProbing(scip->tree) )
   {
      SCIPerrorMessage("cannot aggregate variables during probing\n");
      return SCIP_INVALIDCALL;
   }
   assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);

   /* do not perform aggregation if it is globally deactivated */
   if( scip->set->presol_donotaggr )
      return SCIP_OKAY;

   /* get the corresponding equality in active problem variable space:
    * transform both expressions "a*x + 0" and "b*y + 0" into problem variable space
    */
   constantx = 0.0;
   constanty = 0.0;
   SCIP_CALL( SCIPvarGetProbvarSum(&varx, scip->set, &scalarx, &constantx) );
   SCIP_CALL( SCIPvarGetProbvarSum(&vary, scip->set, &scalary, &constanty) );

   /* we cannot aggregate multi-aggregated variables */
   if( SCIPvarGetStatus(varx) == SCIP_VARSTATUS_MULTAGGR || SCIPvarGetStatus(vary) == SCIP_VARSTATUS_MULTAGGR )
      return SCIP_OKAY;

   /* move the constant to the right hand side to acquire the form "a'*x' + b'*y' == c'" */
   rhs -= (constantx + constanty);

   /* if a scalar is zero, treat the variable as fixed-to-zero variable */
   if( SCIPsetIsZero(scip->set, scalarx) )
      varx = NULL;
   if( SCIPsetIsZero(scip->set, scalary) )
      vary = NULL;

   /* capture the special cases that less than two variables are left, due to resolutions to a fixed variable or
    * to the same active variable
    */
   if( varx == NULL && vary == NULL )
   {
      /* both variables were resolved to fixed variables */
      *infeasible = !SCIPsetIsZero(scip->set, rhs);
      *redundant = TRUE;
   }
   else if( varx == NULL )
   {
      assert(SCIPsetIsZero(scip->set, scalarx));
      assert(!SCIPsetIsZero(scip->set, scalary));

      /* variable x was resolved to fixed variable: variable y can be fixed to c'/b' */
      SCIP_CALL( SCIPvarFix(vary, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
            scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
            rhs/scalary, infeasible, aggregated) );
      *redundant = TRUE;
   }
   else if( vary == NULL )
   {
      assert(SCIPsetIsZero(scip->set, scalary));
      assert(!SCIPsetIsZero(scip->set, scalarx));

      /* variable y was resolved to fixed variable: variable x can be fixed to c'/a' */
      SCIP_CALL( SCIPvarFix(varx, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
            scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
            rhs/scalarx, infeasible, aggregated) );
      *redundant = TRUE;
   }
   else if( varx == vary )
   {
      /* both variables were resolved to the same active problem variable: this variable can be fixed */
      scalarx += scalary;
      if( SCIPsetIsZero(scip->set, scalarx) )
      {
         /* left hand side of equality is zero: equality is potentially infeasible */
         *infeasible = !SCIPsetIsZero(scip->set, rhs);
      }
      else
      {
         /* sum of scalars is not zero: fix variable x' == y' to c'/(a'+b') */
         SCIP_CALL( SCIPvarFix(varx, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
               scip->primal, scip->tree, scip->reopt, scip->lp, scip->branchcand, scip->eventqueue, scip->cliquetable,
               rhs/scalarx, infeasible, aggregated) );
      }
      *redundant = TRUE;
   }
   else
   {
      /* both variables are different active problem variables, and both scalars are non-zero: try to aggregate them */
      SCIP_CALL( SCIPvarTryAggregateVars(scip->set, scip->mem->probmem, scip->stat, scip->transprob, scip->origprob,
            scip->primal, scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventfilter,
            scip->eventqueue, varx, vary, scalarx, scalary, rhs, infeasible, aggregated) );
      *redundant = *aggregated;
   }

   return SCIP_OKAY;
}

/** converts variable into multi-aggregated variable; this changes the variable array returned from
 *  SCIPgetVars() and SCIPgetVarsData();
 *
 *  @warning The integrality condition is not checked anymore on the multi-aggregated variable. You must not
 *           multi-aggregate an integer variable without being sure, that integrality on the aggregation variables
 *           implies integrality on the aggregated variable.
 *
 *  The output flags have the following meaning:
 *  - infeasible: the problem is infeasible
 *  - aggregated: the aggregation was successfully performed (the variables were not aggregated before)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can only be called if @p scip is in stage \ref SCIP_STAGE_PRESOLVING
 */
SCIP_RETCODE SCIPmultiaggregateVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable x to aggregate */
   int                   naggvars,           /**< number n of variables in aggregation x = a_1*y_1 + ... + a_n*y_n + c */
   SCIP_VAR**            aggvars,            /**< variables y_i in aggregation x = a_1*y_1 + ... + a_n*y_n + c */
   SCIP_Real*            scalars,            /**< multipliers a_i in aggregation x = a_1*y_1 + ... + a_n*y_n + c */
   SCIP_Real             constant,           /**< constant shift c in aggregation x = a_1*y_1 + ... + a_n*y_n + c */
   SCIP_Bool*            infeasible,         /**< pointer to store whether the aggregation is infeasible */
   SCIP_Bool*            aggregated          /**< pointer to store whether the aggregation was successful */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPmultiaggregateVar", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   assert(var->scip == scip);

   if( SCIPtreeProbing(scip->tree) )
   {
      SCIPerrorMessage("cannot multi-aggregate variables during probing\n");
      return SCIP_INVALIDCALL;
   }
   assert(SCIPtreeGetCurrentDepth(scip->tree) == 0);

   SCIP_CALL( SCIPvarMultiaggregate(var, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->origprob,
         scip->primal, scip->tree, scip->reopt, scip->lp, scip->cliquetable, scip->branchcand, scip->eventfilter,
         scip->eventqueue, naggvars, aggvars, scalars, constant, infeasible, aggregated) );

   return SCIP_OKAY;
}

/** returns whether aggregation of variables is not allowed */
SCIP_Bool SCIPdoNotAggr(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   return scip->set->presol_donotaggr;
}

/** returns whether multi-aggregation is disabled */
SCIP_Bool SCIPdoNotMultaggr(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   return scip->set->presol_donotmultaggr;
}

/** returns whether variable is not allowed to be multi-aggregated */
SCIP_Bool SCIPdoNotMultaggrVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable x to aggregate */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(var->scip == scip);

   return scip->set->presol_donotmultaggr || SCIPvarDoNotMultaggr(var);
}

/** returns whether dual reductions are allowed during propagation and presolving
 *
 *  @note A reduction is called dual, if it may discard feasible solutions, but leaves at least one optimal solution
 *        intact. Often such reductions are based on analyzing the objective function, reduced costs, and/or dual LPs.
 */
SCIP_Bool SCIPallowDualReds(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   return !scip->set->reopt_enable && scip->set->misc_allowdualreds;
}

/** returns whether propagation w.r.t. current objective is allowed */
SCIP_Bool SCIPallowObjProp(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   return !scip->set->reopt_enable && scip->set->misc_allowobjprop;
}

/** modifies an initial seed value with the global shift of random seeds */
unsigned int SCIPinitializeRandomSeed(
   SCIP*                 scip,               /**< SCIP data structure */
   unsigned int          initialseedvalue    /**< initial seed value to be modified */
   )
{
   assert(scip != NULL);

   return SCIPsetInitializeRandomSeed(scip->set, initialseedvalue);
}

/** marks the variable that it must not be multi-aggregated
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *
 *  @note There exists no "unmark" method since it has to be ensured that if a plugin requires that a variable is not
 *        multi-aggregated that this is will be the case.
 */
SCIP_RETCODE SCIPmarkDoNotMultaggrVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to delete */
   )
{
   assert(scip != NULL);
   assert(var != NULL);
   assert(var->scip == scip);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPmarkDoNotMultaggrVar", TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE) );

   SCIP_CALL( SCIPvarMarkDoNotMultaggr(var) );

   return SCIP_OKAY;
}

/** enables the collection of statistics for a variable
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPenableVarHistory(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPenableVarHistory", FALSE, TRUE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPstatEnableVarHistory(scip->stat);
}

/** disables the collection of any statistic for a variable
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPdisableVarHistory(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPdisableVarHistory", FALSE, TRUE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPstatDisableVarHistory(scip->stat);
}

/** updates the pseudo costs of the given variable and the global pseudo costs after a change of "solvaldelta" in the
 *  variable's solution value and resulting change of "objdelta" in the in the LP's objective value;
 *  the update is ignored, if the objective value difference is infinite
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPupdateVarPseudocost(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             solvaldelta,        /**< difference of variable's new LP value - old LP value */
   SCIP_Real             objdelta,           /**< difference of new LP's objective value - old LP's objective value */
   SCIP_Real             weight              /**< weight in (0,1] of this update in pseudo cost sum */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPupdateVarPseudocost", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( !SCIPsetIsInfinity(scip->set, 2*objdelta) ) /* differences  infinity - eps  should also be treated as infinity */
   {
      if( scip->set->branch_divingpscost || (!scip->lp->diving && !SCIPtreeProbing(scip->tree)) )
      {
         SCIP_CALL( SCIPvarUpdatePseudocost(var, scip->set, scip->stat, solvaldelta, objdelta, weight) );
      }
   }

   return SCIP_OKAY;
}

/** gets the variable's pseudo cost value for the given change of the variable's LP value
 *
 *  @return the variable's pseudo cost value for the given change of the variable's LP value
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             solvaldelta         /**< difference of variable's new LP value - old LP value */
   )
{
   assert( var->scip == scip );

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostVal", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPvarGetPseudocost(var, scip->stat, solvaldelta);
}

/** gets the variable's pseudo cost value for the given change of the variable's LP value,
 *  only using the pseudo cost information of the current run
 *
 *  @return the variable's pseudo cost value for the given change of the variable's LP value,
 *  only using the pseudo cost information of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostValCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             solvaldelta         /**< difference of variable's new LP value - old LP value */
   )
{
   assert( var->scip == scip );

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostValCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPvarGetPseudocostCurrentRun(var, scip->stat, solvaldelta);
}

/** gets the variable's pseudo cost value for the given direction
 *
 *  @return the variable's pseudo cost value for the given direction
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocost(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocost", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );
   assert(dir == SCIP_BRANCHDIR_DOWNWARDS || dir == SCIP_BRANCHDIR_UPWARDS);
   assert(var->scip == scip);

   return SCIPvarGetPseudocost(var, scip->stat, dir == SCIP_BRANCHDIR_DOWNWARDS ? -1.0 : 1.0);
}

/** gets the variable's pseudo cost value for the given direction,
 *  only using the pseudo cost information of the current run
 *
 *  @return the variable's pseudo cost value for the given direction,
 *  only using the pseudo cost information of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );
   assert(dir == SCIP_BRANCHDIR_DOWNWARDS || dir == SCIP_BRANCHDIR_UPWARDS);
   assert(var->scip == scip);

   return SCIPvarGetPseudocostCurrentRun(var, scip->stat, dir == SCIP_BRANCHDIR_DOWNWARDS ? -1.0 : 1.0);
}

/** gets the variable's (possible fractional) number of pseudo cost updates for the given direction
 *
 *  @return the variable's (possible fractional) number of pseudo cost updates for the given direction
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostCount(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostCount", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );
   assert(dir == SCIP_BRANCHDIR_DOWNWARDS || dir == SCIP_BRANCHDIR_UPWARDS);
   assert(var->scip == scip);

   return SCIPvarGetPseudocostCount(var, dir);
}

/** gets the variable's (possible fractional) number of pseudo cost updates for the given direction,
 *  only using the pseudo cost information of the current run
 *
 *  @return the variable's (possible fractional) number of pseudo cost updates for the given direction,
 *  only using the pseudo cost information of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostCountCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostCountCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );
   assert(dir == SCIP_BRANCHDIR_DOWNWARDS || dir == SCIP_BRANCHDIR_UPWARDS);
   assert(var->scip == scip);

   return SCIPvarGetPseudocostCountCurrentRun(var, dir);
}

/** get pseudo cost variance of the variable, either for entire solve or only for current branch and bound run
 *
 *  @return returns the (corrected) variance of pseudo code information collected so far.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostVariance(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir,                /**< branching direction (downwards, or upwards) */
   SCIP_Bool             onlycurrentrun      /**< only for pseudo costs of current branch and bound run */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostVariance", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );
   assert(dir == SCIP_BRANCHDIR_DOWNWARDS || dir == SCIP_BRANCHDIR_UPWARDS);
   assert(var->scip == scip);

   return SCIPvarGetPseudocostVariance(var, dir, onlycurrentrun);
}

/** calculates a confidence bound for this variable under the assumption of normally distributed pseudo costs
 *
 *  The confidence bound \f$ \theta \geq 0\f$ denotes the interval borders \f$ [X - \theta, \ X + \theta]\f$, which contains
 *  the true pseudo costs of the variable, i.e., the expected value of the normal distribution, with a probability
 *  of 2 * clevel - 1.
 *
 *  @return value of confidence bound for this variable
 */
SCIP_Real SCIPcalculatePscostConfidenceBound(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable in question */
   SCIP_BRANCHDIR        dir,                /**< the branching direction for the confidence bound */
   SCIP_Bool             onlycurrentrun,     /**< should only the current run be taken into account */
   SCIP_CONFIDENCELEVEL  clevel              /**< confidence level for the interval */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPcalculatePscostConfidenceBound", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPvarCalcPscostConfidenceBound(var, scip->set, dir, onlycurrentrun, clevel);
}

/** check if variable pseudo-costs have a significant difference in location. The significance depends on
 *  the choice of \p clevel and on the kind of tested hypothesis. The one-sided hypothesis, which
 *  should be rejected, is that fracy * mu_y >= fracx * mu_x, where mu_y and mu_x denote the
 *  unknown location means of the underlying pseudo-cost distributions of x and y.
 *
 *  This method is applied best if variable x has a better pseudo-cost score than y. The method hypothesizes that y were actually
 *  better than x (despite the current information), meaning that y can be expected to yield branching
 *  decisions as least as good as x in the long run. If the method returns TRUE, the current history information is
 *  sufficient to safely rely on the alternative hypothesis that x yields indeed a better branching score (on average)
 *  than y.
 *
 *  @note The order of x and y matters for the one-sided hypothesis
 *
 *  @note set \p onesided to FALSE if you are not sure which variable is better. The hypothesis tested then reads
 *        fracy * mu_y == fracx * mu_x vs the alternative hypothesis fracy * mu_y != fracx * mu_x.
 *
 *  @return TRUE if the hypothesis can be safely rejected at the given confidence level
 */
SCIP_Bool SCIPsignificantVarPscostDifference(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             varx,               /**< variable x */
   SCIP_Real             fracx,              /**< the fractionality of variable x */
   SCIP_VAR*             vary,               /**< variable y */
   SCIP_Real             fracy,              /**< the fractionality of variable y */
   SCIP_BRANCHDIR        dir,                /**< branching direction */
   SCIP_CONFIDENCELEVEL  clevel,             /**< confidence level for rejecting hypothesis */
   SCIP_Bool             onesided            /**< should a one-sided hypothesis y >= x be tested? */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPsignificantVarPscostDifference", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPvarSignificantPscostDifference(scip->set, scip->stat, varx, fracx, vary, fracy, dir, clevel, onesided);
}

/** tests at a given confidence level whether the variable pseudo-costs only have a small probability to
 *  exceed a \p threshold. This is useful to determine if past observations provide enough evidence
 *  to skip an expensive strong-branching step if there is already a candidate that has been proven to yield an improvement
 *  of at least \p threshold.
 *
 *  @note use \p clevel to adjust the level of confidence. For SCIP_CONFIDENCELEVEL_MIN, the method returns TRUE if
 *        the estimated probability to exceed \p threshold is less than 25 %.
 *
 *  @see  SCIP_Confidencelevel for a list of available levels. The used probability limits refer to the one-sided levels
 *        of confidence.
 *
 *  @return TRUE if the variable pseudo-cost probabilistic model is likely to be smaller than \p threshold
 *          at the given confidence level \p clevel.
 */
SCIP_Bool SCIPpscostThresholdProbabilityTest(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable x */
   SCIP_Real             frac,               /**< the fractionality of variable x */
   SCIP_Real             threshold,          /**< the threshold to test against */
   SCIP_BRANCHDIR        dir,                /**< branching direction */
   SCIP_CONFIDENCELEVEL  clevel              /**< confidence level for rejecting hypothesis */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPpscostThresholdProbabilityTest", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPvarPscostThresholdProbabilityTest(scip->set, scip->stat, var, frac, threshold, dir, clevel);
}

/** check if the current pseudo cost relative error in a direction violates the given threshold. The Relative
 *  Error is calculated at a specific confidence level
 *
 *  @return TRUE if relative error in variable pseudo costs is smaller than \p threshold
 */
SCIP_Bool SCIPisVarPscostRelerrorReliable(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable in question */
   SCIP_Real             threshold,          /**< threshold for relative errors to be considered reliable (enough) */
   SCIP_CONFIDENCELEVEL  clevel              /**< a given confidence level */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPisVarPscostRelerrorReliable", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPvarIsPscostRelerrorReliable(var, scip->set, scip->stat, threshold, clevel);
}

/** gets the variable's pseudo cost score value for the given LP solution value
 *
 *  @return the variable's pseudo cost score value for the given LP solution value
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostScore(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             solval              /**< variable's LP solution value */
   )
{
   SCIP_Real downsol;
   SCIP_Real upsol;
   SCIP_Real pscostdown;
   SCIP_Real pscostup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostScore", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   downsol = SCIPsetFeasCeil(scip->set, solval-1.0);
   upsol = SCIPsetFeasFloor(scip->set, solval+1.0);
   pscostdown = SCIPvarGetPseudocost(var, scip->stat, downsol-solval);
   pscostup = SCIPvarGetPseudocost(var, scip->stat, upsol-solval);

   return SCIPbranchGetScore(scip->set, var, pscostdown, pscostup);
}

/** gets the variable's pseudo cost score value for the given LP solution value,
 *  only using the pseudo cost information of the current run
 *
 *  @return the variable's pseudo cost score value for the given LP solution value,
 *  only using the pseudo cost information of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarPseudocostScoreCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             solval              /**< variable's LP solution value */
   )
{
   SCIP_Real downsol;
   SCIP_Real upsol;
   SCIP_Real pscostdown;
   SCIP_Real pscostup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarPseudocostScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   downsol = SCIPsetFeasCeil(scip->set, solval-1.0);
   upsol = SCIPsetFeasFloor(scip->set, solval+1.0);
   pscostdown = SCIPvarGetPseudocostCurrentRun(var, scip->stat, downsol-solval);
   pscostup = SCIPvarGetPseudocostCurrentRun(var, scip->stat, upsol-solval);

   return SCIPbranchGetScore(scip->set, var, pscostdown, pscostup);
}

/** returns the variable's VSIDS value
 *
 *  @return the variable's VSIDS value
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarVSIDS(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarVSIDS", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   if( dir != SCIP_BRANCHDIR_DOWNWARDS && dir != SCIP_BRANCHDIR_UPWARDS )
   {
      SCIPerrorMessage("invalid branching direction %d when asking for VSIDS value\n", dir);
      return SCIP_INVALID;
   }

   return SCIPvarGetVSIDS(var, scip->stat, dir);
}

/** returns the variable's VSIDS value only using conflicts of the current run
 *
 *  @return the variable's VSIDS value only using conflicts of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarVSIDSCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarVSIDSCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   if( dir != SCIP_BRANCHDIR_DOWNWARDS && dir != SCIP_BRANCHDIR_UPWARDS )
   {
      SCIPerrorMessage("invalid branching direction %d when asking for VSIDS value\n", dir);
      return SCIP_INVALID;
   }

   return SCIPvarGetVSIDSCurrentRun(var, scip->stat, dir);
}

/** returns the variable's conflict score value
 *
 *  @return the variable's conflict score value
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarConflictScore(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real downscore;
   SCIP_Real upscore;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarConflictScore", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   downscore = SCIPvarGetVSIDS(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   upscore = SCIPvarGetVSIDS(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, downscore, upscore);
}

/** returns the variable's conflict score value only using conflicts of the current run
 *
 *  @return the variable's conflict score value only using conflicts of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarConflictScoreCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real downscore;
   SCIP_Real upscore;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarConflictScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   downscore = SCIPvarGetVSIDSCurrentRun(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   upscore = SCIPvarGetVSIDSCurrentRun(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, downscore, upscore);
}

/** returns the variable's conflict length score
 *
 *  @return the variable's conflict length score
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarConflictlengthScore(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real downscore;
   SCIP_Real upscore;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarConflictlengthScore", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   downscore = SCIPvarGetAvgConflictlength(var, SCIP_BRANCHDIR_DOWNWARDS);
   upscore = SCIPvarGetAvgConflictlength(var, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, downscore, upscore);
}

/** returns the variable's conflict length score only using conflicts of the current run
 *
 *  @return the variable's conflict length score only using conflicts of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarConflictlengthScoreCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real downscore;
   SCIP_Real upscore;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarConflictlengthScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   downscore = SCIPvarGetAvgConflictlengthCurrentRun(var, SCIP_BRANCHDIR_DOWNWARDS);
   upscore = SCIPvarGetAvgConflictlengthCurrentRun(var, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, downscore, upscore);
}

/** returns the variable's average conflict length
 *
 *  @return the variable's average conflict length
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgConflictlength(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgConflictlength", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   return SCIPvarGetAvgConflictlength(var, dir);
}

/** returns the variable's average  conflict length only using conflicts of the current run
 *
 *  @return the variable's average conflict length only using conflicts of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgConflictlengthCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgConflictlengthCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   return SCIPvarGetAvgConflictlengthCurrentRun(var, dir);
}

/** returns the average number of inferences found after branching on the variable in given direction;
 *  if branching on the variable in the given direction was yet evaluated, the average number of inferences
 *  over all variables for branching in the given direction is returned
 *
 *  @return the average number of inferences found after branching on the variable in given direction
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgInferences(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgInferences", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   return SCIPvarGetAvgInferences(var, scip->stat, dir);
}

/** returns the average number of inferences found after branching on the variable in given direction in the current run;
 *  if branching on the variable in the given direction was yet evaluated, the average number of inferences
 *  over all variables for branching in the given direction is returned
 *
 *  @return the average number of inferences found after branching on the variable in given direction in the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgInferencesCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgInferencesCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   return SCIPvarGetAvgInferencesCurrentRun(var, scip->stat, dir);
}

/** returns the variable's average inference score value
 *
 *  @return the variable's average inference score value
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgInferenceScore(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real inferdown;
   SCIP_Real inferup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgInferenceScore", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   inferdown = SCIPvarGetAvgInferences(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   inferup = SCIPvarGetAvgInferences(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, inferdown, inferup);
}

/** returns the variable's average inference score value only using inferences of the current run
 *
 *  @return the variable's average inference score value only using inferences of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgInferenceScoreCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real inferdown;
   SCIP_Real inferup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgInferenceScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   inferdown = SCIPvarGetAvgInferencesCurrentRun(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   inferup = SCIPvarGetAvgInferencesCurrentRun(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, inferdown, inferup);
}

/** initializes the upwards and downwards pseudocosts, conflict scores, conflict lengths, inference scores, cutoff scores
 *  of a variable to the given values
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPinitVarBranchStats(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable which should be initialized */
   SCIP_Real             downpscost,         /**< value to which pseudocosts for downwards branching should be initialized */
   SCIP_Real             uppscost,           /**< value to which pseudocosts for upwards branching should be initialized */
   SCIP_Real             downvsids,          /**< value to which VSIDS score for downwards branching should be initialized */
   SCIP_Real             upvsids,            /**< value to which VSIDS score for upwards branching should be initialized */
   SCIP_Real             downconflen,        /**< value to which conflict length score for downwards branching should be initialized */
   SCIP_Real             upconflen,          /**< value to which conflict length score for upwards branching should be initialized */
   SCIP_Real             downinfer,          /**< value to which inference counter for downwards branching should be initialized */
   SCIP_Real             upinfer,            /**< value to which inference counter for upwards branching should be initialized */
   SCIP_Real             downcutoff,         /**< value to which cutoff counter for downwards branching should be initialized */
   SCIP_Real             upcutoff            /**< value to which cutoff counter for upwards branching should be initialized */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPinitVarBranchStats", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert(downpscost >= 0.0 && uppscost >= 0.0);
   assert(downvsids >= 0.0 && upvsids >= 0.0);
   assert(downconflen >= 0.0 && upconflen >= 0.0);
   assert(downinfer >= 0.0 && upinfer >= 0.0);
   assert(downcutoff >= 0.0 && upcutoff >= 0.0);

   if( !SCIPisFeasZero(scip, downpscost) || !SCIPisFeasZero(scip, downvsids)
      || !SCIPisFeasZero(scip, downinfer) || !SCIPisFeasZero(scip, downcutoff) )
   {
      SCIP_CALL( SCIPvarIncNBranchings(var, NULL, NULL, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, SCIP_UNKNOWN, 1) );
      SCIP_CALL( SCIPvarUpdatePseudocost(var, scip->set, scip->stat, -1.0, downpscost, 1.0) );
      SCIP_CALL( SCIPvarIncInferenceSum(var,  NULL, NULL, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, SCIP_UNKNOWN, downinfer) );
      SCIP_CALL( SCIPvarIncVSIDS(var, NULL, scip->set, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, SCIP_UNKNOWN, downvsids) );
      SCIP_CALL( SCIPvarIncCutoffSum(var, NULL, NULL, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, SCIP_UNKNOWN, downcutoff) );
   }

   if( !SCIPisFeasZero(scip, downconflen) )
   {
      SCIP_CALL( SCIPvarIncNActiveConflicts(var, NULL, NULL, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, SCIP_UNKNOWN, downconflen) );
   }

   if( !SCIPisFeasZero(scip, uppscost) || !SCIPisFeasZero(scip, upvsids)
      || !SCIPisFeasZero(scip, upinfer) || !SCIPisFeasZero(scip, upcutoff) )
   {
      SCIP_CALL( SCIPvarIncNBranchings(var, NULL, NULL, scip->stat, SCIP_BRANCHDIR_UPWARDS, SCIP_UNKNOWN, 1) );
      SCIP_CALL( SCIPvarUpdatePseudocost(var, scip->set, scip->stat, 1.0, uppscost, 1.0) );
      SCIP_CALL( SCIPvarIncInferenceSum(var, NULL, NULL, scip->stat, SCIP_BRANCHDIR_UPWARDS, SCIP_UNKNOWN, upinfer) );
      SCIP_CALL( SCIPvarIncVSIDS(var, NULL, scip->set, scip->stat, SCIP_BRANCHDIR_UPWARDS, SCIP_UNKNOWN, upvsids) );
      SCIP_CALL( SCIPvarIncCutoffSum(var, NULL, NULL, scip->stat, SCIP_BRANCHDIR_UPWARDS, SCIP_UNKNOWN, upcutoff) );
   }

   if( !SCIPisFeasZero(scip, upconflen) )
   {
      SCIP_CALL( SCIPvarIncNActiveConflicts(var, NULL, NULL, scip->stat, SCIP_BRANCHDIR_UPWARDS, SCIP_UNKNOWN, upconflen) );
   }

   return SCIP_OKAY;
}

/** initializes the upwards and downwards conflict scores, conflict lengths, inference scores, cutoff scores of a
 *  variable w.r.t. a value by the given values (SCIP_VALUEHISTORY)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPinitVarValueBranchStats(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< variable which should be initialized */
   SCIP_Real             value,              /**< domain value, or SCIP_UNKNOWN */
   SCIP_Real             downvsids,          /**< value to which VSIDS score for downwards branching should be initialized */
   SCIP_Real             upvsids,            /**< value to which VSIDS score for upwards branching should be initialized */
   SCIP_Real             downconflen,        /**< value to which conflict length score for downwards branching should be initialized */
   SCIP_Real             upconflen,          /**< value to which conflict length score for upwards branching should be initialized */
   SCIP_Real             downinfer,          /**< value to which inference counter for downwards branching should be initialized */
   SCIP_Real             upinfer,            /**< value to which inference counter for upwards branching should be initialized */
   SCIP_Real             downcutoff,         /**< value to which cutoff counter for downwards branching should be initialized */
   SCIP_Real             upcutoff            /**< value to which cutoff counter for upwards branching should be initialized */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPinitVarValueBranchStats", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert(downvsids >= 0.0 && upvsids >= 0.0);
   assert(downconflen >= 0.0 && upconflen >= 0.0);
   assert(downinfer >= 0.0 && upinfer >= 0.0);
   assert(downcutoff >= 0.0 && upcutoff >= 0.0);

   if( !SCIPisFeasZero(scip, downvsids) || !SCIPisFeasZero(scip, downinfer) || !SCIPisFeasZero(scip, downcutoff) )
   {
      SCIP_CALL( SCIPvarIncNBranchings(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, value, 1) );
      SCIP_CALL( SCIPvarIncInferenceSum(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, value, downinfer) );
      SCIP_CALL( SCIPvarIncVSIDS(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, value, downvsids) );
      SCIP_CALL( SCIPvarIncCutoffSum(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, value, downcutoff) );
   }

   if( !SCIPisFeasZero(scip, downconflen) )
   {
      SCIP_CALL( SCIPvarIncNActiveConflicts(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_DOWNWARDS, value, downconflen) );
   }

   if( !SCIPisFeasZero(scip, upvsids) || !SCIPisFeasZero(scip, upinfer) || !SCIPisFeasZero(scip, upcutoff) )
   {
      SCIP_CALL( SCIPvarIncNBranchings(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_UPWARDS, value, 1) );
      SCIP_CALL( SCIPvarIncInferenceSum(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_UPWARDS, value, upinfer) );
      SCIP_CALL( SCIPvarIncVSIDS(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_UPWARDS, value, upvsids) );
      SCIP_CALL( SCIPvarIncCutoffSum(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_UPWARDS, value, upcutoff) );
   }

   if( !SCIPisFeasZero(scip, upconflen) )
   {
      SCIP_CALL( SCIPvarIncNActiveConflicts(var, SCIPblkmem(scip), scip->set, scip->stat, SCIP_BRANCHDIR_UPWARDS, value, upconflen) );
   }

   return SCIP_OKAY;
}

/** returns the average number of cutoffs found after branching on the variable in given direction;
 *  if branching on the variable in the given direction was yet evaluated, the average number of cutoffs
 *  over all variables for branching in the given direction is returned
 *
 *  @return the average number of cutoffs found after branching on the variable in given direction
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgCutoffs(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgCutoffs", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   return SCIPvarGetAvgCutoffs(var, scip->stat, dir);
}

/** returns the average number of cutoffs found after branching on the variable in given direction in the current run;
 *  if branching on the variable in the given direction was yet evaluated, the average number of cutoffs
 *  over all variables for branching in the given direction is returned
 *
 *  @return the average number of cutoffs found after branching on the variable in given direction in the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgCutoffsCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgCutoffsCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   return SCIPvarGetAvgCutoffsCurrentRun(var, scip->stat, dir);
}

/** returns the variable's average cutoff score value
 *
 *  @return the variable's average cutoff score value
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgCutoffScore(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real cutoffdown;
   SCIP_Real cutoffup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgCutoffScore", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   cutoffdown = SCIPvarGetAvgCutoffs(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   cutoffup = SCIPvarGetAvgCutoffs(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, cutoffdown, cutoffup);
}

/** returns the variable's average cutoff score value, only using cutoffs of the current run
 *
 *  @return the variable's average cutoff score value, only using cutoffs of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgCutoffScoreCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< problem variable */
   )
{
   SCIP_Real cutoffdown;
   SCIP_Real cutoffup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgCutoffScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   cutoffdown = SCIPvarGetAvgCutoffsCurrentRun(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   cutoffup = SCIPvarGetAvgCutoffsCurrentRun(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var, cutoffdown, cutoffup);
}

/** returns the variable's average inference/cutoff score value, weighting the cutoffs of the variable with the given
 *  factor
 *
 *  @return the variable's average inference/cutoff score value
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgInferenceCutoffScore(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             cutoffweight        /**< factor to weigh average number of cutoffs in branching score */
   )
{
   SCIP_Real avginferdown;
   SCIP_Real avginferup;
   SCIP_Real avginfer;
   SCIP_Real inferdown;
   SCIP_Real inferup;
   SCIP_Real cutoffdown;
   SCIP_Real cutoffup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgInferenceCutoffScore", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   avginferdown = SCIPhistoryGetAvgInferences(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS);
   avginferup = SCIPhistoryGetAvgInferences(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS);
   avginfer = (avginferdown + avginferup)/2.0;
   inferdown = SCIPvarGetAvgInferences(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   inferup = SCIPvarGetAvgInferences(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);
   cutoffdown = SCIPvarGetAvgCutoffs(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   cutoffup = SCIPvarGetAvgCutoffs(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var,
      inferdown + cutoffweight * avginfer * cutoffdown, inferup + cutoffweight * avginfer * cutoffup);
}

/** returns the variable's average inference/cutoff score value, weighting the cutoffs of the variable with the given
 *  factor, only using inferences and cutoffs of the current run
 *
 *  @return the variable's average inference/cutoff score value, only using inferences and cutoffs of the current run
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetVarAvgInferenceCutoffScoreCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   SCIP_Real             cutoffweight        /**< factor to weigh average number of cutoffs in branching score */
   )
{
   SCIP_Real avginferdown;
   SCIP_Real avginferup;
   SCIP_Real avginfer;
   SCIP_Real inferdown;
   SCIP_Real inferup;
   SCIP_Real cutoffdown;
   SCIP_Real cutoffup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetVarAvgInferenceCutoffScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert( var->scip == scip );

   avginferdown = SCIPhistoryGetAvgInferences(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_DOWNWARDS);
   avginferup = SCIPhistoryGetAvgInferences(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_UPWARDS);
   avginfer = (avginferdown + avginferup)/2.0;
   inferdown = SCIPvarGetAvgInferencesCurrentRun(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   inferup = SCIPvarGetAvgInferencesCurrentRun(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);
   cutoffdown = SCIPvarGetAvgCutoffsCurrentRun(var, scip->stat, SCIP_BRANCHDIR_DOWNWARDS);
   cutoffup = SCIPvarGetAvgCutoffsCurrentRun(var, scip->stat, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, var,
      inferdown + cutoffweight * avginfer * cutoffdown, inferup + cutoffweight * avginfer * cutoffup);
}

/** outputs variable information to file stream via the message system
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note If the message handler is set to a NULL pointer nothing will be printed
 */
SCIP_RETCODE SCIPprintVar(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var,                /**< problem variable */
   FILE*                 file                /**< output file (or NULL for standard output) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintVar", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIP_CALL( SCIPvarPrint(var, scip->set, scip->messagehdlr, file) );

   return SCIP_OKAY;
}

/*
 * conflict analysis methods
 */


/*
 * constraint methods
 */

#undef SCIPmarkConsPropagate


/*
 * LP methods
 */

/*
 * LP column methods
 */


/*
 * LP row methods
 */


/*
 * NLP methods
 */


/*
 * NLP diving methods
 */

/**@name NLP Diving Methods */
/**@{ */

/**@} */


/*
 * NLP nonlinear row methods
 */


/**@name Expression tree methods */
/**@{ */

/** translate from one value of infinity to another
 *
 *  if val is >= infty1, then give infty2, else give val
 */
#define infty2infty(infty1, infty2, val) (val >= infty1 ? infty2 : val)


#undef infty2infty

/**@} */

/*
 * nonlinear methods
 */

/**@name Nonlinear Methods */
/**@{ */



/**@} */

/*
 * cutting plane methods
 */


/*
 * LP diving methods
 */





/*
 * probing methods
 */




/*
 * branching methods
 */






/*
 * primal solutions
 */

/** creates a primal solution, initialized to zero
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcreateSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateSol", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      SCIP_CALL( SCIPsolCreateOriginal(sol, scip->mem->probmem, scip->set, scip->stat, scip->origprob, scip->origprimal, NULL, heur) );
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
      SCIP_CALL( SCIPsolCreate(sol, scip->mem->probmem, scip->set, scip->stat, scip->primal, scip->tree, heur) );
      return SCIP_OKAY;

   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
   case SCIP_STAGE_FREETRANS:
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDDATA;
   }  /*lint !e788*/
}

/** creates a primal solution, initialized to the current LP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcreateLPSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateLPSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( !SCIPtreeHasCurrentNodeLP(scip->tree) )
   {
      SCIPerrorMessage("LP solution does not exist\n");
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolCreateLPSol(sol, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->primal,
         scip->tree, scip->lp, heur) );

   return SCIP_OKAY;
}

/** creates a primal solution, initialized to the current NLP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcreateNLPSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateNLPSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( !SCIPisNLPConstructed(scip) )
   {
      SCIPerrorMessage("NLP does not exist\n");
      return SCIP_INVALIDCALL;
   }
   assert(scip->nlp != NULL);

   if( !SCIPnlpHasSolution(scip->nlp) )
   {
      SCIPerrorMessage("NLP solution does not exist\n");
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolCreateNLPSol(sol, scip->mem->probmem, scip->set, scip->stat, scip->primal, scip->tree, scip->nlp,
         heur) );

   return SCIP_OKAY;
}

/** creates a primal solution, initialized to the current relaxation solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcreateRelaxSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateRelaxSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( !SCIPrelaxationIsSolValid(scip->relaxation) )
   {
      SCIPerrorMessage("relaxation solution is not valid\n");
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolCreateRelaxSol(sol, scip->mem->probmem, scip->set, scip->stat, scip->primal, scip->tree, scip->relaxation, heur) );

   return SCIP_OKAY;
}

/** creates a primal solution, initialized to the current pseudo solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcreatePseudoSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreatePseudoSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPsolCreatePseudoSol(sol, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->primal,
         scip->tree, scip->lp, heur) );

   return SCIP_OKAY;
}

/** creates a primal solution, initialized to the current LP or pseudo solution, depending on whether the LP was solved
 *  at the current node
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcreateCurrentSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateCurrentSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPsolCreateCurrentSol(sol, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->primal,
         scip->tree, scip->lp, heur) );

   return SCIP_OKAY;
}

/** creates a partial primal solution, initialized to unknown values
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 */
SCIP_RETCODE SCIPcreatePartialSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreatePartialSol", FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPsolCreatePartial(sol, scip->mem->probmem, scip->set, scip->stat, scip->origprimal, heur) );

   return SCIP_OKAY;
}

/** creates a primal solution, initialized to unknown values
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcreateUnknownSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateUnknownSol", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPsolCreateUnknown(sol, scip->mem->probmem, scip->set, scip->stat, scip->primal, scip->tree, heur) );

   return SCIP_OKAY;
}

/** creates a primal solution living in the original problem space, initialized to zero;
 *  a solution in original space allows to set original variables to values that would be invalid in the
 *  transformed problem due to preprocessing fixings or aggregations
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPcreateOrigSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_HEUR*            heur                /**< heuristic that found the solution (or NULL if it's from the tree) */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateOrigSol", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      SCIP_CALL( SCIPsolCreateOriginal(sol, scip->mem->probmem, scip->set, scip->stat, scip->origprob, scip->origprimal, NULL, heur) );
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
      SCIP_CALL( SCIPsolCreateOriginal(sol, scip->mem->probmem, scip->set, scip->stat, scip->origprob, scip->primal, scip->tree, heur) );
      return SCIP_OKAY;

   case SCIP_STAGE_EXITSOLVE:
   case SCIP_STAGE_FREETRANS:
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** creates a copy of a primal solution; note that a copy of a linked solution is also linked and needs to be unlinked
 *  if it should stay unaffected from changes in the LP or pseudo solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_FREETRANS
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPcreateSolCopy(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_SOL*             sourcesol           /**< primal CIP solution to copy */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateSolCopy", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* check if we want to copy the current solution, which is the same as creating a current solution */
   if( sourcesol == NULL )
   {
      SCIP_CALL( SCIPcreateCurrentSol(scip, sol, NULL) );
   }
   else
   {
      SCIP_CALL( SCIPsolCopy(sol, scip->mem->probmem, scip->set, scip->stat, scip->primal, sourcesol) );
   }

   return SCIP_OKAY;
}

/** creates a copy of a solution in the original primal solution space
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPcreateSolCopyOrig(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_SOL*             sourcesol           /**< primal CIP solution to copy */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateSolCopyOrig", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   /* check if we want to copy the current solution, which is the same as creating a current solution */
   if( sourcesol == NULL )
   {
      SCIP_CALL( SCIPcreateCurrentSol(scip, sol, NULL) );
   }
   else
   {
      switch( scip->set->stage )
      {
      case SCIP_STAGE_PROBLEM:
      case SCIP_STAGE_FREETRANS:
      case SCIP_STAGE_SOLVED:
      case SCIP_STAGE_TRANSFORMING:
      case SCIP_STAGE_TRANSFORMED:
      case SCIP_STAGE_INITPRESOLVE:
      case SCIP_STAGE_PRESOLVING:
      case SCIP_STAGE_EXITPRESOLVE:
      case SCIP_STAGE_PRESOLVED:
      case SCIP_STAGE_INITSOLVE:
      case SCIP_STAGE_SOLVING:
         SCIP_CALL( SCIPsolCopy(sol, scip->mem->probmem, scip->set, scip->stat, scip->origprimal, sourcesol) );
         break;
      default:
         assert(FALSE);
      }  /*lint !e788*/
   }

   return SCIP_OKAY;
}

/** helper method that sets up and solves the sub-SCIP for removing infinite values from solutions */
static
SCIP_RETCODE setupAndSolveFiniteSolSubscip(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP*                 subscip,            /**< SCIP data structure of sub-SCIP*/
   SCIP_VAR**            origvars,           /**< original problem variables of main SCIP */
   int                   norigvars,          /**< number of original problem variables of main SCIP */
   SCIP_Real*            solvals,            /**< array with solution values of variables; infinite ones are replaced */
   SCIP_Bool*            success             /**< pointer to store if removing infinite values was successful */
   )
{
   SCIP_HASHMAP* varmap;
   SCIP_VAR* varcopy;
   SCIP_Real fixval;
   SCIP_Bool valid;
   SCIP_SOL* bestsol;
   int v;

   assert(scip != NULL);
   assert(subscip != NULL);
   assert(origvars != NULL);
   assert(solvals != NULL);
   assert(success != NULL);

   /* copy the original problem to the sub-SCIP */
   SCIP_CALL( SCIPhashmapCreate(&varmap, SCIPblkmem(scip), norigvars) );
   SCIP_CALL( SCIPcopyOrig(scip, subscip, varmap, NULL, "removeinffixings", TRUE, TRUE, &valid) );

   SCIP_CALL( SCIPsetIntParam(subscip, "display/verblevel", (int)SCIP_VERBLEVEL_NONE) );

   /* in the sub-SCIP, we try to minimize the absolute values of all variables with infinite values in the solution
    * and fix all other variables to the value they have in the solution
    */
   for( v = 0; v < norigvars; ++v )
   {
      varcopy = (SCIP_VAR*) SCIPhashmapGetImage(varmap, (void*)origvars[v]);
      assert(varcopy != NULL);

      fixval = solvals[v];

      if( SCIPisInfinity(scip, fixval) || SCIPisInfinity(scip, -fixval) )
      {
         /* If a variable with a finite finite lower bound was set to +infinity, we just change its objective to 1.0
          * to minimize its value; if a variable with a finite finite upper bound was set to -infinity, we just
          * change its objective to -1.0 to maximize its value; if a variable is free, we split the variable into
          * positive and negative part by creating two new non-negative variables and one constraint linking those
          * variables.
          */
         if( SCIPisInfinity(scip, fixval) && !SCIPisInfinity(scip, -SCIPvarGetLbLocal(varcopy)) )
         {
            SCIP_CALL( SCIPchgVarObj(subscip, varcopy, 1.0) );
         }
         else if( SCIPisInfinity(scip, -fixval) && !SCIPisInfinity(scip, SCIPvarGetUbLocal(varcopy)) )
         {
            SCIP_CALL( SCIPchgVarObj(subscip, varcopy, -1.0) );
         }
         else
         {
            char name[SCIP_MAXSTRLEN];
            SCIP_VAR* posvar;
            SCIP_VAR* negvar;
            SCIP_CONS* linkcons;

            (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "%s_%s", SCIPvarGetName(varcopy), "run");
            SCIP_CALL( SCIPcreateVar(subscip, &posvar, name, 0.0, SCIPinfinity(scip), 1.0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL) );
            SCIP_CALL( SCIPaddVar(subscip, posvar) );

            (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "%s_%s", SCIPvarGetName(varcopy), "neg");
            SCIP_CALL( SCIPcreateVar(subscip, &negvar, name, 0.0, SCIPinfinity(scip), 1.0,
                  SCIP_VARTYPE_CONTINUOUS, TRUE, FALSE, NULL, NULL, NULL, NULL, NULL) );
            SCIP_CALL( SCIPaddVar(subscip, negvar) );

            (void) SCIPsnprintf(name, SCIP_MAXSTRLEN, "%s_%s", SCIPvarGetName(varcopy), "linkcons");
            SCIP_CALL( SCIPcreateConsBasicLinear(subscip, &linkcons, name, 0, NULL, NULL, 0.0, 0.0 ) );
            SCIP_CALL( SCIPaddCoefLinear(subscip, linkcons, varcopy, 1.0) );
            SCIP_CALL( SCIPaddCoefLinear(subscip, linkcons, posvar, -1.0) );
            SCIP_CALL( SCIPaddCoefLinear(subscip, linkcons, negvar, 1.0) );
            SCIP_CALL( SCIPaddCons(subscip, linkcons) );

            SCIP_CALL( SCIPreleaseCons(subscip, &linkcons) );
            SCIP_CALL( SCIPreleaseVar(subscip, &posvar) );
            SCIP_CALL( SCIPreleaseVar(subscip, &negvar) );

            SCIP_CALL( SCIPchgVarObj(subscip, varcopy, 0.0) );
         }
      }
      else
      {
         SCIP_Bool infeasible;
         SCIP_Bool fixed;

         if( SCIPisFeasLT(scip, solvals[v], SCIPvarGetLbLocal(varcopy)) || SCIPisFeasGT(scip, solvals[v], SCIPvarGetUbLocal(varcopy)) )
         {
            SCIP_CALL( SCIPchgVarType(subscip, varcopy, SCIP_VARTYPE_CONTINUOUS, &infeasible) );
            assert(!infeasible);
         }

         /* fix variable to its value in the solution */
         SCIP_CALL( SCIPfixVar(subscip, varcopy, fixval, &infeasible, &fixed) );
         assert(!infeasible);
      }
   }

   SCIP_CALL( SCIPsolve(subscip) );

   bestsol = SCIPgetBestSol(subscip);

   if( bestsol != NULL )
   {
      /* change the stored solution values for variables fixed to infinite values */
      for( v = 0; v < norigvars; ++v )
      {
         varcopy = (SCIP_VAR*) SCIPhashmapGetImage(varmap, (void*)origvars[v]);
         assert(varcopy != NULL);

         if( (SCIPisInfinity(scip, solvals[v]) || SCIPisInfinity(scip, -solvals[v])) )
         {
            solvals[v] = SCIPgetSolVal(subscip, bestsol, varcopy);
         }
      }
   }
   else
   {
      *success = FALSE;
   }

   SCIPhashmapFree(&varmap);

   return SCIP_OKAY;
}


/** creates a copy of a primal solution, thereby replacing infinite fixings of variables by finite values;
 *  the copy is always defined in the original variable space;
 *  success indicates whether the objective value of the solution was changed by removing infinite values
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPcreateFiniteSolCopy(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to store the solution */
   SCIP_SOL*             sourcesol,          /**< primal CIP solution to copy */
   SCIP_Bool*            success             /**< does the finite solution have the same objective value? */
   )
{
   SCIP_VAR** fixedvars;
   SCIP_VAR** origvars;
   SCIP_Real* solvals;
   SCIP_VAR* var;
   int nfixedvars;
   int norigvars;
   int v;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPcreateFiniteSolCopy", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   assert(scip != NULL);
   assert(sol != NULL);
   assert(sourcesol != NULL);
   assert(success != NULL);

   *success = TRUE;
   *sol = NULL;

   fixedvars = SCIPgetFixedVars(scip);
   nfixedvars = SCIPgetNFixedVars(scip);
   assert(fixedvars != NULL || nfixedvars == 0);

   /* get original variables and their values in the optimal solution */
   SCIP_CALL( SCIPgetOrigVarsData(scip, &origvars, &norigvars, NULL, NULL, NULL, NULL) );
   SCIP_CALL( SCIPallocBufferArray(scip, &solvals, norigvars) );
   SCIP_CALL( SCIPgetSolVals(scip, sourcesol, norigvars, origvars, solvals) );

   /* check whether there are variables fixed to an infinite value */
   for( v = 0; v < nfixedvars; ++v )
   {
      var = fixedvars[v]; /*lint !e613*/

      /* skip (multi-)aggregated variables */
      if( SCIPvarGetStatus(var) != SCIP_VARSTATUS_FIXED )
         continue;

      assert(SCIPisEQ(scip, SCIPvarGetLbGlobal(var), SCIPvarGetUbGlobal(var)));

      if( (SCIPisInfinity(scip, SCIPvarGetLbGlobal(var)) || SCIPisInfinity(scip, -SCIPvarGetLbGlobal(var))) )
      {
         SCIPdebugMsg(scip, "var <%s> is fixed to infinite value %g\n", SCIPvarGetName(var), SCIPvarGetLbGlobal(var));
         break;
      }
   }

   /* there were variables fixed to infinite values */
   if( v < nfixedvars )
   {
      SCIP* subscip;
      SCIP_RETCODE retcode;

      /* if one of the variables was fixed to infinity in the original problem, we stop here */
      for( v = 0; v < norigvars; ++v )
      {
         var = origvars[v];

         if( SCIPisInfinity(scip, SCIPvarGetLbOriginal(var)) || SCIPisInfinity(scip, -SCIPvarGetUbOriginal(var)) )
         {
            assert(SCIPisEQ(scip, SCIPvarGetLbOriginal(var), SCIPvarGetUbOriginal(var)));

            SCIPdebugMsg(scip, "--> var <%s> is fixed to infinite value %g in the original problem, stop making solution finite\n",
               SCIPvarGetName(var), SCIPvarGetLbOriginal(var));

            *success = FALSE;

            goto TERMINATE;
         }
      }

      /* create sub-SCIP */
      SCIP_CALL( SCIPcreate(&subscip) );

      retcode = setupAndSolveFiniteSolSubscip(scip, subscip, origvars, norigvars, solvals, success);

      /* free sub-SCIP */
      SCIP_CALL( SCIPfree(&subscip) );

      SCIP_CALL( retcode );
   }

   /* create original solution and set the solution values */
   if( *success )
   {
      SCIP_CALL( SCIPcreateOrigSol(scip, sol, NULL) );
      for( v = 0; v < norigvars; ++v )
      {
         SCIP_CALL( SCIPsetSolVal(scip, *sol, origvars[v], solvals[v]) );
      }
   }

#ifdef SCIP_DEBUG
   SCIPdebugMsg(scip, "created finites solution copy:\n");
   SCIP_CALL( SCIPprintSol(scip, *sol, NULL, FALSE) );
#endif

   /* the solution of the sub-SCIP should have the same objective value */
   if( *success && !SCIPisEQ(scip, SCIPgetSolOrigObj(scip, *sol), SCIPgetSolOrigObj(scip, sourcesol)) )
   {
      /* @todo how should we avoid numerical trobles here for large objective values? */
      if( (SCIPgetSolOrigObj(scip, *sol) / SCIPepsilon(scip)) < 1e+15 ||
         REALABS(SCIPgetSolOrigObj(scip, *sol) - SCIPgetSolOrigObj(scip, sourcesol)) > 1e-12 * SCIPgetSolOrigObj(scip, *sol) )
         *success = FALSE;
   }

 TERMINATE:
   SCIPfreeBufferArray(scip, &solvals);

   return SCIP_OKAY;
}

/** frees primal CIP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPfreeSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol                 /**< pointer to the solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPfreeSol", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      SCIP_CALL( SCIPsolFree(sol, scip->mem->probmem, scip->origprimal) );
      break;
   case SCIP_STAGE_FREETRANS:
   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
      SCIP_CALL( SCIPsolFree(sol, scip->mem->probmem, scip->primal) );
      break;
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/

   return SCIP_OKAY;
}

/** links a primal solution to the current LP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPlinkLPSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPlinkLPSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( !SCIPlpIsSolved(scip->lp) )
   {
      SCIPerrorMessage("LP solution does not exist\n");
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolLinkLPSol(sol, scip->set, scip->stat, scip->transprob, scip->tree, scip->lp) );

   return SCIP_OKAY;
}

/** links a primal solution to the current NLP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPlinkNLPSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPlinkNLPSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( scip->nlp == NULL )
   {
      SCIPerrorMessage("NLP does not exist\n");
      return SCIP_INVALIDCALL;
   }

   if( SCIPnlpGetSolstat(scip->nlp) > SCIP_NLPSOLSTAT_FEASIBLE )
   {
      SCIPerrorMessage("NLP solution does not exist\n");
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolLinkNLPSol(sol, scip->stat, scip->tree, scip->nlp) );

   return SCIP_OKAY;
}

/** links a primal solution to the current relaxation solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPlinkRelaxSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPlinkRelaxSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( !SCIPrelaxationIsSolValid(scip->relaxation) )
   {
      SCIPerrorMessage("relaxation solution is not valid\n");
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolLinkRelaxSol(sol, scip->set, scip->stat, scip->tree, scip->relaxation) );

   return SCIP_OKAY;
}

/** links a primal solution to the current pseudo solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPlinkPseudoSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPlinkPseudoSol", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPsolLinkPseudoSol(sol, scip->set, scip->stat, scip->transprob, scip->tree, scip->lp) );

   return SCIP_OKAY;
}

/** links a primal solution to the current LP or pseudo solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPlinkCurrentSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPlinkCurrentSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPsolLinkCurrentSol(sol, scip->set, scip->stat, scip->transprob, scip->tree, scip->lp) );

   return SCIP_OKAY;
}

/** clears a primal solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPclearSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPclearSol", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIP_CALL( SCIPsolClear(sol, scip->stat, scip->tree) );

   return SCIP_OKAY;
}

/** stores solution values of variables in solution's own array
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPunlinkSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPunlinkSol", FALSE, FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIP_CALL( SCIPsolUnlink(sol, scip->set, scip->transprob) );

   return SCIP_OKAY;
}

/** sets value of variable in primal CIP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPsetSolVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution */
   SCIP_VAR*             var,                /**< variable to add to solution */
   SCIP_Real             val                 /**< solution value of variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetSolVal", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   assert( var->scip == scip );

   if( SCIPsolIsOriginal(sol) && SCIPvarIsTransformed(var) )
   {
      SCIPerrorMessage("cannot set value of transformed variable <%s> in original space solution\n",
         SCIPvarGetName(var));
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolSetVal(sol, scip->set, scip->stat, scip->tree, var, val) );

   return SCIP_OKAY;
}

/** sets values of multiple variables in primal CIP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPsetSolVals(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution */
   int                   nvars,              /**< number of variables to set solution value for */
   SCIP_VAR**            vars,               /**< array with variables to add to solution */
   SCIP_Real*            vals                /**< array with solution values of variables */
   )
{
   int v;

   assert(nvars == 0 || vars != NULL);
   assert(nvars == 0 || vals != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPsetSolVals", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   if( SCIPsolIsOriginal(sol) )
   {
      for( v = 0; v < nvars; ++v )
      {
         if( SCIPvarIsTransformed(vars[v]) )
         {
            SCIPerrorMessage("cannot set value of transformed variable <%s> in original space solution\n",
               SCIPvarGetName(vars[v]));
            return SCIP_INVALIDCALL;
         }
      }
   }

   for( v = 0; v < nvars; ++v )
   {
      SCIP_CALL( SCIPsolSetVal(sol, scip->set, scip->stat, scip->tree, vars[v], vals[v]) );
   }

   return SCIP_OKAY;
}

/** increases value of variable in primal CIP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPincSolVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution */
   SCIP_VAR*             var,                /**< variable to increase solution value for */
   SCIP_Real             incval              /**< increment for solution value of variable */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPincSolVal", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   assert( var->scip == scip );

   if( SCIPsolIsOriginal(sol) && SCIPvarIsTransformed(var) )
   {
      SCIPerrorMessage("cannot increase value of transformed variable <%s> in original space solution\n",
         SCIPvarGetName(var));
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolIncVal(sol, scip->set, scip->stat, scip->tree, var, incval) );

   return SCIP_OKAY;
}

/** returns value of variable in primal CIP solution, or in current LP/pseudo solution
 *
 *  @return value of variable in primal CIP solution, or in current LP/pseudo solution
 *
 *  @pre In case the solution pointer @p sol is @b NULL, that means it is asked for the LP or pseudo solution, this method
 *       can only be called if @p scip is in the solving stage \ref SCIP_STAGE_SOLVING. In any other case, this method
 *       can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Real SCIPgetSolVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution, or NULL for current LP/pseudo solution */
   SCIP_VAR*             var                 /**< variable to get value for */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolVal", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   assert( var->scip == scip );

   if( sol != NULL )
      return SCIPsolGetVal(sol, scip->set, scip->stat, var);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolVal(sol==NULL)", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPvarGetSol(var, SCIPtreeHasCurrentNodeLP(scip->tree));
}

/** gets values of multiple variables in primal CIP solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPgetSolVals(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution, or NULL for current LP/pseudo solution */
   int                   nvars,              /**< number of variables to get solution value for */
   SCIP_VAR**            vars,               /**< array with variables to get value for */
   SCIP_Real*            vals                /**< array to store solution values of variables */
   )
{
   assert(nvars == 0 || vars != NULL);
   assert(nvars == 0 || vals != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetSolVals", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   if( sol != NULL )
   {
      int v;

      for( v = 0; v < nvars; ++v )
         vals[v] = SCIPsolGetVal(sol, scip->set, scip->stat, vars[v]);
   }
   else
   {
      SCIP_CALL( SCIPgetVarSols(scip, nvars, vars, vals) );
   }

   return SCIP_OKAY;
}

/** returns objective value of primal CIP solution w.r.t. original problem, or current LP/pseudo objective value
 *
 *  @return objective value of primal CIP solution w.r.t. original problem, or current LP/pseudo objective value
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Real SCIPgetSolOrigObj(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution, or NULL for current LP/pseudo objective value */
   )
{
   /* for original solutions, an original objective value is already available in SCIP_STAGE_PROBLEM
    * for all other solutions, we should be at least in SCIP_STAGE_TRANSFORMING
    */
   if( sol != NULL && SCIPsolIsOriginal(sol) )
   {
      SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolOrigObj", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

      return SCIPsolGetOrigObj(sol);
   }

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolOrigObj", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   if( sol != NULL )
      return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, SCIPsolGetObj(sol, scip->set, scip->transprob, scip->origprob));
   else
   {
      SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolOrigObj(sol==NULL)", \
            FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );
      if( SCIPtreeHasCurrentNodeLP(scip->tree) )
         return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, SCIPlpGetObjval(scip->lp, scip->set, scip->transprob));
      else
         return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, SCIPlpGetPseudoObjval(scip->lp, scip->set, scip->transprob));
   }
}

/** returns transformed objective value of primal CIP solution, or transformed current LP/pseudo objective value
 *
 *  @return transformed objective value of primal CIP solution, or transformed current LP/pseudo objective value
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Real SCIPgetSolTransObj(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution, or NULL for current LP/pseudo objective value */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolTransObj", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   if( sol != NULL )
      return SCIPsolGetObj(sol, scip->set, scip->transprob, scip->origprob);
   else
   {
      SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolTransObj(sol==NULL)", \
            FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );
      if( SCIPtreeHasCurrentNodeLP(scip->tree) )
         return SCIPlpGetObjval(scip->lp, scip->set, scip->transprob);
      else
         return SCIPlpGetPseudoObjval(scip->lp, scip->set, scip->transprob);
   }
}

/** recomputes the objective value of an original solution, e.g., when transferring solutions
 *  from the solution pool (objective coefficients might have changed in the meantime)
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *
 */
SCIP_RETCODE SCIPrecomputeSolObj(
   SCIP*                 scip,
   SCIP_SOL*             sol
   )
{
   assert(scip != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPrecomputeSolObj", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPsolRecomputeObj(sol, scip->set, scip->stat, scip->origprob);

   return SCIP_OKAY;
}

/** maps original space objective value into transformed objective value
 *
 *  @return transformed objective value
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPtransformObj(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             obj                 /**< original space objective value to transform */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPtransformObj", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPprobInternObjval(scip->transprob, scip->origprob, scip->set, obj);
}

/** maps transformed objective value into original space
 *
 *  @return objective value into original space
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPretransformObj(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             obj                 /**< transformed objective value to retransform in original space */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPretransformObj", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, obj);
}

/** gets clock time, when this solution was found
 *
 *  @return clock time, when this solution was found
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Real SCIPgetSolTime(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolTime", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return SCIPsolGetTime(sol);
}

/** gets branch and bound run number, where this solution was found
 *
 *  @return branch and bound run number, where this solution was found
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
int SCIPgetSolRunnum(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolRunnum", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return SCIPsolGetRunnum(sol);
}

/** gets node number of the specific branch and bound run, where this solution was found
 *
 *  @return node number of the specific branch and bound run, where this solution was found
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetSolNodenum(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolNodenum", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return SCIPsolGetNodenum(sol);
}

/** gets heuristic, that found this solution (or NULL if it's from the tree)
 *
 *  @return heuristic, that found this solution (or NULL if it's from the tree)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_HEUR* SCIPgetSolHeur(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal solution */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolHeur", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return SCIPsolGetHeur(sol);
}

/** returns whether two given solutions are exactly equal
 *
 *  @return returns whether two given solutions are exactly equal
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Bool SCIPareSolsEqual(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol1,               /**< first primal CIP solution */
   SCIP_SOL*             sol2                /**< second primal CIP solution */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPareSolsEqual", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return SCIPsolsAreEqual(sol1, sol2, scip->set, scip->stat, scip->origprob, scip->transprob);
}

/** adjusts solution values of implicit integer variables in handed solution. Solution objective value is not
 *  deteriorated by this method.
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPadjustImplicitSolVals(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Bool             uselprows           /**< should LP row information be considered for none-objective variables */
   )
{
   assert(scip != NULL);
   SCIP_CALL( SCIPcheckStage(scip, "SCIPadjustImplicitSolVals", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert(sol != NULL);
   SCIP_CALL( SCIPsolAdjustImplicitSolVals(sol, scip->set, scip->stat, scip->transprob, scip->tree, uselprows) );

   return SCIP_OKAY;
}

/** outputs non-zero variables of solution in original problem space to the given file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre In case the solution pointer @p sol is NULL (asking for the current LP/pseudo solution), this method can be
 *       called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *
 *  @pre In case the solution pointer @p sol is @b not NULL, this method can be called if @p scip is in one of the
 *       following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPprintSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution, or NULL for current LP/pseudo solution */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_Bool             printzeros          /**< should variables set to zero be printed? */
   )
{
   SCIP_Real objvalue;
   SCIP_Bool currentsol;
   SCIP_Bool oldquiet = FALSE;

   assert(SCIPisTransformed(scip) || sol != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintSol", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   currentsol = (sol == NULL);
   if( currentsol )
   {
      SCIP_CALL( SCIPcheckStage(scip, "SCIPprintSol(sol==NULL)", \
            FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

      /* create a temporary solution that is linked to the current solution */
      SCIP_CALL( SCIPsolCreateCurrentSol(&sol, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->primal,
            scip->tree, scip->lp, NULL) );
   }

   if( file != NULL && scip->messagehdlr != NULL )
   {
      oldquiet = SCIPmessagehdlrIsQuiet(scip->messagehdlr);
      SCIPmessagehdlrSetQuiet(scip->messagehdlr, FALSE);
   }

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "objective value:                 ");

   if( SCIPsolIsPartial(sol) )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "unknown\n");
   }
   else
   {
      if( SCIPsolIsOriginal(sol) )
         objvalue = SCIPsolGetOrigObj(sol);
      else
         objvalue = SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, SCIPsolGetObj(sol, scip->set, scip->transprob, scip->origprob));

      SCIPprintReal(scip, file, objvalue, 20, 15);
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "\n");
   }

   SCIP_CALL( SCIPsolPrint(sol, scip->set, scip->messagehdlr, scip->stat, scip->origprob, scip->transprob, file, FALSE,
         printzeros) );

   if( file != NULL && scip->messagehdlr != NULL )
   {
      SCIPmessagehdlrSetQuiet(scip->messagehdlr, oldquiet);
   }

   if( currentsol )
   {
      /* free temporary solution */
      SCIP_CALL( SCIPsolFree(&sol, scip->mem->probmem, scip->primal) );
   }

   return SCIP_OKAY;
}

/** outputs non-zero variables of solution in transformed problem space to file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPprintTransSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution, or NULL for current LP/pseudo solution */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_Bool             printzeros          /**< should variables set to zero be printed? */
   )
{
   SCIP_Bool currentsol;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintTransSol", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   currentsol = (sol == NULL);
   if( currentsol )
   {
      /* create a temporary solution that is linked to the current solution */
      SCIP_CALL( SCIPsolCreateCurrentSol(&sol, scip->mem->probmem, scip->set, scip->stat, scip->transprob, scip->primal,
            scip->tree, scip->lp, NULL) );
   }

   if( SCIPsolIsOriginal(sol) )
   {
      SCIPerrorMessage("cannot print original space solution as transformed solution\n");
      return SCIP_INVALIDCALL;
   }

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "objective value:                 ");
   SCIPprintReal(scip, file, SCIPsolGetObj(sol, scip->set, scip->transprob, scip->origprob), 20, 9);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "\n");

   SCIP_CALL( SCIPsolPrint(sol, scip->set, scip->messagehdlr, scip->stat, scip->transprob, NULL, file, FALSE, printzeros) );

   if( currentsol )
   {
      /* free temporary solution */
      SCIP_CALL( SCIPsolFree(&sol, scip->mem->probmem, scip->primal) );
   }

   return SCIP_OKAY;
}

/** outputs discrete variables of solution in original problem space to the given file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPprintMIPStart(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution */
   FILE*                 file                /**< output file (or NULL for standard output) */
   )
{
   SCIP_Real objvalue;
   SCIP_Bool oldquiet = FALSE;

   assert(sol != NULL);
   assert(!SCIPsolIsPartial(sol));

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintMIPStart", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   if( file != NULL && scip->messagehdlr != NULL )
   {
      oldquiet = SCIPmessagehdlrIsQuiet(scip->messagehdlr);
      SCIPmessagehdlrSetQuiet(scip->messagehdlr, FALSE);
   }

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "objective value:                 ");

   if( SCIPsolIsOriginal(sol) )
      objvalue = SCIPsolGetOrigObj(sol);
   else
      objvalue = SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, SCIPsolGetObj(sol, scip->set, scip->transprob, scip->origprob));

   SCIPprintReal(scip, file, objvalue, 20, 15);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "\n");

   SCIP_CALL( SCIPsolPrint(sol, scip->set, scip->messagehdlr, scip->stat, scip->origprob, scip->transprob, file, TRUE,
         TRUE) );

   if( file != NULL && scip->messagehdlr != NULL )
   {
      SCIPmessagehdlrSetQuiet(scip->messagehdlr, oldquiet);
   }

   return SCIP_OKAY;
}

/** returns dual solution value of a constraint */
SCIP_RETCODE SCIPgetDualSolVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS*            cons,               /**< constraint for which the dual solution should be returned */
   SCIP_Real*            dualsolval,         /**< pointer to store the dual solution value */
   SCIP_Bool*            boundconstraint     /**< pointer to store whether the constraint is a bound constraint (or NULL) */
   )
{
   SCIP_CONS* transcons;
   int nvars;
   SCIP_Bool success;
#ifndef NDEBUG
   SCIP_CONSHDLR* conshdlr;
#endif

   assert(scip != NULL);
   assert(cons != NULL);
   assert(dualsolval != NULL);

#ifndef NDEBUG
   conshdlr = SCIPconsGetHdlr(cons);
   assert(conshdlr != NULL);
   assert(strcmp(SCIPconshdlrGetName(conshdlr), "linear" ) == 0);
#endif

   SCIP_CALL( SCIPconsGetNVars(cons, scip->set, &nvars, &success) );

   if( boundconstraint != NULL )
      *boundconstraint = (nvars == 1);

   if( SCIPconsIsTransformed(cons) )
      transcons = cons;
   else
      transcons = SCIPconsGetTransformed(cons);

   /* it can happen that a transformed constraints gets deleted due to redundancy. by complementary slackness the
    * corresponding dual solution value would be zero. however, if the constraint contains exactly one variable we need
    * to check the reduced costs of the variable.
    */
   if( nvars > 1 && transcons == NULL )
      (*dualsolval) = 0.0;
   else
   {
      if( !success )
      {
         SCIPABORT();
         return SCIP_INVALIDCALL;
      }

      if( nvars > 1 )
         (*dualsolval) = SCIPgetDualsolLinear(scip, transcons);

      /* the constraint is a bound constraint */
      else
      {
         SCIP_VAR** vars;
         SCIP_Real varsolval;

         /* allocate buffer memory */
         SCIP_CALL( SCIPallocBufferArray(scip, &vars, 1) );

         assert(vars != NULL);
         SCIP_CALL( SCIPconsGetVars(cons, scip->set, vars, 1, &success) );

         varsolval = SCIPvarGetLPSol(vars[0]);

         /* return the reduced cost of the variable if the constraint would be tight */
         if( SCIPsetIsEQ(scip->set, varsolval, SCIPgetRhsLinear(scip, cons))
          || SCIPsetIsEQ(scip->set, varsolval, SCIPgetLhsLinear(scip, cons)) )
            (*dualsolval) = SCIPgetVarRedcost(scip, vars[0]);
         else
            (*dualsolval) = 0.0;

         /* free buffer memory */
         SCIPfreeBufferArray(scip, &vars);
      }
   }
   assert(*dualsolval != SCIP_INVALID); /*lint !e777*/

   /* dual values are coming from the LP solver that is always solving a minimization problem */
   if( SCIPgetObjsense(scip) == SCIP_OBJSENSE_MAXIMIZE )
      (*dualsolval) *= -1.0;

   return SCIP_OKAY;
}

/** outputs dual solution from LP solver to file stream */
static
SCIP_RETCODE printDualSol(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_Bool             printzeros          /**< should variables set to zero be printed? */
   )
{
   SCIP_Bool boundconstraint;
   int c;

   assert(scip->lp != NULL);
   assert(scip->lp->solved);
   assert(scip->lp->dualfeasible);

   /* print dual solution values of all constraints */
   for( c = 0; c < scip->origprob->nconss; ++c )
   {
      SCIP_CONS* cons;
      SCIP_Real solval;

      cons = scip->origprob->conss[c];
      assert(cons != NULL);

      SCIP_CALL( SCIPgetDualSolVal(scip, cons, &solval, &boundconstraint) );

      if( printzeros || !SCIPisZero(scip, solval) )
      {
         SCIP_MESSAGEHDLR* messagehdlr = scip->messagehdlr;

         SCIPmessageFPrintInfo(messagehdlr, file, "%-32s", SCIPconsGetName(cons));

         if( SCIPisInfinity(scip, solval) )
            SCIPmessageFPrintInfo(messagehdlr, file, "            +infinity\n");
         else if( SCIPisInfinity(scip, -solval) )
            SCIPmessageFPrintInfo(messagehdlr, file, "            -infinity\n");
         else
         {
            if( boundconstraint )
               SCIPmessageFPrintInfo(messagehdlr, file, " %20.15g*\n", solval);
            else
               SCIPmessageFPrintInfo(messagehdlr, file, " %20.15g\n", solval);
         }
      }

   }

   return SCIP_OKAY;
}

/** check whether the dual solution is available
 *
 * @note This is used when calling \ref SCIPprintDualSol()
 *
 * @return is dual solution available?
 *
 * @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Bool SCIPisDualSolAvailable(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             printreason         /**< print warning message if dualsol is not available? */
   )
{
   int c;

   assert(scip != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPisDualSolAvailable", TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE) );

   if( SCIPgetStage(scip) != SCIP_STAGE_SOLVED )
   {
      if( printreason )
         SCIPmessageFPrintInfo(scip->messagehdlr, NULL, "No dual solution available.\n");
      return FALSE;
   }

   assert(scip->stat != NULL);
   assert(scip->transprob != NULL);

   /* dual solution only useful when no presolving was performed */
   if( scip->stat->performpresol )
   {
      if( printreason )
         SCIPwarningMessage(scip, "No dual information available when presolving was performed.\n");
      return FALSE;
   }

   /* dual solution is created by LP solver and therefore only available for pure LPs */
   if( scip->transprob->nvars != scip->transprob->ncontvars )
   {
      if( printreason )
         SCIPwarningMessage(scip, "Dual information only available for pure LPs (only continuous variables).\n");
      return FALSE;
   }

   /* dual solution is created by LP solver and therefore only available for linear constraints */
   for( c = scip->transprob->nconss - 1; c >= 0; --c )
   {
      SCIP_CONSHDLR* conshdlr;

      conshdlr = SCIPconsGetHdlr(scip->transprob->conss[c]);
      assert(conshdlr != NULL);

      if( strcmp(SCIPconshdlrGetName(conshdlr), "linear" ) != 0 )
      {
         if( printreason )
            SCIPwarningMessage(scip, "Dual information only available for pure LPs (only linear constraints).\n");
         return FALSE;
      }
   }

   return TRUE;
}

/** outputs dual solution from LP solver to file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called in all stages but only prints dual information when called in \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPprintDualSol(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_Bool             printzeros          /**< should variables set to zero be printed? */
   )
{
   if( SCIPisDualSolAvailable(scip, TRUE) )
   {
      /* print dual solution */
      SCIP_CALL( printDualSol(scip, file, printzeros) );
   }

   return SCIP_OKAY;
}


/** outputs non-zero variables of solution representing a ray in original problem space to file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPprintRay(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution representing ray */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_Bool             printzeros          /**< should variables set to zero be printed? */
   )
{
   assert(scip != NULL);
   assert(sol != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintRay", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   SCIP_CALL( SCIPsolPrintRay(sol, scip->set, scip->messagehdlr, scip->stat, scip->origprob, scip->transprob, file, printzeros) );

   return SCIP_OKAY;
}

/** gets number of feasible primal solutions stored in the solution storage in case the problem is transformed;
 *  in case the problem stage is SCIP_STAGE_PROBLEM, the number of solution in the original solution candidate
 *  storage is returned
 *
 *  @return number of feasible primal solutions stored in the solution storage in case the problem is transformed; or
 *          number of solution in the original solution candidate storage if the problem stage is SCIP_STAGE_PROBLEM
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetNSols(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNSols", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      return scip->origprimal->nsols;

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
      return scip->primal->nsols;

   case SCIP_STAGE_INIT:
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_FREETRANS:
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      SCIPABORT();
      return -1; /*lint !e527*/
   }  /*lint !e788*/
}

/** gets array of feasible primal solutions stored in the solution storage in case the problem is transformed; in case
 *  if the problem stage is in SCIP_STAGE_PROBLEM, it returns the number array of solution candidate stored
 *
 *  @return array of feasible primal solutions
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_SOL** SCIPgetSols(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSols", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
      return scip->origprimal->sols;

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
      return scip->primal->sols;

   case SCIP_STAGE_INIT:
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_FREETRANS:
   case SCIP_STAGE_FREE:
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return NULL;
   }  /*lint !e788*/
}

/** gets best feasible primal solution found so far if the problem is transformed; in case the problem is in
 *  SCIP_STAGE_PROBLEM it returns the best solution candidate, or NULL if no solution has been found or the candidate
 *  store is empty;
 *
 *  @return best feasible primal solution so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_SOL* SCIPgetBestSol(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetBestSol", TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );
   switch( scip->set->stage )
   {
   case SCIP_STAGE_INIT:
      return NULL;
   case SCIP_STAGE_PROBLEM:
      assert(scip->origprimal != NULL);
      if(  scip->origprimal->nsols > 0 )
      {
         assert(scip->origprimal->sols != NULL);
         assert(scip->origprimal->sols[0] != NULL);
         return scip->origprimal->sols[0];
      }
      break;

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
      assert(scip->primal != NULL);
      if(  scip->primal->nsols > 0 )
      {
         assert(scip->primal->sols != NULL);
         assert(scip->primal->sols[0] != NULL);
         return scip->primal->sols[0];
      }
      break;

   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_FREETRANS:
   case SCIP_STAGE_FREE:
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return NULL;
   }

   return NULL;
}

/** outputs best feasible primal solution found so far to file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPprintBestSol(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_Bool             printzeros          /**< should variables set to zero be printed? */
   )
{
   SCIP_SOL* sol;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintBestSol", TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   sol = SCIPgetBestSol(scip);

   if( sol == NULL )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "no solution available\n");
   else
   {
      SCIP_CALL( SCIPprintSol(scip, sol, file, printzeros) );
   }

   return SCIP_OKAY;
}

/** outputs best feasible primal solution found so far in transformed variables to file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_RETCODE SCIPprintBestTransSol(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_Bool             printzeros          /**< should variables set to zero be printed? */
   )
{
   SCIP_SOL* sol;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintBestTransSol", TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   sol = SCIPgetBestSol(scip);

   if( sol != NULL && SCIPsolIsOriginal(sol) )
   {
      SCIPerrorMessage("best solution is defined in original space - cannot print it as transformed solution\n");
      return SCIP_INVALIDCALL;
   }

   if( sol == NULL )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "no solution available\n");
   else
   {
      SCIP_CALL( SCIPprintTransSol(scip, sol, file, printzeros) );
   }

   return SCIP_OKAY;
}

/** try to round given solution
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIProundSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal solution */
   SCIP_Bool*            success             /**< pointer to store whether rounding was successful */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIProundSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( SCIPsolIsOriginal(sol) )
   {
      SCIPerrorMessage("cannot round original space solution\n");
      return SCIP_INVALIDCALL;
   }

   SCIP_CALL( SCIPsolRound(sol, scip->set, scip->stat, scip->transprob, scip->tree, success) );

   return SCIP_OKAY;
}

/** retransforms solution to original problem space
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPretransformSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol                 /**< primal CIP solution */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPretransformSol", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   switch ( SCIPsolGetOrigin(sol) )
   {
   case SCIP_SOLORIGIN_ORIGINAL:
      /* nothing to do */
      return SCIP_OKAY;

   case SCIP_SOLORIGIN_LPSOL:
   case SCIP_SOLORIGIN_NLPSOL:
   case SCIP_SOLORIGIN_RELAXSOL:
   case SCIP_SOLORIGIN_PSEUDOSOL:

      /* first unlink solution */
      SCIP_CALL( SCIPunlinkSol(scip, sol) );

      /*lint -fallthrough*/
   case SCIP_SOLORIGIN_ZERO:
   {
      SCIP_Bool hasinfval;

      SCIP_CALL( SCIPsolRetransform(sol, scip->set, scip->stat, scip->origprob, scip->transprob, &hasinfval) );
      break;
   }
   case SCIP_SOLORIGIN_PARTIAL:
   case SCIP_SOLORIGIN_UNKNOWN:
      SCIPerrorMessage("unknown solution origin.\n");
      return SCIP_INVALIDCALL;

   default:
      /* note that this is in an internal SCIP error since all solution origins are covert in the switch above */
      SCIPerrorMessage("invalid solution origin <%d>\n", SCIPsolGetOrigin(sol));
      return SCIP_ERROR;
   }

   return SCIP_OKAY;
}

/** reads a given solution file, problem has to be transformed in advance
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPreadSol(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           filename            /**< name of the input file */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPreadSol", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   /* we pass the reading of the solution file on to reader_sol via the following call */
   SCIP_CALL( SCIPreadProb(scip, filename, "sol") );

   return SCIP_OKAY;
}

/** reads a given solution file and store the solution values in the given solution pointer */
static
SCIP_RETCODE readSolFile(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           filename,           /**< name of the input file */
   SCIP_SOL*             sol,                /**< solution pointer */
   SCIP_Bool*            partial,            /**< pointer to store if the solution is partial (or NULL, if not needed) */
   SCIP_Bool*            error               /**< pointer store if an error occured */
   )
{
   SCIP_FILE* file;
   SCIP_Bool unknownvariablemessage;
   SCIP_Bool localpartial;
   int lineno;

   assert(scip != NULL);
   assert(sol != NULL);
   assert(error != NULL);

   /* open input file */
   file = SCIPfopen(filename, "r");
   if( file == NULL )
   {
      SCIPerrorMessage("cannot open file <%s> for reading\n", filename);
      SCIPprintSysError(filename);
      return SCIP_NOFILE;
   }

   *error = FALSE;
   localpartial = SCIPsolIsPartial(sol);

   unknownvariablemessage = FALSE;
   lineno = 0;

   /* read the file */
   while( !SCIPfeof(file) && !(*error) )
   {
      char buffer[SCIP_MAXSTRLEN];
      char varname[SCIP_MAXSTRLEN];
      char valuestring[SCIP_MAXSTRLEN];
      char objstring[SCIP_MAXSTRLEN];
      SCIP_VAR* var;
      SCIP_Real value;
      int nread;

      /* get next line */
      if( SCIPfgets(buffer, (int) sizeof(buffer), file) == NULL )
         break;
      lineno++;

      /* there are some lines which may preceed the solution information */
      if( strncasecmp(buffer, "solution status:", 16) == 0 || strncasecmp(buffer, "objective value:", 16) == 0 ||
         strncasecmp(buffer, "Log started", 11) == 0 || strncasecmp(buffer, "Variable Name", 13) == 0 ||
         strncasecmp(buffer, "All other variables", 19) == 0 || strncasecmp(buffer, "\n", 1) == 0 ||
         strncasecmp(buffer, "NAME", 4) == 0 || strncasecmp(buffer, "ENDATA", 6) == 0 )    /* allow parsing of SOL-format on the MIPLIB 2003 pages */
         continue;

      /* parse the line */
      /* cppcheck-suppress invalidscanf */
      nread = sscanf(buffer, "%s %s %s\n", varname, valuestring, objstring);
      if( nread < 2 )
      {
         SCIPerrorMessage("Invalid input line %d in solution file <%s>: <%s>.\n", lineno, filename, buffer);
         *error = TRUE;
         break;
      }

      /* find the variable */
      var = SCIPfindVar(scip, varname);
      if( var == NULL )
      {
         if( !unknownvariablemessage )
         {
            SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "unknown variable <%s> in line %d of solution file <%s>\n",
               varname, lineno, filename);
            SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "  (further unknown variables are ignored)\n");
            unknownvariablemessage = TRUE;
         }
         continue;
      }

      /* cast the value */
      if( strncasecmp(valuestring, "inv", 3) == 0 )
         continue;
      else if( strncasecmp(valuestring, "+inf", 4) == 0 || strncasecmp(valuestring, "inf", 3) == 0 )
         value = SCIPinfinity(scip);
      else if( strncasecmp(valuestring, "-inf", 4) == 0 )
         value = -SCIPinfinity(scip);
      else if( strncasecmp(valuestring, "unknown", 7) == 0 )
      {
         value = SCIP_UNKNOWN;
         localpartial = TRUE;
      }
      else
      {
         nread = sscanf(valuestring, "%lf", &value);
         if( nread != 1 )
         {
            SCIPerrorMessage("Invalid solution value <%s> for variable <%s> in line %d of solution file <%s>.\n",
               valuestring, varname, lineno, filename);
            *error = TRUE;
            break;
         }
      }

      /* set the solution value of the variable, if not multiaggregated */
      if( SCIPisTransformed(scip) && SCIPvarGetStatus(SCIPvarGetProbvar(var)) == SCIP_VARSTATUS_MULTAGGR )
      {
         SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "ignored solution value for multiaggregated variable <%s>\n", SCIPvarGetName(var));
      }
      else
      {
         SCIP_RETCODE retcode;

         retcode = SCIPsetSolVal(scip, sol, var, value);

         if( retcode == SCIP_INVALIDDATA )
         {
            if( SCIPvarGetStatus(SCIPvarGetProbvar(var)) == SCIP_VARSTATUS_FIXED )
            {
               SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "ignored conflicting solution value for fixed variable <%s>\n",
                  SCIPvarGetName(var));
            }
            else
            {
               SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "ignored solution value for multiaggregated variable <%s>\n",
                  SCIPvarGetName(var));
            }
         }
         else
         {
            SCIP_CALL_FINALLY( retcode, SCIPfclose(file) );
         }
      }
   }

   /* close input file */
   SCIPfclose(file);

   if( localpartial && !SCIPsolIsPartial(sol) )
   {
      if( SCIPgetStage(scip) == SCIP_STAGE_PROBLEM )
      {
         SCIP_CALL( SCIPsolMarkPartial(sol, scip->set, scip->stat, scip->origprob->vars, scip->origprob->nvars) );
      }
      else
         *error = TRUE;
   }

   if( partial != NULL )
      *partial = localpartial;

   return SCIP_OKAY;
}

/** reads a given xml solution file and store the solution values in the given solution pointer */
static
SCIP_RETCODE readXmlSolFile(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           filename,           /**< name of the input file */
   SCIP_SOL*             sol,                /**< solution pointer */
   SCIP_Bool*            partial,            /**< pointer to store if the solution is partial (or NULL if not needed) */
   SCIP_Bool*            error               /**< pointer store if an error occured */
   )
{
   SCIP_Bool unknownvariablemessage;
   SCIP_Bool localpartial;
   XML_NODE* start;
   const XML_NODE* varsnode;
   const XML_NODE* varnode;
   const char* tag;

   assert(scip != NULL);
   assert(sol != NULL);
   assert(error != NULL);

   /* read xml file */
   start = xmlProcess(filename);

   if( start == NULL )
   {
      SCIPerrorMessage("Some error occured during parsing the XML solution file.\n");
      return SCIP_READERROR;
   }

   *error = FALSE;
   localpartial = SCIPsolIsPartial(sol);

   /* find variable sections */
   tag = "variables";
   varsnode = xmlFindNodeMaxdepth(start, tag, 0, 3);
   if( varsnode == NULL )
   {
      /* free xml data */
      xmlFreeNode(start);

      SCIPerrorMessage("Variable section not found.\n");
      return SCIP_READERROR;
   }

   /* loop through all variables */
   unknownvariablemessage = FALSE;
   for( varnode = xmlFirstChild(varsnode); varnode != NULL; varnode = xmlNextSibl(varnode) )
   {
      SCIP_VAR* var;
      const char* varname;
      const char* valuestring;
      SCIP_Real value;
      int nread;

      /* find variable name */
      varname = xmlGetAttrval(varnode, "name");
      if( varname == NULL )
      {
         SCIPerrorMessage("Attribute \"name\" of variable not found.\n");
         *error = TRUE;
         break;
      }

      /* find the variable */
      var = SCIPfindVar(scip, varname);
      if( var == NULL )
      {
         if( !unknownvariablemessage )
         {
            SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "unknown variable <%s> of solution file <%s>\n",
               varname, filename);
            SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "  (further unknown variables are ignored)\n");
            unknownvariablemessage = TRUE;
         }
         continue;
      }

      /* find value of variable */
      valuestring = xmlGetAttrval(varnode, "value");
      if( valuestring == NULL )
      {
         SCIPerrorMessage("Attribute \"value\" of variable not found.\n");
         *error = TRUE;
         break;
      }

      /* cast the value */
      if( strncasecmp(valuestring, "inv", 3) == 0 )
         continue;
      else if( strncasecmp(valuestring, "+inf", 4) == 0 || strncasecmp(valuestring, "inf", 3) == 0 )
         value = SCIPinfinity(scip);
      else if( strncasecmp(valuestring, "-inf", 4) == 0 )
         value = -SCIPinfinity(scip);
      else if( strncasecmp(valuestring, "unknown", 7) == 0 )
      {
         value = SCIP_UNKNOWN;
         localpartial = TRUE;
      }
      else
      {
         nread = sscanf(valuestring, "%lf", &value);
         if( nread != 1 )
         {
            SCIPwarningMessage(scip, "invalid solution value <%s> for variable <%s> in XML solution file <%s>\n", valuestring, varname, filename);
            *error = TRUE;
            break;
         }
      }

      /* set the solution value of the variable, if not multiaggregated */
      if( SCIPisTransformed(scip) && SCIPvarGetStatus(SCIPvarGetProbvar(var)) == SCIP_VARSTATUS_MULTAGGR )
      {
         SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "ignored solution value for multiaggregated variable <%s>\n", SCIPvarGetName(var));
      }
      else
      {
         SCIP_RETCODE retcode;
         retcode = SCIPsetSolVal(scip, sol, var, value);

         if( retcode == SCIP_INVALIDDATA )
         {
            if( SCIPvarGetStatus(SCIPvarGetProbvar(var)) == SCIP_VARSTATUS_FIXED )
            {
               SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "ignored conflicting solution value for fixed variable <%s>\n",
                  SCIPvarGetName(var));
            }
            else
            {
               SCIPverbMessage(scip, SCIP_VERBLEVEL_NORMAL, NULL, "ignored solution value for multiaggregated variable <%s>\n",
                  SCIPvarGetName(var));
            }
         }
         else
         {
            SCIP_CALL( retcode );
         }
      }
   }

   /* free xml data */
   xmlFreeNode(start);

   if( localpartial && !SCIPsolIsPartial(sol)  )
   {
      if( SCIPgetStage(scip) == SCIP_STAGE_PROBLEM )
      {
         SCIP_CALL( SCIPsolMarkPartial(sol, scip->set, scip->stat, scip->origprob->vars, scip->origprob->nvars) );
      }
      else
         *error = TRUE;
   }

   if( partial != NULL )
      *partial = localpartial;

   return SCIP_OKAY;
}

/** reads a given solution file and store the solution values in the given solution pointer
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPreadSolFile(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           filename,           /**< name of the input file */
   SCIP_SOL*             sol,                /**< solution pointer */
   SCIP_Bool             xml,                /**< true, iff the given solution in written in XML */
   SCIP_Bool*            partial,            /**< pointer to store if the solution is partial */
   SCIP_Bool*            error               /**< pointer store if an error occured */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPreadSolFile", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( xml )
   {
      SCIP_CALL( readXmlSolFile(scip, filename, sol, partial, error) );
   }
   else
   {
      SCIP_CALL( readSolFile(scip, filename, sol, partial, error) );
   }

   return SCIP_OKAY;
}

/** adds feasible primal solution to solution storage by copying it
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note Do not call during propagation, use heur_trysol instead.
 */
SCIP_RETCODE SCIPaddSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Bool*            stored              /**< stores whether given solution was good enough to keep */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddSol", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
   case SCIP_STAGE_FREETRANS:
      assert(SCIPsolIsOriginal(sol));
      SCIP_CALL( SCIPprimalAddOrigSol(scip->origprimal, scip->mem->probmem, scip->set, scip->stat, scip->origprob, sol, stored) );
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
      /* if the solution is added during presolving and it is not defined on original variables,
       * presolving operations will destroy its validity, so we retransform it to the original space
       */
      if( !SCIPsolIsOriginal(sol) )
      {
         SCIP_SOL* bestsol = SCIPgetBestSol(scip);
         SCIP_SOL* tmpsol = sol;
         SCIP_Bool hasinfval;

         SCIP_CALL( SCIPcreateSolCopy(scip, &tmpsol, sol) );

         SCIP_CALL( SCIPsolUnlink(tmpsol, scip->set, scip->transprob) );
         SCIP_CALL( SCIPsolRetransform(tmpsol, scip->set, scip->stat, scip->origprob, scip->transprob, &hasinfval) );

         SCIP_CALL( SCIPprimalAddSolFree(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
               scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter,
               &tmpsol, stored) );

         if( *stored && (bestsol != SCIPgetBestSol(scip)) )
         {
            SCIPstoreSolutionGap(scip);
         }

         return SCIP_OKAY;
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVING:
   {
      SCIP_SOL* bestsol = SCIPgetBestSol(scip);

      SCIP_CALL( SCIPprimalAddSol(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
            scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter, sol,
            stored) );

      /* @todo use solution index rather than pointer */
      if( *stored && (bestsol != SCIPgetBestSol(scip)) )
      {
         SCIPstoreSolutionGap(scip);
      }

      return SCIP_OKAY;
   }
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** adds primal solution to solution storage, frees the solution afterwards
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  @note Do not call during propagation, use heur_trysol instead.
 */
SCIP_RETCODE SCIPaddSolFree(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to primal CIP solution; is cleared in function call */
   SCIP_Bool*            stored              /**< stores whether given solution was good enough to keep */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddSolFree", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, TRUE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_PROBLEM:
   case SCIP_STAGE_FREETRANS:
      assert(SCIPsolIsOriginal(*sol));
      SCIP_CALL( SCIPprimalAddOrigSolFree(scip->origprimal, scip->mem->probmem, scip->set, scip->stat, scip->origprob, sol, stored) );
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
      /* if the solution is added during presolving and it is not defined on original variables,
       * presolving operations will destroy its validity, so we retransform it to the original space
       */
      if( !SCIPsolIsOriginal(*sol) )
      {
         SCIP_Bool hasinfval;

         SCIP_CALL( SCIPsolUnlink(*sol, scip->set, scip->transprob) );
         SCIP_CALL( SCIPsolRetransform(*sol, scip->set, scip->stat, scip->origprob, scip->transprob, &hasinfval) );
      }
      /*lint -fallthrough*/
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVING:
   {
      SCIP_SOL* bestsol = SCIPgetBestSol(scip);

      SCIP_CALL( SCIPprimalAddSolFree(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
            scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter,
            sol, stored) );

      if( *stored )
      {
         if( bestsol != SCIPgetBestSol(scip) )
         {
            assert(SCIPgetBestSol(scip) != NULL);
            SCIPstoreSolutionGap(scip);
         }
      }

      return SCIP_OKAY;
   }
   case SCIP_STAGE_TRANSFORMING:
   case SCIP_STAGE_INITSOLVE:
   case SCIP_STAGE_SOLVED:
   case SCIP_STAGE_EXITSOLVE:
   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** adds current LP/pseudo solution to solution storage
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPaddCurrentSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HEUR*            heur,               /**< heuristic that found the solution */
   SCIP_Bool*            stored              /**< stores whether given solution was good enough to keep */
   )
{
   SCIP_SOL* bestsol;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPaddCurrentSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   bestsol = SCIPgetBestSol(scip);

   SCIP_CALL( SCIPprimalAddCurrentSol(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
         scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter, heur,
         stored) );

   if( *stored )
   {
      if( bestsol != SCIPgetBestSol(scip) )
         SCIPstoreSolutionGap(scip);
   }

   return SCIP_OKAY;
}

/** checks solution for feasibility; if possible, adds it to storage by copying
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note Do not call during propagation, use heur_trysol instead.
 */
SCIP_RETCODE SCIPtrySol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Bool             printreason,        /**< Should all reasons of violation be printed? */
   SCIP_Bool             completely,         /**< Should all violations be checked? */
   SCIP_Bool             checkbounds,        /**< Should the bounds of the variables be checked? */
   SCIP_Bool             checkintegrality,   /**< Has integrality to be checked? */
   SCIP_Bool             checklprows,        /**< Do constraints represented by rows in the current LP have to be checked? */
   SCIP_Bool*            stored              /**< stores whether given solution was feasible and good enough to keep */
   )
{
   SCIP_SOL* bestsol;

   assert(sol != NULL);
   assert(stored != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtrySol", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   bestsol = SCIPgetBestSol(scip);

   if( !printreason )
      completely = FALSE;

   /* we cannot check partial solutions */
   if( SCIPsolIsPartial(sol) )
   {
      SCIPerrorMessage("Cannot check feasibility of partial solutions.\n");
      return SCIP_INVALIDDATA;
   }

   /* if the solution is added during presolving and it is not defined on original variables,
    * presolving operations will destroy its validity, so we retransform it to the original space
    */
   if( scip->set->stage == SCIP_STAGE_PRESOLVING && !SCIPsolIsOriginal(sol) )
   {
      SCIP_Bool hasinfval;

      SCIP_CALL( SCIPsolUnlink(sol, scip->set, scip->transprob) );
      SCIP_CALL( SCIPsolRetransform(sol, scip->set, scip->stat, scip->origprob, scip->transprob, &hasinfval) );
   }

   if( SCIPsolIsOriginal(sol) )
   {
      SCIP_Bool feasible;

      /* SCIPprimalTrySol() can only be called on transformed solutions; therefore check solutions in original problem
       * including modifiable constraints */
      SCIP_CALL( checkSolOrig(scip, sol, &feasible, printreason, completely, checkbounds, checkintegrality, checklprows, TRUE) );
      if( feasible )
      {
         SCIP_CALL( SCIPprimalAddSol(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
               scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter,
               sol, stored) );

         if( *stored )
         {
            if( bestsol != SCIPgetBestSol(scip) )
               SCIPstoreSolutionGap(scip);
         }
      }
      else
         *stored = FALSE;
   }
   else
   {
      SCIP_CALL( SCIPprimalTrySol(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat, scip->origprob,
            scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter, sol, printreason,
            completely, checkbounds, checkintegrality, checklprows, stored) );

      if( *stored )
      {
         if( bestsol != SCIPgetBestSol(scip) )
            SCIPstoreSolutionGap(scip);
      }
   }

   return SCIP_OKAY;
}

/** checks primal solution; if feasible, adds it to storage; solution is freed afterwards
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note Do not call during propagation, use heur_trysol instead.
 */
SCIP_RETCODE SCIPtrySolFree(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL**            sol,                /**< pointer to primal CIP solution; is cleared in function call */
   SCIP_Bool             printreason,        /**< Should all reasons of violations be printed */
   SCIP_Bool             completely,         /**< Should all violation be checked? */
   SCIP_Bool             checkbounds,        /**< Should the bounds of the variables be checked? */
   SCIP_Bool             checkintegrality,   /**< Has integrality to be checked? */
   SCIP_Bool             checklprows,        /**< Do constraints represented by rows in the current LP have to be checked? */
   SCIP_Bool*            stored              /**< stores whether solution was feasible and good enough to keep */
   )
{
   SCIP_SOL* bestsol;

   assert(stored != NULL);
   assert(sol != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtrySolFree", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   bestsol = SCIPgetBestSol(scip);

   if( !printreason )
      completely = FALSE;

   /* we cannot check partial solutions */
   if( SCIPsolIsPartial(*sol) )
   {
      SCIPerrorMessage("Cannot check feasibility of partial solutions.\n");
      return SCIP_INVALIDDATA;
   }

   /* if the solution is added during presolving and it is not defined on original variables,
    * presolving operations will destroy its validity, so we retransform it to the original space
    */
   if( scip->set->stage == SCIP_STAGE_PRESOLVING && !SCIPsolIsOriginal(*sol) )
   {
      SCIP_Bool hasinfval;

      SCIP_CALL( SCIPsolUnlink(*sol, scip->set, scip->transprob) );
      SCIP_CALL( SCIPsolRetransform(*sol, scip->set, scip->stat, scip->origprob, scip->transprob, &hasinfval) );
   }

   if( SCIPsolIsOriginal(*sol) )
   {
      SCIP_Bool feasible;

      /* SCIPprimalTrySol() can only be called on transformed solutions; therefore check solutions in original problem
       * including modifiable constraints
       */
      SCIP_CALL( checkSolOrig(scip, *sol, &feasible, printreason, completely, checkbounds, checkintegrality, checklprows, TRUE) );

      if( feasible )
      {
         SCIP_CALL( SCIPprimalAddSolFree(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
               scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter,
               sol, stored) );

         if( *stored )
         {
            if( bestsol != SCIPgetBestSol(scip) )
               SCIPstoreSolutionGap(scip);
         }
      }
      else
      {
         SCIP_CALL( SCIPsolFree(sol, scip->mem->probmem, scip->primal) );
         *stored = FALSE;
      }
   }
   else
   {
      SCIP_CALL( SCIPprimalTrySolFree(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
            scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter,
            sol, printreason, completely, checkbounds, checkintegrality, checklprows, stored) );

      if( *stored )
      {
         if( bestsol != SCIPgetBestSol(scip) )
            SCIPstoreSolutionGap(scip);
      }
   }

   return SCIP_OKAY;
}

/** checks current LP/pseudo solution for feasibility; if possible, adds it to storage
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPtryCurrentSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_HEUR*            heur,               /**< heuristic that found the solution */
   SCIP_Bool             printreason,        /**< Should all reasons of violations be printed? */
   SCIP_Bool             completely,         /**< Should all violation be checked? */
   SCIP_Bool             checkintegrality,   /**< Has integrality to be checked? */
   SCIP_Bool             checklprows,        /**< Do constraints represented by rows in the current LP have to be checked? */
   SCIP_Bool*            stored              /**< stores whether given solution was feasible and good enough to keep */
   )
{
   SCIP_SOL* bestsol;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPtryCurrentSol", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   bestsol = SCIPgetBestSol(scip);

   if( !printreason )
      completely = FALSE;

   SCIP_CALL( SCIPprimalTryCurrentSol(scip->primal, scip->mem->probmem, scip->set, scip->messagehdlr, scip->stat,
         scip->origprob, scip->transprob, scip->tree, scip->reopt, scip->lp, scip->eventqueue, scip->eventfilter, heur,
         printreason, completely, checkintegrality, checklprows, stored) );

   if( *stored )
   {
      if( bestsol != SCIPgetBestSol(scip) )
         SCIPstoreSolutionGap(scip);
   }

   return SCIP_OKAY;
}

/** returns all partial solutions
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_SOL** SCIPgetPartialSols(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPartialSols", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->origprimal->partialsols;

}

/** returns number of partial solutions
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNPartialSols(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPartialSols", FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->origprimal->npartialsols;
}

/** checks solution for feasibility without adding it to the solution store
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPcheckSol(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Bool             printreason,        /**< Should all reasons of violations be printed? */
   SCIP_Bool             completely,         /**< Should all violation be checked? */
   SCIP_Bool             checkbounds,        /**< Should the bounds of the variables be checked? */
   SCIP_Bool             checkintegrality,   /**< Has integrality to be checked? */
   SCIP_Bool             checklprows,        /**< Do constraints represented by rows in the current LP have to be checked? */
   SCIP_Bool*            feasible            /**< stores whether given solution is feasible */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcheckSol", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* return immediately if the solution is of type partial */
   if( SCIPsolIsPartial(sol) )
   {
      SCIPerrorMessage("Cannot check feasibility of partial solutions.");
      return SCIP_INVALIDDATA;
   }

   /* if we want to solve exactly, the constraint handlers cannot rely on the LP's feasibility */
   checklprows = checklprows || scip->set->misc_exactsolve;

   if( !printreason )
      completely = FALSE;

   if( SCIPsolIsOriginal(sol) )
   {
      /* SCIPsolCheck() can only be called on transformed solutions */
      SCIP_CALL( checkSolOrig(scip, sol, feasible, printreason, completely, checkbounds, checkintegrality, checklprows, FALSE) );
   }
   else
   {
      SCIP_CALL( SCIPsolCheck(sol, scip->set, scip->messagehdlr, scip->mem->probmem, scip->stat, scip->transprob,
            printreason, completely, checkbounds, checkintegrality, checklprows, feasible) );
   }

   return SCIP_OKAY;
}

/** checks solution for feasibility in original problem without adding it to the solution store;
 *  this method is used to double check a solution in order to validate the presolving process
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPcheckSolOrig(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             sol,                /**< primal CIP solution */
   SCIP_Bool*            feasible,           /**< stores whether given solution is feasible */
   SCIP_Bool             printreason,        /**< should the reason for the violation be printed? */
   SCIP_Bool             completely          /**< should all violation be checked? */
   )
{
   assert(scip != NULL);
   assert(sol != NULL);
   assert(feasible != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPcheckSolOrig", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* return immediately if the solution is of type partial */
   if( SCIPsolIsPartial(sol) )
   {
      SCIPerrorMessage("Cannot check feasibility of partial solutions.");
      return SCIP_INVALIDDATA;
   }

   if( !printreason )
      completely = FALSE;

   /* check solution in original problem; that includes bounds, integrality, and non modifiable constraints */
   SCIP_CALL( checkSolOrig(scip, sol, feasible, printreason, completely, TRUE, TRUE, TRUE, FALSE) );

   return SCIP_OKAY;
}

/** return whether a primal ray is stored that proves unboundedness of the LP relaxation
 *
 *  @return return whether a primal ray is stored that proves unboundedness of the LP relaxation
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Bool SCIPhasPrimalRay(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPhasPrimalRay", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->primal->primalray != NULL;
}

/** gets value of given variable in primal ray causing unboundedness of the LP relaxation;
 *  should only be called if such a ray is stored (check with SCIPhasPrimalRay())
 *
 *  @return value of given variable in primal ray causing unboundedness of the LP relaxation
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetPrimalRayVal(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR*             var                 /**< variable to get value for */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPrimalRayVal", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   assert(var != NULL);
   assert(scip->primal->primalray != NULL);
   assert(var->scip == scip);

   return SCIPsolGetRayVal(scip->primal->primalray, scip->set, scip->stat, var);
}

/** updates the primal ray thats proves unboundedness
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPupdatePrimalRay(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_SOL*             primalray           /**< the new primal ray */
   )
{
   assert(scip != NULL);
   assert(primalray != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPupdatePrimalRay", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPprimalUpdateRay(scip->primal, scip->set, scip->stat, primalray, scip->mem->probmem) );

   return SCIP_OKAY;
}


/*
 * event methods
 */



/*
 * tree methods
 */

/** gets focus node in the tree
 *
 *  if we are in probing/diving mode this method returns the node in the tree where the probing/diving mode was started.
 *
 *  @return the current node of the search tree
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetFocusNode(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetFocusNode", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetFocusNode(scip->tree);
}

/** gets current node in the tree
 *
 *  @return the current node of the search tree
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetCurrentNode(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetCurrentNode", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetCurrentNode(scip->tree);
}

/** gets the root node of the tree
 *
 *  @return the root node of the search tree
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetRootNode(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetRootNode", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetRootNode(scip->tree);
}

/** gets the effective root depth, i.e., the depth of the deepest node which is part of all paths from the root node
 *  to the unprocessed nodes.
 *
 *  @return effective root depth
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetEffectiveRootDepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetEffectiveRootDepth", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetEffectiveRootDepth(scip->tree);
}

/** returns whether the current node is already solved and only propagated again
 *
 *  @return TRUE is returned if \SCIP performance repropagation, otherwise FALSE.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_Bool SCIPinRepropagation(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPinRepropagation", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeInRepropagation(scip->tree);
}

/** gets children of focus node along with the number of children
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPgetChildren(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE***          children,           /**< pointer to store children array, or NULL if not needed */
   int*                  nchildren           /**< pointer to store number of children, or NULL if not needed */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetChildren", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( children != NULL )
      *children = scip->tree->children;
   if( nchildren != NULL )
      *nchildren = scip->tree->nchildren;

   return SCIP_OKAY;
}

/** gets number of children of focus node
 *
 *  @return number of children of the focus node
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNChildren(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNChildren", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->tree->nchildren;
}

/** gets siblings of focus node along with the number of siblings
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPgetSiblings(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE***          siblings,           /**< pointer to store siblings array, or NULL if not needed */
   int*                  nsiblings           /**< pointer to store number of siblings, or NULL if not needed */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetSiblings", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( siblings != NULL )
      *siblings = scip->tree->siblings;
   if( nsiblings != NULL )
      *nsiblings = scip->tree->nsiblings;

   return SCIP_OKAY;
}

/** gets number of siblings of focus node
 *
 *  @return the number of siblings of focus node
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNSiblings(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNSiblings", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->tree->nsiblings;
}

/** gets leaves of the tree along with the number of leaves
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPgetLeaves(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE***          leaves,             /**< pointer to store leaves array, or NULL if not needed */
   int*                  nleaves             /**< pointer to store number of leaves, or NULL if not needed */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetLeaves", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( leaves != NULL )
      *leaves = SCIPnodepqNodes(scip->tree->leaves);
   if( nleaves != NULL )
      *nleaves = SCIPnodepqLen(scip->tree->leaves);

   return SCIP_OKAY;
}

/** gets number of leaves in the tree
 *
 *  @return the number of leaves in the tree
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNLeaves(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNLeaves", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPnodepqLen(scip->tree->leaves);
}

/** gets the best child of the focus node w.r.t. the node selection priority assigned by the branching rule
 *
 *  @return the best child of the focus node w.r.t. the node selection priority assigned by the branching rule
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetPrioChild(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPrioChild", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetPrioChild(scip->tree);
}

/** gets the best sibling of the focus node w.r.t. the node selection priority assigned by the branching rule
 *
 *  @return the best sibling of the focus node w.r.t. the node selection priority assigned by the branching rule
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetPrioSibling(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPrioSibling", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetPrioSibling(scip->tree);
}

/** gets the best child of the focus node w.r.t. the node selection strategy
 *
 *  @return the best child of the focus node w.r.t. the node selection strategy
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetBestChild(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetBestChild", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetBestChild(scip->tree, scip->set);
}

/** gets the best sibling of the focus node w.r.t. the node selection strategy
 *
 *  @return the best sibling of the focus node w.r.t. the node selection strategy
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetBestSibling(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetBestSibling", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetBestSibling(scip->tree, scip->set);
}

/** gets the best leaf from the node queue w.r.t. the node selection strategy
 *
 *  @return the best leaf from the node queue w.r.t. the node selection strategy
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetBestLeaf(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetBestLeaf", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetBestLeaf(scip->tree);
}

/** gets the best node from the tree (child, sibling, or leaf) w.r.t. the node selection strategy
 *
 *  @return the best node from the tree (child, sibling, or leaf) w.r.t. the node selection strategy
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetBestNode(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetBestNode", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetBestNode(scip->tree, scip->set);
}

/** gets the node with smallest lower bound from the tree (child, sibling, or leaf)
 *
 *  @return the node with smallest lower bound from the tree (child, sibling, or leaf)
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_NODE* SCIPgetBestboundNode(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetBestboundNode", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetLowerboundNode(scip->tree, scip->set);
}

/** access to all data of open nodes (leaves, children, and siblings)
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPgetOpenNodesData(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE***          leaves,             /**< pointer to store the leaves, or NULL if not needed */
   SCIP_NODE***          children,           /**< pointer to store the children, or NULL if not needed */
   SCIP_NODE***          siblings,           /**< pointer to store the siblings, or NULL if not needed */
   int*                  nleaves,            /**< pointer to store the number of leaves, or NULL */
   int*                  nchildren,          /**< pointer to store the number of children, or NULL */
   int*                  nsiblings           /**< pointer to store the number of siblings, or NULL */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPgetOpenNodesData", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( leaves != NULL )
      *leaves = SCIPnodepqNodes(scip->tree->leaves);
   if( children != NULL )
      *children = scip->tree->children;
   if( siblings != NULL )
      *siblings = scip->tree->siblings;
   if( nleaves != NULL )
      *nleaves = SCIPnodepqLen(scip->tree->leaves);
   if( nchildren != NULL )
      *nchildren = SCIPtreeGetNChildren(scip->tree);
   if( nsiblings != NULL )
      *nsiblings = SCIPtreeGetNSiblings(scip->tree);

   return SCIP_OKAY;
}

/** cuts off node and whole sub tree from branch and bound tree
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPcutoffNode(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node                /**< node that should be cut off */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPcutoffNode", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIP_CALL( SCIPnodeCutoff(node, scip->set, scip->stat, scip->tree, scip->transprob, scip->origprob, scip->reopt,
         scip->lp, scip->mem->probmem) );

   return SCIP_OKAY;
}

/** marks the given node to be propagated again the next time a node of its subtree is processed
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPrepropagateNode(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node                /**< node that should be propagated again */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPrepropagateNode", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPnodePropagateAgain(node, scip->set, scip->stat, scip->tree);

   return SCIP_OKAY;
}

/** returns depth of first node in active path that is marked being cutoff
 *
 *  @return depth of first node in active path that is marked being cutoff
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetCutoffdepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetCutoffdepth", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return scip->tree->cutoffdepth;
}

/** returns depth of first node in active path that has to be propagated again
 *
 *  @return depth of first node in active path that has to be propagated again
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetRepropdepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetRepropdepth", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return scip->tree->repropdepth;
}

/** prints all branching decisions on variables from the root to the given node
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPprintNodeRootPath(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_NODE*            node,               /**< node data */
   FILE*                 file                /**< output file (or NULL for standard output) */
   )
{
   SCIP_VAR**            branchvars;         /* array of variables on which the branchings has been performed in all ancestors */
   SCIP_Real*            branchbounds;       /* array of bounds which the branchings in all ancestors set */
   SCIP_BOUNDTYPE*       boundtypes;         /* array of boundtypes which the branchings in all ancestors set */
   int*                  nodeswitches;       /* marks, where in the arrays the branching decisions of the next node on the path start
                                              * branchings performed at the parent of node always start at position 0. For single variable branching,
                                              * nodeswitches[i] = i holds */
   int                   nbranchvars;        /* number of variables on which branchings have been performed in all ancestors
                                              *   if this is larger than the array size, arrays should be reallocated and method should be called again */
   int                   branchvarssize;     /* available slots in arrays */
   int                   nnodes;             /* number of nodes in the nodeswitch array */
   int                   nodeswitchsize;     /* available slots in node switch array */

   branchvarssize = SCIPnodeGetDepth(node);
   nodeswitchsize = branchvarssize;

   /* memory allocation */
   SCIP_CALL( SCIPallocBufferArray(scip, &branchvars, branchvarssize) );
   SCIP_CALL( SCIPallocBufferArray(scip, &branchbounds, branchvarssize) );
   SCIP_CALL( SCIPallocBufferArray(scip, &boundtypes, branchvarssize) );
   SCIP_CALL( SCIPallocBufferArray(scip, &nodeswitches, nodeswitchsize) );

   SCIPnodeGetAncestorBranchingPath(node, branchvars, branchbounds, boundtypes, &nbranchvars, branchvarssize, nodeswitches, &nnodes, nodeswitchsize );

   /* if the arrays were to small, we have to reallocate them and recall SCIPnodeGetAncestorBranchingPath */
   if( nbranchvars > branchvarssize || nnodes > nodeswitchsize )
   {
      branchvarssize = nbranchvars;
      nodeswitchsize = nnodes;

      /* memory reallocation */
      SCIP_CALL( SCIPreallocBufferArray(scip, &branchvars, branchvarssize) );
      SCIP_CALL( SCIPreallocBufferArray(scip, &branchbounds, branchvarssize) );
      SCIP_CALL( SCIPreallocBufferArray(scip, &boundtypes, branchvarssize) );
      SCIP_CALL( SCIPreallocBufferArray(scip, &nodeswitches, nodeswitchsize) );

      SCIPnodeGetAncestorBranchingPath(node, branchvars, branchbounds, boundtypes, &nbranchvars, branchvarssize, nodeswitches, &nnodes, nodeswitchsize);
      assert(nbranchvars == branchvarssize);
   }

   /* we only want to create output, if branchings were performed */
   if( nbranchvars >= 1 )
   {
      int i;
      int j;

      /* print all nodes, starting from the root, which is last in the arrays */
      for( j = nnodes-1; j >= 0; --j)
      {
         int end;
         if(j == nnodes-1)
            end =  nbranchvars;
         else
            end =  nodeswitches[j+1];

         for( i = nodeswitches[j]; i < end; ++i )
         {
            if( i > nodeswitches[j] )
               SCIPmessageFPrintInfo(scip->messagehdlr, file, " AND ");
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "<%s> %s %.1f",SCIPvarGetName(branchvars[i]), boundtypes[i] == SCIP_BOUNDTYPE_LOWER ? ">=" : "<=", branchbounds[i]);
         }
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "\n");
         if( j > 0 )
         {
            if(  nodeswitches[j]-nodeswitches[j-1] != 1 )
               SCIPmessageFPrintInfo(scip->messagehdlr, file, " |\n |\n");
            else if( boundtypes[i-1] == SCIP_BOUNDTYPE_LOWER )
               SCIPmessageFPrintInfo(scip->messagehdlr, file, "\\ \n \\\n");
            else
               SCIPmessageFPrintInfo(scip->messagehdlr, file, " /\n/ \n");
         }
      }
   }

   /* free all local memory */
   SCIPfreeBufferArray(scip, &nodeswitches);
   SCIPfreeBufferArray(scip, &boundtypes);
   SCIPfreeBufferArray(scip, &branchbounds);
   SCIPfreeBufferArray(scip, &branchvars);

   return SCIP_OKAY;
}

/** sets whether the LP should be solved at the focus node
 *
 *  @note In order to have an effect, this method needs to be called after a node is focused but before the LP is
 *        solved.
 *
 *  @pre This method can be called if @p scip is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
void SCIPsetFocusnodeLP(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Bool             solvelp             /**< should the LP be solved? */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPsetFocusnodeLP", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   SCIPtreeSetFocusNodeLP(scip->tree, solvelp);
}


/*
 * parallel interface methods
 */




/*
 * statistic methods
 */

/** gets number of branch and bound runs performed, including the current run
 *
 *  @return the number of branch and bound runs performed, including the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
int SCIPgetNRuns(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNRuns", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->nruns;
}

/** gets number of reoptimization runs performed, including the current run
 *
 *  @return the number of reoptimization runs performed, including the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
int SCIPgetNReoptRuns(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNReoptRuns", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->nreoptruns;
}

/** add given number to the number of processed nodes in current run and in all runs, including the focus node
 *
 *  @return the number of processed nodes in current run, including the focus node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
void SCIPaddNNodes(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Longint          nnodes              /**< number of processed nodes to add to the statistics */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPaddNNodes", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   scip->stat->nnodes += nnodes;
   scip->stat->ntotalnodes += nnodes;
}

/** gets number of processed nodes in current run, including the focus node
 *
 *  @return the number of processed nodes in current run, including the focus node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetNNodes(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNNodes", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->nnodes;
}

/** gets total number of processed nodes in all runs, including the focus node
 *
 *  @return the total number of processed nodes in all runs, including the focus node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetNTotalNodes(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNTotalNodes", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->ntotalnodes;
}

/** gets number of nodes left in the tree (children + siblings + leaves)
 *
 *  @return the number of nodes left in the tree (children + siblings + leaves)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNNodesLeft(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNNodesLeft", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetNNodes(scip->tree);
}

/** gets number of leaf nodes processed with feasible relaxation solution
 *
 * @return number of leaf nodes processed with feasible relaxation solution
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetNFeasibleLeaves(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNFeasibleLeaves", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->nfeasleaves;
}

/** gets number of infeasible leaf nodes processed
 *
 * @return number of infeasible leaf nodes processed
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetNInfeasibleLeaves(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNInfeasibleLeaves", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->ninfeasleaves;
}

/** gets number of processed leaf nodes that hit LP objective limit
 *
 * @return number of processed leaf nodes that hit LP objective limit
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetNObjlimLeaves(
   SCIP*                 scip                /**< Scip data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNObjlimLeaves", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->nobjleaves;
}


/** gets number of times a selected node was from a cut off subtree
 *
 *  @return number of times a selected node was from a cut off subtree
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetNDelayedCutoffs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNDelayedCutoffs", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->ndelayedcutoffs;
}

/** gets total number of LPs solved so far
 *
 *  @return the total number of LPs solved so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_Longint SCIPgetNLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNLPs", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   return scip->stat->nlps;
}

/** gets total number of iterations used so far in primal and dual simplex and barrier algorithm
 *
 *  @return the total number of iterations used so far in primal and dual simplex and barrier algorithm
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nlpiterations;
}

/** gets number of active non-zeros in the current transformed problem
 *
 *  @return the number of active non-zeros in the current transformed problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetNNZs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNNZs", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->stat->nnz;
}

/** gets total number of iterations used so far in primal and dual simplex and barrier algorithm for the root node
 *
 *  @return the total number of iterations used so far in primal and dual simplex and barrier algorithm for the root node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNRootLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNRootLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nrootlpiterations;
}

/** gets total number of iterations used in primal and dual simplex and barrier algorithm for the first LP at the root
 *  node
 *
 *  @return the total number of iterations used in primal and dual simplex and barrier algorithm for the first root LP
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNRootFirstLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNRootFirstLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nrootfirstlpiterations;
}

/** gets total number of primal LPs solved so far
 *
 *  @return the total number of primal LPs solved so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNPrimalLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPrimalLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nprimallps;
}

/** gets total number of iterations used so far in primal simplex
 *
 *  @return total number of iterations used so far in primal simplex
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNPrimalLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPrimalLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nprimallpiterations;
}

/** gets total number of dual LPs solved so far
 *
 *  @return the total number of dual LPs solved so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNDualLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNDualLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nduallps;
}

/** gets total number of iterations used so far in dual simplex
 *
 *  @return the total number of iterations used so far in dual simplex
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNDualLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNDualLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nduallpiterations;
}

/** gets total number of barrier LPs solved so far
 *
 *  @return the total number of barrier LPs solved so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNBarrierLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNBarrierLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nbarrierlps;
}

/** gets total number of iterations used so far in barrier algorithm
 *
 *  @return the total number of iterations used so far in barrier algorithm
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNBarrierLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNBarrierLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nbarrierlpiterations;
}

/** gets total number of LPs solved so far that were resolved from an advanced start basis
 *
 *  @return the total number of LPs solved so far that were resolved from an advanced start basis
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNResolveLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNResolveLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nprimalresolvelps + scip->stat->ndualresolvelps;
}

/** gets total number of simplex iterations used so far in primal and dual simplex calls where an advanced start basis
 *  was available
 *
 *  @return the total number of simplex iterations used so far in primal and dual simplex calls where an advanced start
 *          basis was available
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNResolveLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNResolveLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nprimalresolvelpiterations + scip->stat->ndualresolvelpiterations;
}

/** gets total number of primal LPs solved so far that were resolved from an advanced start basis
 *
 *  @return the total number of primal LPs solved so far that were resolved from an advanced start basis
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNPrimalResolveLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPrimalResolveLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nprimalresolvelps;
}

/** gets total number of simplex iterations used so far in primal simplex calls where an advanced start basis
 *  was available
 *
 *  @return the total number of simplex iterations used so far in primal simplex calls where an advanced start
 *          basis was available
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNPrimalResolveLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPrimalResolveLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nprimalresolvelpiterations;
}

/** gets total number of dual LPs solved so far that were resolved from an advanced start basis
 *
 *  @return the total number of dual LPs solved so far that were resolved from an advanced start basis
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNDualResolveLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNDualResolveLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->ndualresolvelps;
}

/** gets total number of simplex iterations used so far in dual simplex calls where an advanced start basis
 *  was available
 *
 *  @return the total number of simplex iterations used so far in dual simplex calls where an advanced start
 *          basis was available
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNDualResolveLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNDualResolveLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->ndualresolvelpiterations;
}

/** gets total number of LPs solved so far for node relaxations
 *
 *  @return the total number of LPs solved so far for node relaxations
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNNodeLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNNodeLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nnodelps;
}

/** gets total number of simplex iterations used so far for node relaxations
 *
 *  @return the total number of simplex iterations used so far for node relaxations
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNNodeLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNNodeLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nnodelpiterations;
}

/** gets total number of LPs solved so far for initial LP in node relaxations
 *
 *  @return the total number of LPs solved so far for initial LP in node relaxations
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNNodeInitLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNNodeInitLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->ninitlps;
}

/** gets total number of simplex iterations used so far for initial LP in node relaxations
 *
 *  @return the total number of simplex iterations used so far for initial LP in node relaxations
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNNodeInitLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNNodeInitLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->ninitlpiterations;
}

/** gets total number of LPs solved so far during diving and probing
 *
 *  @return total number of LPs solved so far during diving and probing
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNDivingLPs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNDivingLPs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->ndivinglps;
}

/** gets total number of simplex iterations used so far during diving and probing
 *
 *  @return the total number of simplex iterations used so far during diving and probing
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNDivingLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNDivingLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->ndivinglpiterations;
}

/** gets total number of times, strong branching was called (each call represents solving two LPs)
 *
 *  @return the total number of times, strong branching was called (each call represents solving two LPs)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNStrongbranchs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNStrongbranchs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nstrongbranchs;
}

/** gets total number of simplex iterations used so far in strong branching
 *
 *  @return the total number of simplex iterations used so far in strong branching
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNStrongbranchLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNStrongbranchLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nsblpiterations;
}

/** gets total number of times, strong branching was called at the root node (each call represents solving two LPs)
 *
 *  @return the total number of times, strong branching was called at the root node (each call represents solving two LPs)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNRootStrongbranchs(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNRootStrongbranchs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nrootstrongbranchs;
}

/** gets total number of simplex iterations used so far in strong branching at the root node
 *
 *  @return the total number of simplex iterations used so far in strong branching at the root node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Longint SCIPgetNRootStrongbranchLPIterations(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNRootStrongbranchLPIterations", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nrootsblpiterations;
}

/** gets number of pricing rounds performed so far at the current node
 *
 *  @return the number of pricing rounds performed so far at the current node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetNPriceRounds(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPriceRounds", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return scip->stat->npricerounds;
}

/** get current number of variables in the pricing store
 *
 *  @return the current number of variables in the pricing store
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNPricevars(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPricevars", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPpricestoreGetNVars(scip->pricestore);
}

/** get total number of pricing variables found so far
 *
 *  @return the total number of pricing variables found so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNPricevarsFound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPricevarsFound", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPpricestoreGetNVarsFound(scip->pricestore);
}

/** get total number of pricing variables applied to the LPs
 *
 *  @return the total number of pricing variables applied to the LPs
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNPricevarsApplied(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNPricevarsApplied", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPpricestoreGetNVarsApplied(scip->pricestore);
}

/** gets number of separation rounds performed so far at the current node
 *
 *  @return the number of separation rounds performed so far at the current node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetNSepaRounds(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNSepaRounds", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return scip->stat->nseparounds;
}

/** get total number of cuts found so far
 *
 *  @return the total number of cuts found so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNCutsFound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNCutsFound", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPsepastoreGetNCutsFound(scip->sepastore);
}

/** get number of cuts found so far in current separation round
 *
 *  @return the number of cuts found so far in current separation round
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNCutsFoundRound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNCutsFoundRound", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPsepastoreGetNCutsFoundRound(scip->sepastore);
}

/** get total number of cuts applied to the LPs
 *
 *  @return the total number of cuts applied to the LPs
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNCutsApplied(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNCutsApplied", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPsepastoreGetNCutsApplied(scip->sepastore);
}

/** get total number of constraints found in conflict analysis (conflict, reconvergence constraints, and dual proofs)
 *
 *  @return the total number of constraints found in conflict analysis (conflict, reconvergence constraints, and dual proofs)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetNConflictConssFound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNConflictConssFound", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPconflictGetNPropConflictConss(scip->conflict)
      + SCIPconflictGetNPropReconvergenceConss(scip->conflict)
      + SCIPconflictGetNInfeasibleLPConflictConss(scip->conflict)
      + SCIPconflictGetNInfeasibleLPReconvergenceConss(scip->conflict)
      + SCIPconflictGetNBoundexceedingLPConflictConss(scip->conflict)
      + SCIPconflictGetNBoundexceedingLPReconvergenceConss(scip->conflict)
      + SCIPconflictGetNStrongbranchConflictConss(scip->conflict)
      + SCIPconflictGetNStrongbranchReconvergenceConss(scip->conflict)
      + SCIPconflictGetNPseudoConflictConss(scip->conflict)
      + SCIPconflictGetNPseudoReconvergenceConss(scip->conflict)
      + SCIPconflictGetNDualrayBndGlobal(scip->conflict)
      + SCIPconflictGetNDualrayInfGlobal(scip->conflict);
}

/** get number of conflict constraints found so far at the current node
 *
 *  @return the number of conflict constraints found so far at the current node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetNConflictConssFoundNode(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNConflictConssFoundNode", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPconflictGetNConflicts(scip->conflict);
}

/** get total number of conflict constraints added to the problem
 *
 *  @return the total number of conflict constraints added to the problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetNConflictConssApplied(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNConflictConssApplied", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPconflictGetNAppliedConss(scip->conflict);
}

/** gets depth of current node, or -1 if no current node exists; in probing, the current node is the last probing node,
 *  such that the depth includes the probing path
 *
 *  @return the depth of current node, or -1 if no current node exists; in probing, the current node is the last probing node,
 *  such that the depth includes the probing path
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetDepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetDepth", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPtreeGetCurrentDepth(scip->tree);
}

/** gets depth of the focus node, or -1 if no focus node exists; the focus node is the currently processed node in the
 *  branching tree, excluding the nodes of the probing path
 *
 *  @return the depth of the focus node, or -1 if no focus node exists; the focus node is the currently processed node in the
 *  branching tree, excluding the nodes of the probing path
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetFocusDepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetFocusDepth", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPtreeGetFocusDepth(scip->tree);
}

/** gets maximal depth of all processed nodes in current branch and bound run (excluding probing nodes)
 *
 *  @return the maximal depth of all processed nodes in current branch and bound run (excluding probing nodes)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetMaxDepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetMaxDepth", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->stat->maxdepth;
}

/** gets maximal depth of all processed nodes over all branch and bound runs
 *
 *  @return the maximal depth of all processed nodes over all branch and bound runs
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
int SCIPgetMaxTotalDepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetMaxTotalDepth", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->stat->maxtotaldepth;
}

/** gets total number of backtracks, i.e. number of times, the new node was selected from the leaves queue
 *
 *  @return the total number of backtracks, i.e. number of times, the new node was selected from the leaves queue
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetNBacktracks(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNBacktracks", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->stat->nbacktracks;
}

/** gets current plunging depth (successive times, a child was selected as next node)
 *
 *  @return the current plunging depth (successive times, a child was selected as next node)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetPlungeDepth(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPlungeDepth", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return scip->stat->plungedepth;
}

/** gets total number of active constraints at the current node
 *
 *  @return the total number of active constraints at the current node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetNActiveConss(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNActiveConss", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return scip->stat->nactiveconss;
}

/** gets total number of enabled constraints at the current node
 *
 *  @return the total number of enabled constraints at the current node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 */
int SCIPgetNEnabledConss(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNEnabledConss", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   return scip->stat->nenabledconss;
}

/** gets average dual bound of all unprocessed nodes for original problem
 *
 *  @return the average dual bound of all unprocessed nodes for original problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgDualbound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgDualbound", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set,
	 SCIPtreeGetAvgLowerbound(scip->tree, scip->primal->cutoffbound));
}

/** gets average lower (dual) bound of all unprocessed nodes in transformed problem
 *
 *  @return the average lower (dual) bound of all unprocessed nodes in transformed problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgLowerbound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgLowerbound", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPtreeGetAvgLowerbound(scip->tree, scip->primal->cutoffbound);
}

/** gets global dual bound
 *
 *  @return the global dual bound
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Real SCIPgetDualbound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetDualbound", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   /* in case we are in presolving we use the stored dual bound if it exits */
   if( scip->set->stage <= SCIP_STAGE_INITSOLVE && scip->transprob->dualbound < SCIP_INVALID )
      return scip->transprob->dualbound;

   return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, SCIPgetLowerbound(scip));
}

/** gets global lower (dual) bound in transformed problem
 *
 *  @return the global lower (dual) bound in transformed problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetLowerbound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetLowerbound", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( scip->set->stage <= SCIP_STAGE_INITSOLVE )
      return -SCIPinfinity(scip);
   else if( SCIPgetStatus(scip) == SCIP_STATUS_INFORUNBD || SCIPgetStatus(scip) == SCIP_STATUS_UNBOUNDED )
   {
      /* in case we could not prove whether the problem is unbounded or infeasible, we want to terminate with lower
       * bound = -inf instead of lower bound = upper bound = +inf also in case we prove that the problem is unbounded,
       * it seems to make sense to return with lower bound = -inf, since -infinity is the only valid lower bound
       */
      return -SCIPinfinity(scip);
   }
   else
   {
      SCIP_Real treelowerbound;

      /* it may happen that the remaining tree is empty or all open nodes have a lower bound above the cutoff bound, but
       * have not yet been cut off, e.g., when the user calls SCIPgetDualbound() in some event handler; in this case,
       * the global lower bound is given by the upper bound value
       */
      treelowerbound = SCIPtreeGetLowerbound(scip->tree, scip->set);

      if( treelowerbound < scip->primal->upperbound)
         return treelowerbound;
      else
         return scip->primal->upperbound;
   }
}

/** gets dual bound of the root node for the original problem
 *
 *  @return the dual bound of the root node for the original problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetDualboundRoot(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetDualboundRoot", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( SCIPsetIsInfinity(scip->set, scip->stat->rootlowerbound) )
      return SCIPgetPrimalbound(scip);
   else
      return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, scip->stat->rootlowerbound);
}

/** gets lower (dual) bound in transformed problem of the root node
 *
 *  @return the lower (dual) bound in transformed problem of the root node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetLowerboundRoot(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetLowerboundRoot", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( SCIPsetIsInfinity(scip->set, scip->stat->rootlowerbound) )
      return SCIPgetUpperbound(scip);
   else
      return scip->stat->rootlowerbound;
}

/** gets dual bound for the original problem obtained by the first LP solve at the root node
 *
 *  @return the dual bound for the original problem of the first LP solve at the root node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetFirstLPDualboundRoot(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetFirstLPDualboundRoot", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->firstlpdualbound;
}

/** gets lower (dual) bound in transformed problem obtained by the first LP solve at the root node
 *
 *  @return the lower (dual) bound in transformed problem obtained by first LP solve at the root node
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetFirstLPLowerboundRoot(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetFirstLPLowerboundRoot", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( scip->stat->firstlpdualbound == SCIP_INVALID ) /*lint !e777*/
      return -SCIPinfinity(scip);
   else
      return SCIPprobInternObjval(scip->transprob, scip->origprob, scip->set, scip->stat->firstlpdualbound);
}

/** the primal bound of the very first solution */
SCIP_Real SCIPgetFirstPrimalBound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   return scip->stat->firstprimalbound;
}

/** gets global primal bound (objective value of best solution or user objective limit) for the original problem
 *
 *  @return the global primal bound (objective value of best solution or user objective limit) for the original problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Real SCIPgetPrimalbound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPrimalbound", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPprobExternObjval(scip->transprob, scip->origprob, scip->set, SCIPgetUpperbound(scip));
}

/** gets global upper (primal) bound in transformed problem (objective value of best solution or user objective limit)
 *
 *  @return the global upper (primal) bound in transformed problem (objective value of best solution or user objective limit)
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Real SCIPgetUpperbound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetUpperbound", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   if( SCIPgetStatus(scip) == SCIP_STATUS_UNBOUNDED )
      return -SCIPinfinity(scip);
   else
      return scip->primal->upperbound;
}

/** gets global cutoff bound in transformed problem: a sub problem with lower bound larger than the cutoff
 *  cannot contain a better feasible solution; usually, this bound is equal to the upper bound, but if the
 *  objective value is always integral, the cutoff bound is (nearly) one less than the upper bound;
 *  additionally, due to objective function domain propagation, the cutoff bound can be further reduced
 *
 *  @return global cutoff bound in transformed problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Real SCIPgetCutoffbound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetCutoffbound", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->primal->cutoffbound;
}

/** updates the cutoff bound
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @note using this method in the solving stage can lead to an erroneous SCIP solving status; in particular,
 *        if a solution not respecting the cutoff bound was found before installing a cutoff bound which
 *        renders the remaining problem infeasible, this solution may be reported as optimal
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *
 *  @note the given cutoff bound has to better or equal to known one (SCIPgetCutoffbound())
 *  @note a given cutoff bound is also used for updating the objective limit, if possible
 */
SCIP_RETCODE SCIPupdateCutoffbound(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             cutoffbound         /**< new cutoff bound */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPupdateCutoffbound", FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   assert(cutoffbound <= SCIPgetCutoffbound(scip));

   SCIP_CALL( SCIPprimalSetCutoffbound(scip->primal, scip->mem->probmem, scip->set, scip->stat, scip->eventqueue,
         scip->transprob, scip->origprob, scip->tree, scip->reopt, scip->lp, cutoffbound, FALSE) );

   return SCIP_OKAY;
}


/** returns whether the current primal bound is justified with a feasible primal solution; if not, the primal bound
 *  was set from the user as objective limit
 *
 *  @return TRUE if the current primal bound is justified with a feasible primal solution, otherwise FALSE
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Bool SCIPisPrimalboundSol(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPisPrimalboundSol", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return SCIPprimalUpperboundIsSol(scip->primal, scip->set, scip->transprob, scip->origprob);
}

/** gets current gap |(primalbound - dualbound)/min(|primalbound|,|dualbound|)| if both bounds have same sign,
 *  or infinity, if they have opposite sign
 *
 *  @return the current gap |(primalbound - dualbound)/min(|primalbound|,|dualbound|)| if both bounds have same sign,
 *  or infinity, if they have opposite sign
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetGap(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetGap", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* in case we could not prove whether the problem is unbounded or infeasible, we want to terminate with gap = +inf;
    * if the problem was proven to be unbounded or proven to be infeasible we return gap = 0
    */
   if( SCIPgetStatus(scip) == SCIP_STATUS_INFORUNBD )
      return SCIPsetInfinity(scip->set);
   else if( SCIPgetStatus(scip) == SCIP_STATUS_INFEASIBLE || SCIPgetStatus(scip) == SCIP_STATUS_UNBOUNDED )
      return 0.0;

   /* the lowerbound is infinity, but SCIP may not have updated the status; in this case, the problem was already solved
    * so we return gap = 0
    */
   if( SCIPsetIsInfinity(scip->set, SCIPgetLowerbound(scip)) )
      return 0.0;

   return SCIPcomputeGap(SCIPsetEpsilon(scip->set), SCIPsetInfinity(scip->set), SCIPgetPrimalbound(scip), SCIPgetDualbound(scip));
}

/** gets current gap |(upperbound - lowerbound)/min(|upperbound|,|lowerbound|)| in transformed problem if both bounds
 *  have same sign, or infinity, if they have opposite sign
 *
 *  @return current gap |(upperbound - lowerbound)/min(|upperbound|,|lowerbound|)| in transformed problem if both bounds
 *  have same sign, or infinity, if they have opposite sign
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetTransGap(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetTransGap", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* in case we could not prove whether the problem is unbounded or infeasible, we want to terminate with gap = +inf;
    * if the problem was proven to be unbounded or proven to be infeasible we return gap = 0
    */
   if( SCIPgetStatus(scip) == SCIP_STATUS_INFORUNBD )
      return SCIPsetInfinity(scip->set);
   else if( SCIPgetStatus(scip) == SCIP_STATUS_INFEASIBLE || SCIPgetStatus(scip) == SCIP_STATUS_UNBOUNDED )
      return 0.0;

   /* the lowerbound is infinity, but SCIP may not have updated the status; in this case, the problem was already solved
    * so we return gap = 0
    */
   if( SCIPsetIsInfinity(scip->set, SCIPgetLowerbound(scip)) )
      return 0.0;

   return SCIPcomputeGap(SCIPsetEpsilon(scip->set), SCIPsetInfinity(scip->set), SCIPgetUpperbound(scip), SCIPgetLowerbound(scip));
}

/** gets number of feasible primal solutions found so far
 *
 *  @return the number of feasible primal solutions found so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetNSolsFound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNSolsFound", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->primal->nsolsfound;
}

/** gets number of feasible primal solutions respecting the objective limit found so far
 *
 *  @return the number of feasible primal solutions respecting the objective limit found so far
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetNLimSolsFound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   if( SCIPgetStage(scip) < SCIP_STAGE_TRANSFORMED)
      return 0;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNLimSolsFound", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->primal->nlimsolsfound;
}

/** gets number of feasible primal solutions found so far, that improved the primal bound at the time they were found
 *
 *  @return the number of feasible primal solutions found so far, that improved the primal bound at the time they were found
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 */
SCIP_Longint SCIPgetNBestSolsFound(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNBestSolsFound", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE) );

   return scip->primal->nbestsolsfound;
}

/** gets the average pseudo cost value for the given direction over all variables
 *
 *  @return the average pseudo cost value for the given direction over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgPseudocost(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             solvaldelta         /**< difference of variable's new LP value - old LP value */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgPseudocost", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetPseudocost(scip->stat->glbhistory, solvaldelta);
}

/** gets the average pseudo cost value for the given direction over all variables,
 *  only using the pseudo cost information of the current run
 *
 *  @return the average pseudo cost value for the given direction over all variables,
 *  only using the pseudo cost information of the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgPseudocostCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             solvaldelta         /**< difference of variable's new LP value - old LP value */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgPseudocostCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetPseudocost(scip->stat->glbhistorycrun, solvaldelta);
}

/** gets the average number of pseudo cost updates for the given direction over all variables
 *
 *  @return the average number of pseudo cost updates for the given direction over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgPseudocostCount(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgPseudocostCount", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetPseudocostCount(scip->stat->glbhistory, dir)
      / MAX(scip->transprob->nbinvars + scip->transprob->nintvars, 1);
}

/** gets the average number of pseudo cost updates for the given direction over all variables,
 *  only using the pseudo cost information of the current run
 *
 *  @return the average number of pseudo cost updates for the given direction over all variables,
 *  only using the pseudo cost information of the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgPseudocostCountCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgPseudocostCountCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetPseudocostCount(scip->stat->glbhistorycrun, dir)
      / MAX(scip->transprob->nbinvars + scip->transprob->nintvars, 1);
}

/** gets the average pseudo cost score value over all variables, assuming a fractionality of 0.5
 *
 *  @return the average pseudo cost score value over all variables, assuming a fractionality of 0.5
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgPseudocostScore(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real pscostdown;
   SCIP_Real pscostup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgPseudocostScore", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   pscostdown = SCIPhistoryGetPseudocost(scip->stat->glbhistory, -0.5);
   pscostup = SCIPhistoryGetPseudocost(scip->stat->glbhistory, +0.5);

   return SCIPbranchGetScore(scip->set, NULL, pscostdown, pscostup);
}

/** returns the variance of pseudo costs for all variables in the requested direction
 *
 *  @return the variance of pseudo costs for all variables in the requested direction
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetPseudocostVariance(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        branchdir,          /**< the branching direction, up or down */
   SCIP_Bool             onlycurrentrun      /**< use only history of current run? */
   )
{
   SCIP_HISTORY* history;

   assert(scip != NULL);
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPseudocostVariance", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   history = (onlycurrentrun ? scip->stat->glbhistorycrun : scip->stat->glbhistory);
   assert(history != NULL);

   return SCIPhistoryGetPseudocostVariance(history, branchdir);
}

/** gets the number of pseudo cost updates for the given direction over all variables
 *
 *  @return the number of pseudo cost updates for the given direction over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetPseudocostCount(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        dir,                /**< branching direction (downwards, or upwards) */
   SCIP_Bool             onlycurrentrun      /**< use only history of current run? */
   )
{
   SCIP_HISTORY* history;

   assert(scip != NULL);
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPseudocostCount", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   history = (onlycurrentrun ? scip->stat->glbhistorycrun : scip->stat->glbhistory);

   return SCIPhistoryGetPseudocostCount(history, dir);
}

/** gets the average pseudo cost score value over all variables, assuming a fractionality of 0.5,
 *  only using the pseudo cost information of the current run
 *
 *  @return the average pseudo cost score value over all variables, assuming a fractionality of 0.5,
 *  only using the pseudo cost information of the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgPseudocostScoreCurrentRun(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real pscostdown;
   SCIP_Real pscostup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgPseudocostScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   pscostdown = SCIPhistoryGetPseudocost(scip->stat->glbhistorycrun, -0.5);
   pscostup = SCIPhistoryGetPseudocost(scip->stat->glbhistorycrun, +0.5);

   return SCIPbranchGetScore(scip->set, NULL, pscostdown, pscostup);
}

/** gets the average conflict score value over all variables
 *
 *  @return the average conflict score value over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgConflictScore(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real conflictscoredown;
   SCIP_Real conflictscoreup;
   SCIP_Real scale;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgConflictScore", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   scale = scip->transprob->nvars * scip->stat->vsidsweight;
   conflictscoredown = SCIPhistoryGetVSIDS(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS) / scale;
   conflictscoreup = SCIPhistoryGetVSIDS(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS) / scale;

   return SCIPbranchGetScore(scip->set, NULL, conflictscoredown, conflictscoreup);
}

/** gets the average conflict score value over all variables, only using the conflict score information of the current run
 *
 *  @return the average conflict score value over all variables, only using the conflict score information of the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgConflictScoreCurrentRun(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real conflictscoredown;
   SCIP_Real conflictscoreup;
   SCIP_Real scale;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgConflictScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   scale = scip->transprob->nvars * scip->stat->vsidsweight;
   conflictscoredown = SCIPhistoryGetVSIDS(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_DOWNWARDS) / scale;
   conflictscoreup = SCIPhistoryGetVSIDS(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_UPWARDS) / scale;

   return SCIPbranchGetScore(scip->set, NULL, conflictscoredown, conflictscoreup);
}

/** gets the average inference score value over all variables
 *
 *  @return the average inference score value over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgConflictlengthScore(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real conflictlengthdown;
   SCIP_Real conflictlengthup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgConflictlengthScore", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   conflictlengthdown = SCIPhistoryGetAvgConflictlength(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS);
   conflictlengthup = SCIPhistoryGetAvgConflictlength(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, NULL, conflictlengthdown, conflictlengthup);
}

/** gets the average conflictlength score value over all variables, only using the conflictlength information of the
 *  current run
 *
 *  @return the average conflictlength score value over all variables, only using the conflictlength information of the
 *          current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgConflictlengthScoreCurrentRun(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real conflictlengthdown;
   SCIP_Real conflictlengthup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgConflictlengthScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   conflictlengthdown = SCIPhistoryGetAvgConflictlength(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_DOWNWARDS);
   conflictlengthup = SCIPhistoryGetAvgConflictlength(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, NULL, conflictlengthdown, conflictlengthup);
}

/** returns the average number of inferences found after branching in given direction over all variables
 *
 *  @return the average number of inferences found after branching in given direction over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgInferences(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgInferences", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetAvgInferences(scip->stat->glbhistory, dir);
}

/** returns the average number of inferences found after branching in given direction over all variables,
 *  only using the inference information of the current run
 *
 *  @return the average number of inferences found after branching in given direction over all variables,
 *          only using the inference information of the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgInferencesCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgInferencesCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetAvgInferences(scip->stat->glbhistorycrun, dir);
}

/** gets the average inference score value over all variables
 *
 *  @return the average inference score value over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgInferenceScore(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real inferencesdown;
   SCIP_Real inferencesup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgInferenceScore", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   inferencesdown = SCIPhistoryGetAvgInferences(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS);
   inferencesup = SCIPhistoryGetAvgInferences(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, NULL, inferencesdown, inferencesup);
}

/** gets the average inference score value over all variables, only using the inference information of the
 *  current run
 *
 *  @return the average inference score value over all variables, only using the inference information of the
 *          current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgInferenceScoreCurrentRun(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real inferencesdown;
   SCIP_Real inferencesup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgInferenceScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   inferencesdown = SCIPhistoryGetAvgInferences(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_DOWNWARDS);
   inferencesup = SCIPhistoryGetAvgInferences(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, NULL, inferencesdown, inferencesup);
}

/** returns the average number of cutoffs found after branching in given direction over all variables
 *
 *  @return the average number of cutoffs found after branching in given direction over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgCutoffs(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgCutoffs", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetAvgCutoffs(scip->stat->glbhistory, dir);
}

/** returns the average number of cutoffs found after branching in given direction over all variables,
 *  only using the cutoff information of the current run
 *
 *  @return the average number of cutoffs found after branching in given direction over all variables,
 *          only using the cutoff information of the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgCutoffsCurrentRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_BRANCHDIR        dir                 /**< branching direction (downwards, or upwards) */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgCutoffsCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPhistoryGetAvgCutoffs(scip->stat->glbhistorycrun, dir);
}

/** gets the average cutoff score value over all variables
 *
 *  @return the average cutoff score value over all variables
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgCutoffScore(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real cutoffsdown;
   SCIP_Real cutoffsup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgCutoffScore", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   cutoffsdown = SCIPhistoryGetAvgCutoffs(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS);
   cutoffsup = SCIPhistoryGetAvgCutoffs(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, NULL, cutoffsdown, cutoffsup);
}

/** gets the average cutoff score value over all variables, only using the cutoff score information of the current run
 *
 *  @return the average cutoff score value over all variables, only using the cutoff score information of the current run
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetAvgCutoffScoreCurrentRun(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real cutoffsdown;
   SCIP_Real cutoffsup;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetAvgCutoffScoreCurrentRun", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   cutoffsdown = SCIPhistoryGetAvgCutoffs(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_DOWNWARDS);
   cutoffsup = SCIPhistoryGetAvgCutoffs(scip->stat->glbhistorycrun, SCIP_BRANCHDIR_UPWARDS);

   return SCIPbranchGetScore(scip->set, NULL, cutoffsdown, cutoffsup);
}

/** computes a deterministic measure of time from statistics
 *
 *  @return the deterministic  time
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_Real SCIPgetDeterministicTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
/* TODO:    SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetDeterministicTime", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) ); */
   if(scip->stat == NULL)
      return 0.0;

   return 1e-6 * scip->stat->nnz * (
          0.00328285264101 * scip->stat->nprimalresolvelpiterations +
          0.00531625104146 * scip->stat->ndualresolvelpiterations +
          0.000738719124051 * scip->stat->nprobboundchgs +
          0.0011123144764 * scip->stat->nisstoppedcalls );
}

/** outputs problem to file stream */
static
SCIP_RETCODE printProblem(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_PROB*            prob,               /**< problem data */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   const char*           extension,          /**< file format (or NULL for default CIP format) */
   SCIP_Bool             genericnames        /**< using generic variable and constraint names? */
   )
{
   SCIP_RESULT result;
   int i;
   assert(scip != NULL);
   assert(prob != NULL);

   /* try all readers until one could read the file */
   result = SCIP_DIDNOTRUN;
   for( i = 0; i < scip->set->nreaders && result == SCIP_DIDNOTRUN; ++i )
   {
      SCIP_RETCODE retcode;

      if( extension != NULL )
         retcode = SCIPreaderWrite(scip->set->readers[i], prob, scip->set, file, extension, genericnames, &result);
      else
         retcode = SCIPreaderWrite(scip->set->readers[i], prob, scip->set, file, "cip", genericnames, &result);

      /* check for reader errors */
      if( retcode == SCIP_WRITEERROR )
         return retcode;

      SCIP_CALL( retcode );
   }

   switch( result )
   {
   case SCIP_DIDNOTRUN:
      return SCIP_PLUGINNOTFOUND;

   case SCIP_SUCCESS:
      return SCIP_OKAY;

   default:
      assert(i < scip->set->nreaders);
      SCIPerrorMessage("invalid result code <%d> from reader <%s> writing <%s> format\n",
         result, SCIPreaderGetName(scip->set->readers[i]), extension);
      return SCIP_READERROR;
   }  /*lint !e788*/
}

/** outputs original problem to file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPprintOrigProblem(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   const char*           extension,          /**< file format (or NULL for default CIP format)*/
   SCIP_Bool             genericnames        /**< using generic variable and constraint names? */
   )
{
   SCIP_RETCODE retcode;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintOrigProblem", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   assert(scip != NULL);
   assert( scip->origprob != NULL );

   retcode = printProblem(scip, scip->origprob, file, extension, genericnames);

   /* check for write errors */
   if( retcode == SCIP_WRITEERROR || retcode == SCIP_PLUGINNOTFOUND )
      return retcode;
   else
   {
      SCIP_CALL( retcode );
   }

   return SCIP_OKAY;
}

/** outputs transformed problem of the current node to file stream
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 */
SCIP_RETCODE SCIPprintTransProblem(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   const char*           extension,          /**< file format (or NULL for default CIP format)*/
   SCIP_Bool             genericnames        /**< using generic variable and constraint names? */
   )
{
   SCIP_RETCODE retcode;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintTransProblem", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   assert(scip != NULL);
   assert(scip->transprob != NULL );

   retcode = printProblem(scip, scip->transprob, file, extension, genericnames);

   /* check for write errors */
   if( retcode == SCIP_WRITEERROR || retcode == SCIP_PLUGINNOTFOUND )
      return retcode;
   else
   {
      SCIP_CALL( retcode );
   }

   return SCIP_OKAY;
}

/** outputs status statistics
 *
 *  @note If limits have been changed between the solution and the call to this function, the status is recomputed and
 *        thus may to correspond to the original status.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintStatusStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintStatusStatistics", TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "SCIP Status        : ");
   SCIP_CALL_ABORT( SCIPprintStage(scip, file) );
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "\n");
}

/** outputs statistics for original problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintOrigProblemStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintOrigProblemStatistics", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Original Problem   :\n");
   SCIPprobPrintStatistics(scip->origprob, scip->set, scip->messagehdlr, file);
}

/** outputs statistics for transformed problem
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintTransProblemStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintTransProblemStatistics", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Presolved Problem  :\n");
   SCIPprobPrintStatistics(scip->transprob, scip->set, scip->messagehdlr, file);
}

/** outputs presolver statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintPresolverStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintPresolverStatistics", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Presolvers         :   ExecTime  SetupTime  Calls  FixedVars   AggrVars   ChgTypes  ChgBounds   AddHoles    DelCons    AddCons   ChgSides   ChgCoefs\n");

   /* sort presolvers w.r.t. their name */
   SCIPsetSortPresolsName(scip->set);

   /* presolver statistics */
   for( i = 0; i < scip->set->npresols; ++i )
   {
      SCIP_PRESOL* presol;
      presol = scip->set->presols[i];
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s:", SCIPpresolGetName(presol));
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10.2f %6d %10d %10d %10d %10d %10d %10d %10d %10d %10d\n",
         SCIPpresolGetTime(presol),
         SCIPpresolGetSetupTime(presol),
         SCIPpresolGetNCalls(presol),
         SCIPpresolGetNFixedVars(presol),
         SCIPpresolGetNAggrVars(presol),
         SCIPpresolGetNChgVarTypes(presol),
         SCIPpresolGetNChgBds(presol),
         SCIPpresolGetNAddHoles(presol),
         SCIPpresolGetNDelConss(presol),
         SCIPpresolGetNAddConss(presol),
         SCIPpresolGetNChgSides(presol),
         SCIPpresolGetNChgCoefs(presol));
   }

   /* sort propagators w.r.t. their name */
   SCIPsetSortPropsName(scip->set);

   for( i = 0; i < scip->set->nprops; ++i )
   {
      SCIP_PROP* prop;
      prop = scip->set->props[i];
      if( SCIPpropDoesPresolve(prop) )
      {
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s:", SCIPpropGetName(prop));
         SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10.2f %6d %10d %10d %10d %10d %10d %10d %10d %10d %10d\n",
            SCIPpropGetPresolTime(prop),
	    SCIPpropGetSetupTime(prop),
            SCIPpropGetNPresolCalls(prop),
            SCIPpropGetNFixedVars(prop),
            SCIPpropGetNAggrVars(prop),
            SCIPpropGetNChgVarTypes(prop),
            SCIPpropGetNChgBds(prop),
            SCIPpropGetNAddHoles(prop),
            SCIPpropGetNDelConss(prop),
            SCIPpropGetNAddConss(prop),
            SCIPpropGetNChgSides(prop),
            SCIPpropGetNChgCoefs(prop));
      }
   }

   /* constraint handler presolving methods statistics */
   for( i = 0; i < scip->set->nconshdlrs; ++i )
   {
      SCIP_CONSHDLR* conshdlr;
      int maxnactiveconss;

      conshdlr = scip->set->conshdlrs[i];
      maxnactiveconss = SCIPconshdlrGetMaxNActiveConss(conshdlr);
      if( SCIPconshdlrDoesPresolve(conshdlr)
         && (maxnactiveconss > 0 || !SCIPconshdlrNeedsCons(conshdlr)
            || SCIPconshdlrGetNFixedVars(conshdlr) > 0
            || SCIPconshdlrGetNAggrVars(conshdlr) > 0
            || SCIPconshdlrGetNChgVarTypes(conshdlr) > 0
            || SCIPconshdlrGetNChgBds(conshdlr) > 0
            || SCIPconshdlrGetNAddHoles(conshdlr) > 0
            || SCIPconshdlrGetNDelConss(conshdlr) > 0
            || SCIPconshdlrGetNAddConss(conshdlr) > 0
            || SCIPconshdlrGetNChgSides(conshdlr) > 0
            || SCIPconshdlrGetNChgCoefs(conshdlr) > 0
            || SCIPconshdlrGetNUpgdConss(conshdlr) > 0) )
      {
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s:", SCIPconshdlrGetName(conshdlr));
         SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10.2f %6d %10d %10d %10d %10d %10d %10d %10d %10d %10d\n",
            SCIPconshdlrGetPresolTime(conshdlr),
            SCIPconshdlrGetSetupTime(conshdlr),
            SCIPconshdlrGetNPresolCalls(conshdlr),
            SCIPconshdlrGetNFixedVars(conshdlr),
            SCIPconshdlrGetNAggrVars(conshdlr),
            SCIPconshdlrGetNChgVarTypes(conshdlr),
            SCIPconshdlrGetNChgBds(conshdlr),
            SCIPconshdlrGetNAddHoles(conshdlr),
            SCIPconshdlrGetNDelConss(conshdlr),
            SCIPconshdlrGetNAddConss(conshdlr),
            SCIPconshdlrGetNChgSides(conshdlr),
            SCIPconshdlrGetNChgCoefs(conshdlr));
      }
   }

   /* root node bound changes */
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  root node        :          -          -      - %10d          -          - %10d          -          -          -          -          -\n",
      scip->stat->nrootintfixings, scip->stat->nrootboundchgs);
}

/** outputs constraint statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintConstraintStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintConstraintStatistics", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* Add maximal number of constraints of the same type? So far this information is not added because of lack of space. */
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Constraints        :     Number  MaxNumber  #Separate #Propagate    #EnfoLP    #EnfoRelax  #EnfoPS    #Check   #ResProp    Cutoffs    DomReds       Cuts    Applied      Conss   Children\n");

   for( i = 0; i < scip->set->nconshdlrs; ++i )
   {
      SCIP_CONSHDLR* conshdlr;
      int startnactiveconss;
      int maxnactiveconss;

      conshdlr = scip->set->conshdlrs[i];
      startnactiveconss = SCIPconshdlrGetStartNActiveConss(conshdlr);
      maxnactiveconss = SCIPconshdlrGetMaxNActiveConss(conshdlr);
      if( maxnactiveconss > 0 || !SCIPconshdlrNeedsCons(conshdlr) )
      {
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s:", SCIPconshdlrGetName(conshdlr));
         SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10d%c%10d %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
            startnactiveconss,
            maxnactiveconss > startnactiveconss ? '+' : ' ',
            maxnactiveconss,
            SCIPconshdlrGetNSepaCalls(conshdlr),
            SCIPconshdlrGetNPropCalls(conshdlr),
            SCIPconshdlrGetNEnfoLPCalls(conshdlr),
            SCIPconshdlrGetNEnfoRelaxCalls(conshdlr),
            SCIPconshdlrGetNEnfoPSCalls(conshdlr),
            SCIPconshdlrGetNCheckCalls(conshdlr),
            SCIPconshdlrGetNRespropCalls(conshdlr),
            SCIPconshdlrGetNCutoffs(conshdlr),
            SCIPconshdlrGetNDomredsFound(conshdlr),
            SCIPconshdlrGetNCutsFound(conshdlr),
            SCIPconshdlrGetNCutsApplied(conshdlr),
            SCIPconshdlrGetNConssFound(conshdlr),
            SCIPconshdlrGetNChildren(conshdlr));
      }
   }
}

/** outputs constraint timing statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintConstraintTimingStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintConstraintTimingStatistics", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Constraint Timings :  TotalTime  SetupTime   Separate  Propagate     EnfoLP     EnfoPS     EnfoRelax   Check    ResProp    SB-Prop\n");

   for( i = 0; i < scip->set->nconshdlrs; ++i )
   {
      SCIP_CONSHDLR* conshdlr;
      int maxnactiveconss;

      conshdlr = scip->set->conshdlrs[i];
      maxnactiveconss = SCIPconshdlrGetMaxNActiveConss(conshdlr);
      if( maxnactiveconss > 0 || !SCIPconshdlrNeedsCons(conshdlr) )
      {
         SCIP_Real totaltime;

         totaltime = SCIPconshdlrGetSepaTime(conshdlr) + SCIPconshdlrGetPropTime(conshdlr)
            + SCIPconshdlrGetStrongBranchPropTime(conshdlr)
            + SCIPconshdlrGetEnfoLPTime(conshdlr)
            + SCIPconshdlrGetEnfoPSTime(conshdlr)
            + SCIPconshdlrGetEnfoRelaxTime(conshdlr)
            + SCIPconshdlrGetCheckTime(conshdlr)
            + SCIPconshdlrGetRespropTime(conshdlr)
	    + SCIPconshdlrGetSetupTime(conshdlr);

         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s:", SCIPconshdlrGetName(conshdlr));
         SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f\n",
            totaltime,
	    SCIPconshdlrGetSetupTime(conshdlr),
            SCIPconshdlrGetSepaTime(conshdlr),
            SCIPconshdlrGetPropTime(conshdlr),
            SCIPconshdlrGetEnfoLPTime(conshdlr),
            SCIPconshdlrGetEnfoPSTime(conshdlr),
            SCIPconshdlrGetEnfoRelaxTime(conshdlr),
            SCIPconshdlrGetCheckTime(conshdlr),
            SCIPconshdlrGetRespropTime(conshdlr),
            SCIPconshdlrGetStrongBranchPropTime(conshdlr));
      }
   }
}

/** outputs propagator statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintPropagatorStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintPropagatorStatistics", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Propagators        : #Propagate   #ResProp    Cutoffs    DomReds\n");

   /* sort propagaters w.r.t. their name */
   SCIPsetSortPropsName(scip->set);

   for( i = 0; i < scip->set->nprops; ++i )
   {
      SCIP_PROP* prop;
      prop = scip->set->props[i];

      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
         SCIPpropGetName(prop),
         SCIPpropGetNCalls(prop),
         SCIPpropGetNRespropCalls(prop),
         SCIPpropGetNCutoffs(prop),
         SCIPpropGetNDomredsFound(prop));
   }

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Propagator Timings :  TotalTime  SetupTime   Presolve  Propagate    ResProp    SB-Prop\n");

   for( i = 0; i < scip->set->nprops; ++i )
   {
      SCIP_PROP* prop;
      SCIP_Real totaltime;

      prop = scip->set->props[i];
      totaltime = SCIPpropGetPresolTime(prop) + SCIPpropGetTime(prop) + SCIPpropGetRespropTime(prop)
         + SCIPpropGetStrongBranchPropTime(prop) + SCIPpropGetSetupTime(prop);

      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s:", SCIPpropGetName(prop));
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10.2f %10.2f %10.2f %10.2f %10.2f\n",
         totaltime,
	 SCIPpropGetSetupTime(prop),
	 SCIPpropGetPresolTime(prop),
	 SCIPpropGetTime(prop),
	 SCIPpropGetRespropTime(prop),
	 SCIPpropGetStrongBranchPropTime(prop));
   }
}

/** outputs conflict statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintConflictStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   char initstoresize[SCIP_MAXSTRLEN];
   char maxstoresize[SCIP_MAXSTRLEN];

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintConflictStatistics", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( scip->set->conf_maxstoresize == 0 )
   {
      (void)SCIPsnprintf(initstoresize, SCIP_MAXSTRLEN, "inf");
      (void)SCIPsnprintf(maxstoresize, SCIP_MAXSTRLEN, "inf");
   }
   else
   {
      int initsize = SCIPconflictstoreGetInitPoolSize(scip->conflictstore);
      int maxsize = SCIPconflictstoreGetMaxPoolSize(scip->conflictstore);

      if( maxsize == -1 )
      {
         (void)SCIPsnprintf(initstoresize, SCIP_MAXSTRLEN, "--");
         (void)SCIPsnprintf(maxstoresize, SCIP_MAXSTRLEN, "--");
      }
      else
      {
         assert(initsize >= 0);
         assert(maxsize >= 0);

         (void)SCIPsnprintf(initstoresize, SCIP_MAXSTRLEN, "%d", initsize);
         (void)SCIPsnprintf(maxstoresize, SCIP_MAXSTRLEN, "%d", maxsize);
      }
   }
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Conflict Analysis  :       Time      Calls    Success    DomReds  Conflicts   Literals    Reconvs ReconvLits   Dualrays   Nonzeros   LP Iters   (pool size: [%s,%s])\n", initstoresize, maxstoresize);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  propagation      : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "          - %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT " %10.1f          -          -          -\n",
      SCIPconflictGetPropTime(scip->conflict),
      SCIPconflictGetNPropCalls(scip->conflict),
      SCIPconflictGetNPropSuccess(scip->conflict),
      SCIPconflictGetNPropConflictConss(scip->conflict),
      SCIPconflictGetNPropConflictConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNPropConflictLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNPropConflictConss(scip->conflict) : 0,
      SCIPconflictGetNPropReconvergenceConss(scip->conflict),
      SCIPconflictGetNPropReconvergenceConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNPropReconvergenceLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNPropReconvergenceConss(scip->conflict) : 0);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  infeasible LP    : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "          - %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT "\n",
      SCIPconflictGetInfeasibleLPTime(scip->conflict),
      SCIPconflictGetNInfeasibleLPCalls(scip->conflict),
      SCIPconflictGetNInfeasibleLPSuccess(scip->conflict),
      SCIPconflictGetNInfeasibleLPConflictConss(scip->conflict),
      SCIPconflictGetNInfeasibleLPConflictConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNInfeasibleLPConflictLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNInfeasibleLPConflictConss(scip->conflict) : 0,
      SCIPconflictGetNInfeasibleLPReconvergenceConss(scip->conflict),
      SCIPconflictGetNInfeasibleLPReconvergenceConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNInfeasibleLPReconvergenceLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNInfeasibleLPReconvergenceConss(scip->conflict) : 0,
      SCIPconflictGetNDualrayInfSuccess(scip->conflict),
      SCIPconflictGetNDualrayInfSuccess(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNDualrayInfNonzeros(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNDualrayInfSuccess(scip->conflict) : 0,
      SCIPconflictGetNInfeasibleLPIterations(scip->conflict));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  bound exceed. LP : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "          - %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT "\n",
      SCIPconflictGetBoundexceedingLPTime(scip->conflict),
      SCIPconflictGetNBoundexceedingLPCalls(scip->conflict),
      SCIPconflictGetNBoundexceedingLPSuccess(scip->conflict),
      SCIPconflictGetNBoundexceedingLPConflictConss(scip->conflict),
      SCIPconflictGetNBoundexceedingLPConflictConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNBoundexceedingLPConflictLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNBoundexceedingLPConflictConss(scip->conflict) : 0,
      SCIPconflictGetNBoundexceedingLPReconvergenceConss(scip->conflict),
      SCIPconflictGetNBoundexceedingLPReconvergenceConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNBoundexceedingLPReconvergenceLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNBoundexceedingLPReconvergenceConss(scip->conflict) : 0,
      SCIPconflictGetNDualrayBndSuccess(scip->conflict),
      SCIPconflictGetNDualrayBndSuccess(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNDualrayBndNonzeros(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNDualrayBndSuccess(scip->conflict) : 0,
      SCIPconflictGetNBoundexceedingLPIterations(scip->conflict));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  strong branching : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "          - %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT " %10.1f          -          - %10" SCIP_LONGINT_FORMAT "\n",
      SCIPconflictGetStrongbranchTime(scip->conflict),
      SCIPconflictGetNStrongbranchCalls(scip->conflict),
      SCIPconflictGetNStrongbranchSuccess(scip->conflict),
      SCIPconflictGetNStrongbranchConflictConss(scip->conflict),
      SCIPconflictGetNStrongbranchConflictConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNStrongbranchConflictLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNStrongbranchConflictConss(scip->conflict) : 0,
      SCIPconflictGetNStrongbranchReconvergenceConss(scip->conflict),
      SCIPconflictGetNStrongbranchReconvergenceConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNStrongbranchReconvergenceLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNStrongbranchReconvergenceConss(scip->conflict) : 0,
      SCIPconflictGetNStrongbranchIterations(scip->conflict));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  pseudo solution  : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "          - %10" SCIP_LONGINT_FORMAT " %10.1f %10" SCIP_LONGINT_FORMAT " %10.1f          -          -          -\n",
      SCIPconflictGetPseudoTime(scip->conflict),
      SCIPconflictGetNPseudoCalls(scip->conflict),
      SCIPconflictGetNPseudoSuccess(scip->conflict),
      SCIPconflictGetNPseudoConflictConss(scip->conflict),
      SCIPconflictGetNPseudoConflictConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNPseudoConflictLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNPseudoConflictConss(scip->conflict) : 0,
      SCIPconflictGetNPseudoReconvergenceConss(scip->conflict),
      SCIPconflictGetNPseudoReconvergenceConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNPseudoReconvergenceLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNPseudoReconvergenceConss(scip->conflict) : 0);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  applied globally : %10.2f          -          - %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.1f          -          - %10" SCIP_LONGINT_FORMAT "          -          -\n",
      SCIPconflictGetGlobalApplTime(scip->conflict),
      SCIPconflictGetNGlobalChgBds(scip->conflict),
      SCIPconflictGetNAppliedGlobalConss(scip->conflict),
      SCIPconflictGetNAppliedGlobalConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNAppliedGlobalLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNAppliedGlobalConss(scip->conflict) : 0,
      SCIPconflictGetNDualrayInfGlobal(scip->conflict) + SCIPconflictGetNDualrayBndGlobal(scip->conflict));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  applied locally  :          -          -          - %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.1f          -          - %10" SCIP_LONGINT_FORMAT "          -          -\n",
      SCIPconflictGetNLocalChgBds(scip->conflict),
      SCIPconflictGetNAppliedLocalConss(scip->conflict),
      SCIPconflictGetNAppliedLocalConss(scip->conflict) > 0
      ? (SCIP_Real)SCIPconflictGetNAppliedLocalLiterals(scip->conflict)
      / (SCIP_Real)SCIPconflictGetNAppliedLocalConss(scip->conflict) : 0,
      SCIPconflictGetNDualrayInfSuccess(scip->conflict) + SCIPconflictGetNDualrayBndSuccess(scip->conflict)
      - SCIPconflictGetNDualrayInfGlobal(scip->conflict) - SCIPconflictGetNDualrayBndGlobal(scip->conflict));
}

/** outputs separator statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintSeparatorStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintSeparatorStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Separators         :   ExecTime  SetupTime      Calls    Cutoffs    DomReds       Cuts    Applied      Conss\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  cut pool         : %10.2f            %10" SCIP_LONGINT_FORMAT "          -          - %10" SCIP_LONGINT_FORMAT "          -          -    (maximal pool size: %d)\n",
      SCIPcutpoolGetTime(scip->cutpool),
      SCIPcutpoolGetNCalls(scip->cutpool),
      SCIPcutpoolGetNCutsFound(scip->cutpool),
      SCIPcutpoolGetMaxNCuts(scip->cutpool));

   /* sort separators w.r.t. their name */
   SCIPsetSortSepasName(scip->set);

   for( i = 0; i < scip->set->nsepas; ++i )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10.2f %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
         SCIPsepaGetName(scip->set->sepas[i]),
         SCIPsepaGetTime(scip->set->sepas[i]),
         SCIPsepaGetSetupTime(scip->set->sepas[i]),
         SCIPsepaGetNCalls(scip->set->sepas[i]),
         SCIPsepaGetNCutoffs(scip->set->sepas[i]),
         SCIPsepaGetNDomredsFound(scip->set->sepas[i]),
         SCIPsepaGetNCutsFound(scip->set->sepas[i]),
         SCIPsepaGetNCutsApplied(scip->set->sepas[i]),
         SCIPsepaGetNConssFound(scip->set->sepas[i]));
   }
}

/** outputs pricer statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintPricerStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintPricerStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Pricers            :   ExecTime  SetupTime      Calls       Vars\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  problem variables: %10.2f          - %10d %10d\n",
      SCIPpricestoreGetProbPricingTime(scip->pricestore),
      SCIPpricestoreGetNProbPricings(scip->pricestore),
      SCIPpricestoreGetNProbvarsFound(scip->pricestore));

   /* sort pricers w.r.t. their name */
   SCIPsetSortPricersName(scip->set);

   for( i = 0; i < scip->set->nactivepricers; ++i )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10.2f %10.2f %10d %10d\n",
         SCIPpricerGetName(scip->set->pricers[i]),
         SCIPpricerGetTime(scip->set->pricers[i]),
         SCIPpricerGetSetupTime(scip->set->pricers[i]),
         SCIPpricerGetNCalls(scip->set->pricers[i]),
         SCIPpricerGetNVarsFound(scip->set->pricers[i]));
   }
}

/** outputs branching rule statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintBranchruleStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintBranchruleStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Branching Rules    :   ExecTime  SetupTime   BranchLP  BranchExt   BranchPS    Cutoffs    DomReds       Cuts      Conss   Children\n");

   /* sort branching rules  w.r.t. their name */
   SCIPsetSortBranchrulesName(scip->set);

   for( i = 0; i < scip->set->nbranchrules; ++i )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10.2f %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
         SCIPbranchruleGetName(scip->set->branchrules[i]),
         SCIPbranchruleGetTime(scip->set->branchrules[i]),
         SCIPbranchruleGetSetupTime(scip->set->branchrules[i]),
         SCIPbranchruleGetNLPCalls(scip->set->branchrules[i]),
         SCIPbranchruleGetNExternCalls(scip->set->branchrules[i]),
         SCIPbranchruleGetNPseudoCalls(scip->set->branchrules[i]),
         SCIPbranchruleGetNCutoffs(scip->set->branchrules[i]),
         SCIPbranchruleGetNDomredsFound(scip->set->branchrules[i]),
         SCIPbranchruleGetNCutsFound(scip->set->branchrules[i]),
         SCIPbranchruleGetNConssFound(scip->set->branchrules[i]),
         SCIPbranchruleGetNChildren(scip->set->branchrules[i]));
   }
}

/** outputs heuristics statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintHeuristicStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int ndivesets = 0;
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);
   assert(scip->tree != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintHeuristicStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Primal Heuristics  :   ExecTime  SetupTime      Calls      Found       Best\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  LP solutions     : %10.2f          -          - %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
      SCIPclockGetTime(scip->stat->lpsoltime),
      scip->stat->nlpsolsfound, scip->stat->nlpbestsolsfound);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  relax solutions  : %10.2f          -          - %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
      SCIPclockGetTime(scip->stat->relaxsoltime),
      scip->stat->nrelaxsolsfound, scip->stat->nrelaxbestsolsfound);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  pseudo solutions : %10.2f          -          - %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
      SCIPclockGetTime(scip->stat->pseudosoltime),
      scip->stat->npssolsfound, scip->stat->npsbestsolsfound);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  strong branching : %10.2f          -          - %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
      SCIPclockGetTime(scip->stat->sbsoltime),
      scip->stat->nsbsolsfound, scip->stat->nsbbestsolsfound);

   /* sort heuristics w.r.t. their names */
   SCIPsetSortHeursName(scip->set);

   for( i = 0; i < scip->set->nheurs; ++i )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10.2f %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
         SCIPheurGetName(scip->set->heurs[i]),
         SCIPheurGetTime(scip->set->heurs[i]),
         SCIPheurGetSetupTime(scip->set->heurs[i]),
         SCIPheurGetNCalls(scip->set->heurs[i]),
         SCIPheurGetNSolsFound(scip->set->heurs[i]),
         SCIPheurGetNBestSolsFound(scip->set->heurs[i]));
         
      /* count heuristics that use diving; needed to determine output later */
      ndivesets += SCIPheurGetNDivesets(scip->set->heurs[i]);
   }

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  other solutions  :          -          -          - %10" SCIP_LONGINT_FORMAT "          -\n",
      scip->stat->nexternalsolsfound);

   if ( ndivesets > 0 )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "Diving Statistics  :      Calls      Nodes   LP Iters Backtracks  Conflicts   MinDepth   MaxDepth   AvgDepth  RoundSols  NLeafSols  MinSolDpt  MaxSolDpt  AvgSolDpt\n");
      for( i = 0; i < scip->set->nheurs; ++i )
      {
         int s;
         for( s = 0; s < SCIPheurGetNDivesets(scip->set->heurs[i]); ++s )
         {
            SCIP_DIVESET* diveset = SCIPheurGetDivesets(scip->set->heurs[i])[s];
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10d", SCIPdivesetGetName(diveset), SCIPdivesetGetNCalls(diveset));
            if( SCIPdivesetGetNCalls(diveset) > 0 )
            {
               SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10d %10d %10.1f %10" SCIP_LONGINT_FORMAT,
                  SCIPdivesetGetNProbingNodes(diveset),
                  SCIPdivesetGetNLPIterations(diveset),
                  SCIPdivesetGetNBacktracks(diveset),
                  SCIPdivesetGetNConflicts(diveset),
                  SCIPdivesetGetMinDepth(diveset),
                  SCIPdivesetGetMaxDepth(diveset),
                  SCIPdivesetGetAvgDepth(diveset),
                  SCIPdivesetGetNSols(diveset) - SCIPdivesetGetNSolutionCalls(diveset));

               if( SCIPdivesetGetNSolutionCalls(diveset) > 0 )
               {
                  SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10d %10d %10d %10.1f\n",
                     SCIPdivesetGetNSolutionCalls(diveset),
                     SCIPdivesetGetMinSolutionDepth(diveset),
                     SCIPdivesetGetMaxSolutionDepth(diveset),
                     SCIPdivesetGetAvgSolutionDepth(diveset));
               }
               else
                  SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -          -          -          -\n");
            }
            else
               SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -          -          -          -          -          -          -          -          -          -          -          -\n");
         }
      }
   }
}

/** outputs compression statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintCompressionStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintCompressionStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* only print compression statistics if tree reoptimization is enabled */
   if( !scip->set->reopt_enable )
      return;

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Tree Compressions  :   ExecTime  SetupTime      Calls      Found\n");

   /* sort compressions w.r.t. their names */
   SCIPsetSortComprsName(scip->set);

   for( i = 0; i < scip->set->ncomprs; ++i )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10.2f %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
         SCIPcomprGetName(scip->set->comprs[i]),
         SCIPcomprGetTime(scip->set->comprs[i]),
         SCIPcomprGetSetupTime(scip->set->comprs[i]),
         SCIPcomprGetNCalls(scip->set->comprs[i]),
         SCIPcomprGetNFound(scip->set->comprs[i]));
   }
}

/** outputs LP statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintLPStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   assert(scip != NULL);
   assert(scip->stat != NULL);
   assert(scip->lp != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintLPStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "LP                 :       Time      Calls Iterations  Iter/call   Iter/sec  Time-0-It Calls-0-It    ItLimit\n");

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  primal LP        : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->primallptime),
      scip->stat->nprimallps + scip->stat->nprimalzeroitlps,
      scip->stat->nprimallpiterations,
      scip->stat->nprimallps > 0 ? (SCIP_Real)scip->stat->nprimallpiterations/(SCIP_Real)scip->stat->nprimallps : 0.0);
   if( SCIPclockGetTime(scip->stat->primallptime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f", (SCIP_Real)scip->stat->nprimallpiterations/SCIPclockGetTime(scip->stat->primallptime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10" SCIP_LONGINT_FORMAT "\n",
      scip->stat->primalzeroittime,
      scip->stat->nprimalzeroitlps);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  dual LP          : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->duallptime),
      scip->stat->nduallps + scip->stat->ndualzeroitlps,
      scip->stat->nduallpiterations,
      scip->stat->nduallps > 0 ? (SCIP_Real)scip->stat->nduallpiterations/(SCIP_Real)scip->stat->nduallps : 0.0);
   if( SCIPclockGetTime(scip->stat->duallptime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f", (SCIP_Real)scip->stat->nduallpiterations/SCIPclockGetTime(scip->stat->duallptime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10" SCIP_LONGINT_FORMAT "\n",
      scip->stat->dualzeroittime,
      scip->stat->ndualzeroitlps);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  lex dual LP      : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->lexduallptime),
      scip->stat->nlexduallps,
      scip->stat->nlexduallpiterations,
      scip->stat->nlexduallps > 0 ? (SCIP_Real)scip->stat->nlexduallpiterations/(SCIP_Real)scip->stat->nlexduallps : 0.0);
   if( SCIPclockGetTime(scip->stat->lexduallptime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f\n", (SCIP_Real)scip->stat->nlexduallpiterations/SCIPclockGetTime(scip->stat->lexduallptime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -\n");

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  barrier LP       : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->barrierlptime),
      scip->stat->nbarrierlps,
      scip->stat->nbarrierlpiterations,
      scip->stat->nbarrierlps > 0 ? (SCIP_Real)scip->stat->nbarrierlpiterations/(SCIP_Real)scip->stat->nbarrierlps : 0.0);
   if( SCIPclockGetTime(scip->stat->barrierlptime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f", (SCIP_Real)scip->stat->nbarrierlpiterations/SCIPclockGetTime(scip->stat->barrierlptime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f %10" SCIP_LONGINT_FORMAT "\n",
      scip->stat->barrierzeroittime,
      scip->stat->nbarrierzeroitlps);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  resolve instable : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->resolveinstablelptime),
      scip->stat->nresolveinstablelps,
      scip->stat->nresolveinstablelpiters,
      scip->stat->nresolveinstablelps > 0 ? (SCIP_Real)scip->stat->nresolveinstablelpiters/(SCIP_Real)scip->stat->nresolveinstablelps : 0.0);
   if( SCIPclockGetTime(scip->stat->resolveinstablelptime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f\n", (SCIP_Real)scip->stat->nresolveinstablelpiters/SCIPclockGetTime(scip->stat->resolveinstablelptime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -\n");

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  diving/probing LP: %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->divinglptime),
      scip->stat->ndivinglps,
      scip->stat->ndivinglpiterations,
      scip->stat->ndivinglps > 0 ? (SCIP_Real)scip->stat->ndivinglpiterations/(SCIP_Real)scip->stat->ndivinglps : 0.0);
   if( SCIPclockGetTime(scip->stat->divinglptime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f\n", (SCIP_Real)scip->stat->ndivinglpiterations/SCIPclockGetTime(scip->stat->divinglptime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -\n");

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  strong branching : %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->strongbranchtime),
      scip->stat->nstrongbranchs,
      scip->stat->nsblpiterations,
      scip->stat->nstrongbranchs > 0 ? (SCIP_Real)scip->stat->nsblpiterations/(SCIP_Real)scip->stat->nstrongbranchs : 0.0);
   if( SCIPclockGetTime(scip->stat->strongbranchtime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f", (SCIP_Real)scip->stat->nsblpiterations/SCIPclockGetTime(scip->stat->strongbranchtime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -          - %10d\n", scip->stat->nsbtimesiterlimhit);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "    (at root node) :          - %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f          -\n",
      scip->stat->nrootstrongbranchs,
      scip->stat->nrootsblpiterations,
      scip->stat->nrootstrongbranchs > 0
      ? (SCIP_Real)scip->stat->nrootsblpiterations/(SCIP_Real)scip->stat->nrootstrongbranchs : 0.0);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  conflict analysis: %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10.2f",
      SCIPclockGetTime(scip->stat->conflictlptime),
      scip->stat->nconflictlps,
      scip->stat->nconflictlpiterations,
      scip->stat->nconflictlps > 0 ? (SCIP_Real)scip->stat->nconflictlpiterations/(SCIP_Real)scip->stat->nconflictlps : 0.0);
   if( SCIPclockGetTime(scip->stat->conflictlptime) >= 0.01 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, " %10.2f\n", (SCIP_Real)scip->stat->nconflictlpiterations/SCIPclockGetTime(scip->stat->conflictlptime));
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "          -\n");
}

/** outputs NLP statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintNLPStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   assert(scip != NULL);
   assert(scip->stat != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintNLPStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( scip->nlp == NULL )
      return;

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "NLP                :       Time      Calls\n");

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  all NLPs         : %10.2f %10" SCIP_LONGINT_FORMAT "\n",
      SCIPclockGetTime(scip->stat->nlpsoltime),
      scip->stat->nnlps);
}

/** outputs relaxator statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintRelaxatorStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintRelaxatorStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( scip->set->nrelaxs == 0 )
      return;

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Relaxators         :       Time      Calls\n");

   /* sort relaxators w.r.t. their name */
   SCIPsetSortRelaxsName(scip->set);

   for( i = 0; i < scip->set->nrelaxs; ++i )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10.2f %10" SCIP_LONGINT_FORMAT "\n",
         SCIPrelaxGetName(scip->set->relaxs[i]),
         SCIPrelaxGetTime(scip->set->relaxs[i]),
         SCIPrelaxGetNCalls(scip->set->relaxs[i]));
   }
}

/** outputs tree statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintTreeStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   assert(scip != NULL);
   assert(scip->stat != NULL);
   assert(scip->tree != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintTreeStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "B&B Tree           :\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  number of runs   : %10d\n", scip->stat->nruns);
   SCIPmessageFPrintInfo(scip->messagehdlr, file,
      "  nodes            : %10" SCIP_LONGINT_FORMAT " (%" SCIP_LONGINT_FORMAT " internal, %" SCIP_LONGINT_FORMAT " leaves)\n",
      scip->stat->nnodes, scip->stat->ninternalnodes, scip->stat->nnodes - scip->stat->ninternalnodes );
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  feasible leaves  : %10d\n", scip->stat->nfeasleaves);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  infeas. leaves   : %10d\n", scip->stat->ninfeasleaves);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  objective leaves : %10d\n", scip->stat->nobjleaves);
   SCIPmessageFPrintInfo(scip->messagehdlr, file,
      "  nodes (total)    : %10" SCIP_LONGINT_FORMAT " (%" SCIP_LONGINT_FORMAT " internal, %" SCIP_LONGINT_FORMAT " leaves)\n",
      scip->stat->ntotalnodes, scip->stat->ntotalinternalnodes, scip->stat->ntotalnodes - scip->stat->ntotalinternalnodes);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  nodes left       : %10d\n", SCIPtreeGetNNodes(scip->tree));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  max depth        : %10d\n", scip->stat->maxdepth);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  max depth (total): %10d\n", scip->stat->maxtotaldepth);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  backtracks       : %10" SCIP_LONGINT_FORMAT " (%.1f%%)\n", scip->stat->nbacktracks,
      scip->stat->nnodes > 0 ? 100.0 * (SCIP_Real)scip->stat->nbacktracks / (SCIP_Real)scip->stat->nnodes : 0.0);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  early backtracks : %10" SCIP_LONGINT_FORMAT " (%.1f%%)\n", scip->stat->nearlybacktracks,
       scip->stat->nbacktracks > 0 ? 100.0 * (SCIP_Real)scip->stat->nearlybacktracks / (SCIP_Real)scip->stat->nbacktracks : 0.0);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  nodes exc. ref.  : %10" SCIP_LONGINT_FORMAT " (%.1f%%)\n", scip->stat->nnodesaboverefbound,
       scip->stat->nnodes > 0 ? 100.0 * (SCIP_Real)scip->stat->nnodesaboverefbound / (SCIP_Real)scip->stat->nnodes : 0.0);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  delayed cutoffs  : %10" SCIP_LONGINT_FORMAT "\n", scip->stat->ndelayedcutoffs);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  repropagations   : %10" SCIP_LONGINT_FORMAT " (%" SCIP_LONGINT_FORMAT " domain reductions, %" SCIP_LONGINT_FORMAT " cutoffs)\n",
      scip->stat->nreprops, scip->stat->nrepropboundchgs, scip->stat->nrepropcutoffs);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  avg switch length: %10.2f\n",
      scip->stat->nnodes > 0
      ? (SCIP_Real)(scip->stat->nactivatednodes + scip->stat->ndeactivatednodes) / (SCIP_Real)scip->stat->nnodes : 0.0);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  switching time   : %10.2f\n", SCIPclockGetTime(scip->stat->nodeactivationtime));
}

/** outputs solution statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintSolutionStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   SCIP_Real primalbound;
   SCIP_Real dualbound;
   SCIP_Real bestsol;
   SCIP_Real gap;
   SCIP_Real firstprimalbound;
   SCIP_Bool objlimitreached;
   char limsolstring[SCIP_MAXSTRLEN];

   assert(scip != NULL);
   assert(scip->stat != NULL);
   assert(scip->primal != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintSolutionStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   primalbound = SCIPgetPrimalbound(scip);
   dualbound = SCIPgetDualbound(scip);
   gap = SCIPgetGap(scip);

   objlimitreached = FALSE;
   if( SCIPgetStage(scip) == SCIP_STAGE_SOLVED && scip->primal->nlimsolsfound == 0
      && !SCIPisInfinity(scip, primalbound)  )
      objlimitreached = TRUE;

   if( scip->primal->nsolsfound != scip->primal->nlimsolsfound )
      (void) SCIPsnprintf(limsolstring, SCIP_MAXSTRLEN, ", %" SCIP_LONGINT_FORMAT " respecting the objective limit", scip->primal->nlimsolsfound);
   else
      limsolstring[0] = '\0';

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Solution           :\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Solutions found  : %10" SCIP_LONGINT_FORMAT " (%" SCIP_LONGINT_FORMAT " improvements%s)\n",
      scip->primal->nsolsfound, scip->primal->nbestsolsfound, limsolstring);

   if( objlimitreached || SCIPsetIsInfinity(scip->set, REALABS(primalbound)) )
   {
      if( scip->set->stage == SCIP_STAGE_SOLVED )
      {
         if( scip->primal->nlimsolsfound == 0 )
         {
            if( SCIPgetStatus(scip) == SCIP_STATUS_INFORUNBD )
            {
               assert(!objlimitreached);
               SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Primal Bound     : infeasible or unbounded\n");
            }
            else
            {
               assert(SCIPgetStatus(scip) == SCIP_STATUS_INFEASIBLE);
               if( objlimitreached )
                  SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Primal Bound     : infeasible (objective limit reached)\n");
               else
                  SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Primal Bound     : infeasible\n");
            }
         }
         else
         {
            assert(!objlimitreached);
            assert(SCIPgetStatus(scip) == SCIP_STATUS_UNBOUNDED);
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Primal Bound     :  unbounded\n");
         }
      }
      else
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Primal Bound     :          -\n");
   }
   else
   {
      if( scip->primal->nlimsolsfound == 0 )
      {
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Primal Bound     : %+21.14e", primalbound);
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "   (user objective limit)\n");
      }
      else
      {
         /* display first primal bound line */
         firstprimalbound = scip->stat->firstprimalbound;
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  First Solution   : %+21.14e", firstprimalbound);

         SCIPmessageFPrintInfo(scip->messagehdlr, file, "   (in run %d, after %" SCIP_LONGINT_FORMAT " nodes, %.2f seconds, depth %d, found by <%s>)\n",
            scip->stat->nrunsbeforefirst,
            scip->stat->nnodesbeforefirst,
            scip->stat->firstprimaltime,
            scip->stat->firstprimaldepth,
            ( scip->stat->firstprimalheur != NULL )
            ? ( SCIPheurGetName(scip->stat->firstprimalheur) )
            : (( scip->stat->nrunsbeforefirst == 0 ) ? "initial" : "relaxation"));

         if( SCIPisInfinity(scip, scip->stat->firstsolgap) )
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Gap First Sol.   :   infinite\n");
         else
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Gap First Sol.   : %10.2f %%\n", 100.0 * scip->stat->firstsolgap);

         if( SCIPisInfinity(scip, scip->stat->lastsolgap) )
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Gap Last Sol.    :   infinite\n");
         else
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Gap Last Sol.    : %10.2f %%\n",  100.0 * scip->stat->lastsolgap);

         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Primal Bound     : %+21.14e", primalbound);

         /* display (best) primal bound */
         bestsol = SCIPsolGetObj(scip->primal->sols[0], scip->set, scip->transprob, scip->origprob);
         bestsol = SCIPretransformObj(scip, bestsol);
         if( SCIPsetIsGT(scip->set, bestsol, primalbound) )
         {
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "   (user objective limit)\n");
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Best Solution    : %+21.14e", bestsol);
         }
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "   (in run %d, after %" SCIP_LONGINT_FORMAT " nodes, %.2f seconds, depth %d, found by <%s>)\n",
            SCIPsolGetRunnum(scip->primal->sols[0]),
            SCIPsolGetNodenum(scip->primal->sols[0]),
            SCIPsolGetTime(scip->primal->sols[0]),
            SCIPsolGetDepth(scip->primal->sols[0]),
            SCIPsolGetHeur(scip->primal->sols[0]) != NULL
            ? SCIPheurGetName(SCIPsolGetHeur(scip->primal->sols[0]))
            : (SCIPsolGetRunnum(scip->primal->sols[0]) == 0 ? "initial" : "relaxation"));
      }
   }
   if( objlimitreached || SCIPsetIsInfinity(scip->set, REALABS(dualbound)) )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Dual Bound       :          -\n");
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Dual Bound       : %+21.14e\n", dualbound);
   if( SCIPsetIsInfinity(scip->set, gap) )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Gap              :   infinite\n");
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Gap              : %10.2f %%\n", 100.0 * gap);

   if( scip->set->misc_calcintegral )
   {
      if( SCIPgetStatus(scip) == SCIP_STATUS_INFEASIBLE )
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Avg. Gap         :          - (problem infeasible)\n");
      else
      {
         SCIP_Real avggap;
         SCIP_Real primaldualintegral;

         if( !SCIPisFeasZero(scip, SCIPgetSolvingTime(scip)) )
         {
            primaldualintegral = SCIPstatGetPrimalDualIntegral(scip->stat, scip->set, scip->transprob, scip->origprob);
            avggap = primaldualintegral/SCIPgetSolvingTime(scip);
         }
         else
         {
            avggap = 0.0;
            primaldualintegral = 0.0;
         }

         /* caution: this assert is non-deterministic since it depends on the solving time */
         assert(0.0 <= avggap && SCIPisLE(scip, avggap, 100.0));

         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Avg. Gap         : %10.2f %% (%.2f primal-dual integral)\n",
            avggap, primaldualintegral);
      }
   }
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Avg. Gap         :          - (not evaluated)\n");
}

/** outputs concurrent solver statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintConcsolverStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   SCIP_CONCSOLVER** concsolvers;
   int               nconcsolvers;
   int               i;
   int               winner;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintConcsolverStatistics", FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( !SCIPsyncstoreIsInitialized(scip->syncstore) )
      return;

   nconcsolvers = SCIPgetNConcurrentSolvers(scip);
   concsolvers = SCIPgetConcurrentSolvers(scip);
   winner = SCIPsyncstoreGetWinner(scip->syncstore);

   if( nconcsolvers > 0 )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "Concurrent Solvers : SolvingTime    SyncTime       Nodes    LP Iters SolsShared   SolsRecvd TighterBnds TighterIntBnds\n");
      for( i = 0; i < nconcsolvers; ++i )
      {
         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %c%-16s: %11.2f %11.2f %11" SCIP_LONGINT_FORMAT " %11" SCIP_LONGINT_FORMAT "%11i %11i %11" SCIP_LONGINT_FORMAT " %14" SCIP_LONGINT_FORMAT "\n",
                               winner == i ? '*' : ' ',
                               SCIPconcsolverGetName(concsolvers[i]),
                               SCIPconcsolverGetSolvingTime(concsolvers[i]),
                               SCIPconcsolverGetSyncTime(concsolvers[i]),
                               SCIPconcsolverGetNNodes(concsolvers[i]),
                               SCIPconcsolverGetNLPIterations(concsolvers[i]),
                               SCIPconcsolverGetNSolsShared(concsolvers[i]),
                               SCIPconcsolverGetNSolsRecvd(concsolvers[i]),
                               SCIPconcsolverGetNTighterBnds(concsolvers[i]),
                               SCIPconcsolverGetNTighterIntBnds(concsolvers[i])
                              );
      }
   }
}

/** display Benders' decomposition statistics */
void SCIPprintBendersStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   SCIP_BENDERS** benders;
   int nbenders;
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintBendersStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   if( SCIPgetNActiveBenders(scip) == 0 )
      return;

   nbenders = SCIPgetNBenders(scip);
   benders = SCIPgetBenders(scip);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Benders Decomp     :   ExecTime  SetupTime      Calls      Found   Transfer\n");
   for( i = 0; i < nbenders; ++i )
   {
      if( SCIPbendersIsActive(benders[i]) )
      {
         SCIP_BENDERSCUT** benderscuts;
         int nbenderscuts;
         int j;

         SCIPmessageFPrintInfo(scip->messagehdlr, file, "  %-17.17s: %10.2f %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "\n",
            SCIPbendersGetName(scip->set->benders[i]),
            SCIPbendersGetTime(scip->set->benders[i]),
            SCIPbendersGetSetupTime(scip->set->benders[i]),
            SCIPbendersGetNCalls(scip->set->benders[i]),
            SCIPbendersGetNCutsFound(scip->set->benders[i]),
            SCIPbendersGetNTransferredCuts(scip->set->benders[i]));

         nbenderscuts = SCIPbendersGetNBenderscuts(scip->set->benders[i]);
         benderscuts = SCIPbendersGetBenderscuts(scip->set->benders[i]);

         for( j = 0; j < nbenderscuts; j++ )
         {
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "    %-15.17s: %10.2f %10.2f %10" SCIP_LONGINT_FORMAT " %10" SCIP_LONGINT_FORMAT "          -\n",
               SCIPbenderscutGetName(benderscuts[j]),
               SCIPbenderscutGetTime(benderscuts[j]),
               SCIPbenderscutGetSetupTime(benderscuts[j]),
               SCIPbenderscutGetNCalls(benderscuts[j]),
               SCIPbenderscutGetNFound(benderscuts[j]));
         }
      }
   }
}

/** outputs root statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintRootStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   SCIP_Real dualboundroot;
   SCIP_Real firstdualboundroot;
   SCIP_Real firstlptime;
   SCIP_Real firstlpspeed;

   assert(scip != NULL);
   assert(scip->stat != NULL);
   assert(scip->primal != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintRootStatistics", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   dualboundroot = SCIPgetDualboundRoot(scip);
   firstdualboundroot = SCIPgetFirstLPDualboundRoot(scip);
   firstlptime = SCIPgetFirstLPTime(scip);

   if( firstlptime > 0.0 )
      firstlpspeed = (SCIP_Real)scip->stat->nrootfirstlpiterations/firstlptime;
   else
      firstlpspeed = 0.0;

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Root Node          :\n");
   if( SCIPsetIsInfinity(scip->set, REALABS(firstdualboundroot)) )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  First LP value   :          -\n");
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  First LP value   : %+21.14e\n", firstdualboundroot);
   if( firstlpspeed > 0.0 )
      SCIPmessageFPrintInfo(scip->messagehdlr, file,    "  First LP Iters   : %10" SCIP_LONGINT_FORMAT " (%.2f Iter/sec)\n",
         scip->stat->nrootfirstlpiterations,
         (SCIP_Real)scip->stat->nrootfirstlpiterations/firstlptime);
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file,    "  First LP Iters   : %10" SCIP_LONGINT_FORMAT "\n", scip->stat->nrootfirstlpiterations);
   SCIPmessageFPrintInfo(scip->messagehdlr, file,    "  First LP Time    : %10.2f\n", firstlptime);

   if( SCIPsetIsInfinity(scip->set, REALABS(dualboundroot)) )
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Final Dual Bound :          -\n");
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Final Dual Bound : %+21.14e\n", dualboundroot);
   SCIPmessageFPrintInfo(scip->messagehdlr, file,    "  Final Root Iters : %10" SCIP_LONGINT_FORMAT "\n", scip->stat->nrootlpiterations);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  Root LP Estimate : ");
   if( scip->stat->rootlpbestestimate != SCIP_INVALID ) /*lint !e777*/
   {
       SCIPmessageFPrintInfo(scip->messagehdlr, file, "%+21.14e\n", SCIPretransformObj(scip, scip->stat->rootlpbestestimate));
   }
   else
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "%21s\n","-");
}

/** outputs timing statistics
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
void SCIPprintTimingStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file */
   )
{
   SCIP_Real readingtime;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPprintTimingStatistics", FALSE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   readingtime = SCIPgetReadingTime(scip);

   if( SCIPgetStage(scip) == SCIP_STAGE_PROBLEM )
   {
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "Total Time         : %10.2f\n", readingtime);
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  reading          : %10.2f\n", readingtime);
   }
   else
   {
      SCIP_Real totaltime;
      SCIP_Real solvingtime;

      solvingtime  = SCIPclockGetTime(scip->stat->solvingtime);

      if( scip->set->time_reading )
         totaltime = solvingtime;
      else
         totaltime = solvingtime + readingtime;

      SCIPmessageFPrintInfo(scip->messagehdlr, file, "Total Time         : %10.2f\n", totaltime);
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  solving          : %10.2f\n", solvingtime);
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  presolving       : %10.2f (included in solving)\n", SCIPclockGetTime(scip->stat->presolvingtime));
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "  reading          : %10.2f%s\n", readingtime, scip->set->time_reading ? " (included in solving)" : "");

      if( scip->stat->ncopies > 0 )
      {
	 SCIP_Real copytime;

	 copytime = SCIPclockGetTime(scip->stat->copyclock);

	 SCIPmessageFPrintInfo(scip->messagehdlr, file, "  copying          : %10.2f (%d #copies) (minimal %.2f, maximal %.2f, average %.2f)\n",
	    copytime, scip->stat->ncopies, scip->stat->mincopytime, scip->stat->maxcopytime, copytime / scip->stat->ncopies);
      }
      else
	 SCIPmessageFPrintInfo(scip->messagehdlr, file, "  copying          : %10.2f %s\n", 0.0, "(0 times copied the problem)");
   }
}

/** comparison method for statistics tables */
static
SCIP_DECL_SORTPTRCOMP(tablePosComp)
{  /*lint --e{715}*/
   return (SCIPtableGetPosition((SCIP_TABLE*)elem1) - (SCIPtableGetPosition((SCIP_TABLE*)elem2)));
}

/** outputs solving statistics
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @note If limits have been changed between the solution and the call to this function, the status is recomputed and
 *        thus may to correspond to the original status.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPprintStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file (or NULL for standard output) */
   )
{
   SCIP_TABLE** tables;
   int ntables;
   int i;

   assert(scip != NULL);
   assert(scip->set != NULL);

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintStatistics", TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   ntables = SCIPgetNTables(scip);
   tables = SCIPgetTables(scip);

   /* sort all tables by position unless this has already been done */
   if( ! scip->set->tablessorted )
   {
      SCIPsortPtr((void**)tables, tablePosComp, ntables);

      scip->set->tablessorted = TRUE;
   }

   for( i = 0; i < ntables; ++i )
   {
      /* skip tables which are not active or only used in later stages */
      if( ( ! SCIPtableIsActive(tables[i]) ) || SCIPtableGetEarliestStage(tables[i]) > SCIPgetStage(scip) )
         continue;

      SCIP_CALL( SCIPtableOutput(tables[i], scip->set, file) );
   }

   return SCIP_OKAY;
}

/** outputs reoptimization statistics
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPprintReoptStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file (or NULL for standard output) */
   )
{
   SCIP_Real solving;
   SCIP_Real presolving;
   SCIP_Real updatetime;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintReoptStatistics", TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   /* skip if reoptimization is disabled */
   if( !scip->set->reopt_enable )
      return SCIP_OKAY;

   solving = SCIPclockGetTime(scip->stat->solvingtimeoverall);
   presolving = SCIPclockGetTime(scip->stat->presolvingtimeoverall);
   updatetime = SCIPclockGetTime(scip->stat->reoptupdatetime);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "SCIP Reopt Status  : finish after %d runs.\n", scip->stat->nreoptruns);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Time         (sec) :\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  solving          : %10.2f\n", solving);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  presolving       : %10.2f (included in solving)\n", presolving);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  save time        : %10.2f\n", SCIPreoptGetSavingtime(scip->reopt));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  update time      : %10.2f\n", updatetime);
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Nodes              :       feas     infeas     pruned     cutoff\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  total            : %10d %10d %10d %10d\n",
         SCIPreoptGetNTotalFeasNodes(scip->reopt), SCIPreoptGetNTotalInfNodes(scip->reopt),
         SCIPreoptGetNTotalPrunedNodes(scip->reopt), SCIPreoptGetNTotalCutoffReoptnodes(scip->reopt));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  avg              : %10.2f %10.2f %10.2f %10.2f\n",
         (SCIP_Real)SCIPreoptGetNTotalFeasNodes(scip->reopt)/scip->stat->nreoptruns,
         (SCIP_Real)SCIPreoptGetNTotalInfNodes(scip->reopt)/scip->stat->nreoptruns,
         (SCIP_Real)SCIPreoptGetNTotalPrunedNodes(scip->reopt)/scip->stat->nreoptruns,
         (SCIP_Real)SCIPreoptGetNTotalCutoffReoptnodes(scip->reopt)/scip->stat->nreoptruns);

   SCIPmessageFPrintInfo(scip->messagehdlr, file, "Restarts           :     global      local\n");
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  first            : %10d         --\n", SCIPreoptGetFirstRestarts(scip->reopt));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  last             : %10d         --\n", SCIPreoptGetLastRestarts(scip->reopt));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  total            : %10d %10d\n", SCIPreoptGetNRestartsGlobal(scip->reopt),
         SCIPreoptGetNTotalRestartsLocal(scip->reopt));
   SCIPmessageFPrintInfo(scip->messagehdlr, file, "  avg              :         -- %10.2f\n",
         (SCIP_Real)SCIPreoptGetNTotalRestartsLocal(scip->reopt)/scip->stat->nreoptruns);

   return SCIP_OKAY;
}

/** outputs history statistics about branchings on variables
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
SCIP_RETCODE SCIPprintBranchingStatistics(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file                /**< output file (or NULL for standard output) */
   )
{
   SCIP_VAR** vars;
   int totalnstrongbranchs;
   int v;

   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintBranchingStatistics", TRUE, TRUE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   switch( scip->set->stage )
   {
   case SCIP_STAGE_INIT:
   case SCIP_STAGE_PROBLEM:
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "problem not yet solved. branching statistics not available.\n");
      return SCIP_OKAY;

   case SCIP_STAGE_TRANSFORMED:
   case SCIP_STAGE_INITPRESOLVE:
   case SCIP_STAGE_PRESOLVING:
   case SCIP_STAGE_EXITPRESOLVE:
   case SCIP_STAGE_PRESOLVED:
   case SCIP_STAGE_SOLVING:
   case SCIP_STAGE_SOLVED:
      SCIP_CALL( SCIPallocBufferArray(scip, &vars, scip->transprob->nvars) );
      for( v = 0; v < scip->transprob->nvars; ++v )
      {
         SCIP_VAR* var;
         int i;

         var = scip->transprob->vars[v];
         for( i = v; i > 0 && strcmp(SCIPvarGetName(var), SCIPvarGetName(vars[i-1])) < 0; i-- )
            vars[i] = vars[i-1];
         vars[i] = var;
      }

      SCIPmessageFPrintInfo(scip->messagehdlr, file, "                                      locks              branchings              inferences      cutoffs                     LP gain          pscostcount                gain variance    \n");
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "variable          prio   factor   down     up  depth    down      up    sb     down       up   down     up            down              up    down      up            down              up\n");

      totalnstrongbranchs = 0;
      for( v = 0; v < scip->transprob->nvars; ++v )
      {
         if( SCIPvarGetNBranchings(vars[v], SCIP_BRANCHDIR_DOWNWARDS) > 0
            || SCIPvarGetNBranchings(vars[v], SCIP_BRANCHDIR_UPWARDS) > 0
            || SCIPgetVarNStrongbranchs(scip, vars[v]) > 0 )
         {
            int nstrongbranchs;

            nstrongbranchs = SCIPgetVarNStrongbranchs(scip, vars[v]);
            totalnstrongbranchs += nstrongbranchs;
            SCIPmessageFPrintInfo(scip->messagehdlr, file, "%-16s %5d %8.1f %6d %6d %6.1f %7" SCIP_LONGINT_FORMAT " %7" SCIP_LONGINT_FORMAT " %5d %8.1f %8.1f %5.1f%% %5.1f%% %15.4f %15.4f %7.1f %7.1f %15.2f %15.2f\n",
               SCIPvarGetName(vars[v]),
               SCIPvarGetBranchPriority(vars[v]),
               SCIPvarGetBranchFactor(vars[v]),
               SCIPvarGetNLocksDownType(vars[v], SCIP_LOCKTYPE_MODEL),
               SCIPvarGetNLocksUpType(vars[v], SCIP_LOCKTYPE_MODEL),
               (SCIPvarGetAvgBranchdepth(vars[v], SCIP_BRANCHDIR_DOWNWARDS)
                  + SCIPvarGetAvgBranchdepth(vars[v], SCIP_BRANCHDIR_UPWARDS))/2.0 - 1.0,
               SCIPvarGetNBranchings(vars[v], SCIP_BRANCHDIR_DOWNWARDS),
               SCIPvarGetNBranchings(vars[v], SCIP_BRANCHDIR_UPWARDS),
               nstrongbranchs,
               SCIPvarGetAvgInferences(vars[v], scip->stat, SCIP_BRANCHDIR_DOWNWARDS),
               SCIPvarGetAvgInferences(vars[v], scip->stat, SCIP_BRANCHDIR_UPWARDS),
               100.0 * SCIPvarGetAvgCutoffs(vars[v], scip->stat, SCIP_BRANCHDIR_DOWNWARDS),
               100.0 * SCIPvarGetAvgCutoffs(vars[v], scip->stat, SCIP_BRANCHDIR_UPWARDS),
               SCIPvarGetPseudocost(vars[v], scip->stat, -1.0),
               SCIPvarGetPseudocost(vars[v], scip->stat, +1.0),
               SCIPvarGetPseudocostCount(vars[v], SCIP_BRANCHDIR_DOWNWARDS),
               SCIPvarGetPseudocostCount(vars[v], SCIP_BRANCHDIR_UPWARDS),
               SCIPvarGetPseudocostVariance(vars[v], SCIP_BRANCHDIR_DOWNWARDS, FALSE),
               SCIPvarGetPseudocostVariance(vars[v], SCIP_BRANCHDIR_UPWARDS, FALSE));
         }
      }
      SCIPmessageFPrintInfo(scip->messagehdlr, file, "total                                                %7" SCIP_LONGINT_FORMAT " %7" SCIP_LONGINT_FORMAT " %5d %8.1f %8.1f %5.1f%% %5.1f%% %15.4f %15.4f %7.1f %7.1f %15.2f %15.2f\n",
         SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS),
         SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS),
         totalnstrongbranchs,
         SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS) > 0
         ? SCIPhistoryGetInferenceSum(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS)
         / (SCIP_Real)SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS) : 0.0,
         SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS) > 0
         ? SCIPhistoryGetInferenceSum(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS)
         / (SCIP_Real)SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS) : 0.0,
         SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS) > 0
         ? SCIPhistoryGetCutoffSum(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS)
         / (SCIP_Real)SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS) : 0.0,
         SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS) > 0
         ? SCIPhistoryGetCutoffSum(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS)
         / (SCIP_Real)SCIPhistoryGetNBranchings(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS) : 0.0,
         SCIPhistoryGetPseudocost(scip->stat->glbhistory, -1.0),
         SCIPhistoryGetPseudocost(scip->stat->glbhistory, +1.0),
         SCIPhistoryGetPseudocostCount(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS),
         SCIPhistoryGetPseudocostCount(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS),
         SCIPhistoryGetPseudocostVariance(scip->stat->glbhistory, SCIP_BRANCHDIR_DOWNWARDS),
         SCIPhistoryGetPseudocostVariance(scip->stat->glbhistory, SCIP_BRANCHDIR_UPWARDS));

      SCIPfreeBufferArray(scip, &vars);

      return SCIP_OKAY;

   default:
      SCIPerrorMessage("invalid SCIP stage <%d>\n", scip->set->stage);
      return SCIP_INVALIDCALL;
   }  /*lint !e788*/
}

/** outputs node information display line
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_SOLVING
 */
SCIP_RETCODE SCIPprintDisplayLine(
   SCIP*                 scip,               /**< SCIP data structure */
   FILE*                 file,               /**< output file (or NULL for standard output) */
   SCIP_VERBLEVEL        verblevel,          /**< minimal verbosity level to actually display the information line */
   SCIP_Bool             endline             /**< should the line be terminated with a newline symbol? */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPprintDisplayLine", FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE) );

   if( (SCIP_VERBLEVEL)scip->set->disp_verblevel >= verblevel )
   {
      SCIP_CALL( SCIPdispPrintLine(scip->set, scip->messagehdlr, scip->stat, file, TRUE, endline) );
   }

   return SCIP_OKAY;
}

/** gets total number of implications between variables that are stored in the implication graph
 *
 *  @return the total number of implications between variables that are stored in the implication graph
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 */
int SCIPgetNImplications(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetNImplications", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->nimplications;
}

/** stores conflict graph of binary variables' implications into a file, which can be used as input for the DOT tool
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *
 *  @deprecated because binary implications are now stored as cliques, please use SCIPwriteCliqueGraph() instead
 *
 */
SCIP_RETCODE SCIPwriteImplicationConflictGraph(
   SCIP*                 scip,               /**< SCIP data structure */
   const char*           filename            /**< file name, or NULL for stdout */
   )
{  /*lint --e{715}*/
   SCIPwarningMessage(scip, "SCIPwriteImplicationConflictGraph() is deprecated and does not do anything anymore. All binary to binary implications are now stored in the clique data structure, which can be written to a GML formatted file via SCIPwriteCliqueGraph().\n");

   return SCIP_OKAY;
}

/** update statistical information when a new solution was found */
void SCIPstoreSolutionGap(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   scip->stat->lastsolgap = SCIPcomputeGap(SCIPsetEpsilon(scip->set), SCIPsetInfinity(scip->set), SCIPgetPrimalbound(scip), SCIPgetDualbound(scip));

   if( scip->primal->nsols == 1 )
      scip->stat->firstsolgap = scip->stat->lastsolgap;

   if( scip->set->stage == SCIP_STAGE_SOLVING && scip->set->misc_calcintegral )
   {
      SCIPstatUpdatePrimalDualIntegral(scip->stat, scip->set, scip->transprob, scip->origprob, SCIPgetUpperbound(scip), SCIPgetLowerbound(scip) );
   }
}




/*
 * timing methods
 */

/** gets current time of day in seconds (standard time zone)
 *
 *  @return the current time of day in seconds (standard time zone).
 */
SCIP_Real SCIPgetTimeOfDay(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   return SCIPclockGetTimeOfDay();
}

/** creates a clock using the default clock type
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPcreateClock(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK**          clck                /**< pointer to clock timer */
   )
{
   assert(scip != NULL);

   SCIP_CALL( SCIPclockCreate(clck, SCIP_CLOCKTYPE_DEFAULT) );

   return SCIP_OKAY;
}

/** creates a clock counting the CPU user seconds
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPcreateCPUClock(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK**          clck                /**< pointer to clock timer */
   )
{
   assert(scip != NULL);

   SCIP_CALL( SCIPclockCreate(clck, SCIP_CLOCKTYPE_CPU) );

   return SCIP_OKAY;
}

/** creates a clock counting the wall clock seconds
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPcreateWallClock(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK**          clck                /**< pointer to clock timer */
   )
{
   assert(scip != NULL);

   SCIP_CALL( SCIPclockCreate(clck, SCIP_CLOCKTYPE_WALL) );

   return SCIP_OKAY;
}

/** frees a clock
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPfreeClock(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK**          clck                /**< pointer to clock timer */
   )
{
   assert(scip != NULL);

   SCIPclockFree(clck);

   return SCIP_OKAY;
}

/** resets the time measurement of a clock to zero and completely stops the clock
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPresetClock(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK*           clck                /**< clock timer */
   )
{
   assert(scip != NULL);

   SCIPclockReset(clck);

   return SCIP_OKAY;
}

/** starts the time measurement of a clock
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPstartClock(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK*           clck                /**< clock timer */
   )
{
   assert(scip != NULL);

   SCIPclockStart(clck, scip->set);

   return SCIP_OKAY;
}

/** stops the time measurement of a clock
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPstopClock(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK*           clck                /**< clock timer */
   )
{
   assert(scip != NULL);

   SCIPclockStop(clck, scip->set);

   return SCIP_OKAY;
}

/** enables or disables all statistic clocks of SCIP concerning plugin statistics,
 *  LP execution time, strong branching time, etc.
 *
 *  Method reads the value of the parameter timing/statistictiming. In order to disable statistic timing,
 *  set the parameter to FALSE.
 *
 *  @note: The (pre-)solving time clocks which are relevant for the output during (pre-)solving
 *         are not affected by this method
 *
 *  @see: For completely disabling all timing of SCIP, consider setting the parameter timing/enabled to FALSE
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INIT
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPenableOrDisableStatisticTiming(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPenableOrDisableStatisticTiming", TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIPsetEnableOrDisablePluginClocks(scip->set, scip->set->time_statistictiming);

   if( scip->set->stage > SCIP_STAGE_INIT )
   {
      assert(scip->stat != NULL);
      SCIPstatEnableOrDisableStatClocks(scip->stat, scip->set->time_statistictiming);
   }
   if( scip->set->stage >= SCIP_STAGE_TRANSFORMING )
   {
      assert(scip->conflict != NULL);
      SCIPconflictEnableOrDisableClocks(scip->conflict, scip->set->time_statistictiming);
   }

   return SCIP_OKAY;
}

/** starts the current solving time
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPstartSolvingTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPstartSolvingTime", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIPclockStart(scip->stat->solvingtime, scip->set);
   SCIPclockStart(scip->stat->solvingtimeoverall, scip->set);

   return SCIP_OKAY;
}

/** stops the current solving time in seconds
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *       - \ref SCIP_STAGE_EXITSOLVE
 *       - \ref SCIP_STAGE_FREETRANS
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_RETCODE SCIPstopSolvingTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL( SCIPcheckStage(scip, "SCIPstopSolvingTime", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE) );

   SCIPclockStop(scip->stat->solvingtime, scip->set);
   SCIPclockStop(scip->stat->solvingtimeoverall, scip->set);

   return SCIP_OKAY;
}

/** gets the measured time of a clock in seconds
 *
 *  @return the measured time of a clock in seconds.
 */
SCIP_Real SCIPgetClockTime(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK*           clck                /**< clock timer */
   )
{
   assert(scip != NULL);

   return SCIPclockGetTime(clck);
}

/** sets the measured time of a clock to the given value in seconds
 *
 *  @return \ref SCIP_OKAY is returned if everything worked. Otherwise a suitable error code is passed. See \ref
 *          SCIP_Retcode "SCIP_RETCODE" for a complete list of error codes.
 */
SCIP_RETCODE SCIPsetClockTime(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CLOCK*           clck,               /**< clock timer */
   SCIP_Real             sec                 /**< time in seconds to set the clock's timer to */
   )
{
   assert(scip != NULL);

   SCIPclockSetTime(clck, sec);

   return SCIP_OKAY;
}

/** gets the current total SCIP time in seconds, possibly accumulated over several problems.
 *
 *  @return the current total SCIP time in seconds, ie. the total time since the SCIP instance has been created
 */
SCIP_Real SCIPgetTotalTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   assert(scip != NULL);

   return SCIPclockGetTime(scip->totaltime);
}

/** gets the current solving time in seconds
 *
 *  @return the current solving time in seconds.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_Real SCIPgetSolvingTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetSolvingTime", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPclockGetTime(scip->stat->solvingtime);
}

/** gets the current reading time in seconds
 *
 *  @return the current reading time in seconds.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_PROBLEM
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_Real SCIPgetReadingTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_Real readingtime;
   int r;

   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetReadingTime", FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   readingtime = 0.0;

   /* sum up the reading time of all readers */
   for( r = 0; r < scip->set->nreaders; ++r )
   {
      assert(scip->set->readers[r] != NULL);
      assert(!SCIPisNegative(scip, SCIPreaderGetReadingTime(scip->set->readers[r])));
      readingtime += SCIPreaderGetReadingTime(scip->set->readers[r]);
   }

   return readingtime;
}

/** gets the current presolving time in seconds
 *
 *  @return the current presolving time in seconds.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_Real SCIPgetPresolvingTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetPresolvingTime", FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return SCIPclockGetTime(scip->stat->presolvingtime);
}

/** gets the time need to solve the first LP in the root node
 *
 *  @return the solving time for the first LP in the root node in seconds.
 *
 *  @pre This method can be called if SCIP is in one of the following stages:
 *       - \ref SCIP_STAGE_TRANSFORMING
 *       - \ref SCIP_STAGE_TRANSFORMED
 *       - \ref SCIP_STAGE_INITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVING
 *       - \ref SCIP_STAGE_EXITPRESOLVE
 *       - \ref SCIP_STAGE_PRESOLVED
 *       - \ref SCIP_STAGE_INITSOLVE
 *       - \ref SCIP_STAGE_SOLVING
 *       - \ref SCIP_STAGE_SOLVED
 *
 *  See \ref SCIP_Stage "SCIP_STAGE" for a complete list of all possible solving stages.
 */
SCIP_Real SCIPgetFirstLPTime(
   SCIP*                 scip                /**< SCIP data structure */
   )
{
   SCIP_CALL_ABORT( SCIPcheckStage(scip, "SCIPgetFirstLPTime", FALSE, FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) );

   return scip->stat->firstlptime;
}



/*
 * numeric values and comparisons
 */

/*
 * memory management
 */





/*
 * simple functions implemented as defines
 */

/* In debug mode, the following methods are implemented as function calls to ensure
 * type validity.
 * In optimized mode, the methods are implemented as defines to improve performance.
 * However, we want to have them in the library anyways, so we have to undef the defines.
 */

#undef SCIPinfinity
#undef SCIPisInfinity
#undef SCIPisEQ
#undef SCIPisLT
#undef SCIPisLE
#undef SCIPisGT
#undef SCIPisGE
#undef SCIPisZero
#undef SCIPisPositive
#undef SCIPisNegative
#undef SCIPisIntegral
#undef SCIPisScalingIntegral
#undef SCIPisFracIntegral
#undef SCIPfloor
#undef SCIPceil
#undef SCIPround
#undef SCIPfrac
#undef SCIPisSumEQ
#undef SCIPisSumLT
#undef SCIPisSumLE
#undef SCIPisSumGT
#undef SCIPisSumGE
#undef SCIPisSumZero
#undef SCIPisSumPositive
#undef SCIPisSumNegative
#undef SCIPisFeasEQ
#undef SCIPisFeasLT
#undef SCIPisFeasLE
#undef SCIPisFeasGT
#undef SCIPisFeasGE
#undef SCIPisFeasZero
#undef SCIPisFeasPositive
#undef SCIPisFeasNegative
#undef SCIPisFeasIntegral
#undef SCIPisFeasFracIntegral
#undef SCIPfeasFloor
#undef SCIPfeasCeil
#undef SCIPfeasRound
#undef SCIPfeasFrac
#undef SCIPisDualfeasEQ
#undef SCIPisDualfeasLT
#undef SCIPisDualfeasLE
#undef SCIPisDualfeasGT
#undef SCIPisDualfeasGE
#undef SCIPisDualfeasZero
#undef SCIPisDualfeasPositive
#undef SCIPisDualfeasNegative
#undef SCIPisDualfeasIntegral
#undef SCIPisDualfeasFracIntegral
#undef SCIPdualfeasFloor
#undef SCIPdualfeasCeil
#undef SCIPdualfeasRound
#undef SCIPdualfeasFrac
#undef SCIPisLbBetter
#undef SCIPisUbBetter
#undef SCIPisRelEQ
#undef SCIPisRelLT
#undef SCIPisRelLE
#undef SCIPisRelGT
#undef SCIPisRelGE
#undef SCIPisSumRelEQ
#undef SCIPisSumRelLT
#undef SCIPisSumRelLE
#undef SCIPisSumRelGT
#undef SCIPisSumRelGE
#undef SCIPconvertRealToInt
#undef SCIPconvertRealToLongint
#undef SCIPisUpdateUnreliable
#undef SCIPisHugeValue
#undef SCIPgetHugeValue


/** validate the result of the solve
 *
 *  the validation includes
 *
 *  - checking the feasibility of the incumbent solution in the original problem (using SCIPcheckSolOrig())
 *
 *  - checking if the objective bounds computed by SCIP agree with external primal and dual reference bounds.
 *
 *  All external reference bounds the original problem space and the original objective sense.
 *
 *  For infeasible problems, +/-SCIPinfinity() should be passed as reference bounds depending on the objective sense
 *  of the original problem.
 */
SCIP_RETCODE SCIPvalidateSolve(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_Real             primalreference,    /**< external primal reference value for the problem, or SCIP_UNKNOWN */
   SCIP_Real             dualreference,      /**< external dual reference value for the problem, or SCIP_UNKNOWN */
   SCIP_Real             reftol,             /**< relative tolerance for acceptable violation of reference values */
   SCIP_Bool             quiet,              /**< TRUE if no status line should be printed */
   SCIP_Bool*            feasible,           /**< pointer to store if the best solution is feasible in the original problem,
                                               *  or NULL */
   SCIP_Bool*            primalboundcheck,   /**< pointer to store if the primal bound respects the given dual reference
                                               *  value, or NULL */
   SCIP_Bool*            dualboundcheck      /**< pointer to store if the dual bound respects the given primal reference
                                               *  value, or NULL */
   )
{
   SCIP_Bool localfeasible;
   SCIP_Bool localprimalboundcheck;
   SCIP_Bool localdualboundcheck;
   SCIP_Real primviol;
   SCIP_Real dualviol;
   assert(scip != NULL);

   /* if no problem exists, there is no need for validation */
   if( SCIPgetStage(scip) < SCIP_STAGE_PROBLEM )
   {
      if( feasible != NULL )
         *feasible = TRUE;
      if( primalboundcheck != NULL )
         *primalboundcheck = TRUE;
      if( dualboundcheck != NULL )
         *dualboundcheck = TRUE;

      return SCIP_OKAY;
   }

   localfeasible = TRUE;
   localdualboundcheck = TRUE;

   /* check the best solution for feasibility in the original problem */
   if( SCIPgetNSols(scip) > 0 )
   {
      SCIP_SOL* bestsol = SCIPgetBestSol(scip);
      SCIP_Real checkfeastolfac;
      SCIP_Real oldfeastol;

      assert(bestsol != NULL);

      /* scale feasibility tolerance by set->num_checkfeastolfac */
      oldfeastol = SCIPfeastol(scip);
      SCIP_CALL( SCIPgetRealParam(scip, "numerics/checkfeastolfac", &checkfeastolfac) );
      if( !SCIPisEQ(scip, checkfeastolfac, 1.0) )
      {
         SCIP_CALL( SCIPchgFeastol(scip, oldfeastol * checkfeastolfac) );
      }

      SCIP_CALL( SCIPcheckSolOrig(scip, bestsol, &localfeasible, !quiet, TRUE) );

      /* restore old feasibilty tolerance */
      if( !SCIPisEQ(scip, checkfeastolfac, 1.0) )
      {
         SCIP_CALL( SCIPchgFeastol(scip, oldfeastol) );
      }
   }
   else
   {
      localfeasible = TRUE;
   }

   primviol = 0.0;
   dualviol = 0.0;
   /* check the primal and dual bounds computed by SCIP against the external reference values within reference tolerance */
   /* solution for an infeasible problem */
   if( SCIPgetNSols(scip) > 0 && ((SCIPgetObjsense(scip) == SCIP_OBJSENSE_MINIMIZE && SCIPisInfinity(scip, dualreference))
            || (SCIPgetObjsense(scip) == SCIP_OBJSENSE_MAXIMIZE && SCIPisInfinity(scip, -dualreference))) )
      localprimalboundcheck = FALSE;
   else
   {
      /* check if reference primal bound is not better than the proven dual bound and, if SCIP claims to be optimal,
       * if the
       */
      SCIP_Real pb = SCIPgetPrimalbound(scip);
      SCIP_Real db = SCIPgetDualbound(scip);

      /* compute the relative violation between the primal bound and dual reference value, and vice versa */
      if( SCIPgetObjsense(scip) == SCIP_OBJSENSE_MINIMIZE )
      {
         if( dualreference != SCIP_UNKNOWN ) /*lint !e777 */
            primviol = SCIPrelDiff(dualreference, pb);
         if( primalreference != SCIP_UNKNOWN ) /*lint !e777 */
            dualviol = SCIPrelDiff(db, primalreference);
      }
      else
      {
         if( dualreference != SCIP_UNKNOWN ) /*lint !e777 */
            primviol = SCIPrelDiff(pb, dualreference);

         if( primalreference != SCIP_UNKNOWN ) /*lint !e777 */
            dualviol = SCIPrelDiff(primalreference, db);
      }
      primviol = MAX(primviol, 0.0);
      dualviol = MAX(dualviol, 0.0);

      localprimalboundcheck = EPSP(reftol, primviol);
      localdualboundcheck = EPSP(reftol, dualviol);
   }

   if( !quiet )
   {
      SCIPinfoMessage(scip, NULL, "Validation         : ");
      if( ! localfeasible )
         SCIPinfoMessage(scip, NULL, "Fail (infeasible)");
      else if( ! localprimalboundcheck )
         SCIPinfoMessage(scip, NULL, "Fail (primal bound)");
      else if( ! localdualboundcheck )
         SCIPinfoMessage(scip, NULL, "Fail (dual bound)");
      else
         SCIPinfoMessage(scip, NULL, "Success");
      SCIPinfoMessage(scip, NULL, "\n");
      SCIPinfoMessage(scip, NULL, "  %-17s: %10u\n", "cons violation", !localfeasible);
      SCIPinfoMessage(scip, NULL, "  %-17s: %10.8g (reference: %16.9e)\n", "primal violation", primviol, dualreference);
      SCIPinfoMessage(scip, NULL, "  %-17s: %10.8g (reference: %16.9e)\n", "dual violation", dualviol, primalreference);
   }

   if( feasible != NULL )
      *feasible = localfeasible;
   if( primalboundcheck != NULL )
      *primalboundcheck = localprimalboundcheck;
   if( dualboundcheck != NULL )
      *dualboundcheck = localdualboundcheck;

   return SCIP_OKAY;
}


