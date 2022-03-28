#ifndef __TYPESYS_H__
#define __TYPESYS_H__

typedef struct TypeDescriptor{
    enum {BASIC, ARRAY, STRUCTURE, ERROR} typeform;
    union{
        /* typeform == BASIC */
        enum {INT,FLOAT} basic;
        /* typeform == ARRAY */
        struct {TypeDescriptor * elem; size_t size;} _array;
        /* typeform == STRUCTURE */
        FieldList * _structure;
        /* typeform == ERROR */
        /* empty */
    };
} TypeDescriptor;

typedef struct FieldList{
    char * fieldname;
    TypeDescriptor * fieldtype;
    FieldList * nextfield;
} FieldList;

TypeDescriptor * CreatTypeDescriptor();
void DestoryTypeDescriptor(TypeDescriptor *);

#endif