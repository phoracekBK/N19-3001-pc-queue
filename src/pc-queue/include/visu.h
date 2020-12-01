/*
 * visu.h
 *
 *  Created on: 27. 11. 2020
 *      Author: petr.horacek
 */

#ifndef PC_QUEUE_INCLUDE_VISU_H_
#define PC_QUEUE_INCLUDE_VISU_H_

#include "glass_info.h"


struct _visu_
{
	glass_info visual_queue[10];
	uint16_t queue_size;
};

typedef struct _visu_ visu;


#endif /* PC_QUEUE_INCLUDE_VISU_H_ */
