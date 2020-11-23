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
 * @brief  tests separation and estimation of cos()
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "scip/expr_cos.h"
#include "../estimation.h"

/* note: for the tests, SCIPcomputeInitialCutsTrig() does:
 * 1. secant -> done if success
 * 2. left mid tangent (left secant) -> if fails tries left tangent
 * 3. right mid tangent (right secant) -> if fails tries right tangent
 * when underestimating.
 * when overestimating 2. and 3. are swapped
 */

/* tests for interval [-1,5] */
Test(separation, cosine_x, .init = setup, .fini = teardown,
   .description = "test separation for a cosine expression in large range"
   )
{
   SCIP_EXPR* expr;
   SCIP_Real newtonpoint;
   SCIP_Real childlb;
   SCIP_Real childub;
   SCIP_Real linconst;
   SCIP_Real lincoef;
   SCIP_Bool success;

   /* variable x and expression xexpr are defined in ../estimation.h */
   SCIP_CALL( SCIPcreateExprCos(scip, &expr, xexpr, NULL ,NULL) );
   childlb = SCIPvarGetLbLocal(x);
   childub = SCIPvarGetUbLocal(x);

   /* cos(x) between -1 and 5 looks like
    *
    *   |      .........
    *   |    ..         ..
    *   |   /             \
    *   |  /               ..
    *   | /                  \
    *   |                     \
    *   |                      \
    *   |                       \                               /
    *   | -----------------------\-----------------------------/-
    *   |                         \                           /
    *   |                          \                         /
    *   |                           \                       /
    *   |                            \                     /
    *   |                             \                  ..
    *   |                              ..               /
    *   |                                \             /
    *   |                                 ..         ..
    *   |                                   .........
    *     -1                     2                          5
    *
    * when overestimating we should get a right secant and a left tangent, in that order
    * when underestimating we should get a left secant and a right secant, in that order (cosine is concave at 5)
    */

   /*
    * test initial overestimation
    */
   SCIP_CALL( SCIPcomputeInitialCutsTrig(scip, expr, childlb, childub, FALSE, coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 2, "expected %d, got %d\n", 2, nreturned);

   /* check right secant */
   newtonpoint = 0.145902085052;
   EXPECTFEQ( coefs[0][0], -sin(newtonpoint) );
   EXPECTFEQ( constants[0], 5 * sin(newtonpoint) + cos(5) );

   /* check left tangent */
   EXPECTFEQ( coefs[1][0], -sin(-1) );
   EXPECTFEQ( constants[1], cos(-1) - sin(-1) );

   /*
    * test initial underestimation
    */

   SCIP_CALL( SCIPcomputeInitialCutsTrig(scip, expr, childlb, childub, TRUE, coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 2, "expected %d, got %d\n", 2, nreturned);

   /* check left secant */
   newtonpoint = 2.740339007021;
   EXPECTFEQ( coefs[0][0], -sin(newtonpoint) );
   EXPECTFEQ( constants[0], cos(-1) - sin(newtonpoint) );

   /* check right secant */
   newtonpoint = 4.568732341350;
   EXPECTFEQ( coefs[1][0], -sin(newtonpoint) );
   EXPECTFEQ( constants[1], cos(5) + 5 * sin(newtonpoint) );

   /*
    * test overestimation
    */

   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, -0.5, childlb, childub, FALSE);
   cr_expect(success);
   cr_expect_float_eq(linconst, -0.5 * sin(-0.5) + cos(-0.5), 1e-12);
   cr_expect_float_eq(lincoef, -sin(-0.5), 1e-12);

   /*
    * test underestimation
    */
   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, 4.0, childlb, childub, TRUE);
   cr_expect(success);
   cr_expect_float_eq(linconst, 4.0 * sin(4.0) + cos(4.0), 1e-12);
   cr_expect_float_eq(lincoef, -sin(4.0), 1e-12);

   /*
    * test point where solution tangent is not overestimating
    */
   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, 1.7, childlb, childub, TRUE);
   cr_expect(success);

   /* check lmidtangent */
   newtonpoint = 2.740339007021;
   cr_expect_float_eq(linconst, -sin(newtonpoint) + cos(-1), 1e-12);
   cr_expect_float_eq(lincoef, -sin(newtonpoint), 1e-12);

   /* release expression */
   SCIP_CALL( SCIPreleaseExpr(scip, &expr) );
}

