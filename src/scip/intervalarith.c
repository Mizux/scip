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
#pragma ident "@(#) $Id: intervalarith.c,v 1.40 2010/05/10 15:13:12 bzfviger Exp $"

/**@file   intervalarith.c
 * @brief  interval arithmetics for provable bounds
 * @author Tobias Achterberg
 * @author Stefan Vigerske
 */

/*---+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include <stdlib.h>
#include <assert.h>

#include "scip/def.h"
#include "scip/message.h"
#include "scip/intervalarith.h"



#ifdef ROUNDING_FE
#define ROUNDING
/*
 * Linux rounding operations
 */

#include <fenv.h>

/** Linux rounding mode settings */
enum RoundMode
{
   SCIP_ROUND_DOWNWARDS = FE_DOWNWARD,  /**< round always down */
   SCIP_ROUND_UPWARDS   = FE_UPWARD     /**< round always up */
};

/** sets rounding mode of floating point operations */
void setRoundingMode(
   ROUNDMODE        roundmode           /**< rounding mode to activate */
   )
{
   if( fesetround(roundmode) != 0 )
   {
      SCIPerrorMessage("error setting rounding mode to %d\n", roundmode);
      SCIPABORT();
   }
}

/** gets current rounding mode of floating point operations */
ROUNDMODE getRoundingMode(
   void
   )
{
   return (ROUNDMODE)fegetround();
}
#endif



#ifdef ROUNDING_FP
#define ROUNDING
/*
 * OSF rounding operations
 */

#include <float.h>

/** OSF rounding mode settings */
enum RoundMode
{
   SCIP_ROUND_DOWNWARDS = FP_RND_RM,    /**< round always down */
   SCIP_ROUND_UPWARDS   = FP_RND_RP     /**< round always up */
};

/** sets rounding mode of floating point operations */
void setRoundingMode(
   ROUNDMODE        roundmode           /**< rounding mode to activate */
   )
{
   if( write_rnd(roundmode) != 0 )
   {
      SCIPerrorMessage("error setting rounding mode to %d\n", roundmode);
      SCIPABORT();
   }
}

/** gets current rounding mode of floating point operations */
ROUNDMODE getRoundingMode(
   void
   )
{
   return read_rnd();
}
#endif

#ifdef ROUNDING_MS
#define ROUNDING
/*
 * Microsoft compiler rounding operations
 */

#include <float.h>

/** Microsoft rounding mode settings */
enum RoundMode
{
   SCIP_ROUND_DOWNWARDS = RC_DOWN,           /**< round always down */
   SCIP_ROUND_UPWARDS   = RC_UP              /**< round always up */
};

/** sets rounding mode of floating point operations */
void setRoundingMode(
   ROUNDMODE        roundmode           /**< rounding mode to activate */
   )
{
   if( (_controlfp(roundmode, _MCW_RC) & _MCW_RC) != roundmode )
   {
      SCIPerrorMessage("error setting rounding mode to %x\n", roundmode);
      SCIPABORT();
   }
}

/** gets current rounding mode of floating point operations */
ROUNDMODE getRoundingMode(
   void
   )
{
	return _controlfp(0, 0) & _MCW_RC;
}
#endif


#ifndef ROUNDING
/*
 * rouding operations not available
 */
enum RoundMode
{
   SCIP_ROUND_DOWNWARDS = 0,            /**< round always down */
   SCIP_ROUND_UPWARDS   = 1             /**< round always up */
};

/** sets rounding mode of floating point operations */
void setRoundingMode(
   ROUNDMODE        roundmode           /**< rounding mode to activate */
   )
{  /*lint --e{715}*/
   SCIPwarningMessage("setting rounding mode not available - interval arithmetic is invalid!\n");
}

/** gets current rounding mode of floating point operations */
ROUNDMODE getRoundingMode(
   void
   )
{
   return SCIP_ROUND_DOWNWARDS;
}
#else
#undef ROUNDING
#endif


/** sign of a value (-1 or +1)
 * 
 * 0.0 has sign +1
 */
#define SIGN(x) ((x) >= 0.0 ? 1.0 : -1.0)

static SCIP_Bool warned_unsafe_pow = FALSE;
static SCIP_Bool warned_unsafe_exp = FALSE;
static SCIP_Bool warned_unsafe_log = FALSE;

/*
 * Interval arithmetic operations
 */

/** stores given value as interval */
void SCIPintervalSet(
   SCIP_INTERVAL*        resultant,          /**< interval to store value into */
   SCIP_Real             value               /**< value to store */
   )
{
   assert(resultant != NULL);

   resultant->inf = value;
   resultant->sup = value;
}

/** stores given infimum and supremum as interval */
void SCIPintervalSetBounds(
   SCIP_INTERVAL*        resultant,          /**< interval to store value into */
   SCIP_Real             inf,                /**< value to store as infimum */
   SCIP_Real             sup                 /**< value to store as supremum */
   )
{
   assert(resultant != NULL);
   assert(inf <= sup);

   resultant->inf = inf;
   resultant->sup = sup;
}

/** sets interval to empty interval, which will be [infinity, -infinity] */
void SCIPintervalSetEmpty(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant           /**< resultant interval of operation */
   )
{
   assert(resultant != NULL);
   
   resultant->inf =  infinity;
   resultant->sup = -infinity;
}

/** indicates whether interval is empty, i.e., whether inf > sup */
SCIP_Bool SCIPintervalIsEmpty(
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   return operand.sup < operand.inf;
}

/** sets interval to entire [-infinity, +infinity] */
void SCIPintervalSetEntire(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant           /**< resultant interval of operation */
   )
{
   assert(resultant != NULL);

   resultant->inf = -infinity;
   resultant->sup =  infinity;
}

/** indicates whether interval is entire, i.e., whether inf <= -infinity and sup >= infinity */
SCIP_Bool SCIPintervalIsEntire(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   return operand.inf <= -infinity && operand.sup >= infinity;
}

