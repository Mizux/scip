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

/**@file   reader_stp.h
 * @brief  Steiner tree problem file reader
 * @author Gerald Gamrath
 * @author Thorsten Koch
 * @author Daniel Rehfeldt
 * @author Michael Winkler
 *
 * This file implements a reader used to read and write Steiner tree problems.
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_READER_STP_H__
#define __SCIP_READER_STP_H__


#include "scip/scip.h"

#ifdef __cplusplus
extern "C" {
#endif

/** include user parameters */
SCIP_EXPORT
SCIP_RETCODE SCIPStpReaderIncludeParams(
   SCIP*                 scip                /**< SCIP data structure */
   );

/** includes the stp file reader in SCIP */
SCIP_EXPORT
SCIP_RETCODE SCIPincludeReaderStp(
   SCIP*                 scip                /**< SCIP data structure */
   );

#ifdef __cplusplus
}
#endif

#endif
