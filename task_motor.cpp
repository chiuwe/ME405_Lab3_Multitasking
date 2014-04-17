//**************************************************************************************
/** \file motor_controller.cpp
 *    This file contains the code for a motor controller class which controls speed and
 *    direction of a motor using a voltage measured from the A/D as input. One button
 *    will trigger stop and go. A second button will determine which motor is being
 *    controlled. */
//**************************************************************************************

#include "frt_text_queue.h"                 // Header for text queue class
#include "task_motor.h"                     // Header for this motor controller
#include "shares.h"                         // Shared inter-task communications


//-------------------------------------------------------------------------------------
/** This constructor creates a task which controls the speed of a motor using
 *  input from an A/D converter run through a potentiometer as well as an input from.
 *  a button. The main job of this constructor is to call the
 *  constructor of parent class (\c frt_task ); the parent's constructor the work.
 *  @param a_name A character string which will be the name of this task
 *  @param a_priority The priority at which this task will initially run (default: 0)
 *  @param a_stack_size The size of this task's stack in bytes 
 *                      (default: configMINIMAL_STACK_SIZE)
 *  @param p_ser_dev Pointer to a serial device (port, radio, SD card, etc.) which can
 *                   be used by this task to communicate (default: NULL)
 */

task_motor::task_motor (const char* a_name, 
                         unsigned portBASE_TYPE a_priority, 
                         size_t a_stack_size,
                         uint8_t brake_mask,
                         motor_driver* p_driver,
                         shared_data<bool>* p_brake,
                         shared_data<int16_t>* p_power,
                         shared_data<bool>* p_pot,
                         emstream* p_ser_dev
                        )
   : frt_task (a_name, a_priority, a_stack_size, p_ser_dev) {
   brake_pin = brake_mask;
   driver = p_driver;
   brake = p_brake;
   power = p_power;
   pot = p_pot;
}


//-------------------------------------------------------------------------------------
/** This method is called once by the RTOS scheduler. Each time around the for (;;)
 *  loop, it reads the A/D converter and change the selected motors speed. Each loop
 *  also check the two additional buttons, which control the brakes of the individual
 *  motors.
 */

void task_motor::run (void) {
   uint16_t a2d_reading;

   adc *p_my_adc = new adc(p_serial);
   PORTC |= (1 << 3) | (1 << 4);
   
   // This is the task loop for the motor control task. This loop runs until the
   // power is turned off or something equally dramatic occurs.
   for (;;) {
      if (PINC & (1 << brake_pin) || brake->get()) {
         driver->brake();
      } else {
         if (pot) {
            a2d_reading = p_my_adc->read_once(0);
            driver->set_power((a2d_reading / 2) - 255);
         } else {
            driver->set_power(power);
         }
      }
   }
   delay (100);
}

void task_motor::print_status (emstream& ser_thing) {
   // Call the parent task's printing function first
   frt_task::print_status (ser_thing);

   // Now add the additional data
   ser_thing << "\t " << runs << PMS (" runs");
}