/** indicates whether operand1 is contained in operand2 */
SCIP_Bool SCIPintervalIsSubsetEQ(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   /* the empty interval is contained everywhere */
   if( operand1.inf > operand1.sup )
      return TRUE;
   
   /* something not-empty is not contained in the empty interval */
   if( operand2.inf > operand2.sup )
      return FALSE;
   
   return (MAX(-infinity, operand1.inf) >= operand2.inf) &&
          (MIN( infinity, operand1.sup) <= operand2.sup);
}

/** intersection of two intervals */
void SCIPintervalIntersect(
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   assert(resultant != NULL);
   
   resultant->inf = MAX(operand1.inf, operand2.inf);
   resultant->sup = MIN(operand1.sup, operand2.sup);
}

/** interval enclosure of the union of two intervals */
void SCIPintervalUnify(
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   assert(resultant != NULL);
   
   resultant->inf = MIN(operand1.inf, operand2.inf);
   resultant->sup = MAX(operand1.sup, operand2.sup);
}


/** adds operand1 and operand2 and stores result in resultant */
void SCIPintervalAdd(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2.inf <  infinity);
   assert(operand2.sup > -infinity);

   roundmode = getRoundingMode();

   if( operand1.inf <= -infinity || operand2.inf <= -infinity )
   {
      resultant->inf = -infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = operand1.inf + operand2.inf;
   }
   
   if( operand1.sup >=  infinity || operand2.sup >=  infinity )
   {
      resultant->sup =  infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = operand1.sup + operand2.sup;
   }
   
   setRoundingMode(roundmode);
}


/** adds operand1 and scalar operand2 and stores result in resultant */
void SCIPintervalAddScalar(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_Real             operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2     <  infinity);
   assert(operand2     > -infinity);

   roundmode = getRoundingMode();

   if( operand1.inf <= -infinity )
   {
      resultant->inf = -infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = operand1.inf + operand2;
   }
   
   if( operand1.sup >=  infinity )
   {
      resultant->sup =  infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = operand1.sup + operand2;
   }
   
   setRoundingMode(roundmode);
}

/** substracts operand2 from operand1 and stores result in resultant */
void SCIPintervalSub(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2.inf <  infinity);
   assert(operand2.sup > -infinity);

   roundmode = getRoundingMode();

   if( operand1.inf <= -infinity || operand2.sup >=  infinity )
   {
      resultant->inf = -infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = operand1.inf - operand2.sup;
   }
   
   if( operand1.sup >=  infinity || operand2.inf <= -infinity )
   {
      resultant->sup =  infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = operand1.sup - operand2.inf;
   }

   setRoundingMode(roundmode);
}

/** substracts scalar operand2 from operand1 and stores result in resultant */
void SCIPintervalSubScalar(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_Real             operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2     <  infinity);
   assert(operand2     > -infinity);

   roundmode = getRoundingMode();

   if( operand1.inf <= -infinity )
   {
      resultant->inf = -infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = operand1.inf - operand2;
   }
   
   if( operand1.sup >=  infinity )
   {
      resultant->sup =  infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = operand1.sup - operand2;
   }

   setRoundingMode(roundmode);
}

/** undoes a substraction operation.
 * 
 * In number arithmetic, this would be addition.
 * Substractions of unbounded intervals cannot be undone, but resultant gives still a valid (but probably larger) interval.
 * 
 * This is a ''dirty'' operation.
 */
void SCIPintervalUndoSub(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2.inf <  infinity);
   assert(operand2.sup > -infinity);

   roundmode = getRoundingMode();

   if( operand1.inf <= -infinity || operand2.sup >=  infinity )
   {
      resultant->inf = -infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = operand1.inf + operand2.sup;
   }
   
   if( operand1.sup >=  infinity || operand2.inf <= -infinity )
   {
      resultant->sup =  infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = operand1.sup + operand2.inf;
   }

   setRoundingMode(roundmode);
}

/** multiplies operand1 with operand2 and stores result in resultant */
void SCIPintervalMul(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;
   SCIP_Real cand1;
   SCIP_Real cand2;
   SCIP_Real cand3;
   SCIP_Real cand4;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2.inf <  infinity);
   assert(operand2.sup > -infinity);

   roundmode = getRoundingMode();

   if( (operand1.inf <= -infinity && operand2.sup > 0.0       ) ||
       (operand1.sup >   0.0      && operand2.inf <= -infinity) ||
       (operand1.inf <   0.0      && operand2.sup >=  infinity) ||
       (operand1.sup >=  infinity && operand2.inf < 0.0       ) )
   {
      resultant->inf = -infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      cand1 = operand1.inf * operand2.inf;
      cand2 = operand1.inf * operand2.sup;
      cand3 = operand1.sup * operand2.inf;
      cand4 = operand1.sup * operand2.sup;
      resultant->inf = MIN(MIN(cand1, cand2), MIN(cand3, cand4));
   }
   
   if( (operand1.inf <= -infinity && operand2.inf <   0.0     ) ||
       (operand1.inf <   0.0      && operand2.inf <= -infinity) ||
       (operand1.sup >   0.0      && operand2.sup >=  infinity) ||
       (operand1.sup >=  infinity && operand2.sup >   0.0     ) )
   {
      resultant->sup =  infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      cand1 = operand1.inf * operand2.inf;
      cand2 = operand1.inf * operand2.sup;
      cand3 = operand1.sup * operand2.inf;
      cand4 = operand1.sup * operand2.sup;
      resultant->sup = MAX(MAX(cand1, cand2), MAX(cand3, cand4));
   }
   
