//
// Created by 张元清 on 2019-06-26.
//

#include "MTMaxTree.h"

#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MT_INDEX_OF(PIXEL) ((PIXEL).location.y * mt->img.width + \
  (PIXEL).location.x)

#define STR_LEN 10

const int mt_conn_12[MT_CONN_12_HEIGHT * MT_CONN_12_WIDTH] =
        {
                0, 0, 1, 0, 0,
                0, 1, 1, 1, 0,
                1, 1, 0, 1, 1,
                0, 1, 1, 1, 0,
                0, 0, 1, 0, 0
        };

const int mt_conn_8[MT_CONN_8_HEIGHT * MT_CONN_8_WIDTH] =
        {
                1, 1, 1,
                1, 0, 1,
                1, 1, 1,
        };

const int mt_conn_4[MT_CONN_4_HEIGHT * MT_CONN_4_WIDTH] =
        {
                0, 1, 0,
                1, 0, 1,
                0, 1, 0,
        };


void inner_flike() {
    printf("haha");
}

FLOAT_TYPE calculateMiuij (int index, int i, int j, mt_data *mt) {
    mt_node_auxiliary aux = mt->auxiliaries[index];
    mt_node node = mt->nodes[index];
    float averX = (float)aux.M10 / (float)aux.M00;
    float averY = (float)aux.M01 / (float)aux.M00;
    if (i == 0 && j == 0) {
        return aux.M00;
    } else if (i == 0 && j == 1) {
        return 0;
    } else if (i == 1 && j == 0) {
        return 0;
    } else if (i == 1 && j == 1) {
        return aux.M11 - averX * aux.M01;
    } else if (i == 2 && j == 0) {
        return aux.M20 - averX * aux.M10;
    } else if (i == 0 && j == 2) {
        return aux.M02 - averY * aux.M01;
    } else if (i == 2 && j == 1) {
        return aux.M21 - 2 * averX * aux.M11 - averY * aux.M20 + 2 * pow(averX , 2) * aux.M01;
    } else if (i == 1 && j == 2) {
        return aux.M12 - 2 * averY * aux.M11 - averX * aux.M02 + 2 * pow(averY, 2) * aux.M10;
    } else if (i == 3 && j == 0) {
        return aux.M30 - 3 * averX * aux.M20 + 2 * pow(averX, 2) * aux.M10;
    } else if (i == 0 && j == 3) {
        return aux.M03 - 3 * averY * aux.M02 + 2 * pow(averY, 2) * aux.M01;
    }

    return 0;
}

//FLOAT_TYPE calculateMupq(int index, mt_data *mt, int p, int q) {
//    mt_node_auxiliary aux = mt->auxiliaries[index];
//    mt_node node = mt->nodes[index];
//    float averX = (float)aux.M10 / (float)aux.M00;
//    float averY = (float)aux.M01 / (float)aux.M00;
//
//    mt_node_indexes node_index = mt->nodeIndexes[index];
//
//    FLOAT_TYPE sum = 0.0;
//    for (int k = 0; k < node_index.count; ++k) {
//        int ind = node_index.indexes[k];
//        sum += pow((ind % mt->img.width - averX) ,p) * pow((int)(ind / mt->img.width) ,q) * mt->img.data[ind];
//    }
//    return sum;
//}

FLOAT_TYPE calculateItaij(int index, mt_data *mt, int i, int j) {
    FLOAT_TYPE miuij = calculateMiuij(index, i, j, mt);
    FLOAT_TYPE miu00 = calculateMiuij(index, 0, 0, mt);
//    FLOAT_TYPE miuij = calculateMupq(index, mt, i, j);
//    FLOAT_TYPE miu00 = calculateMupq(index, mt, 0 ,0);
    FLOAT_TYPE senorita = (1.0 + (FLOAT_TYPE)(i + j) / 2.0);
    return miuij / pow(miu00, senorita);
}


FLOAT_TYPE calculateM1 (int index, mt_data *mt) {

    FLOAT_TYPE ita20 = calculateItaij(index, mt, 2, 0);
    FLOAT_TYPE ita02 = calculateItaij(index, mt, 0, 2);

    return calculateItaij(index, mt, 2, 0) + calculateItaij(index, mt, 0, 2);
}

