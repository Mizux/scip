/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2010 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma ident "@(#) $Id: expression.h,v 1.10 2010/06/01 19:22:31 bzfviger Exp $"

/**@file   expression.h
 * @brief  methods for expressions and expression trees
 * @author Stefan Vigerske
 * @author Thorsten Gellermann
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __NLPI_EXPRESSION_H__
#define __NLPI_EXPRESSION_H__

#include "scip/def.h"
#include "blockmemshell/memory.h"
#include "nlpi/type_expression.h"
#include "nlpi/type_exprinterpret.h"
#include "scip/intervalarith.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@name Expression operand methods */
/**@{ */

/** gives the name of an operand */
extern
const char* SCIPexpropGetName(
   SCIP_EXPROP           op                  /**< expression operand */
);

/** gives the number of children of a simple operand
 * @return -1 for invalid operands and -2 for complex operands (those where the number of children depends on the expression)
 */
extern
int SCIPexpropGetNChildren(
   SCIP_EXPROP           op                  /**< expression operand */
);

/**@} */

/**@name Expression methods */
/**@{ */

/** creates an expression */
extern
SCIP_RETCODE SCIPexprCreate(
   BMS_BLKMEM*           blkmem,             /**< block memory data structure */
   SCIP_EXPR**           expr,               /**< pointer to buffer for expression address */
   SCIP_EXPROP           op,                 /**< operand of expression */
   ...                                       /**< arguments of operand */
);

/** creates an expression
 * Note, that the expression is allocated but for the children only the pointer is copied.
 */
extern
SCIP_RETCODE SCIPexprCreateDirect(
   BMS_BLKMEM*           blkmem,             /**< block memory data structure */
   SCIP_EXPR**           expr,               /**< pointer to buffer for expression address */
   SCIP_EXPROP           op,                 /**< operand of expression */
   int                   nchildren,          /**< number of children */
   SCIP_EXPR**           children,           /**< children */
   SCIP_EXPROPDATA       opdata              /**< operand data */
);

/** copies an expression including its children */
extern
SCIP_RETCODE SCIPexprCopyDeep(
   BMS_BLKMEM*           blkmem,             /**< block memory data structure */
   SCIP_EXPR**           targetexpr,         /**< buffer to store pointer to copied expression */
   SCIP_EXPR*            sourceexpr          /**< expression to copy */
);

/** frees an expression including its children */
extern
void SCIPexprFreeDeep(
   BMS_BLKMEM*           blkmem,             /**< block memory data structure */
   SCIP_EXPR**           expr                /**< pointer to expression to free */
);

/** gives operator of expression */
extern
SCIP_EXPROP SCIPexprGetOperator(
   SCIP_EXPR*            expr                /**< expression */
);

/** gives number of children of an expression */
extern
int SCIPexprGetNChildren(
   SCIP_EXPR*            expr                /**< expression */
);

/** gives pointer to array with children of an expression */
extern
SCIP_EXPR** SCIPexprGetChildren(
   SCIP_EXPR*            expr                /**< expression */
);

/** gives index belonging to a SCIP_EXPR_VARIDX or SCIP_EXPR_PARAM operand */
extern
int SCIPexprGetOpIndex(
   SCIP_EXPR*            expr                /**< expression */
);

/** gives real belonging to a SCIP_EXPR_CONST operand */ 
extern
SCIP_Real SCIPexprGetOpReal(
   SCIP_EXPR* expr                           /**< expression */
);

/** gives void* belonging to a complex operand */
extern
void* SCIPexprGetOpData(
   SCIP_EXPR*            expr                /**< expression */
);

/** gives exponent belonging to a SCIP_EXPR_INTPOWER operand */
extern
int SCIPexprGetIntPowerExponent(
   SCIP_EXPR*            expr                /**< expression */
);

/** indicates whether the expression contains a SCIP_EXPR_PARAM */
extern
SCIP_Bool SCIPexprHasParam(
   SCIP_EXPR*            expr                /**< expression */
);

/** gets maximal degree of expression, or 65535 if not a polynom */
extern
SCIP_RETCODE SCIPexprGetMaxDegree(
   SCIP_EXPR*            expr,               /**< expression */
   int*                  maxdegree           /**< buffer to store maximal degree */
);