#if 0   
   if( operand1.inf >= 0.0 )
   {
      if( operand2.inf >= 0.0 )
      {
         /* [+,+] * [+,+] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.inf * operand2.inf;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.sup * operand2.sup;
      }
      else if( operand2.sup >= 0.0 )
      {
         /* [+,+] * [-,+] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.sup * operand2.inf;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.sup * operand2.sup;
      }
      else
      {
         /* [+,+] * [-,-] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.sup * operand2.inf;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.inf * operand2.sup;
      }
   }
   else if( operand1.sup >= 0.0 )
   {
      if( operand2.inf >= 0.0 )
      {
         /* [-,+] * [+,+] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.inf * operand2.sup;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.sup * operand2.sup;
      }
      else if( operand2.sup >= 0.0 )
      {
         SCIP_Real x;
         SCIP_Real y;

         /* [-,+] * [-,+] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         x = operand1.inf * operand2.sup;
         y = operand1.sup * operand2.inf;
         resultant->inf = MIN(x, y);
         setRoundingMode(SCIP_ROUND_UPWARDS);
         x = operand1.inf * operand2.inf;
         y = operand1.sup * operand2.sup;
         resultant->sup = MAX(x, y);
      }
      else
      {
         /* [-,+] * [-,-] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.sup * operand2.inf;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.inf * operand2.inf;
      }
   }
   else
   {
      if( operand2.inf >= 0.0 )
      {
         /* [-,-] * [+,+] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.inf * operand2.sup;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.sup * operand2.inf;
      }
      else if( operand2.sup >= 0.0 )
      {
         /* [-,-] * [-,+] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.inf * operand2.sup;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.inf * operand2.inf;
      }
      else
      {
         /* [-,-] * [-,-] */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.sup * operand2.sup;
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.inf * operand2.inf;
      }
   }
#endif
   
   setRoundingMode(roundmode);
}

/** multiplies operand1 with scalar operand2 and stores result in resultant */
void SCIPintervalMulScalar(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_Real             operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand1.inf <  infinity);
   assert(operand2     <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2     > -infinity);

   if( operand2 == 0.0 )
   {
      resultant->inf = 0.0;
      resultant->sup = 0.0;
      return;
   }

   roundmode = getRoundingMode();

   if( operand2 >= 0.0 )
   {
      if( operand1.inf <= -infinity )
      {
         resultant->inf = -infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.inf * operand2;
      }
      if( operand1.sup >=  infinity )
      {
         resultant->sup =  infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.sup * operand2;
      }
   }
   else
   {
      if( operand1.sup >=  infinity )
      {
         resultant->inf = -infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.sup * operand2;
      }
      if( operand1.inf <= -infinity )
      {
         resultant->sup = infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.inf * operand2;
      }
   }
  
   setRoundingMode(roundmode);
}

/** divides operand1 by operand2 and stores result in resultant */
void SCIPintervalDiv(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);
   assert(operand1.inf <  infinity);
   assert(operand2.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2.sup > -infinity);

   if( operand2.inf == 0.0 && operand2.sup == 0.0 )
   {  /* division by [0,0] */
      SCIPintervalSetEmpty(infinity, resultant);
      return;
   }
  
   if( operand1.inf == 0.0 && operand1.sup == 0.0 )
   {  /* division of [0,0] by something */
      SCIPintervalSet(resultant, 0.0);
      return;
   }
  
   roundmode = getRoundingMode();
  
   if( operand2.inf > 0.0 || operand2.sup < 0.0 )
   {  /* divison by nonzero: resultant = x * (1/y) */
      SCIP_INTERVAL intmed;
      if( operand2.sup >=  infinity )
      {
         intmed.inf = 0.0;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         intmed.inf = 1 / operand2.sup;
      }
      if( operand2.inf <= -infinity )
      {
         intmed.sup = 0.0;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         intmed.sup = 1 / operand2.inf;
      }
      SCIPintervalMul(infinity, resultant, operand1, intmed);
   }
   else if( operand1.inf >= 0.0 )
   {
      if( operand2.inf == 0.0 )
      {
         if( operand2.sup >=  infinity )
         {
            resultant->inf = 0.0;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_DOWNWARDS);
            resultant->inf = operand1.inf / operand2.sup;
         }
         resultant->sup = infinity;
      }
      else if( operand2.sup == 0.0 )
      {
         resultant->inf = -infinity;
         if( operand2.inf <= -infinity )
         {
            resultant->sup = 0.0;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_UPWARDS);
            resultant->sup = operand1.inf / operand2.inf;
         }
      }
      else
      {
         resultant->inf = -infinity;
         resultant->sup =  infinity;
      }
   }
   else if( operand1.sup <= 0.0 )
   {
      if( operand2.inf == 0.0 )
      {
         resultant->inf = -infinity;
         if( operand2.sup >= infinity )
         {
            resultant->sup = 0.0;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_UPWARDS);
            resultant->sup = operand1.sup / operand2.sup;
         }
      }
      else if( operand2.sup == 0.0 )
      {
         if( operand2.inf <= -infinity )
         {
            resultant->inf = 0.0;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_DOWNWARDS);
            resultant->inf = operand1.sup / operand2.inf;
         }
         resultant->sup = infinity;
      }
      else
      {
         resultant->inf = -infinity;
         resultant->sup =  infinity;
      }
   }
   else
   {
      resultant->inf = -infinity;
      resultant->sup =  infinity;
   }
 
  setRoundingMode(roundmode);
}

/** divides operand1 by scalar operand2 and stores result in resultant
 * 
 * if operand2 is 0.0, gives an empty interval as result */
void SCIPintervalDivScalar(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_Real             operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand1.inf <  infinity);
   assert(operand2     <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2     > -infinity);

   roundmode = getRoundingMode();
   
   if( operand2 > 0.0 )
   {
      if( operand1.inf <= -infinity )
      {
         resultant->inf = -infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.inf / operand2;
      }
      if( operand1.sup >= infinity )
      {
         resultant->sup =  infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.sup / operand2;
      }
   }
   else if( operand2 < 0.0 )
   {
      if( operand1.sup >=  infinity )
      {
         resultant->inf = -infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = operand1.sup / operand2;
      }
      if( operand1.inf <= -infinity )
      {
         resultant->sup = infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand1.inf / operand2;
      }
   }
   else
   { /* division by 0.0 */
     SCIPintervalSetEmpty(infinity, resultant);
   }
  
   setRoundingMode(roundmode);
}