FLOAT_TYPE calculateM2(int index, mt_data *mt) {
    float ita11 = calculateItaij(index, mt, 1,1);
    float ita20 = calculateItaij(index, mt, 2,0);
    float ita02 = calculateItaij(index, mt, 0, 2);
    return pow(ita20 - ita02, 2) + 4 * pow(ita11, 2);
}

FLOAT_TYPE calculateM3 (int index, mt_data *mt) {
    float ita30 = calculateItaij(index, mt, 3,0);
    float ita12 = calculateItaij(index, mt, 1,2);
    float ita21 = calculateItaij(index, mt, 2, 1);
    float ita03 = calculateItaij(index, mt, 0, 3);
    return pow(ita30 - 3 * ita12, 2) + pow(3 * ita21 - ita03 , 2);
}

FLOAT_TYPE calculateM4 (int index, mt_data *mt) {
    float ita30 = calculateItaij(index, mt, 3,0);
    float ita12 = calculateItaij(index, mt, 1,2);
    float ita21 = calculateItaij(index, mt, 2, 1);
    float ita03 = calculateItaij(index, mt, 0, 3);

    return pow(ita30 + ita12, 2) + pow(ita21 + ita03, 2);
}

FLOAT_TYPE calculateM5(int index, mt_data *mt) {
    float ita30 = calculateItaij(index, mt, 3,0);
    float ita12 = calculateItaij(index, mt, 1,2);
    float ita21 = calculateItaij(index, mt, 2, 1);
    float ita03 = calculateItaij(index, mt, 0, 3);
    return (ita30 - 3 * ita12) * (ita30 + ita12) * (pow(ita30 + ita12, 2) - 3 * pow(ita21 + ita03, 2)) + (3 * ita21 - ita03) * (ita21 + ita03) * (3 * pow(ita30 + ita12 , 2) - pow(ita21 + ita03, 2));

}

FLOAT_TYPE calculateM6(int index, mt_data *mt) {
    float ita30 = calculateItaij(index, mt, 3,0);
    float ita12 = calculateItaij(index, mt, 1,2);
    float ita21 = calculateItaij(index, mt, 2, 1);
    float ita03 = calculateItaij(index, mt, 0, 3);
    float ita20 = calculateItaij(index, mt, 2,0);
    float ita02 = calculateItaij(index, mt, 0,2);
    float ita11 = calculateItaij(index, mt, 1, 1);
    return (ita20 - ita02) * (pow(ita30 + ita12, 2) - pow(ita21 + ita03, 2)) + 4 * ita11 * (ita30 + ita12) * (ita21 + ita03);
}

FLOAT_TYPE calculateM7(int index, mt_data *mt) {
    float ita30 = calculateItaij(index, mt, 3,0);
    float ita12 = calculateItaij(index, mt, 1,2);
    float ita21 = calculateItaij(index, mt, 2, 1);
    float ita03 = calculateItaij(index, mt, 0, 3);
    return (3 * ita21 - ita03) * (ita30 + ita12) * (pow(ita30 + ita12, 2) - 3 * pow(ita21 + ita03, 2)) - (ita30 - 3 * ita12) * (ita21 + ita03) * (3 * pow(ita30 + ita12, 2) - pow(ita21 + ita03, 2));
}

char *GenerateStr()
{
    char str[STR_LEN + 1] = {0};
    int i,flag;

    srand(time(NULL));
    for(i = 0; i < STR_LEN; i ++)
    {
        flag = rand()%3;
        switch(flag)
        {
            case 0:
                str[i] = rand()%26 + 'a';
                break;
            case 1:
                str[i] = rand()%26 + 'A';
                break;
            case 2:
                str[i] = rand()%10 + '0';
                break;
        }
    }
//    printf("%s\n", str);

    return str;
}


