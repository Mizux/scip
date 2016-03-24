/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2016 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   cons_expr.h
 * @ingroup CONSHDLRS
 * @brief  constraint handler for expression constraints (in particular, nonlinear constraints)
 * @author Stefan Vigerske
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_CONS_EXPR_H__
#define __SCIP_CONS_EXPR_H__


#include "scip/scip.h"
#include "scip/type_cons_expr.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@name Expression Operand Methods */
/**@{ */

/** creates the handler for an expression operand and includes it into the expression constraint handler */
EXTERN
SCIP_RETCODE SCIPincludeOperandHdlrBasic(
   SCIP*                      scip,          /**< SCIP data structure */
   SCIP_CONSHDLR*             conshdlr,      /**< expression constraint handler */
   SCIP_CONSEXPR_OPERANDHDLR** ophdlr,       /**< buffer where to store operand handler */
   const char*                name,          /**< name of operand (must not be NULL) */
   const char*                desc,          /**< description of operand (can be NULL) */
   SCIP_CONSEXPR_OPERANDHDLRDATA* data       /**< data of operand handler (can be NULL) */
   );

/** set the operand handler callbacks to copy and free an operand handler */
EXTERN
SCIP_RETCODE SCIPsetOperandHdlrCopyFreeHdlr(
   SCIP*                      scip,          /**< SCIP data structure */
   SCIP_CONSHDLR*             conshdlr,      /**< expression constraint handler */
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr,        /**< operand handler */
   SCIP_DECL_CONSEXPR_OPERANDCOPYHDLR((*copyhdlr)), /**< handler copy method (can be NULL) */
   SCIP_DECL_CONSEXPR_OPERANDFREEHDLR((*freehdlr)) /**< handler free method (can be NULL) */
);

/** set the operand handler callbacks to copy and free operand data */
EXTERN
SCIP_RETCODE SCIPsetOperandHdlrCopyFreeData(
   SCIP*                      scip,          /**< SCIP data structure */
   SCIP_CONSHDLR*             conshdlr,      /**< expression constraint handler */
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr,        /**< operand handler */
   SCIP_DECL_CONSEXPR_OPERANDCOPYDATA((*copydata)), /**< copy method of operand data (can be NULL for operands without data) */
   SCIP_DECL_CONSEXPR_OPERANDFREEDATA((*freedata))  /**< free method of operand data (can be NULL if data does not need to be freed) */
);

/** set the print callback of an operand handler */
EXTERN
SCIP_RETCODE SCIPsetOperandHdlrPrint(
   SCIP*                      scip,          /**< SCIP data structure */
   SCIP_CONSHDLR*             conshdlr,      /**< expression constraint handler */
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr,        /**< operand handler */
   SCIP_DECL_CONSEXPR_OPERANDPRINT((*print)) /**< print method of operand data (can be NULL) */
);

/** gives the name of an operand handler */
EXTERN
const char* SCIPgetOperandHdlrName(
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr         /**< operand handler */
);

/** gives the description of an operand handler (can be NULL) */
EXTERN
const char* SCIPgetOperandHdlrDescription(
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr         /**< operand handler */
);

/** gives the data of an operand handler */
EXTERN
SCIP_CONSEXPR_OPERANDHDLRDATA* SCIPgetOperandHdlrData(
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr         /**< operand handler */
);

/** @} */

/**@name Expression Methods */
/**@{ */

/** creates and captures a multivariate expression with given operand and children */
EXTERN
SCIP_RETCODE SCIPcreateConsExprExprMultivariate(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        consexprhdlr,       /**< expression constraint handler */
   SCIP_CONSEXPR_EXPR**  expr,               /**< pointer where to store expression */
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr,        /**< operand handler */
   SCIP_CONSEXPR_OPERANDDATA* opdata,        /**< operand data (expression assumes ownership) */
   int                   nchildren,          /**< number of children */
   SCIP_CONSEXPR_EXPR*   children            /**< children */
   );

/** creates and captures a bivariate expression with given operand and children */
EXTERN
SCIP_RETCODE SCIPcreateConsExprExprBivariate(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        consexprhdlr,       /**< expression constraint handler */
   SCIP_CONSEXPR_EXPR**  expr,               /**< pointer where to store expression */
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr,        /**< operand handler */
   SCIP_CONSEXPR_OPERANDDATA* opdata,        /**< operand data */
   SCIP_CONSEXPR_EXPR*   child1,             /**< first child */
   SCIP_CONSEXPR_EXPR*   child2              /**< second child */
   );

/** creates and captures a univariate expression with given operand and child */
EXTERN
SCIP_RETCODE SCIPcreateConsExprExprUnivariate(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        consexprhdlr,       /**< expression constraint handler */
   SCIP_CONSEXPR_EXPR**  expr,               /**< pointer where to store expression */
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr,        /**< operand handler */
   SCIP_CONSEXPR_OPERANDDATA* opdata,        /**< operand data */
   SCIP_CONSEXPR_EXPR*   child               /**< child */
   );

/** creates and captures a variate expression with given operand */
EXTERN
SCIP_RETCODE SCIPcreateConsExprExprInvariate(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSHDLR*        consexprhdlr,       /**< expression constraint handler */
   SCIP_CONSEXPR_EXPR**  expr,               /**< pointer where to store expression */
   SCIP_CONSEXPR_OPERANDHDLR* ophdlr,        /**< operand handler */
   SCIP_CONSEXPR_OPERANDDATA* opdata         /**< operand data */
   );