/** squares operand and stores result in resultant */
void SCIPintervalSquare(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand.inf <= operand.sup);
   assert(operand.inf <  infinity);
   assert(operand.sup > -infinity);
  
   roundmode = getRoundingMode();

   if( operand.sup <= 0.0 )
   {  /* operand is left of 0.0 */
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = operand.sup * operand.sup;
      if( operand.inf <= -infinity )
      {
         resultant->sup = infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand.inf * operand.inf;
      }
   }
   else if( operand.inf >= 0.0 )
   {  /* operand is right of 0.0 */
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = operand.inf * operand.inf;
      if( operand.sup >= infinity )
      {
         resultant->sup = infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = operand.sup * operand.sup;
      }
   }
   else
   {  /* 0.0 inside resultant */
      SCIP_Real x;
      SCIP_Real y;
      resultant->inf = 0.0;
      setRoundingMode(SCIP_ROUND_UPWARDS);
      x = operand.inf * operand.inf;
      y = operand.sup * operand.sup;
      resultant->sup = MAX(x, y);
   }

   setRoundingMode(roundmode);
}

/** stores (positive part of) square root of operand in resultant */
void SCIPintervalSquareRoot(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand.inf <= operand.sup);
   assert(operand.inf <  infinity);
   assert(operand.sup > -infinity);
   
   if( operand.sup < 0.0 )
   {
      SCIPintervalSetEmpty(infinity, resultant);
      return;
   }
  
   roundmode = getRoundingMode();
   
   if( operand.inf <= 0.0 )
   {
      resultant->inf = 0.0;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = sqrt(operand.inf);
   }
   
   if( operand.sup >= infinity )
   {
      resultant->sup = infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = sqrt(operand.sup);
   }
  
   setRoundingMode(roundmode);
}

/** stores operand1 to the power of operand2 in resultant
 * 
 * uses SCIPintervalPowerScalar if operand2 is a scalar, otherwise computes exp(op2*log(op1)) */
void SCIPintervalPower(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{  /*lint --e{777}*/
   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);
   assert(operand1.inf <  infinity);
   assert(operand2.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2.sup > -infinity);

   if( operand2.inf == operand2.sup )
   {  /* operand is number */
      SCIPintervalPowerScalar(infinity, resultant, operand1, operand2.inf);
      return;
   }
   
   /* resultant := log(op1) */
   SCIPintervalLog(infinity, resultant, operand1);
   if( SCIPintervalIsEmpty(*resultant) )
     return;
   
   /* resultant := op2 * resultant */
   SCIPintervalMul(infinity, resultant, operand2, *resultant);
   
   /* resultant := exp(resultant) */
   SCIPintervalExp(infinity, resultant, *resultant);
}

/** stores operand1 to the power of the scalar operand2 in resultant */
void SCIPintervalPowerScalar(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_Real             operand2            /**< second operand of operation */
   )
{  /*lint --e{777}*/
   ROUNDMODE roundmode;
   SCIP_Bool op2isint;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2     <  infinity);
   assert(operand2     > -infinity);
   
   op2isint = (ceil(operand2) == operand2);
   
   if( !op2isint && operand1.inf < 0.0 )
   {  /* x^n with x negative not defined for n not integer*/
      operand1.inf = 0.0;
      if( operand1.sup < operand1.inf )
      {
         SCIPintervalSetEmpty(infinity, resultant);
         return;
      }
   }

   if( !warned_unsafe_pow )
   {
      SCIPwarningMessage("Warning: interval arithmetic for pow function is not rounding safe!\n");
      warned_unsafe_pow = TRUE;
   }

   roundmode = getRoundingMode();
   
   if( operand1.inf >= 0.0 )
   {  /* easy case: x^n with x>=0 */
      if( operand2 >= 0.0 )
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = pow(operand1.inf, operand2);
         if( operand1.sup >= infinity )
         {
            resultant->sup = infinity;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_UPWARDS);
            resultant->sup = pow(operand1.sup, operand2);
         }
      }
      else
      {
         if( operand1.sup >= infinity )
         {
            resultant->inf = 0.0;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_DOWNWARDS);
            resultant->inf = pow(operand1.sup, operand2);
         }
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = pow(operand1.inf, operand2);
      }
   }
   else if( operand1.sup < 0.0 )
   {  /* more difficult case: x^n with x < 0; we now know, that n is integer */
      assert(op2isint);
      if( (operand2 >= 0.0 && ceil(operand2/2) == operand2/2) || (operand2 <= 0.0 && ceil(operand2/2) != operand2/2) )
      {  /* x^n with (n>=0 and even) or (n<=0 and odd) -> x^n is mon. decreasing for x<0 */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = pow(operand1.sup, operand2);
         if( operand1.inf <= -infinity )
         {
            resultant->sup = infinity;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_UPWARDS);
            resultant->sup = pow(operand1.inf, operand2);
         }
      }
      else
      {  /* x^n with (n<0 and even) or (n>0 and odd) -> x^n is mon. increasing for x<0 */
         if( operand1.inf <= -infinity )
         {
            resultant->inf = -infinity;
         }
         else
         {
            setRoundingMode(SCIP_ROUND_DOWNWARDS);
            resultant->inf = pow(operand1.inf, operand2);
         }
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = pow(operand1.sup, operand2);
      }
   }
   else
   {  /* most difficult case: x^n with x in [-,+], but n is integer */
      assert(op2isint); /* otherwise we had set operand1.inf == 0.0, which was handled in first case */
      if( operand2 < 0.0 )
      {  /* division of [-,+] by zero */
         resultant->inf = -infinity;
         resultant->sup =  infinity;
      }
      else
      {  /* x^n with n positive integer */
         if( operand2/2 == ceil(operand2/2) )
         {  /* n is even */
            if( -operand1.inf >= operand1.sup )
            {
               if( operand1.sup >= infinity )
               {  /* and so is inf == -infty */
                  resultant->inf = -infinity;
                  resultant->sup =  infinity;
               }
               else if( operand1.inf <= -infinity )
               {
                  setRoundingMode(SCIP_ROUND_DOWNWARDS);
                  resultant->inf = pow(operand1.sup, operand2);
                  resultant->sup = infinity;
               }
               else
               {
                  setRoundingMode(SCIP_ROUND_DOWNWARDS);
                  resultant->inf = pow(operand1.sup, operand2);
                   setRoundingMode(SCIP_ROUND_UPWARDS);
                  resultant->sup = pow(operand1.inf, operand2);
               }
            }
            else
            {  /* -inf < sup, so -inf is not -infty */
                setRoundingMode(SCIP_ROUND_DOWNWARDS);
                resultant->inf = pow(operand1.inf, operand2);
               if( operand1.sup >= infinity )
               {
                  resultant->sup = infinity;
               }
               else
               {
                  setRoundingMode(SCIP_ROUND_UPWARDS);
                  resultant->sup = pow(operand1.sup, operand2);
               }
            }
         }
         else
         {  /* n is odd */
            if( operand1.inf <= -infinity )
            {
               resultant->inf = -infinity;
            }
            else
            {
               setRoundingMode(SCIP_ROUND_DOWNWARDS);
               resultant->inf = pow(operand1.inf, operand2);
            }
            if( operand1.sup >= infinity )
            {
               resultant->sup =  infinity;
            }
            else
            {
               setRoundingMode(SCIP_ROUND_UPWARDS);
               resultant->sup = pow(operand1.sup, operand2);
            }
         }
      }
   }
  
   setRoundingMode(roundmode);
}