mt_pixel mt_starting_pixel(mt_data* mt)
{
    // Find the minimum pixel value in the image
    SHORT_TYPE y;
    SHORT_TYPE x;

    mt_pixel pixel;
    pixel.location.x = 0;
    pixel.location.y = 0;

    pixel.value = mt->img.data[0];

    // iterate over image pixels
    for (y = 0; y != mt->img.height; ++y)
    {
        for (x = 0; x != mt->img.width; ++x)
        {
            // Convert from x,y coordinates to an array index
            INT_TYPE index = y * mt->img.width + x;

            // If the pixel is less than the current minimum, update the minimum
            if (mt->img.data[index] < pixel.value)
            {
                pixel.value = mt->img.data[index];
                pixel.location.x = x;
                pixel.location.y = y;
            }
        }
    }

    return pixel;
}

static void mt_init_nodes(mt_data* mt)
{
    // Initialise the nodes of the maxtree
    INT_TYPE i;

    // Set all parents as unassigned, and all areas as 1
    for (i = 0; i != mt->img.size; ++i)
    {
        mt->nodes[i].parent = MT_UNASSIGNED;
        mt->nodes[i].area = 1;
    }
}

static void mt_init_node_auxiliaries(mt_data *mt) {

    for (int j = 0; j != mt->img.size; ++j) {
        mt->auxiliaries[j].index = j;

        mt->auxiliaries[j].sumX = j % mt->img.width;
        mt->auxiliaries[j].sumY = (int)(j / mt->img.width);
        mt->auxiliaries[j].M00 = mt->img.data[j];
        mt->auxiliaries[j].M11 = mt->auxiliaries[j].sumX * mt->auxiliaries[j].sumY * mt->img.data[j];
        mt->auxiliaries[j].M10 = mt->auxiliaries[j].sumX * mt->img.data[j];
        mt->auxiliaries[j].M01 = mt->auxiliaries[j].sumY * mt->img.data[j];
        mt->auxiliaries[j].M20 = pow(mt->auxiliaries[j].sumX, 2.0) * mt->img.data[j];
        mt->auxiliaries[j].M02 = pow(mt->auxiliaries[j].sumY, 2.0) * mt->img.data[j];
        mt->auxiliaries[j].M12 = mt->auxiliaries[j].sumX * pow(mt->auxiliaries[j].sumY, 2.0) * mt->img.data[j];
        mt->auxiliaries[j].M21 = mt->auxiliaries[j].sumY * mt->img.data[j] * pow(mt->auxiliaries[j].sumX, 2.0);
        mt->auxiliaries[j].M30 = pow(mt->auxiliaries[j].sumX, 3.0) * mt->img.data[j];
        mt->auxiliaries[j].M03 = pow(mt->auxiliaries[j].sumY, 3.0) * mt->img.data[j];

    }
}

//static void mt_init_node_indexes(mt_data *mt) {
//    for (int i = 0; i < mt->img.size; ++i) {
//        mt->nodeIndexes[i].index = i;
//        mt->nodeIndexes[i].size = 100;
//        mt->nodeIndexes[i].count = 1;
//        mt->nodeIndexes[i].indexes = malloc(sizeof(int) * 100);
//        mt->nodeIndexes[i].indexes[0] = i;
//    }
//}

static void mt_init_node_moments (mt_data *mt) {
    INT_TYPE i;

    for (i = 0; i != mt->img.size; ++i) {
        mt->moments[i].index = i;
        mt->moments[i].value = 0.0;
        mt->moments[i].imageArea = mt->img.area;

        mt->moments[i].moment1 = 0.0;
        mt->moments[i].moment2 = 0.0;
        mt->moments[i].moment3 = 0.0;
        mt->moments[i].moment4 = 0.0;
        mt->moments[i].moment5 = 0.0;
        mt->moments[i].moment6 = 0.0;
        mt->moments[i].moment7 = 0,0;

    }
}

static void herkennen_de_makkelijke_meisjes (int count ) {

    FILE *lijstFile = NULL;
    char *namenlijst_file_route = (char*)malloc(sizeof(char)*512);
}