/** captures an expression (increments usage count) */
EXTERN
void SCIPcaptureConsExprExpr(
   SCIP_CONSEXPR_EXPR*   expr                /**< expression to be captured */
   );

/** releases an expression (decrements usage count and possibly frees expression) */
EXTERN
SCIP_RETCODE SCIPreleaseConsExprExpr(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONSEXPR_EXPR**  expr                /**< pointer to expression to be released */
   );

/** gives the number of children of an expression */
EXTERN
int SCIPgetConsExprExprNChildren(
   SCIP_CONSEXPR_EXPR*   expr                /**< expression */
   );

/** gives the child of a univariate expression */
EXTERN
SCIP_CONSEXPR_EXPR* SCIPgetConsExprExprChild(
   SCIP_CONSEXPR_EXPR*   expr                /**< expression */
   );

/** gives the children of a non-invariate expression */
EXTERN
SCIP_CONSEXPR_EXPR** SCIPgetConsExprExprChildren(
   SCIP_CONSEXPR_EXPR*   expr                /**< expression */
   );

/** gets the operator handler of an expression
 *
 * This identifies the type of the expression (sum, variable, ...).
 */
EXTERN
SCIP_CONSEXPR_OPERANDHDLR* SCIPgetConsExprExprOperatorHdlr(
   SCIP_CONSEXPR_EXPR*   expr                /**< expression */
   );

/** gets the operator data of an expression */
EXTERN
SCIP_CONSEXPR_OPERANDDATA* SCIPgetConsExprExprOperatorData(
   SCIP_CONSEXPR_EXPR*   expr                /**< expression */
   );

/** sets the operator data of an expression
 *
 * The pointer to possible old data is overwritten and the
 * freedata-callback is not called before.
 * This function is intended to be used by operand handler.
 */
EXTERN
void SCIPsetConsExprExprOperatorData(
   SCIP_CONSEXPR_EXPR*        expr,          /**< expression */
   SCIP_CONSEXPR_OPERANDDATA* operanddata    /**< operand data to be set (can be NULL) */
   );

/** @} */



/**@name Expression Constraint Handler Methods */
/**@{ */

/** creates the handler for expr constraints and includes it in SCIP */
EXTERN
SCIP_RETCODE SCIPincludeConshdlrExpr(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** creates and captures a expr constraint
 *
 *  @note the constraint gets captured, hence at one point you have to release it using the method SCIPreleaseCons()
 */
EXTERN
SCIP_RETCODE SCIPcreateConsExpr(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS**           cons,               /**< pointer to hold the created constraint */
   const char*           name,               /**< name of constraint */
   int                   nvars,              /**< number of variables in the constraint */
   SCIP_VAR**            vars,               /**< array with variables of constraint entries */
   SCIP_Real*            coefs,              /**< array with coefficients of constraint entries */
   SCIP_Real             lhs,                /**< left hand side of constraint */
   SCIP_Real             rhs,                /**< right hand side of constraint */
   SCIP_Bool             initial,            /**< should the LP relaxation of constraint be in the initial LP?
                                              *   Usually set to TRUE. Set to FALSE for 'lazy constraints'. */
   SCIP_Bool             separate,           /**< should the constraint be separated during LP processing?
                                              *   Usually set to TRUE. */
   SCIP_Bool             enforce,            /**< should the constraint be enforced during node processing?
                                              *   TRUE for model constraints, FALSE for additional, redundant constraints. */
   SCIP_Bool             check,              /**< should the constraint be checked for feasibility?
                                              *   TRUE for model constraints, FALSE for additional, redundant constraints. */
   SCIP_Bool             propagate,          /**< should the constraint be propagated during node processing?
                                              *   Usually set to TRUE. */
   SCIP_Bool             local,              /**< is constraint only valid locally?
                                              *   Usually set to FALSE. Has to be set to TRUE, e.g., for branching constraints. */
   SCIP_Bool             modifiable,         /**< is constraint modifiable (subject to column generation)?
                                              *   Usually set to FALSE. In column generation applications, set to TRUE if pricing
                                              *   adds coefficients to this constraint. */
   SCIP_Bool             dynamic,            /**< is constraint subject to aging?
                                              *   Usually set to FALSE. Set to TRUE for own cuts which
                                              *   are separated as constraints. */
   SCIP_Bool             removable,          /**< should the relaxation be removed from the LP due to aging or cleanup?
                                              *   Usually set to FALSE. Set to TRUE for 'lazy constraints' and 'user cuts'. */
   SCIP_Bool             stickingatnode      /**< should the constraint always be kept at the node where it was added, even
                                              *   if it may be moved to a more global node?
                                              *   Usually set to FALSE. Set to TRUE to for constraints that represent node data. */
   );

/** creates and captures a expr constraint with all its constraint flags set to their
 *  default values
 *
 *  @note the constraint gets captured, hence at one point you have to release it using the method SCIPreleaseCons()
 */
EXTERN
SCIP_RETCODE SCIPcreateConsExprBasic(
   SCIP*                 scip,               /**< SCIP data structure */
   SCIP_CONS**           cons,               /**< pointer to hold the created constraint */
   const char*           name,               /**< name of constraint */
   int                   nvars,              /**< number of variables in the constraint */
   SCIP_VAR**            vars,               /**< array with variables of constraint entries */
   SCIP_Real*            coefs,              /**< array with coefficients of constraint entries */
   SCIP_Real             lhs,                /**< left hand side of constraint */
   SCIP_Real             rhs                 /**< right hand side of constraint */
   );

/** @} */

#ifdef __cplusplus
}
#endif

#endif
