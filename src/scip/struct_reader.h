/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*                  This file is part of the program and library             */
/*         SCIP --- Solving Constraint Integer Programs                      */
/*                                                                           */
/*    Copyright (C) 2002-2005 Tobias Achterberg                              */
/*                                                                           */
/*                  2002-2005 Konrad-Zuse-Zentrum                            */
/*                            fuer Informationstechnik Berlin                */
/*                                                                           */
/*  SCIP is distributed under the terms of the ZIB Academic License.         */
/*                                                                           */
/*  You should have received a copy of the ZIB Academic License              */
/*  along with SCIP; see the file COPYING. If not email to scip@zib.de.      */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#pragma ident "@(#) $Id: struct_reader.h,v 1.8 2005/08/22 18:35:51 bzfpfend Exp $"

/**@file   struct_reader.h
 * @brief  datastructures for input file readers
 * @author Tobias Achterberg
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#ifndef __SCIP_STRUCT_READER_H__
#define __SCIP_STRUCT_READER_H__


#include "scip/def.h"
#include "scip/type_reader.h"


/** input file reader */
struct SCIP_Reader
{
   const char*           name;               /**< name of reader */
   const char*           desc;               /**< description of reader */
   const char*           extension;          /**< file extension that reader processes */
   SCIP_DECL_READERFREE  ((*readerfree));    /**< destructor of reader */
   SCIP_DECL_READERREAD  ((*readerread));    /**< read method */
   SCIP_READERDATA*      readerdata;         /**< reader data */
};


#endif
