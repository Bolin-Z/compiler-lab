#ifndef __TYPESYS_H__
#define __TYPESYS_H__

#include<stdarg.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdbool.h>

typedef struct TypeDescriptor TypeDescriptor;
typedef struct FieldList FieldList;

/* Object that describe type. */
struct TypeDescriptor{
    enum {ERROR, BASIC, ARRAY, STRUCTURE} TypeClass;
    /* Width of type (byte) */
    int typeWidth;
    union{
        /* TypeClass == BASIC */
        enum {BASICINT,BASICFLOAT} Basic;
        /* TypeClass == ARRAY */
        struct {TypeDescriptor * elem; int size;} Array;
        /* TypeClass == STRUCTURE */
        FieldList * Structure;
        /* TypeClass == ERROR */
        /* empty */
    };
};

/* Auxiliary data structure to help describe Structure type. */
struct FieldList{
    char * FieldName; // points to the id field of cst_id_node
    TypeDescriptor * FieldType;
    FieldList * NextField;
};

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