/** stores operand1 to the signed power of the scalar positive operand2 in resultant 
 * 
 * the signed power of x w.r.t. an exponent n >= 0 is given as sign(x) * abs(x)^n
 * */
extern
void SCIPintervalSignPowerScalar(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_Real             operand2            /**< second operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand1.inf <  infinity);
   assert(operand1.sup > -infinity);
   assert(operand2     <  infinity);
   assert(operand2     >= 0.0);
   
   if( operand2 == 0.0 )
   { /* special case, since x^0 = 1 for x != 0, but 0^0 = 0 */
      if( operand1.inf < 0.0 && operand1.sup > 0.0 )
      { /* 0.0 inside the interval gives [-1,1] */
         resultant->inf = -1.0;
         resultant->sup =  1.0;
      }
      else if( operand1.inf < 0.0 && operand1.sup == 0.0 )
      { /* 0.0 as right bound of interval gives [-1,0] */ 
         resultant->inf = -1.0;
         resultant->sup =  0.0;
      }
      else if( operand1.inf == 0.0 && operand1.sup > 0.0 )
      { /* 0.0 as left bound of interval gives [0,1] */
         resultant->inf =  0.0;
         resultant->sup =  1.0;
      }
      else if( operand1.inf == 0.0 && operand1.sup == 0.0 )
      { /* 0.0^0.0 gives 0 */
         resultant->inf =  0.0;
         resultant->sup =  0.0;
      }
      else if( operand1.inf > 0.0 )
      { /* interval right of 0.0 gives [1,1] */
         resultant->inf =  1.0;
         resultant->sup =  1.0;
      }
      else
      { /* last case left: interval is left of 0.0, which gives [-1,-1] */
         assert(operand1.sup < 0.0);
         resultant->inf = -1.0;
         resultant->sup = -1.0;
      }
      
      return;
   }

   if( operand2 == 1.0 )
   { /* easy case that should run fast */
      *resultant = operand1;
      return;
   }

   roundmode = getRoundingMode();

   if( operand2 == 2.0 )
   { /* common case where pow can easily be avoided */
      if( operand1.inf <= -infinity )
      {
         resultant->inf = -infinity;
      }
      else if( operand1.inf > 0.0 )
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf =  operand1.inf * operand1.inf;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS); /* need upwards since we negate result of multiplication! */
         resultant->inf = -operand1.inf * operand1.inf;
      }

      if( operand1.sup >=  infinity )
      {
         resultant->sup =  infinity;
      }
      else if( operand1.sup > 0.0 )
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup =  operand1.sup * operand1.sup;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS); /* need downwards since we negate result of multiplication! */
         resultant->sup = -operand1.sup * operand1.sup;
      }
   }
   else if( operand2 == 0.5 )
   { /* another common case where pow can easily be avoided */
      if( operand1.inf <= -infinity )
      {
         resultant->inf = -infinity;
      }
      else if( operand1.inf >= 0.0 )
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf =  sqrt(operand1.inf);
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS); /* need upwards since we negate result of multiplication! */
         resultant->inf = -sqrt(-operand1.inf);
      }

      if( operand1.sup >=  infinity )
      {
         resultant->sup =  infinity;
      }
      else if( operand1.sup > 0.0 )
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup =  sqrt(operand1.sup);
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS); /* need downwards since we negate result of multiplication! */
         resultant->sup = -sqrt(-operand1.sup);
      }
   }
   else
   {
      if( !warned_unsafe_pow )
      {
         SCIPwarningMessage("Warning: interval arithmetic for pow function is not rounding safe!\n");
         warned_unsafe_pow = TRUE;
      }
      if( operand1.inf <= -infinity )
      {
         resultant->inf = -infinity;
      }
      else if( operand1.inf > 0.0 )
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf =  pow( operand1.inf, operand2);
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS); /* need upwards since we negate result of pow! */
         resultant->inf = -pow(-operand1.inf, operand2);
      }

      if( operand1.sup >=  infinity )
      {
         resultant->sup =  infinity;
      }
      else if( operand1.sup > 0.0 )
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup =  pow( operand1.sup, operand2);
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS); /* need downwards since we negate result of pow! */
         resultant->sup = -pow(-operand1.sup, operand2);
      }
   }
   
   setRoundingMode(roundmode);
}

/** computes the reciprocal of an interval
 *
 * if operand is 0.0, gives empty interval */
