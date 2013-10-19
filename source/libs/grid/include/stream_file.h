/**
	The Grid Core Library
 */

/**
	Posix file
	ZhaoYu,Yaosihai
 */

#ifndef __STRING_FILE_H__
#define __STRING_FILE_H__

struct stdio_file;

/* We are operating on the file exclusively */
#define LOCK_FILE(F)
#define UNLOCK_FILE(F)

/************************************************************************/
/* Stream file buffer configuration                                     */
/************************************************************************/
#define BUF_SIZE				(1024 * 1024 * 4)
#define BUF_BLOCK_SIZE			(1024 * 256)
#define BUF_BLOCK_NUM			((BUF_SIZE) / (BUF_BLOCK_SIZE))

#define	BUF_BLOCK_VALID_FLAG	0x0001
#define BUF_BLOCK_DIRTY_FLAG	0x0002
#define BUF_BLOCK_BUSY_FLAG		0x0004

#define	BUF_BLOCK_ID(pos)		((pos) / (BUF_BLOCK_SIZE))
#define BUF_BLOCK_OFFSET(pos)	((pos) % (BUF_BLOCK_SIZE))

struct buffer_block
{
	int						valid_size;					/* ��Ч���ݴ�С����󲻳���size*/
	int						pre_id;
	int						access_count;
	volatile unsigned int	flags;						/* CPU֮��ȴ�flags */
	void					*base;
	struct stdio_file		*file;						/* file�ֶβ�ΪNULL˵����buffer_block�ѱ�ռ�� */
};

struct stdio_file
{
	int								flags;
	int								fd;					/* file descriptor */
	uoffset							pos;				/* ��BUF_BLOCK_ID(pos)��BUF_BLOCK_OFFSET(pos)���ʹ�� */
	uoffset							size;
	unsigned long					lock;
	struct buffer_block				*block;
	const struct file_operations	*ops;
	void							*private_file;
};

void stream_file_init_ops(struct stdio_file *file);
bool stream_file_buffer_init();

#endif