static void mt_calculate_local_spectrum(mt_data *mt) {

    FILE *wfp = NULL;
    char *result_file_path = (char*)malloc(sizeof(char)*512);

    if(result_file_path == NULL){
        printf("allocate memory for file path failed.\n");
    }

    memset(result_file_path, 0, sizeof(char)*512);
    char *finalname = (char*)malloc(sizeof(char)*512);

    char randoanmega[STR_LEN + 1];
    for (int i = 0; i < STR_LEN; ++i) {
        randoanmega[i] = GenerateStr()[i];
    }

    sprintf(finalname, "%d_%s_localSpectrum.csv", mt->img.height, randoanmega);

    sprintf(result_file_path, "%s/%s", "/Volumes/DISK1/spectrums/2DElliptics", finalname);
    sleep(1);

    wfp = fopen(result_file_path, "a");

    while (NULL == wfp) {

        printf("Didn't create file");

        char randoanme[STR_LEN + 1] = {0};
        for (int i = 0; i < STR_LEN; ++i) {
            randoanme[i] = GenerateStr()[i];
        }

        sprintf(finalname, "%d_%s_localSpectrum.csv", mt->img.height, randoanme);

        sprintf(result_file_path, "%s/%s", "/Volumes/DISK1/spectrums/2DElliptics", finalname); //This path should be changed.

        wfp = fopen(result_file_path, "a");
    }

    printf("I start calculate local spectrum here！");

    for (int i = 0; i < mt->img.size; ++i) {
        if (mt->nodes[i].area > 1) {
            int parent = mt->nodes[i].parent;
            if (parent == MT_NO_PARENT) {
                continue;
            }

            int currentArea = mt->nodes[i].area;
            int wholeArea = mt->img.area;

            float currentM1 = mt->moments[i].moment1;
            float currentM2 = mt->moments[i].moment2;
            float currentM3 = mt->moments[i].moment3;
            float currentM4 = mt->moments[i].moment4;
            float currentM5 = mt->moments[i].moment5;
            float currentM6 = mt->moments[i].moment6;
            float currentM7 = mt->moments[i].moment7;


            float fatherM1 = mt->moments[parent].moment1;
            float fatherM2 = mt->moments[parent].moment2;
            float fatherM3 = mt->moments[parent].moment3;
            float fatherM4 = mt->moments[parent].moment4;
            float fatherM5 = mt->moments[parent].moment5;
            float fatherM6 = mt->moments[parent].moment6;
            float fatherM7 = mt->moments[parent].moment7;

            float deltaValue = mt->img.data[i] - mt->img.data[parent];

            float spectrums[17] = {currentArea, wholeArea, deltaValue, currentM1 - fatherM1, currentM2 - fatherM2, currentM3 - fatherM3, currentM4 - fatherM4, currentM5 - fatherM5, currentM6 - fatherM6, currentM7 - fatherM7, currentM1, currentM2, currentM3, currentM4, currentM5, currentM6, currentM7};


            for (int j = 0; j < 17; ++j) {
                if (j ==  16) {
                    fprintf(wfp, "%f", spectrums[j]);
                } else {
                    fprintf(wfp, "%f,", spectrums[j]);
                }
            }
            fprintf(wfp, "\n");
        }
    }

    fprintf(wfp, "\n");
//
    close(wfp);
}

static void mt_calculate_moments (mt_data *mt) {
    INT_TYPE i;
    printf("Start to calculate moments herer!\n");
    for (i = 0; i < mt->img.size; ++i) {
        if (mt->nodes[i].area > 1) {

            float m1 = calculateM1(i, mt);
            float m2 = calculateM2(i, mt);
            float m3 = calculateM3(i, mt);
            float m4 = calculateM4(i, mt);
            float m5 = calculateM5(i, mt);
            float m6 = calculateM6(i, mt);
            float m7 = calculateM7(i, mt);

            mt->moments[i].moment1 = m1;
            mt->moments[i].moment2 = m2;
            mt->moments[i].moment3 = m3;
            mt->moments[i].moment4 = m4;
            mt->moments[i].moment5 = m5;
            mt->moments[i].moment6 = m6;
            mt->moments[i].moment7 = m7;
        }
    }
    printf("it's here!?\n");
}

