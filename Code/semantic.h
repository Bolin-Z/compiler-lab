#ifndef __SEMANTIC_H__
#define __SEMANTIC_H__

#include "defs.h"
#include "decls.h"
#include "cst.h"
#include "symtab.h"
#include "typesys.h"
#include "error.h"
#include "ir.h"

void SemanticAnalysis(struct CST_node* root, irSystem * irSys);


#endif