#ifndef __TYPESYS_H__
#define __TYPESYS_H__

#include<stdarg.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>

/* Object that describe type. */
typedef struct TypeDescriptor{
    enum {ERROR, BASIC, ARRAY, STRUCTURE} TypeClass;
    union{
        /* TypeClass == BASIC */
        enum {INT,FLOAT} Basic;
        /* TypeClass == ARRAY */
        struct {TypeDescriptor * elem; int size;} Array;
        /* TypeClass == STRUCTURE */
        FieldList * Structure;
        /* TypeClass == ERROR */
        /* empty */
    };
} TypeDescriptor;

/* Auxiliary data structure to help describe Structure type. */
typedef struct FieldList{
    char * FieldName; // points to the id field of cst_id_node
    TypeDescriptor * FieldType;
    FieldList * NextField;
} FieldList;

void CreatTypeSystem();
void DestoryTypeSystem();

TypeDescriptor * CreatTypeDescriptor();
void DestoryTypeDescriptor(TypeDescriptor * s);
TypeDescriptor * CopyTypeDescriptor(TypeDescriptor * src);

FieldList * CreatField();
void DestoryFieldList(FieldList * head);
FieldList * CopyFieldList(FieldList * src);

TypeDescriptor * CreatArrayDescriptor(TypeDescriptor * arraytype, int size, bool copy);
TypeDescriptor * CreatStructureDescriptor(FieldList * fields, bool copy);

TypeDescriptor * BasicInt();
TypeDescriptor * BasicFloat();
TypeDescriptor * BasicError();

bool IsEqualType(TypeDescriptor * a,TypeDescriptor * b);
bool IsErrorType(TypeDescriptor * a);

#endif  