static int mt_queue_neighbour(mt_data* mt, PIXEL_TYPE val,
                              SHORT_TYPE x, SHORT_TYPE y)
{
    // Add a pixel to the queue for processing

    //Create a pixel and set its location
    mt_pixel neighbour;
    neighbour.location.x = x;
    neighbour.location.y = y;

    // Convert from x,y coordinates to an array index
    INT_TYPE neighbour_index = MT_INDEX_OF(neighbour);
    // Get a pointer to the neighbour
    mt_node *neighbour_node = mt->nodes + neighbour_index;

    // If the neighbour has not already been processed, add it to the queue
    if (neighbour_node->parent == MT_UNASSIGNED)
    {
        neighbour.value = mt->img.data[neighbour_index];
        neighbour_node->parent = MT_IN_QUEUE;
        mt_heap_insert(&mt->heap, &neighbour);

        // If the neighbour has a higher value than the current node, return 1
        if (neighbour.value > val)
        {
            return 1;
        }
    }

    return 0;
}

static void mt_queue_neighbours(mt_data* mt,
                                mt_pixel* pixel)
{
    // Seems to be limiting conn values within image coordinates

    // Radius is half size of connectivity
    INT_TYPE radius_y = mt->connectivity.height / 2;
    INT_TYPE radius_x = mt->connectivity.width / 2;

    // If pixel's x is less than radius, conn = the difference
    INT_TYPE conn_x_min = 0;
    if (pixel->location.x < radius_x)
        conn_x_min = radius_x - pixel->location.x;

    // Ditto for y
    INT_TYPE conn_y_min = 0;
    if (pixel->location.y < radius_y)
        conn_y_min = radius_y - pixel->location.y;

    // If pixel's x + radius > image width, conn = radius + width - location - 1
    INT_TYPE conn_x_max = 2 * radius_x;
    if (pixel->location.x + radius_x >= mt->img.width)
        conn_x_max = radius_x + mt->img.width - pixel->location.x - 1;

    INT_TYPE conn_y_max = 2 * radius_y;
    if (pixel->location.y + radius_y >= mt->img.height)
        conn_y_max = radius_y + mt->img.height - pixel->location.y - 1;

    INT_TYPE conn_y;
    // Conn coordinates refer to position with connectivity grid
    for (conn_y = conn_y_min; conn_y <= conn_y_max; ++conn_y)
    {
        INT_TYPE conn_x;
        for (conn_x = conn_x_min; conn_x <= conn_x_max; ++conn_x)
        {
            // Skip iteration if 0 in connectivity grid
            if (mt->connectivity.
                    neighbors[conn_y * mt->connectivity.width + conn_x] == 0)
            {
                continue;
            }

            // Try to queue neighbour at x = x-rad+conn
            // If successfully queued and value higher than current,
            // break out of function
            if (mt_queue_neighbour(mt, pixel->value,
                                   pixel->location.x - radius_x + conn_x,
                                   pixel->location.y - radius_y + conn_y))
            {
                return;
            }
        }
    }
}

static void mt_merge_nodes(mt_data* mt,
                           INT_TYPE merge_to_idx,
                           INT_TYPE merge_from_idx)
{
    // Merge two nodes

    mt_node *merge_to = mt->nodes + merge_to_idx;
    mt_node_attributes *merge_to_attr = mt->nodes_attributes +
                                        merge_to_idx;

    mt_node *merge_from = mt->nodes + merge_from_idx;
    mt_node_attributes *merge_from_attr = mt->nodes_attributes +
                                          merge_from_idx;

    merge_to->area += merge_from->area;

    FLOAT_TYPE delta = mt->img.data[merge_from_idx] -
                       mt->img.data[merge_to_idx];

    merge_from_attr->power += delta *
                              (2 * merge_from_attr->volume + delta * merge_from->area);
    merge_to_attr->power += merge_from_attr->power;

    merge_from_attr->volume += delta * merge_from->area;
    merge_to_attr->volume += merge_from_attr->volume;

    mt_node_auxiliary *to_aux = mt->auxiliaries + merge_to_idx;
    mt_node_auxiliary *from_aux = mt->auxiliaries + merge_from_idx;

    to_aux->sumX += from_aux->sumX;
    to_aux->sumY += from_aux->sumY;
    to_aux->M00 += from_aux->M00;
    to_aux->M11 += from_aux->M11;
    to_aux->M10 += from_aux->M10;
    to_aux->M01 += from_aux->M01;
    to_aux->M20 += from_aux->M20;
    to_aux->M02 += from_aux->M02;
    to_aux->M12 += from_aux->M12;
    to_aux->M21 += from_aux->M21;
    to_aux->M30 += from_aux->M30;
    to_aux->M03 += from_aux->M03;


//    mt_node_indexes *to_node = mt->nodeIndexes + merge_to_idx;
//    mt_node_indexes *from_node = mt->nodeIndexes + merge_from_idx;
//
//    if (to_node->size < (to_node->count + from_node->count)) {
//
//        int *reAlloc = malloc(sizeof(int) * (to_node->count + from_node->count) * 2);
//        for (int i = 0; i < to_node->size; ++i) {
//            reAlloc[i] = to_node->indexes[i];
//        }
//
//        to_node->size = (to_node->count + from_node->count) * 2;
//        to_node->indexes = reAlloc;
//
////        to_node->indexes = (int *)realloc(to_node->indexes,(to_node->count + from_node->count) * 2);
//    }
//
//    for (int i = 0; i < from_node->count; ++i) {
//        to_node->indexes[to_node->count] = from_node->indexes[i];
//        to_node->count++;
//    }
}

