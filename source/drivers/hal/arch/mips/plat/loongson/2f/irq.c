#include <loongson.h>
#include <compiler.h>
#include <types.h>
#include <hal_print.h>
#include <hal_debug.h>
#include <hal_bitops.h>

#include <private/interrupt.h>
#include <private/irq.h>

#include <asm/irq.h>
#include <asm/irq_cpu.h>
#include <asm/i8259.h>
#include <ASM/MIPSREGs.h>

#define LOONGSON_TIMER_IRQ	(MIPS_CPU_IRQ_BASE + 7)	/* cpu timer */
#define LOONGSON_NORTH_BRIDGE_IRQ	(MIPS_CPU_IRQ_BASE + 6)	/* bonito */
#define LOONGSON_UART_IRQ	(MIPS_CPU_IRQ_BASE + 3)	/* cpu serial port */
#define LOONGSON_SOUTH_BRIDGE_IRQ	(MIPS_CPU_IRQ_BASE + 2)	/* i8259 */

#define LOONGSON_INT_BIT_INT0		(1 << 11)
#define LOONGSON_INT_BIT_INT1		(1 << 12)

/*
 * The generic i8259_irq() make the kernel hang on booting.  Since we cannot
 * get the irq via the IRR directly, we access the ISR instead.
 */
int mach_i8259_irq(void)
{
	int irq, isr;

	irq = -1;

	if ((LOONGSON_INTISR & LOONGSON_INTEN) & LOONGSON_INT_BIT_INT0) {
		raw_spin_lock(&i8259A_lock);
		isr = inb(PIC_MASTER_CMD) &
			~inb(PIC_MASTER_IMR) & ~(1 << PIC_CASCADE_IR);
		if (!isr)
			isr = (inb(PIC_SLAVE_CMD) & ~inb(PIC_SLAVE_IMR)) << 8;
		irq = ffs(isr) - 1;
		if (unlikely(irq == 7)) {
			/*
			 * This may be a spurious interrupt.
			 *
			 * Read the interrupt status register (ISR). If the most
			 * significant bit is not set then there is no valid
			 * interrupt.
			 */
			outb(0x0B, PIC_MASTER_ISR);	/* ISR register */
			if (~inb(PIC_MASTER_ISR) & 0x80)
				irq = -1;
		}
		raw_spin_unlock(&i8259A_lock);
	}

	return irq;
}

static void i8259_irqdispatch(void)
{
	int irq;

	irq = mach_i8259_irq();
	if (irq >= 0)
		do_IRQ(irq);
	else
		spurious_interrupt();
}

void irq_dispatch_2f(unsigned int pending)
{
	if (pending & CAUSEF_IP7)
		do_IRQ(LOONGSON_TIMER_IRQ);
	else if (pending & CAUSEF_IP6) {	/* North Bridge, Perf counter */
		//do_perfcnt_IRQ();
		bonito_irqdispatch();
	} else if (pending & CAUSEF_IP3)	/* CPU UART */
		do_IRQ(LOONGSON_UART_IRQ);
	else if (pending & CAUSEF_IP2)	/* South Bridge */
		i8259_irqdispatch();
	else
		spurious_interrupt();
}

static irqreturn_t ip6_action(int cpl, void *dev_id)
{
	return IRQ_HANDLED;
}

struct irqaction ip6_irqaction = {
	.handler = ip6_action,
	.name = "cascade",
	.flags = IRQF_SHARED | IRQF_NO_THREAD,
};

struct irqaction cascade_irqaction = {
	.handler = no_action,
	.name = "cascade",
	.flags = IRQF_NO_THREAD,
};

void __init init_irq_2f(void)
{
	unsigned int mask;
	/* init all controller
	 *   0-15         ------> i8259 interrupt
	 *   16-23        ------> mips cpu interrupt
	 *   32-63        ------> bonito irq
	 */

	/* setup cs5536 as high level trigger */
	LOONGSON_INTPOL = LOONGSON_INT_BIT_INT0 | LOONGSON_INT_BIT_INT1;
	LOONGSON_INTEDGE &= ~(LOONGSON_INT_BIT_INT0 | LOONGSON_INT_BIT_INT1);

	/* Sets the first-level interrupt dispatcher. */
	mips_cpu_irq_init();
	init_i8259_irqs();
	bonito_irq_init();
	/* setup north bridge irq (bonito) */
	setup_irq(LOONGSON_NORTH_BRIDGE_IRQ, &ip6_irqaction);
	/* setup source bridge irq (i8259) */
	setup_irq(LOONGSON_SOUTH_BRIDGE_IRQ, &cascade_irqaction);
}