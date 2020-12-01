#ifndef _GLASS_INFO_H_
#define _GLASS_INFO_H_


#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#include "config.h"


struct _glass_info_;
typedef struct _glass_info_ glass_info;

struct _glass_info_
{
	char job_number[JOB_NUMBER_LENGTH];
	char calloff_date_time[CALLOFF_DATE_TIME_LENGTH];
	char vehicle_number[VEHICLE_NUMBER_LENGTH];
	char rear_window_type[REAR_WINDOW_TYPE_LENGTH];
	char vehicle_model;
	uint32_t id;
	char ProductionSeq[PRODUCTION_SEQ_LENGTH];
};


glass_info * glass_info_new(char *, char *, char *, char*, char, uint32_t, char *);
char * glass_info_fet_job_number(glass_info *);
char * glass_info_get_calloff_date_time(glass_info *);
char * glass_info_get_vehicle_number(glass_info *);
char * glass_info_get_rear_window_type(glass_info *);
char glass_info_get_vehicle_model(glass_info *);
uint32_t glass_info_get_id(glass_info *);
char * glass_info_get_production_num(glass_info *);
void glass_info_finalize(void *);
size_t glass_info_get_sizeof();


#endif
