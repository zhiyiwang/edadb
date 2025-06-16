/**
 * @file DbMapAll.h
 * @brief DbMap.h provides a way to map objects to relations in the database.
 * @note We do have the following assumptions:
 *    1. ok = ok && func(); if ok = false, then func() will not be called.
 *    2. if class T want to persist vector<ElemT> member variable:
 *       a. ElemT could be a type T or a pointer to type T, no multi-level pointer allowed.
 *       b. ElemT must have define default/copy constructor and assignment operator.
 *       c. each ElemT object must have a unique primary key, even for different T object.
 */


#progma once

#include "DbMapOperation.h"
#include "DbMapBase.h"
#include "DbMap.h"
#include "DbMapDbStmtOp.h"
#include "DbMapWriter.h"
#include "DbMapReader.h"