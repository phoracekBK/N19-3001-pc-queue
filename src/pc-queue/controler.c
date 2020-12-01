#include "include/controler.h"
#include "include/config.h"
#include <time.h>
#include <stdio.h>


struct _queue_
{
	glass_info * array;
	uint16_t size;
};




typedef struct _queue_ queue;


struct _controler_
{
	s7lib * s7lib_ref;

	queue * glass_list;
	visu * visual_queue;

	uint8_t state;
	uint8_t state_last_cycle;
};





static void controler_init(controler *);
static queue * controler_read_file(FILE *);
static queue * controler_load_from_file(controler *, char *);
static bool controler_save_to_file(controler *, char *);
static queue * controler_enqueue(queue * glass_list, glass_info * glass_record);
static visu * sync_visu(queue * glass_list, visu * visual_queue);
static queue * controler_priority_enqueue(queue * glass_list, glass_info * glass_record);
static queue * controler_dequeue(queue * glass_list);
static void controler_work_sequence_callback(controler *);




controler * controler_new(char * ip_address, int rack, int slot, int db_index)
{
	controler * this = malloc(sizeof(controler));

	this->s7lib_ref = s7lib_new(ip_address, rack, slot, db_index);

	this->glass_list = controler_load_from_file(this, QUEUE_FILE_PATH);
	this->visual_queue = malloc(sizeof(visu));
	this->visual_queue = sync_visu(this->glass_list, this->visual_queue);

	this->state = 0;
	this->state_last_cycle = 255;

	return this;
}


static queue * controler_read_file(FILE * input)
{
	fseek(input, 0L, SEEK_END);
	uint32_t file_size = ftell(input);
	fseek(input, 0L, SEEK_SET);

	queue * glass_list = malloc(sizeof(queue));

	if(file_size > 0)
	{
		glass_list->array = malloc(file_size);
		glass_list->size = file_size / sizeof(glass_info);

		fread(glass_list->array, 1, file_size, input);
	}
	else
	{
		glass_list->array = NULL;
		glass_list->size = 0;
	}

	return glass_list;
}


static queue * controler_load_from_file(controler * this, char * address)
{
	FILE * input = fopen(address, "r");

	if(input != NULL)
	{
		queue * glass_list = controler_read_file(input);
		fclose(input);

		return glass_list;
	}
	else
	{
		queue * glass_list = malloc(sizeof(queue));

		glass_list->array = NULL;
		glass_list->size = 0;

		return glass_list;
	}
}


static bool controler_save_to_file(controler * this, char * address)
{
	if(this->glass_list->array != NULL)
	{
		FILE * output = fopen(address, "w");

		if(output != NULL)
		{
			fwrite((uint8_t*) this->glass_list->array, this->glass_list->size*sizeof(glass_info), 1, output);
			fclose(output);

			return true;
		}

		return false;
	}
	else
	{
		if(remove (address) == 0)
			return true;
		else
			return false;
	}
}


static void controler_init(controler * this)
{
	printf("Initializing...\n");

	if(model_write_visu_queue(this->s7lib_ref, (uint8_t*) this->visual_queue) == true)
		printf("visual queue synchronization success\n");
	else
		printf("visual queue synchronization error\n");


	printf("waiting for request\n");

	fflush(stdout);
}


static char * controler_time_string(const char * format)
{
	char * time_string = (char*) malloc(sizeof(char)*23);

    time_t my_time;
    struct tm* time_info;

    time(&my_time);
    time_info = localtime(&my_time);
    strftime(time_string, 23, format, time_info);

	return time_string;
}


static void controler_log(char * format, ...)
{
	va_list params;
	va_start(params, format);

	char * time_stamp = controler_time_string("%d.%m.%y - %H:%M:%S");
	printf("%s - ", time_stamp);
	vprintf(format, params);
	printf("\n");

	fflush(stdout);

	free(time_stamp);
	va_end(params);
}


