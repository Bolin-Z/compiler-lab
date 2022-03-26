#ifndef __TYPESYS_H__
#define __TYPESYS_H__

typedef struct TypeDescriptor{
    enum {BASIC, ARRAY, STRUCTURE, ERROR} typeform;
    union{
        /* typeform == Basic */
        enum {INT,FLOAT} basic;
        /* typeform == array */
        struct {TypeDescriptor * elem; size_t size;} _array;
        /* typeform == structure */
        FieldList * _structure;
    };
} TypeDescriptor;

typedef struct FieldList{
    char * fieldname;
    TypeDescriptor * fieldtype;
    FieldList * nextfield;
} FieldList;

#endif