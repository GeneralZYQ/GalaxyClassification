//
// Created by 张元清 on 2019-06-26.
//

#include "MTMain_double.h"

#ifndef MTTEST_MTMAXTREE_H
#define MTTEST_MTMAXTREE_H

#define MT_UNASSIGNED -1
#define MT_IN_QUEUE -2
#define MT_NO_PARENT -3

#define MT_NO_OBJECT -1

#define MT_IS_ROOT(MT_PTR, IDX) ((MT_PTR)->nodes + IDX == \
  (MT_PTR)->root)

#define MT_CONN_12_WIDTH 5
#define MT_CONN_12_HEIGHT 5

#define MT_CONN_8_WIDTH 3
#define MT_CONN_8_HEIGHT 3

#define MT_CONN_4_WIDTH 3
#define MT_CONN_4_HEIGHT 3

extern const int mt_conn_12[MT_CONN_12_HEIGHT * MT_CONN_12_WIDTH];
extern const int mt_conn_8[MT_CONN_8_HEIGHT * MT_CONN_8_WIDTH];
extern const int mt_conn_4[MT_CONN_4_HEIGHT * MT_CONN_4_WIDTH];

typedef struct
{
    INT_TYPE parent;
    INT_TYPE area;
} mt_node;

typedef struct
{
    FLOAT_TYPE volume;
    FLOAT_TYPE power;
} mt_node_attributes;

//typedef struct { // Maar this can not be used as the final data
//
//    INT_TYPE index; //The start index
//    int *indexes; // The indexes, separated by ','. Such as '4,5,6,7,9'
//    int count; // How many indexes are there in the *indexes
//    int size; // The current size of the *indexes
//
//}mt_node_indexes;

typedef struct {
    INT_TYPE index;
    INT_TYPE sumX;
    INT_TYPE sumY;
    FLOAT_TYPE M00;
    FLOAT_TYPE M11;
    FLOAT_TYPE M01;
    FLOAT_TYPE M10;
    FLOAT_TYPE M20;
    FLOAT_TYPE M02;
    FLOAT_TYPE M12;
    FLOAT_TYPE M21;
    FLOAT_TYPE M30;
    FLOAT_TYPE M03;
}mt_node_auxiliary;

typedef struct {
    INT_TYPE index;//The index of the node
    INT_TYPE imageArea;
    PIXEL_TYPE value;
    FLOAT_TYPE moment1;
    FLOAT_TYPE moment2;
    FLOAT_TYPE moment3;
    FLOAT_TYPE moment4;
    FLOAT_TYPE moment5;
    FLOAT_TYPE moment6;
    FLOAT_TYPE moment7;
}mt_node_moments;

typedef struct
{
    SHORT_TYPE x;
    SHORT_TYPE y;
} mt_image_location;

typedef struct
{
    mt_image_location location;
    PIXEL_TYPE value;
} mt_pixel;

#include "MTStack.h"
#include "MTHeap.h"


typedef struct
{
    const int* neighbors;
    int height;
    int width;
} mt_connectivity;

typedef struct
{
    mt_node *root;
    mt_node *nodes;
    mt_node_attributes *nodes_attributes;
    mt_heap heap;
    mt_stack stack;
    image img;
    mt_connectivity connectivity;
    int verbosity_level;
//    mt_node_indexes *nodeIndexes;
    mt_node_auxiliary *auxiliaries;
    mt_node_moments *moments;
} mt_data;

void mt_init(mt_data* mt, const image* img);
void mt_flood(mt_data* mt);
void mt_free(mt_data* mt);


#endif //MTTEST_MTMAXTREE_H