static void mt_descend(mt_data* mt, mt_pixel *next_pixel)
{
    mt_pixel old_top = *mt_stack_remove(&mt->stack);
    INT_TYPE old_top_index = MT_INDEX_OF(old_top);


    mt_pixel* stack_top = MT_STACK_TOP(&mt->stack);

    if (stack_top->value < next_pixel->value)
    {
        mt_stack_insert(&mt->stack, next_pixel);
    }

    stack_top = MT_STACK_TOP(&mt->stack);
    INT_TYPE stack_top_index = MT_INDEX_OF(*stack_top);

    mt->nodes[old_top_index].parent = stack_top_index;

    // This is the start


    mt_merge_nodes(mt, stack_top_index, old_top_index);
}

static void mt_remaining_stack(mt_data* mt)
{
    while (MT_STACK_SIZE(&mt->stack) > 1)
    {
        mt_pixel old_top = *mt_stack_remove(&mt->stack);
        INT_TYPE old_top_index = MT_INDEX_OF(old_top);

        mt_pixel* stack_top = MT_STACK_TOP(&mt->stack);
        INT_TYPE stack_top_index = MT_INDEX_OF(*stack_top);

        mt->nodes[old_top_index].parent = stack_top_index;
        mt_merge_nodes(mt, stack_top_index, old_top_index);
    }
}

void mt_init(mt_data* mt, const image* img)
{
    printf("jaha");

    mt->img = *img;

    mt->nodes = safe_malloc(mt->img.size * sizeof(mt_node));
    mt->nodes_attributes = safe_calloc(mt->img.size,
                                       sizeof(mt_node_attributes));

    mt->auxiliaries = safe_malloc(mt->img.size * sizeof(mt_node_auxiliary));
    mt->moments = safe_malloc(mt->img.size * sizeof(mt_node_moments));

//    mt->nodeIndexes = safe_malloc(mt->img.size * sizeof(mt_node_indexes));
//    mt_init_node_indexes(mt);


    mt_stack_alloc_entries(&mt->stack);
    mt_heap_alloc_entries(&mt->heap);

    mt_init_nodes(mt);
    mt_init_node_auxiliaries(mt);
    mt_init_node_moments(mt);

    mt->connectivity.neighbors = mt_conn_4;
    mt->connectivity.width = MT_CONN_4_WIDTH;
    mt->connectivity.height = MT_CONN_4_HEIGHT;

    mt->verbosity_level = 0;
}

