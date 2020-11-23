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

/**@file   estimation.c
 * @brief  tests separation of abs()
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "scip/expr_abs.c"
#include "../estimation.h"

Test(separation, absolute, .init = setup, .fini = teardown, .description = "test separation for an absolute expression")
{
   SCIP_CALL( SCIPevalExprActivity(scip, xexpr) );

   /* compute overestimating secant */
   SCIP_CALL( computeCutsAbs(scip, SCIPexprGetActivity(xexpr), TRUE, coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 1);

   /* check secant */
   EXPECTFEQ( coefs[0][0], 2.0 / 3.0 );
   EXPECTFEQ( constants[0], 5.0 / 3.0 );

   /* compute underestimators */
   SCIP_CALL( computeCutsAbs(scip, SCIPexprGetActivity(xexpr), FALSE, coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 2);

   /* check left tangent */
   EXPECTFEQ( coefs[0][0], -1.0 );
   EXPECTFEQ( constants[0], 0.0 );

   /* check right tangent */
   EXPECTFEQ( coefs[1][0], 1.0 );
   EXPECTFEQ( constants[1], 0.0 );

}
