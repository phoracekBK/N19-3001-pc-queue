#include "include/controler.h"
#include "include/config.h"
#include <time.h>
#include <stdio.h>

struct _controler_
{
	s7lib * s7lib_ref;
	c_linked_list * queue;

	uint8_t enqueue_state;
	uint8_t priority_enqueue_state;
	uint8_t dequeue_state;
	uint8_t delete_state;
	uint8_t reload_visu_state;
};



static void controler_init(controler *);
static bool controler_synchronize_visu_queue(controler *);
static uint8_t * controler_generate_visu_queue(controler *);
static uint8_t * controler_copy_first_n_glass(c_linked_list *, uint8_t *, int);
static c_linked_list * controler_delete_glass_in_queue(c_linked_list *, glass_info *);
static c_linked_list * controler_read_file(FILE *);
static c_linked_list * controler_load_from_file(controler *, char *);
static bool controler_save_to_file(controler *, char *);


static void controler_enqueue_callback(controler *);
static void controler_priority_enqueue_callback(controler *);
static void controler_dequeue_callback(controler *);
static void controler_delete_callback(controler *);
static void controler_reload_visu_callback(controler *);
static char * controler_get_current_time_date(char *);



controler * controler_new(char * ip_address, int rack, int slot, int db_index)
{
	controler * this = malloc(sizeof(controler));

	this->s7lib_ref = s7lib_new(ip_address, rack, slot, db_index);

	this->queue = controler_load_from_file(this, QUEUE_FILE_PATH);

	this->enqueue_state = 0;
	this->priority_enqueue_state = 0;
	this->dequeue_state = 0;
	this->delete_state = 0;
	this->reload_visu_state = 0;

	return this;
}


static c_linked_list * controler_read_file(FILE * input)
{
	void * buffer = malloc(glass_info_get_sizeof());

	if(fread(buffer, glass_info_get_sizeof(), 1, input) > 0)
	{
		return c_linked_list_add_as_previous(controler_read_file(input), buffer);
	}
	else
	{
		free(buffer);
		return NULL;
	}
}


static c_linked_list * controler_load_from_file(controler * this, char * address)
{
	FILE * input = fopen(address, "r");

	if(input != NULL)
	{
		c_linked_list * queue = controler_read_file(input);
		fclose(input);

		return queue;
	}

	return NULL;
}

static void controler_write_file(FILE * output, c_linked_list * list)
{
	if(list != NULL)
	{
		glass_info * glass = c_linked_list_get_data(list);

		fwrite((void *) glass, glass_info_get_sizeof(), 1, output);
		controler_write_file(output, c_linked_list_next(list));
	}
}

static bool controler_save_to_file(controler * this, char * address)
{
	FILE * output = fopen(address, "w");

	if(output != NULL)
	{
		controler_write_file(output, this->queue);
		fclose(output);
		return true;
	}

	return false;
}

static void controler_init(controler * this)
{
	if(model_reset_error_status(this->s7lib_ref) == false)
		printf("%s\n", "reset error status issue");
	else
		printf("error reset success\n");

	if(model_reset_done_status(this->s7lib_ref) == false)
		printf("%s\n", "reset done status issue");
	else
		printf("done flag reset success\n");

	if(controler_synchronize_visu_queue(this) == false)
			printf("%s\n", "initialize visual controler issue");
		else
			printf("visual queue synchronized\n");
}


static void controler_enqueue_callback(controler * this)
{
	if(this->enqueue_state == 0)
	{
		if(model_read_cmd_enqueue_status(this->s7lib_ref) == true)
			this->enqueue_state = 1;
	}
	else if(this->enqueue_state == 1)
	{
		char current_time[20];
		glass_info * glass = model_read_glass_info(this->s7lib_ref, 0);
		printf("%s - enqueue glass -> %s\n", controler_get_current_time_date(current_time), glass_info_get_vehicle_number(glass));

		this->queue = c_linked_list_find_first(c_linked_list_add_as_following(c_linked_list_find_last(this->queue), glass));
		this->enqueue_state = 2;
	}
	else if(this->enqueue_state == 2)
	{
		controler_save_to_file(this, QUEUE_FILE_PATH);
		controler_synchronize_visu_queue(this);

		model_set_done_status(this->s7lib_ref);
		this->enqueue_state = 3;
	}
	else if(this->enqueue_state == 3)
	{
		if(model_read_cmd_enqueue_status(this->s7lib_ref) == false)
			this->enqueue_state = 0;
	}
	else
	{
		this->enqueue_state = 0;
	}
}


