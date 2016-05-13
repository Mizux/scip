/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2015 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   spaplugins.c
 * @brief  SCIP plugins for Sparse Approximation of Transition Networks
 * @author Leon Eifler
 */

/*--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "spaplugins.h"

#include "scip/debug.h"
#include "scip/scipdefplugins.h"
#include "event_newsol.h"
#include "sepa_sparseapprox.h"
#include "heur_fuzzyround.h"
#include "heur_spakerlin.h"

/** includes default plugins for coloring into SCIP */
SCIP_RETCODE SCIPincludeSpaPlugins(
   SCIP*                 scip                /**< SCIP data structure */
)
{
   SCIP_CALL( SCIPincludeDefaultPlugins(scip) );
   SCIP_CALL( SCIPincludeReaderSpa(scip) );
   SCIP_CALL( SCIPincludeHeurSpaGreedy(scip) );
   SCIP_CALL( SCIPincludeHeurSpaswitch(scip) );
   SCIP_CALL( SCIPincludeEventHdlrNewsol(scip) );
   SCIP_CALL( SCIPincludeSepaSparseApprox(scip) );
   SCIP_CALL( SCIPincludeHeurFuzzyround(scip) );
   SCIP_CALL( SCIPincludeHeurSpakerlin(scip) );

   SCIP_CALL( SCIPaddRealParam(scip,"coherence_bound","lower bound to within-cluster coherence", NULL, FALSE, 0.1, 0.0, 1.0, NULL, NULL ) );
   SCIP_CALL( SCIPaddCharParam(scip, "model_variant", "which variant of the problem should be used", NULL, FALSE, 'e', "ebrc", NULL, NULL) );

   return SCIP_OKAY;
}