static void controler_work_sequence_callback(controler * this)
{
	if(this->state == 0)
	{
		uint8_t * byte = model_read_cmd_byte(this->s7lib_ref);

		if(byte != NULL)
		{
			if(model_read_cmd_reload_visu_status(byte) == true)
				this->state = 10;
			else if(model_read_cmd_enqueue_status(byte) == true)
				this->state = 30;
			else if(model_read_cmd_priority_enqueue_status(byte) == true)
				this->state = 40;
			else if(model_read_cmd_dequeue_status(byte) == true)
				this->state = 50;
			else
				usleep(100000);

			free(byte);
		}
	}
	else if(this->state == 10)
	{
		controler_log("synchronizing queue...");

		this->visual_queue =  sync_visu(this->glass_list, this->visual_queue);
		this->state = 20;
	}
	else if(this->state == 20)
	{
		 controler_log("reloading visual queue");

		 if(model_write_visu_queue(this->s7lib_ref, (uint8_t *) this->visual_queue) == true)
			 this->state = 190;
		 else
			 this->state = 255;
	}
	else if(this->state == 30)
	{
		glass_info * glass_record = model_read_glass_info(this->s7lib_ref, GLASS_BYTE);

		if(glass_record != NULL)
		{
			controler_log("enqueue glass record: %s", glass_record->vehicle_number);

			this->glass_list = controler_enqueue(this->glass_list, glass_record);
			this->state = 100;

			free(glass_record);
		}
		else
		{
			this->state = 255;
			controler_log("can't load glass record for enqueue!");
		}
	}
	else if(this->state == 40)
	{
		glass_info * glass_record = model_read_glass_info(this->s7lib_ref, GLASS_BYTE);

		if(glass_record != NULL)
		{
			controler_log("priority enqueue glass record: %s", glass_record->vehicle_number);

			this->glass_list = controler_priority_enqueue(this->glass_list, glass_record);
			this->state = 100;
		}
		else
		{
			controler_log("can't load glass record for priority enqueue!");
			this->state = 255;
		}

		free(glass_record);
	}
	else if(this->state == 50)
	{
		if(this->glass_list->array != NULL)
		{
			controler_log("dequeue glass record: %s", this->glass_list->array[0].vehicle_number);

			this->glass_list = controler_dequeue(this->glass_list);
			this->state = 100;
		}
		else
		{
			controler_log("enqueue error, queue is empty!");
			this->state = 255;
		}
	}
	else if(this->state == 100)
	{
		if(controler_save_to_file(this, QUEUE_FILE_PATH) == true)
		{
			controler_log("queue saved to file");
			this->state = 10;
		}
		else
		{
			controler_log("error during saving queue to file!");
			this->state = 255;
		}
	}
	else if(this->state == 190)
	{
		controler_log("set done flag");

		if(model_set_done_status(this->s7lib_ref) == true)
			this->state = 200;
		else
			this->state = 255;
	}
	else if(this->state == 200)
	{
		controler_log("waiting for finish");
		this->state = 201;
	}
	else if(this->state == 201)
	{
		uint8_t * byte = model_read_cmd_byte(this->s7lib_ref);

		if(byte != NULL)
		{
			if(*byte == 0)
			{
				controler_log("request finished");
				model_reset_done_status(this->s7lib_ref);

				this->state = 0;
			}
			else
			{
				usleep(100000);
			}

			free(byte);
		}
	}
	else
	{
		controler_log("set error flag");
		model_set_error_status(this->s7lib_ref);
		this->state = 0;
	}

	this->state_last_cycle = this->state;
}


void controler_handler(controler * this)
{
	controler_init(this);

	while(true)
	{
		controler_work_sequence_callback(this);
	}
}


void controler_finalize(controler * this)
{
	s7lib_finalize(this->s7lib_ref);

	free(this->glass_list->array);
	free(this->glass_list);


	free(this);
}


static void * merge(uint8_t * prefix, uint32_t prefix_size, uint8_t * postfix, uint32_t postfix_size)
{
	if(prefix != NULL && postfix != NULL)
	{
		uint8_t * merge_array = malloc(prefix_size + postfix_size+1);

		for(uint32_t i = 0; i < prefix_size+postfix_size; i++)
		{
			if(i < (prefix_size))
				merge_array[i] = prefix[i];
			else
				merge_array[i] = postfix[i-prefix_size];
		}

		return merge_array;
	}
	else if(prefix == NULL && postfix != NULL)
	{
		uint8_t * merge_array = malloc(postfix_size);

		for(int i = 0; i < postfix_size; i++)
		{
			merge_array[i] = postfix[i];
		}

		return merge_array;
	}
	else if(prefix != NULL && postfix == NULL)
	{
		uint8_t * merge_array = malloc(prefix_size);

		for(int i = 0; i < prefix_size; i++)
		{
			merge_array[i] = prefix[i];
		}

		return merge_array;
	}
	else
	{
		return NULL;
	}
}


static uint8_t * sub(uint8_t * array, uint32_t start, uint32_t end)
{
	if(array != NULL && start < end)
	{
		uint8_t * sub_array = malloc(end-start);

		for(int i = start; i < end; i++)
		{
			sub_array[i-start] = array[i];
		}

		return sub_array;
	}
	else
	{
		return NULL;
	}
}


static queue * controler_enqueue(queue * glass_list, glass_info * glass_record)
{
	glass_info * tmp = (glass_info*) merge((uint8_t*) glass_list->array, sizeof(glass_info)*glass_list->size, (uint8_t*) glass_record, sizeof(glass_info));

	free(glass_list->array);

	glass_list->array = tmp;
	glass_list->size++;

	return glass_list;
}


static queue * controler_priority_enqueue(queue * glass_list, glass_info * glass_record)
{
	glass_info * tmp = (glass_info*) merge((uint8_t*) glass_record, sizeof(glass_info), (uint8_t*) glass_list->array, sizeof(glass_info)*glass_list->size);

	free(glass_list->array);

	glass_list->array = tmp;
	glass_list->size ++;

	return glass_list;
}


static queue * controler_dequeue(queue * glass_list)
{
	if(glass_list->size > 0 && glass_list->array != NULL)
	{
		glass_info * tmp = (glass_info*) sub((uint8_t*) glass_list->array, sizeof(glass_info), sizeof(glass_info)*(glass_list->size));

		free(glass_list->array);

		glass_list->array = tmp;
		glass_list->size --;
	}

	return glass_list;
}


static visu * sync_visu(queue * glass_list, visu * visual_queue)
{
	s7lib_parser_write_int((uint8_t *) visual_queue, sizeof(glass_info)*10, glass_list->size);

	for(int i = 0; i < sizeof(glass_info)*10; i++)
	{
		if(i < (sizeof(glass_info)*glass_list->size))
			((uint8_t*) visual_queue->visual_queue)[i] = ((uint8_t*) glass_list->array)[i];
		else
			((uint8_t*) visual_queue->visual_queue)[i] = 0;

		if((i%sizeof(glass_info)) == 0)
			visual_queue->visual_queue[i/sizeof(glass_info)].rear_window_type[0] = REAR_WINDOW_TYPE_LENGTH-2;
	}

	return visual_queue;
}






