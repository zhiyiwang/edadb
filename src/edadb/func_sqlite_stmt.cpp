/***
 * @file func_sqlite_stmt.cpp
 * @brief test the sqlite3 interface statement
 */


#include <sqlite3.h>
#include <iostream>

// check the size of sqlite3_stmt:
//   each sqlite3_stmt is a pointer to an instance of the Vdbe structure
//   the size of Vdbe is 256 bytes
//   the source code of Vdbe is as following (in sqlite3.c)
// 
//    /*
//    ** An instance of the virtual machine.  This structure contains the complete
//    ** state of the virtual machine.
//    **
//    ** The "sqlite3_stmt" structure pointer that is returned by sqlite3_prepare()
//    ** is really a pointer to an instance of this structure.
//    */
//    struct Vdbe {
//        sqlite3 *db;            /* The database connection that owns this statement */
//        Vdbe **ppVPrev,*pVNext; /* Linked list of VDBEs with the same Vdbe.db */
//        Parse *pParse;          /* Parsing context used to create this Vdbe */
//        ynVar nVar;             /* Number of entries in aVar[] */
//        int nMem;               /* Number of memory locations currently allocated */
//        int nCursor;            /* Number of slots in apCsr[] */
//        u32 cacheCtr;           /* VdbeCursor row cache generation counter */
//        int pc;                 /* The program counter */
//        int rc;                 /* Value to return */
//        i64 nChange;            /* Number of db changes made since last reset */
//        int iStatement;         /* Statement number (or 0 if has no opened stmt) */
//        i64 iCurrentTime;       /* Value of julianday('now') for this statement */
//        i64 nFkConstraint;      /* Number of imm. FK constraints this VM */
//        i64 nStmtDefCons;       /* Number of def. constraints when stmt started */
//        i64 nStmtDefImmCons;    /* Number of def. imm constraints when stmt started */
//        Mem *aMem;              /* The memory locations */
//        Mem **apArg;            /* Arguments xUpdate and xFilter vtab methods */
//        VdbeCursor **apCsr;     /* One element of this array for each open cursor */
//        Mem *aVar;              /* Values for the OP_Variable opcode. */
//      
//        /* When allocating a new Vdbe object, all of the fields below should be
//        ** initialized to zero or NULL */
//      
//        Op *aOp;                /* Space to hold the virtual machine's program */
//        int nOp;                /* Number of instructions in the program */
//        int nOpAlloc;           /* Slots allocated for aOp[] */
//        Mem *aColName;          /* Column names to return */
//        Mem *pResultRow;        /* Current output row */
//        char *zErrMsg;          /* Error message written here */
//        VList *pVList;          /* Name of variables */
//      #ifndef SQLITE_OMIT_TRACE
//        i64 startTime;          /* Time when query started - used for profiling */
//      #endif
//      #ifdef SQLITE_DEBUG
//        int rcApp;              /* errcode set by sqlite3_result_error_code() */
//        u32 nWrite;             /* Number of write operations that have occurred */
//        int napArg;             /* Size of the apArg[] array */
//      #endif
//        u16 nResColumn;         /* Number of columns in one row of the result set */
//        u16 nResAlloc;          /* Column slots allocated to aColName[] */
//        u8 errorAction;         /* Recovery action to do in case of an error */
//        u8 minWriteFileFormat;  /* Minimum file format for writable database files */
//        u8 prepFlags;           /* SQLITE_PREPARE_* flags */
//        u8 eVdbeState;          /* On of the VDBE_*_STATE values */
//        bft expired:2;          /* 1: recompile VM immediately  2: when convenient */
//        bft explain:2;          /* 0: normal, 1: EXPLAIN, 2: EXPLAIN QUERY PLAN */
//        bft changeCntOn:1;      /* True to update the change-counter */
//        bft usesStmtJournal:1;  /* True if uses a statement journal */
//        bft readOnly:1;         /* True for statements that do not write */
//        bft bIsReader:1;        /* True for statements that read */
//        bft haveEqpOps:1;       /* Bytecode supports EXPLAIN QUERY PLAN */
//        yDbMask btreeMask;      /* Bitmask of db->aDb[] entries referenced */
//        yDbMask lockMask;       /* Subset of btreeMask that requires a lock */
//        u32 aCounter[9];        /* Counters used by sqlite3_stmt_status() */
//        char *zSql;             /* Text of the SQL statement that generated this */
//      #ifdef SQLITE_ENABLE_NORMALIZE
//        char *zNormSql;         /* Normalization of the associated SQL statement */
//        DblquoteStr *pDblStr;   /* List of double-quoted string literals */
//      #endif
//        void *pFree;            /* Free this when deleting the vdbe */
//        VdbeFrame *pFrame;      /* Parent frame */
//        VdbeFrame *pDelFrame;   /* List of frame objects to free on VM reset */
//        int nFrame;             /* Number of frames in pFrame list */
//        u32 expmask;            /* Binding to these vars invalidates VM */
//        SubProgram *pProgram;   /* Linked list of all sub-programs used by VM */
//        AuxData *pAuxData;      /* Linked list of auxdata allocations */
//      #ifdef SQLITE_ENABLE_STMT_SCANSTATUS
//        int nScan;              /* Entries in aScan[] */
//        ScanStatus *aScan;      /* Scan definitions for sqlite3_stmt_scanstatus() */
//      #endif
//      };

int test_sqlite_stmt_wo_finalize() {
    sqlite3* db;
    sqlite3_open("test_stmt_wo_final.db", &db);

    const char* create_table_sql = "CREATE TABLE IF NOT EXISTS test (id INTEGER PRIMARY KEY, value TEXT);";
    sqlite3_exec(db, create_table_sql, 0, 0, 0);

    const char* insert_sql = "INSERT INTO test (id, value) VALUES (1, 'Hello, World!');";
    sqlite3_exec(db, insert_sql, 0, 0, 0);


    sqlite3_stmt* stmt;
    const char* select_q1_sql = "SELECT id FROM test;";
    sqlite3_prepare_v2(db, select_q1_sql, -1, &stmt, 0);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Read id: " << sqlite3_column_int(stmt, 0) << std::endl;
    }
// without finalize
//    sqlite3_finalize(stmt);

    const char* select_q2_sql = "SELECT value FROM test;";
    sqlite3_prepare_v2(db, select_q2_sql, -1, &stmt, 0);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::cout << "Read value: " << sqlite3_column_text(stmt, 0) << std::endl;
    }
    sqlite3_finalize(stmt);

    sqlite3_close(db);

    return 0;
}



int test_sqlite_stmt(void) {

    test_sqlite_stmt_wo_finalize();

    return 0;
}