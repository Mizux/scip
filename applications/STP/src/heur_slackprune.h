/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2022 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not visit scipopt.org.         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   heur_slackprune.h
 * @ingroup PRIMALHEURISTICS
 * @brief  dual-ascent and reduction based primal heuristic for Steiner problems
 * @author Daniel Rehfeldt
 *
 * This file implements a dual-ascent and reduction based heuristic for Steiner problems. It is based on an approach
 * described in T. Polzin's "Algorithms for the Steiner problem in networks".
 *
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_HEUR_SLACKPRUNE_H__
#define __SCIP_HEUR_SLACKPRUNE_H__


#include "scip/scip.h"
#include "graph.h"

#ifdef __cplusplus
extern "C" {
#endif

/** creates the slack prune primal heuristic and includes it in SCIP */
SCIP_RETCODE SCIPStpIncludeHeurSlackPrune(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** execute slack-and-prune heuristic on given graph */
SCIP_RETCODE SCIPStpHeurSlackPruneRun(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_VAR**            vars,               /**< problem variables or NULL */
   GRAPH*                g,                  /**< graph data structure */
   int*                  soledge,            /**< array to 1. provide and 2. return primal solution */
   SCIP_Bool*            success,            /**< feasible solution found? */
   SCIP_Bool             initialreduce,      /**< try to reduce graph initially? */
   SCIP_Bool             fullreduce          /**< use full reduction techniques? */
   );


#ifdef __cplusplus
}
#endif

#endif