static void controler_priority_enqueue_callback(controler * this)
{
	if(this->priority_enqueue_state == 0)
	{
		if(model_read_cmd_priority_enqueue_status(this->s7lib_ref) == true)
			this->priority_enqueue_state = 1;
	}
	else if(this->priority_enqueue_state == 1)
	{
		char current_time[20];
		glass_info * glass = model_read_glass_info(this->s7lib_ref, 0);
		printf("%s - priority enqueue glass -> %s\n", controler_get_current_time_date(current_time), glass_info_get_vehicle_number(glass));

		this->queue = c_linked_list_add_as_previous(this->queue, glass);
		this->priority_enqueue_state = 2;
	}
	else if(this->priority_enqueue_state == 2)
	{
		controler_save_to_file(this, QUEUE_FILE_PATH);
		controler_synchronize_visu_queue(this);

		model_set_done_status(this->s7lib_ref);

		this->priority_enqueue_state = 3;
	}
	else if(this->priority_enqueue_state == 3)
	{
		if(model_read_cmd_priority_enqueue_status(this->s7lib_ref) == false)
			this->priority_enqueue_state = 0;
	}
	else
	{
		this->priority_enqueue_state = 0;
	}
}

static void controler_dequeue_callback(controler * this)
{
	if(this->dequeue_state == 0)
	{
		if(model_read_cmd_dequeue_status(this->s7lib_ref) == true)
			this->dequeue_state = 1;
	}
	else if(this->dequeue_state == 1)
	{
		this->queue = c_linked_list_delete_with_release(this->queue, glass_info_finalize);
		this->dequeue_state = 2;
	}
	else if(this->dequeue_state == 2)
	{
		char current_time[20];
		printf("%s - dequeue glass -> %s\n", controler_get_current_time_date(current_time), glass_info_get_vehicle_number(c_linked_list_get_data(this->queue)));

		controler_save_to_file(this, QUEUE_FILE_PATH);
		controler_synchronize_visu_queue(this);

		model_set_done_status(this->s7lib_ref);
		this->dequeue_state = 3;
	}
	else if(this->dequeue_state == 3)
	{
		if(model_read_cmd_dequeue_status(this->s7lib_ref) == false)
			this->dequeue_state = 0;
	}
	else
	{
		this->dequeue_state = 0;
	}
}

static void controler_delete_callback(controler * this)
{
	if(this->delete_state == 0)
	{
		if(model_read_cmd_delete_status(this->s7lib_ref) == true)
			this->delete_state = 1;
	}
	else if(this->delete_state == 1)
	{
		glass_info * glass = model_read_glass_info(this->s7lib_ref, 0);

		if(glass != NULL)
		{
			c_linked_list * glass_for_delete = controler_delete_glass_in_queue(this->queue, glass);

			if (glass_for_delete != NULL)
			{
				this->queue = c_linked_list_find_first(c_linked_list_delete(glass_for_delete));
				char current_time[20];
				printf("%s - delete glass -> %s\n", controler_get_current_time_date(current_time), glass_info_get_vehicle_number(glass));
				this->delete_state = 2;
			}
			else
			{
				this->delete_state = 3;
			}
		}
		else
		{
			this->delete_state = 3;
		}

		free(glass);
	}
	else if(this->delete_state == 2)
	{
		controler_save_to_file(this, QUEUE_FILE_PATH);
		controler_synchronize_visu_queue(this);

		model_set_done_status(this->s7lib_ref);

		this->delete_state = 4;
	}
	else if(this->delete_state == 3)
	{
		model_set_error_status(this->s7lib_ref);
		this->delete_state = 4;
	}
	else if(this->delete_state == 4)
	{
		if(model_read_cmd_delete_status(this->s7lib_ref) == false)
			this->delete_state = 0;
	}
	else
	{
		this->delete_state = 0;
	}
}

