/**
 * @brief Unipolar Stepper Motor Sequencer
 * @date May 7, 2017
 * @author Tennessee Carmel-Veilleux <tcv -at- ro.boto.ca>
 * @copyright 2017, Tennessee Carmel-Veilleux.
 * @license MIT license, see LICENSE file
 */
 
#pragma once

#include <Arduino.h>

typedef void (*freq_setter_t)(uint16_t freq);

class UnipolarSequencer {
public:
    UnipolarSequencer(int pin_a, int pin_b, int pin_c, int pin_d, int16_t speed_max_pps, 
                      uint32_t sched_period_us, freq_setter_t freq_setter);
    virtual ~UnipolarSequencer();
    
    void set_speed_pps(int16_t speed_pps);
    uint16_t get_speed_max_pps(void);
    
    void set_cw_positive(bool cw_positive);
    void set_half_step(bool use_half_step);
    bool is_half_step(void);
    bool is_cw_positive(void);
    
    void step_cw(void);
    void step_ccw(void);
    void step_idle(void);
    
    void kick_sched(void);
    void kick_pulse(void);

protected:
    void step(bool cw);

    bool _is_half_step;
    bool _is_cw_positive;
    int _idx;
    int _pin_a;
    int _pin_b;
    int _pin_c;
    int _pin_d;
    volatile int16_t _speed_max_pps;
    volatile int16_t _speed_target_pps;
    volatile int16_t _speed_current_pps;
    uint32_t _sched_period_us;
    freq_setter_t _freq_setter;
};
