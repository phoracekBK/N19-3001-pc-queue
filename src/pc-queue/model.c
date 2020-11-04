#include "include/model.h"




bool model_set_error_status(s7lib * this)
{
	return s7lib_write_bool(this, ERROR_BYTE, ERROR_BIT, true);
}

bool model_reset_error_status(s7lib * this)
{
	return s7lib_write_bool(this, ERROR_BYTE, ERROR_BIT, false);
}

bool model_get_error_status(s7lib * this)
{
	return s7lib_read_bool(this, ERROR_BYTE, ERROR_BIT);
}

bool model_set_done_status(s7lib * this)
{
	return s7lib_write_bool(this, DONE_BYTE, DONE_BIT, true);
}

bool model_reset_done_status(s7lib * this)
{
	return s7lib_write_bool(this, DONE_BYTE, DONE_BIT, false);
}

bool model_read_cmd_reload_visu_status(s7lib * this)
{
	return s7lib_read_bool(this, CMD_RELOAD_VISU_BYTE, CMD_RELOAD_VISU_BIT);
}

bool model_read_cmd_enqueue_status(s7lib * this)
{
	return s7lib_read_bool(this, CMD_ENQUEUE_BYTE, CMD_ENQUEUE_BIT);
}

bool model_read_cmd_priority_enqueue_status(s7lib * this)
{
	return s7lib_read_bool(this, CMD_PRIORITY_ENQUEUE_BYTE, CMD_PRIORITY_ENQUEUE_BIT);
}

bool model_read_cmd_dequeue_status(s7lib * this)
{
	return s7lib_read_bool(this, CMD_DEQUEUE_BYTE, CMD_DEQUEUE_BIT);
}

bool model_read_cmd_delete_status(s7lib * this)
{
	return s7lib_read_bool(this, CMD_DELETE_BYTE, CMD_DELETE_BIT);
}

bool model_reset_cmd_byte(s7lib * this)
{
	uint8_t byte = 0;
	return s7lib_write(this, &byte, CMD_BYTE, 1);
}

bool model_write_queue_size(s7lib * this, int16_t queue_size)
{
	return s7lib_write_int(this, VISU_QUEUE_SIZE_BYTE, queue_size);
}

uint8_t * model_write_glass_info_to_array(uint8_t * byte_array, glass_info * glass, int base)
{
	memcpy(byte_array+(base*GLASS_SIZE), glass, glass_info_get_sizeof());

	return byte_array;
}

glass_info * model_read_glass_info(s7lib * this, int base)
{
	return (glass_info *) s7lib_read(this, base, GLASS_SIZE);
}

bool model_write_glass_info(s7lib * this, glass_info * glass, int base)
{
	bool return_value = s7lib_write(this, (uint8_t*) glass, base, GLASS_SIZE);

	return return_value;
}

bool model_write_visu_queue(s7lib * this, uint8_t * byte_array, uint16_t list_size)
{
	return s7lib_write(this, byte_array, VISU_BYTE, GLASS_SIZE*VISU_ITEM_NUMBERN) && model_write_queue_size(this, list_size) && model_reset_error_status(this);
}














