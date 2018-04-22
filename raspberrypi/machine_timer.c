#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"
#include "systimer.h"
#include "bcm283x_it.h"

// Timer class uses System timer 3
#define SYST_NUM (3)

typedef enum _timer_mode_t {
    ONE_SHOT = 0,
    PERIODIC = 1,
} timer_mode_t;

typedef struct _machine_timer_obj_t {
    mp_obj_base_t base;
    uint32_t id;
    uint32_t period;
    timer_mode_t mode;
    mp_obj_t callback;
} machine_timer_obj_t;

static machine_timer_obj_t timer_root;

static void timer_enable(void) {
    if ((IRQ_ENABLE1 & IRQ_SYSTIMER(3)) == 0) {
        IRQ_ENABLE1 = IRQ_SYSTIMER(3);
        systimer->C[3] = systimer->CLO + timer_root.period;
    }
}

static void timer_disable(void) {
    IRQ_DISABLE1 = IRQ_SYSTIMER(3);
}

void __attribute__((interrupt("IRQ"))) irq_timer(void) {
    if (IRQ_PEND1 & IRQ_SYSTIMER(SYST_NUM)) {
        if (timer_root.callback) {
            mp_sched_schedule(timer_root.callback, &timer_root);
        }
        if (timer_root.mode == PERIODIC) {
            systimer->C[SYST_NUM] += timer_root.period;
        } else {
            timer_disable();
        }
        systimer->CS |= (1 << SYST_NUM);
    }
}

STATIC mp_obj_t machine_timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    //    machine_timer_obj_t *tim = m_new_obj(machine_timer_obj_t);
    machine_timer_obj_t *tim = (machine_timer_obj_t*) &timer_root;
    tim->base.type = &machine_timer_type;
    tim->id = mp_obj_get_int(args[0]);
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
    timer_enable();

    return mp_const_none;
}

STATIC mp_obj_t machine_timer_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_timer_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_timer_init_obj, 0, machine_timer_init);

STATIC mp_obj_t machine_timer_deinit(mp_obj_t self_in) {
    machine_timer_obj_t *self = self_in;
    if (self->id == 0) {
        timer_disable();
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_deinit_obj, machine_timer_deinit);

STATIC void machine_timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_timer_obj_t *self = self_in;
    mp_printf(print, "Timer(%u); period=%u, mode=%u", self->id, self->period, self->mode);
}

STATIC const mp_rom_map_elem_t machine_timer_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_timer_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_timer_deinit_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(false) },
    { MP_ROM_QSTR(MP_QSTR_PERIODIC), MP_ROM_INT(true) },
};
STATIC MP_DEFINE_CONST_DICT(machine_timer_locals, machine_timer_locals_table);

const mp_obj_type_t machine_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Timer,
    .print = machine_timer_print,
    .make_new = machine_timer_make_new,
    .locals_dict = (mp_obj_t) &machine_timer_locals,
};