extern
void SCIPintervalReciprocal(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand.inf <= operand.sup);
   assert(operand.inf <  infinity);
   assert(operand.sup > -infinity);

   if( operand.inf == 0.0 && operand.sup == 0.0 )
   { /* 1/0 = empty */
      resultant->inf =  infinity;
      resultant->sup = -infinity;
      return;
   }

   roundmode = getRoundingMode();

   if( operand.inf >= 0.0 )
   {  /* 1/x with x >= 0 */
      if( operand.sup >= infinity )
      {
         resultant->inf = 0.0;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = 1.0 / operand.sup;
      }

      if( operand.inf == 0.0 )
      {
         resultant->sup = infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = 1.0 / operand.inf;
      }

      setRoundingMode(roundmode);
   }
   else if( operand.sup <= 0.0 )
   {  /* 1/x with x <= 0 */
      if( operand.sup == 0.0 )
      {
         resultant->inf = -infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         resultant->inf = 1.0 / operand.sup;
      }

      if( operand.inf <= -infinity )
      {
         resultant->sup = infinity;
      }
      else
      {
         setRoundingMode(SCIP_ROUND_UPWARDS);
         resultant->sup = 1.0 / operand.inf;
      }
      setRoundingMode(roundmode);
   }
   else
   {  /* 1/x with x in [-,+] is division by zero */
      resultant->inf = -infinity;
      resultant->sup =  infinity;
   }
}

/** stores exponential of operand in resultant */
void SCIPintervalExp(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand.inf <= operand.sup);
   assert(operand.inf <  infinity);
   assert(operand.sup > -infinity);
  
   roundmode = getRoundingMode();

   if( !warned_unsafe_exp )
   {
      SCIPwarningMessage("Warning: interval arithmetic for exp function is not rounding safe!\n");
      warned_unsafe_exp = TRUE;
   }

   if( operand.inf <= -infinity )
   {
      resultant->inf = 0.0;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = exp(operand.inf);
   }
  
   if( operand.sup >=  infinity )
   {
      resultant->sup = infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = exp(operand.sup);
   }

   setRoundingMode(roundmode);
}

/** stores natural logarithm of operand in resultant */
void SCIPintervalLog(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   ROUNDMODE roundmode;

   assert(resultant != NULL);
   assert(operand.inf <= operand.sup);
   assert(operand.inf <  infinity);
   assert(operand.sup > -infinity);
  
   if( operand.sup <= 0.0 )
   {
      SCIPintervalSetEmpty(infinity, resultant);
      return;
   }
  
   roundmode = getRoundingMode();

   if( !warned_unsafe_log )
   {
      SCIPwarningMessage("Warning: interval arithmetic for log function is not rounding safe!\n");
      warned_unsafe_log = TRUE;
   }
  
   if( operand.inf <= 0.0 )
   {
      resultant->inf = -infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_DOWNWARDS);
      resultant->inf = log(operand.inf);
   }

   if( operand.sup >= infinity )
   {
      resultant->sup = infinity;
   }
   else
   {
      setRoundingMode(SCIP_ROUND_UPWARDS);
      resultant->sup = log(operand.sup);
   }

   setRoundingMode(roundmode);
}

/** stores minimum of operands in resultant */
void SCIPintervalMin(
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);

   resultant->inf = MIN(operand1.inf, operand2.inf);
   resultant->sup = MIN(operand1.sup, operand2.sup);
}

/** stores maximum of operands in resultant */
void SCIPintervalMax(
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand1,           /**< first operand of operation */
   SCIP_INTERVAL         operand2            /**< second operand of operation */
   )
{
   assert(resultant != NULL);
   assert(operand1.inf <= operand1.sup);
   assert(operand2.inf <= operand2.sup);

   resultant->inf = MAX(operand1.inf, operand2.inf);
   resultant->sup = MAX(operand1.sup, operand2.sup);
}

/** stores absolute value of operand in resultant */
void SCIPintervalAbs(
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   assert(resultant != NULL);
   assert(operand.inf <= operand.sup);
   
   if( operand.inf <= 0.0 && operand.sup >= 0.0)
   {
      resultant->inf = 0.0;
      resultant->sup = MAX(-operand.inf, operand.sup);
   }
   else if( operand.inf > 0.0 )
   {
      *resultant = operand;
   }
   else
   {
      resultant->inf = -operand.sup;
      resultant->sup = -operand.inf;
   }
}

/** stores sign of operand in resultant */
void SCIPintervalSign(
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_INTERVAL         operand             /**< operand of operation */
   )
{
   assert(resultant != NULL);
   assert(operand.inf <= operand.sup);

   if( operand.sup < 0.0 )
   {
      resultant->inf = -1.0;
      resultant->sup = -1.0;
   }
   else if( operand.inf >= 0.0 )
   {
      resultant->inf =  1.0;
      resultant->sup =  1.0;
   }
   else
   {
      resultant->inf = -1.0;
      resultant->sup =  1.0;
   }
}

/** computes exact upper bound on \f$ a x^2 + b x \f$ for x in [xlb, xub], b an interval, and a scalar
 * 
 * Uses Algorithm 2.2 from Domes and Neumaier: Constraint propagation on quadratic constraints (2008) */
SCIP_Real SCIPintervalQuadUpperBound(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_Real             a,                  /**< coefficient of x^2 */
   SCIP_INTERVAL         b_,                 /**< coefficient of x */
   SCIP_INTERVAL         x                   /**< range of x */
   )
{
   SCIP_Real b;
   SCIP_Real u;
   
   assert(!SCIPintervalIsEmpty(x));
   assert(b_.inf <  infinity);
   assert(b_.sup > -infinity);
   assert( x.inf <  infinity);
   assert( x.sup > -infinity);

   if( x.sup <= 0.0 )
   { /* change sign of x: enclose a*x^2 + [-bub, -blb]*(-x) for (-x) in [-xub, -xlb] */
      u = x.sup;
      x.sup = -x.inf;
      x.inf = -u;
      b = -b_.inf;
   }
   else
   {
      b = b_.sup;
   }
  
   if( x.inf >= 0.0 )
   {  /* enclose a*x^2 + b*x */
      ROUNDMODE roundmode;
      if( b >= infinity )
         return infinity;
    
      roundmode = getRoundingMode();
      setRoundingMode(SCIP_ROUND_UPWARDS);
    
      if( a == 0.0 )
      {
         u = MAX(x.inf * b, x.sup * b);
      }
      else
      {
         SCIP_Real s,t;
         u = MAX(x.inf * (a*x.inf + b), x.sup * (a*x.sup + b));
         s = b/2;
         t = s/(-a);
         if( t > x.inf && (-2*a)*x.sup > b && s*t > u )
            u = s*t;
      }
      
      setRoundingMode(roundmode);
      return u;
   }
   else
   {
      SCIP_INTERVAL xlow = x;
      SCIP_Real cand1;
      SCIP_Real cand2;
      assert(x.inf < 0.0 && x.sup > 0);

      xlow.sup = 0;  /* so xlow is lower part of interval */ 
      x.inf = 0;     /* so x    is upper part of interval now */
      cand1 = SCIPintervalQuadUpperBound(infinity, a, b_, xlow);
      cand2 = SCIPintervalQuadUpperBound(infinity, a, b_, x);
      return MAX(cand1, cand2);
   }
}

