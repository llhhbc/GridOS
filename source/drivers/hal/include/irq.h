#ifndef HAL_IRQ_H
#define HAL_IRQ_H

#include <types.h>
#include <ddk/irq.h>

/* 
	DPC Interrupt 
*/
enum
{
	HI_DPC_IRQ  = 0,
	TIMER_SOFTIRQ,
	
	NR_DPC_IRQS = BITS_PER_LONG
};
struct dpc_irq_action
{
	void (*action)(struct dpc_irq_action *);
};
//dpc.c
extern void raise_dpc_irq(unsigned int nr);
extern void open_dpc_irq(int nr, void (*action)(struct dpc_irq_action *));

/*
	IRQ Chip
*/
struct irq_chip;
struct irq_desc;

/* Dummy irq-chip implementations: */
extern struct irq_chip no_irq_chip;
extern struct irq_chip dummy_irq_chip;

typedef	void (*irq_flow_handler_t)(unsigned int irq,
					    struct irq_desc *desc);
struct irq_data;
struct irq_chip {
	const char	*name;
	unsigned int	(*irq_startup)(struct irq_data *data);
	void		(*irq_shutdown)(struct irq_data *data);
	void		(*irq_enable)(struct irq_data *data);
	void		(*irq_disable)(struct irq_data *data);

	void		(*irq_ack)(struct irq_data *data);
	void		(*irq_mask)(struct irq_data *data);
	void		(*irq_mask_ack)(struct irq_data *data);
	void		(*irq_unmask)(struct irq_data *data);
	void		(*irq_eoi)(struct irq_data *data);

	int		(*irq_retrigger)(struct irq_data *data);
	int		(*irq_set_type)(struct irq_data *data, unsigned int flow_type);
	int		(*irq_set_wake)(struct irq_data *data, unsigned int on);

	void		(*irq_bus_lock)(struct irq_data *data);
	void		(*irq_bus_sync_unlock)(struct irq_data *data);

	void		(*irq_cpu_online)(struct irq_data *data);
	void		(*irq_cpu_offline)(struct irq_data *data);

	void		(*irq_suspend)(struct irq_data *data);
	void		(*irq_resume)(struct irq_data *data);
	void		(*irq_pm_shutdown)(struct irq_data *data);

	unsigned long	flags;
};

enum {
	IRQ_TYPE_NONE		= 0x00000000,
	IRQ_TYPE_EDGE_RISING	= 0x00000001,
	IRQ_TYPE_EDGE_FALLING	= 0x00000002,
	IRQ_TYPE_EDGE_BOTH	= (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING),
	IRQ_TYPE_LEVEL_HIGH	= 0x00000004,
	IRQ_TYPE_LEVEL_LOW	= 0x00000008,
	IRQ_TYPE_LEVEL_MASK	= (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH),
	IRQ_TYPE_SENSE_MASK	= 0x0000000f,
	IRQ_TYPE_DEFAULT	= IRQ_TYPE_SENSE_MASK,

	IRQ_TYPE_PROBE		= 0x00000010,

	IRQ_LEVEL		= (1 <<  8),
	IRQ_PER_CPU		= (1 <<  9),
	IRQ_NOPROBE		= (1 << 10),
	IRQ_NOREQUEST		= (1 << 11),
	IRQ_NOAUTOEN		= (1 << 12),
	IRQ_NO_BALANCING	= (1 << 13),
	IRQ_MOVE_PCNTXT		= (1 << 14),
	IRQ_NESTED_THREAD	= (1 << 15),
	IRQ_NOTHREAD		= (1 << 16),
	IRQ_PER_CPU_DEVID	= (1 << 17),
};

#ifndef ARCH_IRQ_INIT_FLAGS
# define ARCH_IRQ_INIT_FLAGS	0
#endif
#define IRQ_DEFAULT_INIT_FLAGS	ARCH_IRQ_INIT_FLAGS
#define IRQF_MODIFY_MASK	\
	(IRQ_TYPE_SENSE_MASK | IRQ_NOPROBE | IRQ_NOREQUEST | \
	 IRQ_NOAUTOEN | IRQ_MOVE_PCNTXT | IRQ_LEVEL | IRQ_NO_BALANCING | \
	 IRQ_PER_CPU | IRQ_NESTED_THREAD | IRQ_NOTHREAD | IRQ_PER_CPU_DEVID)

#define IRQ_NO_BALANCING_MASK	(IRQ_PER_CPU | IRQ_NO_BALANCING)

/* Return value for chip->irq_set_affinity() */
enum {
	IRQ_SET_MASK_OK = 0,
	IRQ_SET_MASK_OK_NOCOPY,
};

/* 每个中断的动作的标志*/
#define IRQF_TRIGGER_NONE	0x00000000
#define IRQF_TRIGGER_RISING	0x00000001
#define IRQF_TRIGGER_FALLING	0x00000002
#define IRQF_TRIGGER_HIGH	0x00000004
#define IRQF_TRIGGER_LOW	0x00000008
#define IRQF_TRIGGER_MASK	(IRQF_TRIGGER_HIGH | IRQF_TRIGGER_LOW | \
				 IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING)
#define IRQF_TRIGGER_PROBE	0x00000010

