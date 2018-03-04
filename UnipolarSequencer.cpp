/**
 * @brief Unipolar Stepper Motor Sequencer
 * @date May 7, 2017
 * @author Tennessee Carmel-Veilleux <tcv -at- ro.boto.ca>
 * @copyright 2017-2018, Tennessee Carmel-Veilleux.
 * @license MIT license, see LICENSE file
 */

#include "UnipolarSequencer.hpp"

UnipolarSequencer::UnipolarSequencer(int pin_a, int pin_b, int pin_c, int pin_d, 
                    int16_t speed_max_pps, uint32_t sched_period_us, freq_setter_t freq_setter):
    _is_half_step(false),
    _is_cw_positive(true),
    _idx(7), 
    _pin_a(pin_a), _pin_b(pin_b), _pin_c(pin_c), _pin_d(pin_d),
    _speed_max_pps(speed_max_pps),
    _sched_period_us(sched_period_us),
    _freq_setter(freq_setter)
{
}

UnipolarSequencer::~UnipolarSequencer() {
}
    
void UnipolarSequencer::set_cw_positive(bool cw_positive) {
    _is_cw_positive = cw_positive;
}

bool UnipolarSequencer::is_cw_positive(void) {
    return _is_cw_positive;
}

uint16_t UnipolarSequencer::get_speed_max_pps(void) {
    return _speed_max_pps;
}

void UnipolarSequencer::set_half_step(bool use_half_step) {
    _is_half_step = use_half_step;
}

bool UnipolarSequencer::is_half_step() {
    return _is_half_step;
}
    
void UnipolarSequencer::step_cw() {
    step(true);
}

void UnipolarSequencer::step_ccw() {
    step(false);    
}

void UnipolarSequencer::step_idle() {
    digitalWrite(_pin_a, LOW);
    digitalWrite(_pin_b, LOW);
    digitalWrite(_pin_c, LOW);
    digitalWrite(_pin_d, LOW);
}

void UnipolarSequencer::step(bool cw) {
    uint32_t new_state_a = LOW;
    uint32_t new_state_b = LOW;
    uint32_t new_state_c = LOW;
    uint32_t new_state_d = LOW;

    switch(_idx) {
        case 0:
            new_state_a = 0;
            new_state_b = 0;
            new_state_c = 0;
            new_state_d = 1;
            break;
        case 1:
            new_state_a = 0;
            new_state_b = 0;
            new_state_c = 1;
            new_state_d = 1;
            break;
        case 2:
            new_state_a = 0;
            new_state_b = 0;
            new_state_c = 1;
            new_state_d = 0;
            break;
        case 3:
            new_state_a = 0;
            new_state_b = 1;
            new_state_c = 1;
            new_state_d = 0;
            break;
        case 4:
            new_state_a = 0;
            new_state_b = 1;
            new_state_c = 0;
            new_state_d = 0;
            break;
        case 5:
            new_state_a = 1;
            new_state_b = 1;
            new_state_c = 0;
            new_state_d = 0;
            break;
        case 6:
            new_state_a = 1;
            new_state_b = 0;
            new_state_c = 0;
            new_state_d = 0;
            break;
        case 7:
            new_state_a = 1;
            new_state_b = 0;
            new_state_c = 0;
            new_state_d = 1;
            break;
        default:
            new_state_a = 0;
            new_state_b = 0;
            new_state_c = 0;
            new_state_d = 0;
            break;
    }
    
    // Update GPIOs to new states
    digitalWrite(_pin_a, new_state_a);
    digitalWrite(_pin_b, new_state_b);
    digitalWrite(_pin_c, new_state_c);
    digitalWrite(_pin_d, new_state_d);

    // Find next index in state chart for next step.
    if (cw) {
        _idx -= _is_half_step ? 1 : 2;
    } else {
        _idx += _is_half_step ? 1 : 2;
    }
    
    if (_idx > 7) {
        _idx = _is_half_step ? 0 : 1;
    } else if (_idx < 0) {
        _idx = 7;
    }
}

void UnipolarSequencer::set_speed_pps(int16_t speed_pps) {
    if (speed_pps < -_speed_max_pps) {
        speed_pps = -_speed_max_pps;
    } else if (speed_pps > _speed_max_pps) {
        speed_pps = _speed_max_pps;
    }
    _speed_target_pps = speed_pps;
}

#ifndef abs
#define abs(_x) (((_x) < 0) ? -(_x) : (_x))
#endif

void UnipolarSequencer::kick_sched(void) {
    noInterrupts();
    int16_t target = _speed_target_pps;
    interrupts();

    if (_speed_current_pps == target) {
        return;
    }
    
    // TODO: Implement acceleration
    _speed_current_pps = target;

    if (NULL != _freq_setter) {
        _freq_setter((uint16_t)abs(_speed_current_pps));
    }

    // In case of stopped motor, immediately move to idle as the
    // timer will be hitting very slowly.
    if (0 == _speed_current_pps) {
        this->step_idle();
    }
}

void UnipolarSequencer::kick_pulse(void) {
    int16_t current_speed = _speed_current_pps;
    if (0 == current_speed) {
        this->step_idle();
    } else if (current_speed < 0) {
        if (_is_cw_positive) {
            this->step_ccw();
        } else {
            this->step_cw();
        }
    } else {
        if (_is_cw_positive) {
            this->step_cw();
        } else {
            this->step_ccw();
        }
    }
}