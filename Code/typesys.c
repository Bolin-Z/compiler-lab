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
    BasicTypeInt->_basic = INT;
    BasicTypeFloat = CreatTypeDescriptor();
    BasicTypeFloat->typeform = BASIC;
    BasicTypeFloat->_basic = FLOAT;
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
    if(s){
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
}

TypeDescriptor * CopyTypeDescriptor(TypeDescriptor * src, TypeDescriptor * dst){
    if(src){
        DestoryTypeDescriptor(dst);
        dst = CreatTypeDescriptor();
        if(dst){
            switch(src->typeform){
                case BASIC :
                    dst->_basic = src->_basic;
                    break;
                case ARRAY :
                    
                case ERROR :
                default :
                    break;
            }
            dst->typeform = src->typeform;
        }
    }
    return dst;
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
    TypeDescriptor * pre = NULL;
    bool mallocerror = false;
    va_list ap;
    va_start(ap,dimension);
    for(int i = 1;i <= dimension;i++){
        TypeDescriptor * t = CreatTypeDescriptor();
        if(t == NULL){
            mallocerror = true;
            break;
        }
        t->typeform = ARRAY;
        t->_array.elem = (i == dimension) ? (basetype) : (NULL);
        t->_array.size = va_arg(ap,int);
        if(pre) pre->_array.elem = t;
        else _array_ = t;
        pre = t;
    }
    va_end(ap);
    if(mallocerror){
        while( _array_ != NULL){
            TypeDescriptor * nxt = _array_->_array.elem;
            DestoryTypeDescriptor(_array_);
            _array_ = nxt;
        }
    }
    return _array_;
}

TypeDescriptor * CreatStructureAtOnce(int fieldscnt, ...){
    FieldList * _head_ = NULL;
    FieldList * pre = NULL;
    bool mallocerror = false;
    va_list ap;
    va_start(ap,fieldscnt);
    for(int i = 1;i <= fieldscnt;i++){
        char * _name_ = va_arg(ap,char*);
        TypeDescriptor * _type_ = va_arg(ap,TypeDescriptor*); 
        FieldList * t = CreatFieldList(_name_,_type_,NULL);
        if(t == NULL){
            mallocerror = true;
            break;
        }
        if(pre) pre->nextfield = t;
        else _head_ = t;
        pre = t;
    }
    va_end(ap);
    TypeDescriptor * _structure_ = CreatTypeDescriptor();
    if(_structure_){
        _structure_->typeform = STRUCTURE;
        _structure_->_structure = _head_;
    }else{
        mallocerror = true;
    }
    if(mallocerror){
        while( _head_ != NULL){
            FieldList * nxt = _head_->nextfield;
            free(_head_);
            _head_ = nxt;
        }
    }
    return _structure_;
}

bool IsEqualType(TypeDescriptor * a,TypeDescriptor * b){
    if(a->typeform == b->typeform){
        switch(a->typeform){
            case BASIC : 
                return (a->_basic == b->_basic);
            case ARRAY :
                return IsEqualType(a->_array.elem,b->_array.elem);
            case STRUCTURE :
                FieldList * ptra = a->_structure;
                FieldList * ptrb = b->_structure;
                while((ptra!=NULL)&&(ptrb!=NULL)){
                    if(!IsEqualType(ptra->fieldtype,ptrb->fieldtype)) break;
                    ptra = ptra->nextfield;
                    ptrb = ptrb->nextfield;
                }
                return ((ptra == NULL)&&(ptrb == NULL));
            case ERROR :
            default :
                break;
        }
    }
    return false;
}