/* tests for interval [-6,-3] */
//TODO: reenable test. the problem is that the left tangent is not computed because it is wrongly indetified as nonoverestimating
// see computeRightTangentSin
Test(separation, cosine_y, .init = setup, .fini = teardown,
   .description = "test separation for a cosine expression in mid size range", .disabled = TRUE
)
{
   SCIP_EXPR* expr;
   SCIP_Real newtonpoint;
   SCIP_Real childlb;
   SCIP_Real childub;
   SCIP_Real linconst;
   SCIP_Real lincoef;
   SCIP_Bool success;

   SCIP_CALL( SCIPcreateExprCos(scip, &expr, yexpr, NULL ,NULL) );

   childlb = SCIPvarGetLbLocal(y);
   childub = SCIPvarGetUbLocal(y);

   /* cos(x) between -6 and -3 looks like
    *
    *   |  ....
    *   |      ....
    *   |          ...
    *   |             ..
    *   |               ..
    *   |                 ..
    *   |                   ..
    *   |                     ..
    *   | ----------------------..-------------------------------
    *   |                         ..
    *   |                           ...
    *   |                              ..
    *   |                                ..
    *   |                                  ..
    *   |                                    ...
    *   |                                       ...
    *   |                                          ...
    *   |                                             ...........
    *     -6                     -4.5                       -3
    *
    * when overestimating we should get a right secant and a left tangent, in that order
    * when underestimating we should get a left secant and a right tangent, in that order
    */

   /*
    * test initial overestimation
    */

   SCIP_CALL( SCIPcomputeInitialCutsTrig(scip, expr, childlb, childub, FALSE, coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 2, "expected %d, got %d\n", 2, nreturned);

   /* check right secant */
   newtonpoint = -5.535897406992;
   EXPECTFEQ( coefs[0][0], -sin(newtonpoint) );
   EXPECTFEQ( constants[0], cos(-3) - 3 * sin(newtonpoint) );

   /* check left tangent */
   EXPECTFEQ( coefs[1][0], -sin(-6) );
   EXPECTFEQ( constants[1], cos(-6) - 6 * sin(-6) );

   /*
    * test initial underestimation
    */

   SCIP_CALL( SCIPcomputeInitialCutsTrig(scip, expr, childlb, childub, TRUE, (double **)coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 2, "expected %d, got %d\n", 2, nreturned);

   /* check left secant */
   newtonpoint = -4.082240818442;
   EXPECTFEQ( coefs[0][0], -sin(newtonpoint) );
   EXPECTFEQ( constants[0], cos(-6) - 6 * sin(newtonpoint) );

   /* check right tangent */
   EXPECTFEQ( coefs[1][0], -sin(-3) );
   EXPECTFEQ( constants[1], cos(-3) - 3 * sin(-3) );


   /*
    * test overestimation
    */

   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, -5.7, childlb, childub, FALSE);
   cr_expect(success);
   cr_expect_float_eq(linconst, -5.7 * sin(-5.7) + cos(-5.7), 1e-12);
   cr_expect_float_eq(lincoef, -sin(-5.7), 1e-12);

   /*
    * test underestimation
    */

   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, -3.5, childlb, childub, TRUE);
   cr_expect(success);
   cr_expect_float_eq(linconst, -3.5 * sin(-3.5) + cos(-3.5), 1e-12);
   cr_expect_float_eq(lincoef, -sin(-3.5), 1e-12);

   /*
    * test point where solution tangent in not overestimating
    */

   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, -4.8, childlb, childub, FALSE);
   cr_expect(success);
   newtonpoint = -5.535897406992;
   cr_expect_float_eq(linconst, -3 * sin(newtonpoint) + cos(-3), 1e-11);
   cr_expect_float_eq(lincoef, -sin(newtonpoint), 1e-12);

   /* release expression */
   SCIP_CALL( SCIPreleaseExpr(scip, &expr) );
}

/* tests for interval [2,4] */
Test(separation, cosine_w, .init = setup, .fini = teardown,
   .description = "test separation for a cosine expression in short range"
)
{
   SCIP_EXPR* expr;
   SCIP_Real childlb;
   SCIP_Real childub;
   SCIP_Real linconst;
   SCIP_Real lincoef;
   SCIP_Bool success;

   SCIP_CALL( SCIPcreateExprCos(scip, &expr, wexpr, NULL ,NULL) );

   childlb = SCIPvarGetLbLocal(w);
   childub = SCIPvarGetUbLocal(w);

   /* cos(x) between 2 and 4 looks like
    *
    *   | -----------------------------------------------------..
    *   |
    *   |  .
    *   |   ..
    *   |     \
    *   |      \
    *   |       \
    *   |        \
    *   |         \
    *   |          \                                           ..
    *   |           ..                                        /
    *   |             \                                      /
    *   |              \                                   ..
    *   |               ..                                /
    *   |                 ..                            ..
    *   |                   ..                        ..
    *   |                     ..                    ..
    *   |                       ...              ...
    *   |                          ..............
    *     2                      3                          4
    *
    * when overestimating we should get a secant
    * when underestimating we should get a left and right tangent, in that order
    */

   /*
    * test initial overestimation
    */

   SCIP_CALL( SCIPcomputeInitialCutsTrig(scip, expr, childlb, childub, FALSE, (double **)coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 1, "expected %d, got %d\n", 1, nreturned);

   /* check secant */
   EXPECTFEQ( coefs[0][0], 0.5 * (cos(4) - cos(2)) );
   EXPECTFEQ( constants[0], 2 * cos(2) - cos(4) );

   /*
    * test initial underestimation
    */

   SCIP_CALL( SCIPcomputeInitialCutsTrig(scip, expr, childlb, childub, TRUE, (double **)coefs, constants, &nreturned) );
   cr_expect_eq(nreturned, 2, "expected %d, got %d\n", 2, nreturned);

   /* check left tangent */
   EXPECTFEQ( coefs[0][0], -sin(2) );
   EXPECTFEQ( constants[0], 2 * sin(2) + cos(2) );

   /* check right tangent */
   EXPECTFEQ( coefs[1][0], -sin(4) );
   EXPECTFEQ( constants[1], 4 * sin(4) + cos(4) );


   /*
    * test underestimation
    */

   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, 3.0, childlb, childub, TRUE);
   cr_expect(success);
   cr_expect_float_eq(linconst, 3 * sin(3) + cos(3), 1e-12);
   cr_expect_float_eq(lincoef, -sin(3), 1e-12);

   /*
    * test point where solution tangent is not overestimating
    */

   success = SCIPcomputeEstimatorsTrig(scip, expr, &lincoef, &linconst, 3.0, childlb, childub, FALSE);
   cr_expect(success);
   cr_expect_float_eq(linconst, -cos(4) + 2 * cos(2), 1e-12);
   cr_expect_float_eq(lincoef, 0.5 * (cos(4) - cos(2)), 1e-12);

   /* release expression */
   SCIP_CALL( SCIPreleaseExpr(scip, &expr) );
}
