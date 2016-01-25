#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

/* calls prespecified free function on vector @v s element
 * @elem location of vectors element @v given vector
 * if free function was not specified function does nothing */
static void freeVecElement(vector *v, void *elem){
    assert(elem != NULL);//check if vector is valid
    if (v->freeFn != NULL){
       v->freeFn(elem);
    }
}

/* function is called when we want to add new element
 * to the vector. it checks if there is enough room for it
 * and if its not it reallocates new space */
static void elemAddingRequiered(vector *v){
    assert(v != NULL);//check if vector is valid

    if (v->logicalSize == v->allocatedSize){
       v->allocatedSize += v->reallocSize;
       v->elems = realloc(v->elems, v->elemSize * v->allocatedSize);
       assert(v->elems != NULL);
    }
}

static const int DefaultAllocSize = 1;
void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(v != NULL);//check if vector is valid
    assert(elemSize > 0);//check for valid elemSize


    if (initialAllocation <= 0) initialAllocation = DefaultAllocSize;

    v->reallocSize = initialAllocation;
    v->elemSize = elemSize;
    v->logicalSize = 0;
    v->allocatedSize = initialAllocation;
    v->freeFn = freeFn;
    //make place for elems
    v->elems = malloc(elemSize * initialAllocation);
    assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
    assert(v != NULL);//check if vector is valid

    if (v->freeFn != NULL){
        //dispose elements one by one if free function exists
        for (int i = 0; i < v->logicalSize; i++){
            v->freeFn(VectorNth(v, i));
        }
    }

    //free elems space
    free(v->elems);
}

int VectorLength(const vector *v)
{
   assert(v != NULL);//check if vector is valid
   return v->logicalSize;
}

void *VectorNth(const vector *v, int position)
{
    assert(v != NULL);//check if vector is valid
    assert(position >= 0);//check for valid position value
    assert(position < v->logicalSize);//check if requested is bigger then possible

    return (char *)(v->elems) + (position * v->elemSize);
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    //get pointer on position element
    //(if doensot exists VectorNth function will take care of it)
    void *elemOnPosition = VectorNth(v, position);

    freeVecElement(v, elemOnPosition);
    memmove(elemOnPosition, elemAddr, v->elemSize);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    //check if its adding at last point or not
    if (position == v->logicalSize){
        VectorAppend(v, elemAddr);
        return;
    }

    //get pointer on position element
    //(if doensot exists VectorNth function will take care of it)
    void *elemOnPosition = VectorNth(v, position);

    elemAddingRequiered(v);//make sure there is space for new element
    //shifting old part by 1
    memmove((char *) elemOnPosition + v->elemSize, elemOnPosition, v->elemSize * (v->logicalSize - position));
    //copying new one
    memmove(elemOnPosition, elemAddr, v->elemSize);
    v->logicalSize++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
    elemAddingRequiered(v);//make sure there is space for new element

    v->logicalSize++;
    memmove(VectorNth(v, v->logicalSize - 1), elemAddr, v->elemSize);
}

void VectorDelete(vector *v, int position)
{
    //get pointer on position element
    //(if doensot exists VectorNth function will take care of it)
    void *elemOnPosition = VectorNth(v, position);

    freeVecElement(v, elemOnPosition);
    v->logicalSize--;
    memmove(elemOnPosition, (char *) elemOnPosition + v->elemSize, v->elemSize * (v->logicalSize - position));
}


void VectorSort(vector *v, VectorCompareFunction compare)
{
    //sorting array using quick sort
    qsort(v->elems, v->logicalSize, v->elemSize, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
   assert(mapFn != NULL);
   //iterate over all elemets
   for (int i = 0; i < v->logicalSize; i++){
     mapFn(VectorNth(v, i), auxData);
   }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{
    //check if vector is empty return always kNotFound
    if (v->logicalSize == 0) return kNotFound;

    void *startPos = VectorNth(v, startIndex);
    size_t elemsNumToSearch = v->logicalSize - startIndex;
    void * found;
    if (isSorted){
        //we know that array is sorted so we can use binary search algorithm to find it
        found = bsearch(key, startPos, elemsNumToSearch, v->elemSize, searchFn);
    }else{
        // no guaranties that array is sorted so lfind is used to search for value
        found = lfind(key, startPos, &elemsNumToSearch, v->elemSize, searchFn);
    }

    if (found == NULL) return kNotFound;


    //if we are here and nothing retured so far key found and found is pointer to it
    return ((char *)found - (char *)v->elems) / v->elemSize;
}