/** evaluates an expression w.r.t. a point */
extern
SCIP_RETCODE SCIPexprEval(
   SCIP_EXPR*            expr,               /**< expression */
   SCIP_Real*            varvals,            /**< values for variables, can be NULL if the expression is constant */
   SCIP_Real*            param,              /**< values for parameters, can be NULL if the expression is not parameterized */
   SCIP_Real*            val                 /**< buffer to store value */
);

/** evaluates an expression w.r.t. an interval */
extern
SCIP_RETCODE SCIPexprEvalInt(
   SCIP_EXPR*            expr,               /**< expression */
   SCIP_Real             infinity,           /**< value to use for infinity */
   SCIP_INTERVAL*        varvals,            /**< interval values for variables, can be NULL if the expression is constant */
   SCIP_Real*            param,              /**< values for parameters, can be NULL if the expression is not parameterized */
   SCIP_INTERVAL*        val                 /**< buffer to store value */
);

/** substitutes variables (SCIP_EXPR_VARIDX) by expressions
 * Note than only the children of the given expr are checked!
 * A variable with index i is replaced by a copy of substexprs[i], if that latter is not NULL
 * if substexprs[i] == NULL, then the variable expression i is not touched */
extern
SCIP_RETCODE SCIPexprSubstituteVars(
   BMS_BLKMEM*           blkmem,             /**< block memory data structure */
   SCIP_EXPR*            expr,               /**< expression, which of the children may be replaced */
   SCIP_EXPR**           substexprs          /**< array of substitute expressions; single entries can be NULL */
);

/** updates variable indices in expression tree */
extern
void SCIPexprReindexVars(
   SCIP_EXPR*            expr,               /**< expression to update */
   int*                  newindices          /**< new indices of variables */
);

/** prints an expression */
extern
void SCIPexprPrint(
   SCIP_EXPR*            expr,               /**< expression */
   FILE*                 file,               /**< file for printing, or NULL for stdout */
   const char**          varnames,           /**< names of variables, or NULL for default names */
   const char**          paramnames          /**< names of parameters, or NULL for default names */
);

/**@} */

/**@name Expression tree methods */
/**@{ */

/** creates an expression tree */
extern
SCIP_RETCODE SCIPexprtreeCreate(
   BMS_BLKMEM*           blkmem,             /**< block memory data structure */
   SCIP_EXPRTREE**       tree,               /**< buffer to store address of created expression tree */
   SCIP_EXPR*            root,               /**< pointer to root expression, not copied deep !, can be NULL */
   int                   nvars,              /**< number of variables in variable mapping */
   int                   nparams,            /**< number of parameters in expression */
   SCIP_Real*            params              /**< values for parameters, or NULL (if NULL but nparams > 0, then params is initialized with zeros) */
);

/** copies an expression tree */
extern
SCIP_RETCODE SCIPexprtreeCopy(
   BMS_BLKMEM*           blkmem,             /**< block memory that should be used in new expression tree */
   SCIP_EXPRTREE**       targettree,         /**< buffer to store address of copied expression tree */
   SCIP_EXPRTREE*        sourcetree          /**< expression tree to copy */
);

/** frees an expression tree */
extern
SCIP_RETCODE SCIPexprtreeFree(
   SCIP_EXPRTREE**       tree                /**< pointer to expression tree that is freed */
);

/** returns root expression of an expression tree */
extern
SCIP_EXPR* SCIPexprtreeGetRoot(
   SCIP_EXPRTREE*        tree                /**< expression tree */
);

/** returns number of variables in expression tree */
extern
int SCIPexprtreeGetNVars(
   SCIP_EXPRTREE*        tree                /**< expression tree */
);

/** returns number of parameters in expression tree */
extern
int SCIPexprtreeGetNParams(
   SCIP_EXPRTREE*        tree                /**< expression tree */
);

/** returns values of parameters or NULL if none */
extern
SCIP_Real* SCIPexprtreeGetParamVals(
   SCIP_EXPRTREE*        tree                /**< expression tree */
);

/** sets value of a single parameter in expression tree */
extern
void SCIPexprtreeSetParamVal(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   int                   paramidx,           /**< index of parameter */
   SCIP_Real             paramval            /**< new value of parameter */
);