#define IRQF_DISABLED		0x00000020
#define IRQF_PROBE_SHARED	0x00000100
#define __IRQF_TIMER		0x00000200
#define IRQF_PERCPU		0x00000400
#define IRQF_NOBALANCING	0x00000800
#define IRQF_IRQPOLL		0x00001000
#define IRQF_ONESHOT		0x00002000
#define IRQF_NO_SUSPEND		0x00004000
#define IRQF_FORCE_RESUME	0x00008000
#define IRQF_NO_THREAD		0x00010000
#define IRQF_EARLY_RESUME	0x00020000
#define IRQF_TIMER		(__IRQF_TIMER | IRQF_NO_SUSPEND | IRQF_NO_THREAD)

/* IRQ action */
struct irqaction {
	irq_handler_t		handler;
	void			*dev_id;
	void __percpu		*percpu_dev_id;
	struct irqaction	*next;
	irq_handler_t		thread_fn;
	struct task_struct	*thread;
	unsigned int		irq;
	unsigned int		flags;
	unsigned long		thread_flags;
	unsigned long		thread_mask;
	const char		*name;
	struct proc_dir_entry	*dir;
} ____cacheline_internodealigned_in_smp;


/* irq_chip specific flags */
enum {
	IRQCHIP_SET_TYPE_MASKED		= (1 <<  0),
	IRQCHIP_EOI_IF_HANDLED		= (1 <<  1),
	IRQCHIP_MASK_ON_SUSPEND		= (1 <<  2),
	IRQCHIP_ONOFFLINE_ENABLED	= (1 <<  3),
	IRQCHIP_SKIP_SET_WAKE		= (1 <<  4),
	IRQCHIP_ONESHOT_SAFE		= (1 <<  5),
};

struct irq_data {
	unsigned int		irq;
	unsigned long		hwirq;
	unsigned int		node;
	unsigned int		state_use_accessors;
	struct irq_chip		*chip;
	void			*handler_data;
	void			*chip_data;
};

enum {
	IRQD_TRIGGER_MASK		= 0xf,
	IRQD_SETAFFINITY_PENDING	= (1 <<  8),
	IRQD_NO_BALANCING		= (1 << 10),
	IRQD_PER_CPU			= (1 << 11),
	IRQD_AFFINITY_SET		= (1 << 12),
	IRQD_LEVEL			= (1 << 13),
	IRQD_WAKEUP_STATE		= (1 << 14),
	IRQD_MOVE_PCNTXT		= (1 << 15),
	IRQD_IRQ_DISABLED		= (1 << 16),
	IRQD_IRQ_MASKED			= (1 << 17),
	IRQD_IRQ_INPROGRESS		= (1 << 18),
};

static inline void irqd_mark_affinity_was_set(struct irq_data *d)
{
	d->state_use_accessors |= IRQD_AFFINITY_SET;
}

static inline u32 irqd_get_trigger_type(struct irq_data *d)
{
	return d->state_use_accessors & IRQD_TRIGGER_MASK;
}

/*
 * Must only be called inside irq_chip.irq_set_type() functions.
 */
static inline void irqd_set_trigger_type(struct irq_data *d, u32 type)
{
	d->state_use_accessors &= ~IRQD_TRIGGER_MASK;
	d->state_use_accessors |= type & IRQD_TRIGGER_MASK;
}

static inline bool irqd_is_level_type(struct irq_data *d)
{
	return d->state_use_accessors & IRQD_LEVEL;
}

static inline bool irqd_irq_disabled(struct irq_data *d)
{
	return d->state_use_accessors & IRQD_IRQ_DISABLED;
}

static inline bool irqd_irq_masked(struct irq_data *d)
{
	return d->state_use_accessors & IRQD_IRQ_MASKED;
}

static inline bool irqd_irq_inprogress(struct irq_data *d)
{
	return d->state_use_accessors & IRQD_IRQ_INPROGRESS;
}

//functions for irq
extern int can_request_irq(unsigned int irq, unsigned long irqflags);
extern void free_irq(unsigned int, void *);
extern void disable_irq(unsigned int irq);
extern void enable_irq(unsigned int irq);
extern int setup_irq(unsigned int irq, struct irqaction *new);
extern int request_threaded_irq(unsigned int irq, irq_handler_t handler,
	irq_handler_t thread_fn,
	unsigned long flags, const char *name, void *dev);

extern void disable_irq_nosync(unsigned int irq);
 
//chip.c
extern void handle_percpu_irq(unsigned int irq, struct irq_desc *desc);
extern void	irq_set_chip_and_handler_name(unsigned int irq, struct irq_chip *chip,
	irq_flow_handler_t handle, const char *name);
static inline void irq_set_chip_and_handler(unsigned int irq, struct irq_chip *chip,
					    irq_flow_handler_t handle)
{
	irq_set_chip_and_handler_name(irq, chip, handle, NULL);
}

extern void handle_level_irq(unsigned int irq, struct irq_desc *desc);

//handle.c
extern irqreturn_t no_action(int cpl, void *dev_id);

//hal.c private data, used by arch to link irq event to driver subsystem
extern int (*external_irq_handler)(void *pt_regs, int irq);

#endif
