#include "include/glass_info.h"
#include <s7lib_parser.h>



glass_info * glass_info_new(char *job_number,
							char * calloff_date_time,
							char * vehicle_number,
							char* rear_window_type,
							char vehicle_model,
							uint32_t id,
							char * productionSeq)
{
	glass_info * this = malloc(sizeof(glass_info));

	memcpy(this->job_number, job_number, JOB_NUMBER_LENGTH);
	memcpy(this->calloff_date_time, calloff_date_time, CALLOFF_DATE_TIME_LENGTH);
	memcpy(this->vehicle_number+1, vehicle_number, VEHICLE_NUMBER_LENGTH);
	memcpy(this->rear_window_type, rear_window_type, REAR_WINDOW_TYPE_LENGTH);
	this->vehicle_model = vehicle_model;
	this->id = id;
	memcpy(this->ProductionSeq+1, productionSeq, PRODUCTION_SEQ_LENGTH);

	return this;
}


char * glass_info_get_job_number(glass_info * this)
{
	return this->job_number;
}

char * glass_info_get_calloff_date_time(glass_info * this)
{
	return this->calloff_date_time;
}

char * glass_info_get_vehicle_number(glass_info * this)
{
	return this->vehicle_number;
}

char * glass_info_get_rear_window_type(glass_info * this)
{
	return this->rear_window_type;
}

char glass_info_get_vehicle_model(glass_info * this)
{
	return this->vehicle_model;
}

uint32_t glass_info_get_id(glass_info * this)
{
	return this->id;
}

char * glass_info_get_production_num(glass_info * this)
{
	return this->ProductionSeq;
}


void glass_info_finalize(void * this)
{
	free(this);
}