/** stores range of quadratic term in resultant
 * 
 * given scalar a and intervals b and x, computes interval for \f$ a x^2 + b x \f$ */
void SCIPintervalQuad(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_Real             sqrcoeff,           /**< coefficient of x^2 */
   SCIP_INTERVAL         lincoeff,           /**< coefficient of x */
   SCIP_INTERVAL         xrng                /**< range of x */
   )
{
   SCIP_Real tmp;

   if( SCIPintervalIsEmpty(xrng) )
   {
      SCIPintervalSetEmpty(infinity, resultant);
      return;
   }
   if( sqrcoeff == 0.0 )
   {
      SCIPintervalMul(infinity, resultant, lincoeff, xrng);
      return;
   }

   resultant->sup =  SCIPintervalQuadUpperBound(infinity,  sqrcoeff, lincoeff, xrng);
  
   tmp = lincoeff.inf;
   lincoeff.inf = -lincoeff.sup;
   lincoeff.sup = -tmp;
   resultant->inf = -SCIPintervalQuadUpperBound(infinity, -sqrcoeff, lincoeff, xrng);
   
   assert(resultant->sup >= resultant->inf);
#if 0
   if( resultant->sup < resultant->inf )
   { /* in case upper bound is below -infinity */
      assert(resultant->inf <= -infinity);
      resultant->sup = resultant->inf;
   }
   else if( resultant->inf > resultant->sup )
   { /* in case lower bound is above  infinity */
      assert(resultant->sup >= infinity);
      resultant->inf = resultant->sup;
   }
#endif
}

/** solves a quadratic equation with interval linear and constant coefficients
 * 
 * Given a scalar a and intervals b and c, this function computes an interval that contains all positive solutions of \f$ a x^2 + b x \geq c\f$. */
void SCIPintervalSolveUnivariateQuadExpressionPositive(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_Real             sqrcoeff,           /**< coefficient of x^2 */
   SCIP_INTERVAL         lincoeff,           /**< coefficient of x */
   SCIP_INTERVAL         rhs                 /**< right hand side of equation */
)
{
   assert(resultant != NULL);
  
   /* find x>=0 s.t. ax^2 + b.inf x <= c.sup  -> -ax^2 - b.inf x >= -c.sup */
   if( lincoeff.inf <= -infinity || rhs.sup >= infinity )
   {
      resultant->inf = 0.0;
      resultant->sup = infinity;
   }
   else
   {
      SCIPintervalSolveUnivariateQuadExpressionPositiveAllScalar(infinity, resultant, -sqrcoeff, -lincoeff.inf, -rhs.sup);
      SCIPdebugMessage("solve %g*x^2 + %g*x >= %g gives [%.20f, %.20f]\n", -sqrcoeff, -lincoeff.inf, -rhs.sup, resultant->inf, resultant->sup);
   }
   
   /* find x>=0 s.t. ax^2 + b.sup x >= c.inf */
   if( lincoeff.sup <  infinity && rhs.inf >  -infinity )
   {
      SCIP_INTERVAL res2;
      SCIPintervalSolveUnivariateQuadExpressionPositiveAllScalar(infinity, &res2, sqrcoeff, lincoeff.sup, rhs.inf);
      SCIPdebugMessage("solve %g*x^2 + %g*x >= %g gives [%.20f, %.20f]\n", sqrcoeff, lincoeff.sup, rhs.inf, res2.inf, res2.sup);
      SCIPdebugMessage("intersect [%.20f, %.20f] and [%.20f, %.20f]", resultant->inf, resultant->sup, res2.inf, res2.sup);
      /* intersect both results */
      SCIPintervalIntersect(resultant, *resultant, res2);
      SCIPdebugPrintf(" gives [%.20f, %.20f]\n", resultant->inf, resultant->sup);
   }
   /* else res2 = [0, infty] */
   
   if( resultant->inf >= infinity || resultant->sup <= -infinity )
   {
      SCIPintervalSetEmpty(infinity, resultant);
   }
}

/** solves a quadratic equation with linear and constant coefficients
 * 
 * Given scalar a, b, and c, this function computes an interval that contains all positive solutions of \f$ a x^2 + b x \geq c\f$.
 * Implements Algorithm 3.2 from Domes and Neumaier: Constraint propagation on quadratic constraints (2008). */
