#ifndef _MODEL_H_
#define _MODEL_H_

#include <s7lib.h>
#include <s7lib_parser.h>
#include "glass_info.h"
#include "config.h"




bool model_set_done_status(s7lib *);
bool model_set_error_status(s7lib *);
bool model_reset_error_status(s7lib *);
bool model_get_error_status(s7lib *);
bool model_reset_done_status(s7lib *);
bool model_reset_cmd_byte(s7lib *);
bool model_write_queue_size(s7lib *, int16_t);
bool model_set_priority_input_status(s7lib *);
bool model_reset_priority_input_status(s7lib *);
bool model_read_cmd_reload_visu_status(s7lib *);
bool model_read_cmd_enqueue_status(s7lib *);
bool model_read_cmd_priority_enqueue_status(s7lib *);
bool model_read_cmd_dequeue_status(s7lib *);
bool model_read_cmd_delete_status(s7lib *);
char * model_read_vehicle_number(s7lib *, int);
char * model_read_rear_window_type(s7lib *, int);
char model_read_vehicle_model(s7lib *, int);
uint32_t model_read_id(s7lib *, int);
glass_info * model_read_glass_info(s7lib *, int);
bool model_write_glass_info(s7lib *, glass_info *, int);
uint8_t * model_write_glass_info_to_array(uint8_t *, glass_info *, int);
glass_info * model_read_glass_info_from_array(uint8_t *, int);
bool model_write_visu_queue(s7lib *, uint8_t *, uint16_t);



#endif