/** sets values of all parameters in expression tree */
extern
void SCIPexprtreeSetParamVals(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   SCIP_Real*            paramvals           /**< new values of parameters */
);

/** gets data of expression tree interpreter
 * @return NULL if not set
 */
extern
SCIP_EXPRINTDATA* SCIPexprtreeGetInterpreterData(
   SCIP_EXPRTREE*        tree                /**< expression tree */
);

/** indicates whether there are parameterized constants (SCIP_EXPR_PARAM) in expression tree */
extern
SCIP_Bool SCIPexprtreeHasParam(
   SCIP_EXPRTREE*        tree                /**< expression tree */
);

/** Gives maximal degree of expression in expression tree.
 * If constant expression, gives 0,
 * if linear expression, gives 1,
 * if polynomial expression, gives its maximal degree,
 * otherwise (nonpolynomial nonconstant expressions) gives at least 65535.
 */
extern
SCIP_RETCODE SCIPexprtreeGetMaxDegree(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   int*                  maxdegree           /**< buffer to store maximal degree */
);

/** sets data of expression tree interpreter */
extern
void SCIPexprtreeSetInterpreterData(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   SCIP_EXPRINTDATA*     interpreterdata     /**< expression interpreter data */
);

/** evaluates an expression tree w.r.t. a point */
extern
SCIP_RETCODE SCIPexprtreeEval(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   SCIP_Real*            varvals,            /**< values for variables */
   SCIP_Real*            val                 /**< buffer to store expression tree value */
);

/** evaluates an expression tree w.r.t. an interval */
extern
SCIP_RETCODE SCIPexprtreeEvalInt(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        varvals,            /**< intervals for variables */
   SCIP_INTERVAL*        val                 /**< buffer to store expression tree value */
);

/** substitutes variables (SCIP_EXPR_VARIDX) in an expression tree by expressions
 * A variable with index i is replaced by a copy of substexprs[i], if that latter is not NULL
 * if substexprs[i] == NULL, then the variable expression i is not touched */
extern
SCIP_RETCODE SCIPexprtreeSubstituteVars(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   SCIP_EXPR**           substexprs          /**< array of substitute expressions; single entries can be NULL */
);

/** prints an expression tree */
extern
void SCIPexprtreePrint(
   SCIP_EXPRTREE*        tree,               /**< expression tree */
   FILE*                 file,               /**< file for printing, or NULL for stdout */
   const char**          varnames,           /**< names of variables, or NULL for default names */
   const char**          paramnames          /**< names of parameters, or NULL for default names */
);

/**@} */

/**@name Quadratic element methods */
/**@{ */

/** sorts an array of quadratic elements
 * The elements are sorted such that the first index is increasing and
 * such that among elements with the same first index, the second index is increasing.
 * For elements with same first and second index, the order is not defined.
 */
extern
void SCIPquadelemSort(
   SCIP_QUADELEM*        quadelems,          /**< array of quadratic elements */
   int                   nquadelems          /**< number of quadratic elements */
);

/** Finds an index pair in a sorted array of quadratic elements.
 * If (idx1,idx2) is found in quadelems, then returns TRUE and stores position of quadratic element in *pos.
 * If (idx1,idx2) is not found in quadelems, then returns FALSE and stores position where a quadratic element with these indices would be inserted in *pos.
 * Assumes that idx1 <= idx2.
 */
extern
SCIP_Bool SCIPquadelemSortedFind(
   SCIP_QUADELEM*        quadelems,          /**< array of quadratic elements */
   int                   idx1,               /**< index of first  variable in element to search for */
   int                   idx2,               /**< index of second variable in element to search for */
   int                   nquadelems,         /**< number of quadratic elements in array */
   int*                  pos                 /**< buffer to store position of found quadratic element, or position where it would be inserted */
);

/** Adds quadratic elements with same index and removes elements with coefficient 0.0.
 * Assumes that elements have been sorted before.
 */
extern
void SCIPquadelemSqueeze(
   SCIP_QUADELEM*        quadelems,          /**< array of quadratic elements */
   int                   nquadelems,         /**< number of quadratic elements */
   int*                  nquadelemsnew       /**< pointer to store new (reduced) number of quadratic elements */
);

/**@} */

#ifdef __cplusplus
}
#endif

#endif /* __NLPI_EXPRESSION_H__ */