void SCIPintervalSolveUnivariateQuadExpressionPositiveAllScalar(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_Real             sqrcoeff,           /**< coefficient of x^2 */
   SCIP_Real             lincoeff,           /**< coefficient of x */
   SCIP_Real             rhs                 /**< right hand side of equation */
)
{
   ROUNDMODE     roundmode;
   SCIP_Real     b;
   SCIP_Real     delta;
   SCIP_Real     z;
   
   assert(resultant != NULL);
   assert(sqrcoeff <  infinity);
   assert(sqrcoeff > -infinity);
  
   resultant->inf = 0.0;
   resultant->sup = infinity;
   
   roundmode = getRoundingMode();

   setRoundingMode(SCIP_ROUND_UPWARDS);
   b = lincoeff / 2.;

   if( lincoeff >= 0.0 )
   { /* b >= 0.0 */
      /*setRoundingMode(SCIP_ROUND_UPWARDS); */
      if( rhs > 0.0 )
      { /* b >= 0.0 and c > 0.0 */
         delta = b*b + sqrcoeff*rhs;
         if( delta < 0.0 || (sqrcoeff == 0.0 && lincoeff == 0.0) )
         {
            SCIPintervalSetEmpty(infinity, resultant);
         }
         else
         {
            z = b + sqrt(delta);
            resultant->inf = -((-rhs)/z);
            if( sqrcoeff < 0.0 )
               resultant->sup = z / (-sqrcoeff);
         }
      }
      else
      { /* b >= 0.0 and c <= 0.0 */
         if( sqrcoeff < 0.0 )
         {
            delta = b*b + sqrcoeff*rhs;
            z = b + sqrt(delta);
            resultant->sup = z / (-sqrcoeff);
         }
      }
   }
   else
   { /* b < 0.0 */
      if( rhs > 0.0 )
      { /* b < 0.0 and c > 0.0 */
         setRoundingMode(SCIP_ROUND_DOWNWARDS);
         if( sqrcoeff > 0.0 )
         {
            delta = b*b + sqrcoeff*rhs;
            z = -b + sqrt(delta);
            resultant->inf = z / sqrcoeff;
         }
         else
         {
            SCIPintervalSetEmpty(infinity, resultant);
         }
      }
      else
      { /* b < 0.0 and c <= 0.0 */
         /* setRoundingMode(SCIP_ROUND_DOWNWARDS); */
         delta = b*b + sqrcoeff * rhs;
         if( delta >= 0.0 && sqrcoeff <= 0.0 )
         {
            z = -b + sqrt(delta);
            resultant->sup = -(rhs/z);
         }
/* @TODO actually we could generate a hole here
         if( delta >= 0.0 )
         {
            z = -b + sqrt(delta);
            resultant->sup = -(c/z);
            if( sqrcoeff > 0.0 )
               x2->inf = z/a;
         } */
      }
   }
   
   setRoundingMode(roundmode);
}

/** solves a quadratic equation with interval linear and constant coefficients
 * 
 * Given a scalar a and intervals b and c, this function computes an interval that contains all solutions of \f$ a x^2 + b x \in c\f$ */
void SCIPintervalSolveUnivariateQuadExpression(
   SCIP_Real             infinity,           /**< value for infinity */
   SCIP_INTERVAL*        resultant,          /**< resultant interval of operation */
   SCIP_Real             sqrcoeff,           /**< coefficient of x^2 */
   SCIP_INTERVAL         lincoeff,           /**< coefficient of x */
   SCIP_INTERVAL         rhs                 /**< right hand side of equation */
)
{
   SCIP_Real tmp;
   
   assert(resultant != NULL);
   
   if( sqrcoeff == 0.0 )
   { /* relatively easy case: x \in rhs / lincoeff */
      if( lincoeff.inf == 0.0 && lincoeff.sup == 0.0 )
      { /* equation became 0.0 \in rhs */
         if( rhs.inf <= 0.0 && rhs.sup >= 0.0 )
            SCIPintervalSetEntire(infinity, resultant);
         else
            SCIPintervalSetEmpty(infinity, resultant);
      }
      else
         SCIPintervalDiv(infinity, resultant, rhs, lincoeff);
      SCIPdebugMessage("  solving [%g,%g]*x in [%g,%g] gives [%g,%g]\n", SCIPintervalGetInf(lincoeff), SCIPintervalGetSup(lincoeff), SCIPintervalGetInf(rhs), SCIPintervalGetSup(rhs), SCIPintervalGetInf(*resultant), SCIPintervalGetSup(*resultant));
      return;
   }
   
   if( lincoeff.inf == 0.0 && lincoeff.sup == 0.0 )
   { /* easy case: x \in +/- sqrt(rhs/a) */
      SCIPintervalDivScalar(infinity, resultant, rhs, sqrcoeff);
      SCIPintervalSquareRoot(infinity, resultant, *resultant);
      resultant->inf = -resultant->sup;
      return;
   }

   SCIPintervalSolveUnivariateQuadExpressionPositive(infinity, resultant, sqrcoeff, lincoeff, rhs);
   SCIPdebugMessage("  positive solving %g*x^2 + [%g,%g]*x in [%g,%g] gives [%g,%g]\n", sqrcoeff, SCIPintervalGetInf(lincoeff), SCIPintervalGetSup(lincoeff), SCIPintervalGetInf(rhs), SCIPintervalGetSup(rhs), SCIPintervalGetInf(*resultant), SCIPintervalGetSup(*resultant));

   tmp = lincoeff.inf;
   lincoeff.inf = -lincoeff.sup;
   lincoeff.sup = -tmp;
   
   /* use lincoeff to store result of negated expression */
   SCIPdebugMessage("  positive solving %g*x^2 + [%g,%g]*x in [%g,%g] gives", sqrcoeff, SCIPintervalGetInf(lincoeff), SCIPintervalGetSup(lincoeff), SCIPintervalGetInf(rhs), SCIPintervalGetSup(rhs));
   SCIPintervalSolveUnivariateQuadExpressionPositive(infinity, &lincoeff, sqrcoeff, lincoeff, rhs);
   SCIPdebugMessage(" [%g,%g]\n", SCIPintervalGetInf(lincoeff), SCIPintervalGetSup(lincoeff));
   if( !SCIPintervalIsEmpty(lincoeff) )
   {
      if( !SCIPintervalIsEmpty(*resultant) )
      {
         tmp = lincoeff.sup;
         lincoeff.inf = -lincoeff.sup;
         lincoeff.sup = -tmp;
         SCIPintervalUnify(resultant, lincoeff, *resultant);
         SCIPdebugMessage("  unify gives [%g,%g]\n", SCIPintervalGetInf(*resultant), SCIPintervalGetSup(*resultant));
      }
      else
      {
         resultant->inf = -lincoeff.sup;
         resultant->sup = -lincoeff.inf;
      }
   }
}

/** returns infimum of interval */
SCIP_Real SCIPintervalGetInf(
   SCIP_INTERVAL         interval            /**< interval */
   )
{
   return interval.inf;
}

/** returns supremum of interval */
SCIP_Real SCIPintervalGetSup(
   SCIP_INTERVAL         interval            /**< interval */
   )
{
   return interval.sup;
}
