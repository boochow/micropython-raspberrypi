#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"
#include "rpi.h"
#include "bcm283x_it.h"

/*
   if mode of a timer is FREE, MicroPython will 
   never change the timer compare register.
   (But it would be updated by VCORE.)
 */
typedef enum _timer_mode_t {
    ONE_SHOT = 0,
    PERIODIC = 1,
    FREE     = 2,
} timer_mode_t;

typedef struct _machine_timer_obj_t {
    const mp_obj_base_t base;
    const uint32_t id;
    bool active;
    uint32_t counter;
    uint32_t period;
    timer_mode_t mode;
    mp_obj_t callback;
} machine_timer_obj_t;

static machine_timer_obj_t machine_timer_obj[] = {
    {{&machine_timer_type}, 0, true, 0, 0, FREE, NULL},
    {{&machine_timer_type}, 1, false, 0, 0, PERIODIC, NULL},
    {{&machine_timer_type}, 2, true, 0, 0, FREE, NULL},
    {{&machine_timer_type}, 3, false, 0, 0, PERIODIC, NULL},
};

static void timer_enable(const int num) {
    if ((IRQ_ENABLE1 & IRQ_SYSTIMER(num)) == 0) {
        machine_timer_obj[num].active = true;
        if (machine_timer_obj[num].mode != FREE) {
            systimer->C[num] = systimer->CLO + machine_timer_obj[num].period;
        }
        systimer->CS |= (1 << num);
        IRQ_ENABLE1 = IRQ_SYSTIMER(num);
    }
}

static void timer_disable(const int num) {
    IRQ_DISABLE1 = IRQ_SYSTIMER(num);
    machine_timer_obj[num].active = false;
}

void isr_irq_timer(void) {
    int timer_value;

    if (IRQ_PEND1 & IRQ_SYSTIMER(0)) {
        timer_value = systimer->C[0];
    } else if (IRQ_PEND1 & IRQ_SYSTIMER(1)) {
        timer_value = systimer->C[1];
    } else if (IRQ_PEND1 & IRQ_SYSTIMER(2)) {
        timer_value = systimer->C[2];
    } else if (IRQ_PEND1 & IRQ_SYSTIMER(3)) {
        timer_value = systimer->C[3];
    } else {
        return;
    }
    /* schedule callback func for timers which value == timer_value */
    machine_timer_obj_t *tim = &machine_timer_obj[0];
    for (int i = 0; i < 4; i++) {
        if (timer_value == systimer->C[i]) {
            if (tim[i].active) {
                tim[i].counter++;
                if (tim[i].callback != mp_const_none) {
                    mp_sched_schedule(tim[i].callback, &tim[i]);
                }
                if (tim[i].mode == PERIODIC) {
                    systimer->C[i] += tim[i].period;
                } else if (tim[i].mode == ONE_SHOT) {
                    timer_disable(i);
                } else {
                    // FREE mode
                    tim[i].period = systimer->C[i];
                }
            }
            systimer->CS |= (1 << i);
        }
    }
}

STATIC mp_obj_t machine_timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    int id = mp_obj_get_int(args[0]);
    if (id > 3) {
        mp_raise_ValueError("invalid timer number");
    }
    machine_timer_obj_t *tim = (machine_timer_obj_t*) &machine_timer_obj[id];
    return tim;
}

STATIC mp_obj_t machine_timer_init_helper(machine_timer_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_period,       MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0xffffffff} },
        { MP_QSTR_mode,         MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_callback,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    self->period = args[0].u_int;
    self->mode = args[1].u_int;
    self->callback = args[2].u_obj;
    self->counter = 0;
    timer_enable(self->id);

    return mp_const_none;
}

STATIC mp_obj_t machine_timer_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_timer_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_timer_init_obj, 0, machine_timer_init);

STATIC mp_obj_t machine_timer_deinit(mp_obj_t self_in) {
    machine_timer_obj_t *self = self_in;
    timer_disable(self->id);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_deinit_obj, machine_timer_deinit);

STATIC void machine_timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_timer_obj_t *self = self_in;
    mp_printf(print, "Timer(%u, period=%u", self->id, self->period);
    qstr mode_qstr = MP_QSTR_ONE_SHOT;
    if (self->mode == PERIODIC) {
        mode_qstr = MP_QSTR_PERIODIC;
    } else if (self->mode == FREE) {
        mode_qstr = MP_QSTR_FREE;
    }
    mp_printf(print, ", mode=Timer.%s", qstr_str(mode_qstr));
    mp_printf(print, ", counter=%u)", self->counter);
    mp_printf(print, " # compare=%u", systimer->C[self->id]);
    if (!self->active) {
        mp_printf(print, "; inactive");
    }
}

/// \method counter([value])
/// Get or set the timer counter.
STATIC mp_obj_t machine_timer_counter(size_t n_args, const mp_obj_t *args) {
    machine_timer_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return mp_obj_new_int(self->counter);
    } else {
        // set
        self->counter = mp_obj_get_int(args[1]);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_timer_counter_obj, 1, 2, machine_timer_counter);

/// \method period([value])
/// Get or set the period of the timer.
STATIC mp_obj_t machine_timer_period(size_t n_args, const mp_obj_t *args) {
    machine_timer_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return mp_obj_new_int_from_uint(self->period);
    } else {
        // set
        self->period = mp_obj_get_int(args[1]);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_timer_period_obj, 1, 2, machine_timer_period);

/// \method callback(fun)
/// Set the function to be called when the timer triggers.
/// `fun` is passed 1 argument, the timer object.
STATIC mp_obj_t machine_timer_callback(mp_obj_t self_in, mp_obj_t callback) {
    machine_timer_obj_t *self = self_in;
    if (callback == mp_const_none) {
        self->callback = mp_const_none;
    } else if (mp_obj_is_callable(callback)) {
        self->callback = callback;
    } else {
        mp_raise_ValueError("callback must be None or a callable object");
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_timer_callback_obj, machine_timer_callback);

/// \method compare_register([value])
/// Get or set the timer's current compare register value.
STATIC mp_obj_t machine_timer_compare_register(size_t n_args, const mp_obj_t *args) {
    machine_timer_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return mp_obj_new_int_from_uint(systimer->C[self->id]);
    } else {
        // set
        systimer->C[self->id] = mp_obj_get_int(args[1]);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_timer_compare_register_obj, 1, 2, machine_timer_compare_register);

STATIC const mp_rom_map_elem_t machine_timer_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_timer_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_counter), MP_ROM_PTR(&machine_timer_counter_obj) },
    { MP_ROM_QSTR(MP_QSTR_period), MP_ROM_PTR(&machine_timer_period_obj) },
    { MP_ROM_QSTR(MP_QSTR_callback), MP_ROM_PTR(&machine_timer_callback_obj) },
    { MP_ROM_QSTR(MP_QSTR_compare_register), MP_ROM_PTR(&machine_timer_compare_register_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT((int) ONE_SHOT) },
    { MP_ROM_QSTR(MP_QSTR_PERIODIC), MP_ROM_INT((int) PERIODIC) },
    { MP_ROM_QSTR(MP_QSTR_FREE), MP_ROM_INT((int) FREE) },
};
STATIC MP_DEFINE_CONST_DICT(machine_timer_locals, machine_timer_locals_table);

const mp_obj_type_t machine_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Timer,
    .print = machine_timer_print,
    .make_new = machine_timer_make_new,
    .locals_dict = (mp_obj_t) &machine_timer_locals,
};

