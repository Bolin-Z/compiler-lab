#ifndef __TYPESYS_H__
#define __TYPESYS_H__

#include<stdarg.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>

typedef struct TypeDescriptor{
    enum {ERROR, BASIC, ARRAY, STRUCTURE} typeform;
    union{
        /* typeform == BASIC */
        enum {INT,FLOAT} basic;
        /* typeform == ARRAY */
        struct {TypeDescriptor * elem; int size;} _array;
        /* typeform == STRUCTURE */
        FieldList * _structure;
        /* typeform == ERROR */
        /* empty */
    };
} TypeDescriptor;

typedef struct FieldList{
    char * fieldname; // point to the id field of cst_id_node
    TypeDescriptor * fieldtype;
    FieldList * nextfield;
} FieldList;

void CreatTypeSystem();
void DestoryTypeSystem();

TypeDescriptor * CreatTypeDescriptor();
void DestoryTypeDescriptor(TypeDescriptor *);

TypeDescriptor * CreatArrayDescriptor(TypeDescriptor * arraytype, int size);
TypeDescriptor * CreatArrayAtOnce(TypeDescriptor * basetype, int dimension, ...);
TypeDescriptor * CreatStructureDescriptor(FieldList * fields);
FieldList * CreatFieldList(char * fieldname, TypeDescriptor * fieldtype, FieldList * next);

TypeDescriptor * BasicInt();
TypeDescriptor * BasicFloat();
TypeDescriptor * BasicError();

#endif