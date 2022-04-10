#include "typesys.h"

//   The Concept of Copy in Type System
// 
//   Not Copy: (Same for FieldLists *)                    
//                             +---+
//   TypeDescriptor * src -->  | A |  <-- TypeDescriptor * dst
//                             +---+
//
//   Copy: (Same for FieldLists *) 
//                             +---+                              +---+
//   TypeDescriptor * src -->  | A |    TypeDescriptor * dst -->  | B |
//                             +---+                              +---+


TypeDescriptor * BasicTypeInt;
TypeDescriptor * BasicTypeFloat;
TypeDescriptor * BasicTypeError;

TypeDescriptor * BasicInt(){return BasicTypeInt;}
TypeDescriptor * BasicFloat(){return BasicTypeFloat;}
TypeDescriptor * BasicError(){return BasicTypeError;}

/* Creat basic type objects. */
void CreatTypeSystem(){
    BasicTypeInt = CreatTypeDescriptor();
    BasicTypeInt->TypeClass = BASIC;
    BasicTypeInt->Basic = BASICINT;
    BasicTypeFloat = CreatTypeDescriptor();
    BasicTypeFloat->TypeClass = BASIC;
    BasicTypeFloat->Basic = BASICFLOAT;
    BasicTypeError = CreatTypeDescriptor();
    BasicTypeError->TypeClass = ERROR;
}

/* Destory basic type objects. */
void DestoryTypeSystem(){
    free(BasicTypeInt);
    free(BasicTypeFloat);
    free(BasicTypeError);
}

/* Allocate space for a TypeDescriptor object. */
TypeDescriptor * CreatTypeDescriptor(){
    return (TypeDescriptor * )malloc(sizeof(TypeDescriptor));
}

/* Relase memory of TypeDescriptor s except the basic TypeDescriptors */
void DestoryTypeDescriptor(TypeDescriptor * s){
    if(s){
        switch(s->TypeClass){
            case ARRAY :
                DestoryTypeDescriptor(s->Array.elem);
                free(s);
                break;
            case STRUCTURE :
                DestoryFieldList(s->Structure);
                free(s);
                break;
            case BASIC : /* Do nothing */
            case ERROR : /* Do nothing */
            default :
                break;
        }
    }
}

/*
Creat a TypeDescriptor object same as the one pointed to by src and  
return a pointer to it. If src points to a basic TypeDescriptor, just 
return a pointer to it. 
*/
TypeDescriptor * CopyTypeDescriptor(TypeDescriptor * src){
    TypeDescriptor * dst = NULL;
    if(src){
        switch(src->TypeClass){
            case ARRAY :
                dst = CreatTypeDescriptor();
                if(dst){
                    dst->TypeClass = ARRAY;
                    dst->Array.size = src->Array.size;
                    dst->Array.elem = CopyTypeDescriptor(src->Array.elem);
                    if(!dst->Array.elem){
                        DestoryTypeDescriptor(dst);
                        dst = NULL;
                    }
                }
                break;
            case STRUCTURE :
                dst = CreatTypeDescriptor();
                if(dst){
                    dst->TypeClass = STRUCTURE;
                    dst->Structure = CopyFieldList(src->Structure);
                    if(!dst->Structure){
                        DestoryTypeDescriptor(dst);
                        dst = NULL;
                    }
                }
                break;
            case BASIC :
            case ERROR :
                dst = src;
                break;
            default :
                break;
        }
    }
    return dst;
}

/* Allocate memory for a FieldList object. */
FieldList * CreatField(){
    FieldList * field = (FieldList * )malloc(sizeof(FieldList));
    if(field){
        field->FieldName = NULL;
        field->NextField = NULL;
        field->FieldType = NULL;
    }
    return field;
}

/* Free FieldList stated at head. */
void DestoryFieldList(FieldList * head){
    while(head){
        FieldList * nxt = head->NextField;
        DestoryTypeDescriptor(head->FieldType);
        free(head);
        head = nxt;
    }
}

/* 
Creat a new FieldList same as the one pointed to by src
and return pointer to the head of new list.
*/
FieldList * CopyFieldList(FieldList * src){
    FieldList * cppre = NULL;
    FieldList * srccur = src;
    FieldList * cphead = NULL;
    bool failed = false;
    for(;srccur != NULL;){
        FieldList * cpcur = CreatField();
        if(!cpcur){
            failed = true;
            break;
        }
        cpcur->FieldName = srccur->FieldName;
        cpcur->FieldType = CopyTypeDescriptor(srccur->FieldType);
        if(!cpcur->FieldType){
            failed = true;
            DestoryFieldList(cpcur);
            break;
        }
        if(cphead == NULL) cphead = cpcur;
        if(cppre != NULL) cppre->NextField = cpcur;
        cppre = cpcur;
        srccur = srccur->NextField;
    }
    if(failed){
        DestoryFieldList(cphead);
        cphead = NULL;
    }
    return cphead;
}

/* 
Creat a new Array TypeDescripor. If set copy to true, a new TypeDescriptor 
same as arraytype will be created, else just points to the arraytype object.
*/
TypeDescriptor * CreatArrayDescriptor(TypeDescriptor * arraytype, int arraysize, bool copy){
    TypeDescriptor * array = CreatTypeDescriptor();
    if(array){
        array->TypeClass = ARRAY;
        array->Array.size = arraysize;
        array->Array.elem = (copy)? CopyTypeDescriptor(arraytype) : arraytype;
        if(!array->Array.elem){
            DestoryTypeDescriptor(array);
            array = NULL;
        }
    }
    return array;
}

/* 
Creat a new Structure TypeDescripor. If set copy to true, a new FieldList 
same as fields will be created, else just points to the fields object.
*/
TypeDescriptor * CreatStructureDescriptor(FieldList * fields, bool copy){
    TypeDescriptor * st = CreatTypeDescriptor();
    if(st){
        st->TypeClass = STRUCTURE;
        if(!fields){
            /* structure with no fields */
            st->Structure = NULL;
        }else{
            st->Structure = (copy) ? CopyFieldList(fields) : fields;
            if(!st->Structure){
                DestoryTypeDescriptor(st);
                st = NULL;
            }
        }
    }
    return st;
}

bool IsEqualType(TypeDescriptor * a,TypeDescriptor * b){
    if(a->TypeClass == b->TypeClass){
        switch(a->TypeClass){
            case BASIC : 
                return (a->Basic == b->Basic);
            case ARRAY :
                return IsEqualType(a->Array.elem,b->Array.elem);
            case STRUCTURE :
                FieldList * ptra = a->Structure;
                FieldList * ptrb = b->Structure;
                while((ptra!=NULL)&&(ptrb!=NULL)){
                    if(!IsEqualType(ptra->FieldType,ptrb->FieldType)) break;
                    ptra = ptra->NextField;
                    ptrb = ptrb->NextField;
                }
                return ((ptra == NULL)&&(ptrb == NULL));
            case ERROR : /* treat error type equal to error type */
                return true;
            default :
                break;
        }
    }
    return false;
}

bool IsErrorType(TypeDescriptor * a){
    return a->TypeClass == ERROR;
}