static c_linked_list * controler_delete_glass_in_queue(c_linked_list * queue, glass_info * glass)
{
	if(queue != NULL)
	{
		if(strcmp(glass_info_get_vehicle_number(c_linked_list_get_data(queue)), glass_info_get_vehicle_number(glass)) == 0)
			return queue;
		else
			return controler_delete_glass_in_queue(c_linked_list_next(queue), glass);
	}

	return NULL;
}

static void controler_reload_visu_callback(controler * this)
{
	if(this->reload_visu_state == 0)
	{
		if(model_read_cmd_reload_visu_status(this->s7lib_ref) == true)
			this->reload_visu_state = 1;
	}
	else if(this->reload_visu_state == 1)
	{
		if(controler_synchronize_visu_queue(this) == true)
			this->reload_visu_state = 3;
		else
			this->reload_visu_state = 2;
	}
	else if(this->reload_visu_state == 2)
	{
		model_set_error_status(this->s7lib_ref);
		this->reload_visu_state = 4;
	}
	else if(this->reload_visu_state == 3)
	{
		model_set_done_status(this->s7lib_ref);
		this->reload_visu_state = 4;
	}
	else if(this->reload_visu_state == 4)
	{
		if(model_read_cmd_reload_visu_status(this->s7lib_ref) == false)
			this->reload_visu_state = 0;
	}
	else
	{
		this->reload_visu_state = 0;
	}
}



void controler_handler(controler * this)
{
	controler_init(this);

	while(true)
	{
		controler_enqueue_callback(this);
		controler_priority_enqueue_callback(this);
		controler_dequeue_callback(this);
		controler_delete_callback(this);
		controler_reload_visu_callback(this);

		fflush(stdout);
		usleep(200000);
	}
}

void controler_finalize(controler * this)
{
	s7lib_finalize(this->s7lib_ref);
	c_linked_list_clear_with_release(this->queue, glass_info_finalize);

	free(this);
}






static uint8_t * controler_copy_first_n_glass(c_linked_list * list, uint8_t * visu, int index)
{
	if(index < VISU_ITEM_NUMBERN)
	{
		if(list != NULL)
		{
			model_write_glass_info_to_array(visu, c_linked_list_get_data(list), index);
			return controler_copy_first_n_glass(c_linked_list_next(list), visu, index + 1);
		}
		else
		{
			uint8_t empty_glass[GLASS_SIZE];
			memset(empty_glass, 0, GLASS_SIZE);
			empty_glass[REAR_WINDOW_TYPE_BYTE_INDEX] = REAR_WINDOW_TYPE_LENGTH-2;

			model_write_glass_info_to_array(visu, (glass_info*) empty_glass, index);

			return controler_copy_first_n_glass(NULL, visu, index + 1);
		}
	}

	return visu;
}


static bool controler_synchronize_visu_queue(controler * this)
{
	uint8_t * visu_controler = controler_generate_visu_queue(this);
	bool ret_val = model_write_visu_queue(this->s7lib_ref, visu_controler, (int16_t) c_linked_list_size(this->queue));

	free(visu_controler);

	return ret_val;
}

static uint8_t * controler_generate_visu_queue(controler * this)
{
	uint8_t * visu = malloc(sizeof(uint8_t)*GLASS_SIZE*VISU_ITEM_NUMBERN);

	return controler_copy_first_n_glass(this->queue, visu, 0);
}


static char * controler_get_current_time_date(char * date_time)
{
	 time_t rawtime;
	 struct tm * timeinfo;

	 time (&rawtime);
	 timeinfo = localtime (&rawtime);

	 sprintf(date_time, "%.02d.%.02d.%d-%.02d:%.02d:%.02d", timeinfo->tm_mday, timeinfo->tm_mon, timeinfo->tm_year+1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	 return date_time;
}

