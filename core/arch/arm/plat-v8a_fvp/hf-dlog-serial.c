#include <types_ext.h>
#include <drivers/serial.h>
#include <hf/call.h>
static void flush(struct serial_chip *chip)
{
}

static bool have_rx_data(struct serial_chip *chip)
{
    return false;
	// vaddr_t base = chip_to_base(chip);

	// return !(io_read32(base + UART_FR) & UART_FR_RXFE);
}

static int getchar(struct serial_chip *chip)
{
    return 0;
}

static void putc(struct serial_chip *chip, int ch)
{
    ffa_console_log_64((char *)&ch, 1);
}
static const struct serial_ops s_ops = {
	.flush = flush,
	.getchar = getchar,
	.have_rx_data = have_rx_data,
	.putc = putc,
};
void init_serial_chip(struct serial_chip * chip){
    chip->ops = &s_ops;
}