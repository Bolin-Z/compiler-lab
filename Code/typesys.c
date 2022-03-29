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
    BasicTypeInt->Basic = INT;
    BasicTypeFloat = CreatTypeDescriptor();
    BasicTypeFloat->TypeClass = BASIC;
    BasicTypeFloat->Basic = FLOAT;
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
                FieldList * cur = s->Structure;
                while(cur != NULL){
                    FieldList * nxt = cur->NextField;
                    DestoryTypeDescriptor(cur->FieldType);
                    free(cur);
                    cur = nxt;
                }
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
    for(;src != NULL;){
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

/* 
    TypeDescriptor * CreatArrayAtOnce(TypeDescriptor * basetype, int dimension, bool copy, ...);
    Return an array TypeDescriptor object.
    basetype:  basic element type of array
    dimension: number of dimensions (dimension >= 1)
    copy: if set copy to true, new TypeDescriptor same as basetype will be created
    ... :      list of sizes of each dimension with type int
               format: size1, size2, size3, ...
*/
TypeDescriptor * CreatArrayAtOnce(TypeDescriptor * basetype, int dimension, bool copy, ...){
    if(dimension < 1) return NULL;
    TypeDescriptor * head = NULL;
    TypeDescriptor * tail = NULL;
    TypeDescriptor * pre = NULL;
    bool failed = false;
    va_list(ap);
    va_start(ap,copy);
    for(int i = 1;i <= dimension;i++){
        TypeDescriptor * cur = CreatTypeDescriptor();
        if(!cur){
            failed = true;
            break;
        }
        cur->TypeClass = ARRAY;
        cur->Array.size = va_arg(ap,int);
        if(head == NULL) head = cur;
        if(pre != NULL) pre->Array.elem = cur;
        pre = cur;
        if(i == dimension) tail = cur;
    }
    va_end(ap);
    tail->Array.elem = (copy)? CopyTypeDescriptor(basetype) : basetype;
    if(!tail->Array.elem) failed = true;
    if(failed){
        DestoryTypeDescriptor(head);
        head = NULL;
    }
    return head;
}

/*
    TypeDescriptor * CreatStructureAtOnce(int fieldscnt,...); 
    Return an structure type descriptor on success, NULL if failed.
    fieldscnt : number of fields
    copy: if set copy to true, FieldType will point to new TypeDescriptor
    ... : list of field information of each field
          format: field1_name, field1_type, field2_name, field2_type, ...
*/
TypeDescriptor * CreatStructureAtOnce(int fieldscnt, bool copy, ...){
    TypeDescriptor * st = CreatStructureDescriptor(NULL,false);
    if(fieldscnt > 0){
        FieldList * head = NULL;
        FieldList * pre = NULL;
        bool failed = false;
        va_list(ap);
        va_start(ap,copy);
        for(int i = 0;i < fieldscnt;i++){
            FieldList * cur = CreatField();
            if(!cur){
                failed = true;
                break;
            }
            char * fn = va_arg(ap,char*);
            cur->FieldName = fn;
            TypeDescriptor * ft = va_arg(ap,TypeDescriptor*);
            cur->FieldType = (copy)? CopyTypeDescriptor(ft) : ft;
            if(!cur->FieldType){
                DestoryFieldList(cur);
                failed = true;
                break;
            }
            if(head == NULL) head = cur;
            if(pre != NULL) pre->NextField = cur;
            pre = cur;
        }
        va_end(ap);
        if(failed){
            DestoryFieldList(head);
            head = NULL;
            DestoryTypeDescriptor(st);
            st = NULL;
        }
        st->Structure = head;
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
            case ERROR : /* ERROR type is not equl to any type */
            default :
                break;
        }
    }
    return false;
}
