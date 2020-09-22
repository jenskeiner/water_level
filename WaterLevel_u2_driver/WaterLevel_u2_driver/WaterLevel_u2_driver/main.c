#include <atmel_start.h>
#include <atomic.h>

#define CHAN_CNT  8

volatile uint8_t slave_address;
volatile uint8_t register_address;
volatile uint8_t num_addresses;
volatile uint8_t num_reads;
volatile uint8_t num_writes;
volatile uint8_t num_stops;

extern volatile uint8_t measurement_done_touch;
uint8_t level_status = 0;

volatile uint16_t raw_value[CHAN_CNT];
int i;

void touch_status_display1(void)
{
    for (i = 0 ; i < CHAN_CNT; i++){
        raw_value[i] = get_sensor_node_signal(i);
    }
}

volatile uint8_t read_flag = 0;
volatile uint8_t j;

void address_handler(){
    slave_address = I2C_0_read();
    I2C_0_send_ack();	// or send_nack() if we don't want to ack the address
    num_addresses++;
    read_flag = 0;
    j = 0;
}

void read_handler(){
    if (j == 0) {
        I2C_0_write((uint8_t)(raw_value[read_flag] & 0x00FF));
    } else {
        I2C_0_write((uint8_t)((raw_value[read_flag] & 0xFF00) >> 8));
    }
    j = (j + 1) % 2;
    if (j == 0) {
        read_flag = (read_flag + 1) % CHAN_CNT;
    }
    num_reads++;
}

void write_handler(){  // Master write handler
    register_address = I2C_0_read();
    I2C_0_send_ack();	// or send_nack() if we don't want to receive more data
    num_writes++;
}

void stop_handler(){
    num_stops++;
}

void error_handler(){
    while(1);
}


int main(void)
{
    /* Initializes MCU, drivers and middleware */
    atmel_start_init();
    ENABLE_INTERRUPTS();

    I2C_0_set_read_callback(read_handler);
    I2C_0_set_write_callback(write_handler);
    I2C_0_set_address_callback(address_handler);
    I2C_0_set_stop_callback(stop_handler);
    I2C_0_set_collision_callback(error_handler);
    I2C_0_set_bus_error_callback(error_handler);
    I2C_0_open();

    cpu_irq_enable();
    /* Replace with your application code */
    while (1) {

    touch_process();
    if (measurement_done_touch == 1) {
    touch_status_display1();
    }
    for(int i = 0; i < 30000; i++){}
    }
}
