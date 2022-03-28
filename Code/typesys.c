#include "typesys.h"

TypeDescriptor * BasicTypeInt;
TypeDescriptor * BasicTypeFloat;
TypeDescriptor * BasicTypeError;

TypeDescriptor * BasicInt(){return BasicTypeInt;}
TypeDescriptor * BasicFloat(){return BasicTypeFloat;}
TypeDescriptor * BasicError(){return BasicTypeError;}

void CreatTypeSystem(){
    BasicTypeInt = CreatTypeDescriptor();
    BasicTypeInt->typeform = BASIC;
    BasicTypeInt->basic = INT;
    BasicTypeFloat = CreatTypeDescriptor();
    BasicTypeFloat->typeform = BASIC;
    BasicTypeFloat->basic = FLOAT;
    BasicTypeError = CreatTypeDescriptor();
    BasicTypeError->typeform = ERROR;
}

void DestoryTypeSystem(){
    free(BasicTypeInt);
    free(BasicTypeFloat);
    free(BasicTypeError);
}

TypeDescriptor * CreatTypeDescriptor(){
    return (TypeDescriptor * )malloc(sizeof(TypeDescriptor));
}

void DestoryTypeDescriptor(TypeDescriptor * s){
    switch(s->typeform){
        case BASIC : /* Do nothing */
        case ERROR : /* Do nothing */
            break;
        case ARRAY :
            DestoryTypeDescriptor(s->_array.elem);
            free(s);
            break;
        case STRUCTURE :
            FieldList * cur = s->_structure;
            while(cur != NULL){
                FieldList * nxt = cur->nextfield;
                DestoryTypeDescriptor(cur->fieldtype);
                free(cur);
                cur = nxt;
            }
            free(s);
            break;
        default :
            break;
    }
}

TypeDescriptor * CreatArrayDescriptor(TypeDescriptor * arraytype, int arraysize){
    TypeDescriptor * a = CreatTypeDescriptor();
    if(a){
        a->typeform = ARRAY;
        a->_array.elem = arraytype;
        a->_array.size = arraysize;
    }
    return a;
}

TypeDescriptor * CreatStructureDescriptor(FieldList * fields){
    TypeDescriptor * s = CreatTypeDescriptor();
    if(s){
        s->typeform = STRUCTURE;
        s->_structure = fields;
    }
    return s;
}

FieldList * CreatFieldList(char * fieldname, TypeDescriptor * fieldtype, FieldList * next){
    FieldList * head = (FieldList *)malloc(sizeof(FieldList));
    if(head){
        head->fieldname = fieldname;
        head->fieldtype = fieldtype;
        head->nextfield = next;
    }
    return head;
}

TypeDescriptor * CreatArrayAtOnce(TypeDescriptor * basetype, int dimension, ...){
    TypeDescriptor * _array_ = NULL;
    TypeDescriptor * nxt_dim = NULL;
    bool mallocerror = false;
    va_list ap;
    va_start(ap,dimension);
    for(int i = 0;i < dimension;i++){
        TypeDescriptor * t = CreatTypeDescriptor();
        if(t == NULL){
            mallocerror = true;
            break;
        }
        if(i == 0) _array_ = t;
        
    }
    va_end(ap);
    if(mallocerror){
        while(_array_ != NULL){
            nxt_dim = _array_->_array.elem;
            DestoryTypeDescriptor(_array_);
            _array_ = nxt_dim;
        }
    }
    return _array_;
}