void mt_flood(mt_data* mt)
{

    assert(mt->connectivity.height > 0);
    assert(mt->connectivity.height % 2 == 1);
    assert(mt->connectivity.width > 0);
    assert(mt->connectivity.width % 2 == 1);

    if (mt->verbosity_level)
    {
        int num_neighbors = 0;
        int i;
        for (i = 0; i != mt->connectivity.height; ++i)
        {
            int j;
            for (j = 0; j != mt->connectivity.width; ++j)
            {
                if (mt->connectivity.neighbors[i * mt->connectivity.width + j])
                {
                    ++num_neighbors;
                }
            }
        }

        printf("%d neighbors connectivity.\n", num_neighbors);
    }


    mt_pixel next_pixel = mt_starting_pixel(mt);
    INT_TYPE next_index = MT_INDEX_OF(next_pixel);
    mt->root = mt->nodes + next_index;
    mt->nodes[next_index].parent = MT_NO_PARENT;
    mt_heap_insert(&mt->heap, &next_pixel);
    mt_stack_insert(&mt->stack, &next_pixel);

    while (MT_HEAP_NOT_EMPTY(&mt->heap))
    {
        mt_pixel pixel = next_pixel; // to construct
        INT_TYPE index = next_index;

        mt_queue_neighbours(mt, &pixel);

        next_pixel = *MT_HEAP_TOP(&mt->heap);
        next_index = MT_INDEX_OF(next_pixel);
        // printf("The pixel value is %f\n", next_pixel.value);

        if (next_pixel.value > pixel.value)
        {
            // Higher level
            // printf("heigher : the pixel value is %f\n", next_pixel.value);
            mt_stack_insert(&mt->stack, &next_pixel); // The next one is bigger than the current one.So it's higher. We will process it later.
            continue;
        }

        pixel = *mt_heap_remove(&mt->heap);
        index = MT_INDEX_OF(pixel);
        mt_pixel *stack_top = MT_STACK_TOP(&mt->stack);
        INT_TYPE stack_top_index = MT_INDEX_OF(*stack_top);

        if (index != stack_top_index)
        {
            mt->nodes[index].parent = stack_top_index;
            ++mt->nodes[stack_top_index].area;
            // This is the start

            mt_node_auxiliary *to_aux = mt->auxiliaries + stack_top_index;
            mt_node_auxiliary *from_aux = mt->auxiliaries + index;

            to_aux->sumX += from_aux->sumX;
            to_aux->sumY += from_aux->sumY;
            to_aux->M00 += from_aux->M00;
            to_aux->M11 += from_aux->M11;
            to_aux->M10 += from_aux->M10;
            to_aux->M01 += from_aux->M01;
            to_aux->M20 += from_aux->M20;
            to_aux->M02 += from_aux->M02;
            to_aux->M12 += from_aux->M12;
            to_aux->M21 += from_aux->M21;
            to_aux->M30 += from_aux->M30;
            to_aux->M03 += from_aux->M03;

//            mt_node_indexes to_node = mt->nodeIndexes[stack_top_index];
//            mt_node_indexes from_node = mt->nodeIndexes[index];

//            mt_node_indexes *to_node = mt->nodeIndexes + stack_top_index;
//            mt_node_indexes *from_node = mt->nodeIndexes + index;
//
//            if (to_node->size < (to_node->count + from_node->count)) {
//                to_node->size = (to_node->count + from_node->count) * 2;
//                to_node->indexes = realloc(to_node->indexes,(to_node->count + from_node->count) * 2);
//            }
//
////            to_node->indexes[to_node->count] = index;
////            to_node->count++;
//
//            int *reAlloc = malloc(sizeof(int) * (to_node->count + from_node->count) * 2);
//            for (int i = 0; i < to_node->size; ++i) {
//                reAlloc[i] = to_node->indexes[i];
//            }
//
//            to_node->size = (to_node->count + from_node->count) * 2;
//            to_node->indexes = reAlloc;
//            to_node->indexes[to_node->count] = index;
//            to_node->count++;


            //This is the end
        }

        if (MT_HEAP_EMPTY(&mt->heap))
        {
            break;
        }

        next_pixel = *MT_HEAP_TOP(&mt->heap);
        next_index = MT_INDEX_OF(next_pixel);

        if (next_pixel.value < pixel.value)
        {
            // Lower level

            mt_descend(mt, &next_pixel);
        }
    }

    mt_remaining_stack(mt);

    mt_calculate_moments(mt);

    mt_calculate_local_spectrum(mt);

    mt_stack_free_entries(&mt->stack);
    mt_heap_free_entries(&mt->heap);

}

void mt_free(mt_data* mt)
{

    // Free the memory occupied by the max tree
    free(mt->nodes);
    free(mt->nodes_attributes);
    free(mt->moments);
//    free(mt->nodeIndexes);
    free(mt->auxiliaries);
    //memset(mt, 0, sizeof(mt_data));
}

