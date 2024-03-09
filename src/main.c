#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "i2s_m.pio.h"
#include <math.h>


void set_pio_freq(pio_sm_config* sm_config, int target_freq) {
    float clkdiv = (float)clock_get_hz(clk_sys) / target_freq;
    sm_config_set_clkdiv(sm_config, clkdiv);
}

int main() {
    stdio_init_all();

    PIO pio = pio0;
    int sm = pio_claim_unused_sm(pio, true);

    int pin_dout = 10;
    int pin_bck = pin_dout + 1;
    int pin_lrck = pin_bck + 1;
    unsigned int bck_freq = 3072000;

    uint32_t offset = pio_add_program(pio, &i2s_m_program);

    pio_sm_config sm_config = i2s_m_program_get_default_config(offset);
    sm_config_set_out_shift(&sm_config, false, false, 32); // msb first
    sm_config_set_out_pins(&sm_config, pin_dout, 1);
    sm_config_set_set_pins(&sm_config, pin_dout, 1);
    sm_config_set_sideset_pins(&sm_config, pin_bck);
    set_pio_freq(&sm_config, bck_freq * 2); // pio bck output frequency is half of the target frequency

    pio_sm_set_consecutive_pindirs(pio, sm, pin_dout, 3, true);
    pio_gpio_init(pio, pin_dout);
    pio_gpio_init(pio, pin_bck);
    pio_gpio_init(pio, pin_lrck);

    pio_sm_init(pio, sm, offset, &sm_config);
    pio_sm_set_enabled(pio, sm, true);
    
    int freqs[] = {440, 493, 523, 587, 659, 698, 783, 880};
    int freq;
    int val;
    bool empty;
    int acc_empties;
    int loop_count = 0;
    for (;;) {
        acc_empties = 0;

        freq = freqs[time_us_32() / 1000000 % 8];
        // generate 32-bit sawtooth wave, not using sin()
        val = (int)(2147483647 * (2 * (time_us_32() % (1000000 / freq)) * (float)freq / 1000000 - 1));
        val /= 10;
        
        empty = pio_sm_is_tx_fifo_empty(pio, sm);
        acc_empties += empty;
        pio_sm_put_blocking(pio, sm, val);
        empty = pio_sm_is_tx_fifo_empty(pio, sm);
        acc_empties += empty;
        pio_sm_put_blocking(pio, sm, val);

        loop_count++;
        if (loop_count % 1000 == 0) {
            printf("empty rate: %f\n", (float)acc_empties / 2000);
            acc_empties = 0;
            loop_count = 0;
        }
    }
}
