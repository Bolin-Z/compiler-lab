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
        enum {INT,FLOAT} _basic;
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
/* 
    TypeDescriptor * CreatArrayAtOnce(TypeDescriptor * basetype, int dimension, ...);
    Return an array type descriptor on success, NULL if failed.
    basetype:  basic element type of array
    dimension: number of dimensions
    ... :      list of sizes of each dimension
               format: size1, size2, size3, ...
*/
TypeDescriptor * CreatArrayAtOnce(TypeDescriptor * basetype, int dimension, ...);
TypeDescriptor * CreatStructureDescriptor(FieldList * fields);
/*
    TypeDescriptor * CreatStructureAtOnce(int fieldscnt,...); 
    Return an structure type descriptor on success, NULL if failed.
    fieldscnt : number of fields
    ... : list of field information of each field
          format: field1_name, field1_type, field2_name, field2_type, ...
*/
TypeDescriptor * CreatStructureAtOnce(int fieldscnt,...);
FieldList * CreatFieldList(char * fieldname, TypeDescriptor * fieldtype, FieldList * next);

TypeDescriptor * BasicInt();
TypeDescriptor * BasicFloat();
TypeDescriptor * BasicError();

bool IsEqualType(TypeDescriptor * a,TypeDescriptor * b);

#endif  