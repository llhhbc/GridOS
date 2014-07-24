/**
	The Grid Core Library
 */

/**
	Message slots
 */

#include <types.h>

#include "message.h"

/**
	@brief ����������Ϣ��״̬��ʼ��
 
	@return
		The next slots after count
 */
static struct y_message *message_clean(struct y_message_instance *instance, struct y_message *start, int count)
{
	struct y_message * cur = start;
	count = __add_msg_count__(count);
	
	/* ����������ռ��slot ��flag ���������ѭ������ msg count */
	while (count > 0)
	{
		cur->flags = 0;
		MSG_GET_NEXT_SLOT(instance, cur);
		count --;
	}

	return cur;
}

/**
	@brief ������Ϣ
 
	�����Ϣ��ͬ���ģ���Ҫ�����ں�ȥ��Ӧ����Ϊ�������ڵȴ����ǵĻ�Ӧ��
	�����Ϣ���첽�ģ���ֱ�����ռ��λ���ɡ�
 
	@return
		The next postion to fetch the message.The message length is dynmaic!
 */
static struct y_message *handle_message(struct y_message_instance *instance, struct y_message *what)
{
	int count;
	int r = true;
	struct y_message * next;
	
	if (instance->filter) r = instance->filter(what);
	count = what->count;										//record the src count before handling message, sync msg will overwrite it.
	
	if (r == true)
	{
		
		/*
		 1����ַ�����͵ģ�
		 2����ô���ǲ�ѯ��
		 */
		if (what->flags & MSG_FLAGS_ADDRESS)
		{
			y_message_func fn;
			fn = (y_message_func)what->what;
			fn(what);
		}
		else
		{
			y_message_func fn;
			if (instance->find_handler)
			{
				fn = instance->find_handler(instance, what->what);
				if (fn)
					fn(what);
			}
		}
	}
	
	/*
		ACK the sender if it's a synchronouse message
	 */
	if (what->flags & MSG_FLAGS_SYNC)
	{
		/*
		 The sender is waiting in kernel,
		 so let kernel know the message has bean handled and
		 let the sender copy back the result.
		 When these are all done, we can safely clean the message.
		 */
		instance->response_sync(what);
	}
	
	/*
		Clear the message.
		The head is cleaned when the data part has been cleaned.
	 */
	next = what;
	MSG_GET_NEXT_SLOT(instance, next);
	next = message_clean(instance, next, count);
	what->flags = 0;

	return next;
}

/**
	@brief ��������Ϣ��״̬��ʼ��
 */
void message_reset_all(struct y_message_instance *instance)
{
	struct y_message *cur = instance->slots;
	
	while(1)
	{
		cur->flags = 0;
		MSG_GET_NEXT_SLOT(instance, cur);
		if (cur == instance->slots)
			break;
	}
}

/**
	@brief �ȴ�һ����Ϣ��������
 */
void message_loop(struct y_message_instance *instance)
{
	struct y_message *cur;

	instance->sleep(instance);
	cur = instance->slots;

	while(1)
	{
		/*
			Check for first loop
		 */
		while (cur->flags & MSG_STATUS_NEW)
			cur = handle_message(instance, cur);

		/*
			Ŀǰ,���û����Ϣ,���ߵ�һ�ο��ܲ��ɹ�,��Ϊ������wakeup����������ô����Ҫȥ�������
			���,�������ϻص�������CUR��״̬,���Ϊ�ս������ǵڶ�������һ���ǳɹ���.
		 */
		instance->sleep(instance);
	}
}