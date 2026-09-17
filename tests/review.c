// AVR instruction-simulator regression checks. Hardware timing is not simulated.
#include "grbl.h"
system_t sys;
int32_t sys_position[N_AXIS], sys_probe_position[N_AXIS];
volatile uint8_t sys_probe_state, sys_rt_exec_state, sys_rt_exec_alarm;
volatile uint8_t sys_rt_exec_motion_override, sys_rt_exec_accessory_override;
volatile uint16_t failure_line;
volatile uint8_t parser_status;
static uint8_t homes, syncs, reset_during_delay;
static float last_target[N_AXIS];

void __wrap_protocol_buffer_synchronize(void) { syncs++; }
void __wrap_protocol_execute_realtime(void) {
  if (sys_rt_exec_state & EXEC_RESET) { sys.abort = true; }
}
void __wrap_delay_ms(uint16_t ms) {
  (void)ms;
  if (reset_during_delay) { sys_rt_exec_state |= EXEC_RESET; }
}
void __wrap_limits_go_home(uint8_t mask) { (void)mask; homes++; }
void __wrap_plan_sync_position(void) {}
void __wrap_gc_sync_position(void) {}
uint8_t __wrap_settings_read_coord_data(uint8_t n, float *v) {
  v[0]=10; v[1]=20; v[2]=30; (void)n; return true;
}
void __wrap_mc_line(float *v, plan_line_data_t *p) {
  (void)p; memcpy(last_target, v, sizeof(last_target));
}
__attribute__((noinline)) void tests_finished(void) { asm volatile("nop"); }
// Flash literals avoid the simple simulator's SRAM .data loading limitations.
#define RUN(s) ({ char line[32]; strcpy_P(line, PSTR(s)); gc_execute_line(line); })
#define CHECK(x) do { if (!(x)) { failure_line=__LINE__; goto done; } } while (0)
// Exercise the real alarm path after the parser's captured machine target.
// mc_line is wrapped to prevent simulated physical motion.
#define CHECK_LIMIT_COMMAND(s, rejected) do { \
  sys.state=STATE_IDLE; sys.abort=0; sys.soft_limit=0; \
  sys_rt_exec_state=0; sys_rt_exec_alarm=0; \
  CHECK(RUN(s)==STATUS_OK); \
  limits_soft_check(last_target); \
  CHECK(sys.soft_limit==(rejected)); \
  CHECK(sys_rt_exec_alarm==((rejected) ? EXEC_ALARM_SOFT_LIMIT : 0)); \
} while (0)
int main(void) {
  // Do not enable interrupts: simulator lacks the board's peripheral behavior.
  spindle_init();
  CHECK(TCCR2A == ((1<<WGM20)|(1<<COM2B1)));
  CHECK(TCCR2B == ((1<<WGM22)|(1<<CS22)|(1<<CS21)|(1<<CS20)));
  CHECK(TIMSK2 == 0 && OCR2A == 156 && OCR2B == PEN_UP_US/128);
  CHECK(get_step_pin_mask(Z_AXIS)==0 && get_direction_pin_mask(Z_AXIS)==0);
  CHECK(get_step_pin_mask(Y_AXIS)==bit(4) && get_direction_pin_mask(Y_AXIS)==bit(7));
  _spindle_set_state(SPINDLE_ENABLE_CW);
  CHECK(OCR2B == PEN_DOWN_US/128);
  spindle_stop(); sys_rt_exec_state=EXEC_RESET;
  _spindle_set_state(SPINDLE_ENABLE_CW);
  CHECK(OCR2B == PEN_UP_US/128); // Reset cannot be overwritten by down.
  sys_rt_exec_state=0; sys.state=STATE_CHECK_MODE;
  _spindle_sync(SPINDLE_ENABLE_CW);
  CHECK(OCR2B == PEN_UP_US/128 && syncs==0);
  sys.state=STATE_IDLE; gc_init();
  parser_status=RUN("M3");
  CHECK(parser_status==STATUS_OK && OCR2B==PEN_DOWN_US/128);
  CHECK(RUN("M5")==STATUS_OK && OCR2B==PEN_UP_US/128);
  CHECK(RUN("M4")==STATUS_GCODE_UNSUPPORTED_COMMAND);
  CHECK(RUN("G0Z0")==STATUS_GCODE_UNSUPPORTED_COMMAND);
  CHECK(RUN("G18")==STATUS_GCODE_UNSUPPORTED_COMMAND);
  CHECK(RUN("G19")==STATUS_GCODE_UNSUPPORTED_COMMAND);
  CHECK(RUN("G28")==STATUS_OK && last_target[Z_AXIS]==0);
  CHECK(RUN("G30")==STATUS_OK && last_target[Z_AXIS]==0);
  // Positive XY machine bounds, including both ends and independent overtravel.
  settings.max_travel[X_AXIS]=-800; settings.max_travel[Y_AXIS]=-755;
  settings.max_travel[Z_AXIS]=-200;
  float target[N_AXIS]={0};
  CHECK(!system_check_travel_limits(target));
  target[X_AXIS]=800; target[Y_AXIS]=755;
  CHECK(!system_check_travel_limits(target));
  target[X_AXIS]=800.1; CHECK(system_check_travel_limits(target));
  target[X_AXIS]=-0.1; CHECK(system_check_travel_limits(target));
  target[X_AXIS]=4; target[Y_AXIS]=755.1; CHECK(system_check_travel_limits(target));
  target[Y_AXIS]=-0.1; CHECK(system_check_travel_limits(target));
  target[Y_AXIS]=4; CHECK(!system_check_travel_limits(target));
  // G53 bypasses work/G92 offsets naturally; ordinary G54 still applies them.
  gc_state.coord_system[X_AXIS]=10; gc_state.coord_system[Y_AXIS]=20;
  gc_state.coord_offset[X_AXIS]=2; gc_state.coord_offset[Y_AXIS]=3;
  CHECK(RUN("G21G90G53G0X4Y4")==STATUS_OK);
  CHECK(last_target[X_AXIS]==4 && last_target[Y_AXIS]==4);
  CHECK(RUN("G0X4Y4")==STATUS_OK);
  CHECK(last_target[X_AXIS]==16 && last_target[Y_AXIS]==27);
  settings.homing_dir_mask=3;
  settings.flags=BITFLAG_SOFT_LIMIT_ENABLE;
  CHECK_LIMIT_COMMAND("G53G0X100Y100", false);
  CHECK_LIMIT_COMMAND("G53G0X400Y300", false);
  CHECK_LIMIT_COMMAND("G53G0X790Y745", false);
  CHECK_LIMIT_COMMAND("G53G0X-10", true);
  CHECK_LIMIT_COMMAND("G53G0X4Y4", false);
  CHECK_LIMIT_COMMAND("G53G0Y-10", true);
  CHECK_LIMIT_COMMAND("G53G0X4Y4", false);
  CHECK_LIMIT_COMMAND("G53G0X810", true);
  CHECK_LIMIT_COMMAND("G53G0X4Y4", false);
  CHECK_LIMIT_COMMAND("G53G0Y765", true);
  CHECK_LIMIT_COMMAND("G53G0X0Y0", false);
  CHECK_LIMIT_COMMAND("G53G0X800Y755", false);
  CHECK_LIMIT_COMMAND("G54G0X4Y4", false); // Machine 16,27 with the offsets above.
  CHECK_LIMIT_COMMAND("G54G0X-13Y4", true); // Machine X=-1.
  CHECK_LIMIT_COMMAND("G54G0X789Y4", true); // Machine X=801.
  target[X_AXIS]=4; target[Y_AXIS]=4;
  target[Z_AXIS]=-200; CHECK(!system_check_travel_limits(target));
  target[Z_AXIS]=0.1; CHECK(system_check_travel_limits(target));
  target[Z_AXIS]=-200.1; CHECK(system_check_travel_limits(target));
  sys.abort=0; sys.soft_limit=0; sys_rt_exec_state=0; sys_rt_exec_alarm=0;
  // PINB is plain simulated memory here: test real limit inversion logic.
  settings.flags=BITFLAG_INVERT_LIMIT_PINS; PINB=0;
  CHECK(limits_get_state()==0);
  PINB=bit(1); CHECK(limits_get_state()==bit(X_AXIS));
  PINB=bit(2); CHECK(limits_get_state()==bit(Y_AXIS));
  PINB=bit(3); CHECK(limits_get_state()==(bit(Z_AXIS)|bit(N_AXIS)));
  PINB=0; reset_during_delay=1; sys.state=STATE_HOMING;
  mc_homing_cycle(0);
  CHECK(sys.abort && homes==0 && gc_state.modal.spindle==SPINDLE_DISABLE);
  sys.abort=0; sys_rt_exec_state=0; reset_during_delay=0; sys_rt_exec_alarm=0;
  PINB=bit(3); mc_homing_cycle(0);
  CHECK(homes==0 && sys_rt_exec_alarm==EXEC_ALARM_HARD_LIMIT);
  sys.abort=0; sys_rt_exec_state=0; sys_rt_exec_alarm=0; PINB=0;
  mc_homing_cycle(0); CHECK(homes==2);
done:
  tests_finished();
  for (;;) {}
}
