/* brushed dc motor control example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
 * This example will show you how to use MCPWM module to control brushed dc motor.
 * This code is tested with L298 motor driver.
 * User may need to make changes according to the motor driver they use.
*/

#include <stdio.h>

#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "string.h"
#include "sdkconfig.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"

#define RECEIVER_PIN 34 //set receiver GPIO
int LOCATION = -1;

#define GPIO_PWM0A_OUT 32   //Set GPIO 32 as PWM0A
#define GPIO_PWM0B_OUT 14   //Set GPIO 14 as PWM0B
#define GPIO_MOTOR1_0  13
#define GPIO_MOTOR1_1  12
#define GPIO_MOTOR2_0  27
#define GPIO_MOTOR2_1  33
#define LED_GPIO 4 //led gpio at A5


#define RX 16
#define SAMPLE_NUM 64
#define DEFAULT_VREF 1100

static esp_adc_cal_characteristics_t  *adc_chars;
static const adc1_channel_t channel = ADC1_CHANNEL_3; //GPIO 39
static const adc_atten_t  atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
static const int RX_BUF_SIZE = 256;

static esp_adc_cal_characteristics_t  *adc_chars1;
static const adc1_channel_t channel1 = ADC1_CHANNEL_0; //GPIO 39
static const adc_atten_t  atten1 = ADC_ATTEN_DB_11;
static const adc_unit_t unit1 = ADC_UNIT_1;
static const int RX_BUF_SIZE1 = 256;

 uint32_t LIDAR_DIST = 0;
 uint32_t IR_DIST = 0;
 uint32_t IR_DIST1 = 0;

  /* IR-RANGEFINDER */

 static void check_efuse() {
   //Check TP is burned into eFuse
   if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
     printf("eFuse Two Point: Supported\n");
   } else {
     printf("eFuse Two Point: NOT supported\n");
   }
   //Check Vref is burned into eFuse
   if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
     printf("eFuse Vref: Supported\n");
   } else {
     printf("eFuse Vref: NOT supported\n");
   }
 }

 static void print_char_val_type(esp_adc_cal_value_t val_type) {
   if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
     printf("Characterized using Two Point Value\n");
   } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
     printf("Characterized using eFuse Vref\n");
   } else {
     printf("Characterized using Default Vref\n");
   }
 }

 // ir-rangefinder initialization
 void init_ir() {
   check_efuse();
   //configure ADC
   adc1_config_width(ADC_WIDTH_BIT_12);
   adc1_config_channel_atten(channel, atten);
   adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
   esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
   print_char_val_type(val_type);
 }

void init_ir1() {
  check_efuse();

  //configure ADC
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(channel1, atten1);
  adc_chars1 = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit1, atten1, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars1);
}
 // get distance from ir-rangefinder
 uint32_t getDistance_ir() {
   uint32_t dist;
   //Initialize reading variable
   uint32_t val = 0;
   for (int i = 0; i < SAMPLE_NUM; i++){
     val += adc1_get_raw(channel);
   }
   val /= SAMPLE_NUM;
   uint32_t volts = esp_adc_cal_raw_to_voltage(val, adc_chars);
   dist = 146060 * (pow(volts, -1.126));
   //printf("Distance(cm): %d\n", dist);
   return dist;
 }
 uint32_t getDistance_ir1() {
   uint32_t dist;
   //Initialize reading variable
   uint32_t val = 0;
   for (int i = 0; i < SAMPLE_NUM; i++){
     val += adc1_get_raw(channel1);
   }
   val /= SAMPLE_NUM;
   uint32_t volts = esp_adc_cal_raw_to_voltage(val, adc_chars1);
   dist = 146060 * (pow(volts, -1.126));
   //printf("Distance(cm): %d\n", dist);
   return dist;
 }

 void init_uart() {
     const uart_config_t uart_config2 = {
         .baud_rate = 1200,
         .data_bits = UART_DATA_8_BITS,
         .parity = UART_PARITY_DISABLE,
         .stop_bits = UART_STOP_BITS_1,
         .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
     };
     uart_param_config(UART_NUM_2, &uart_config2);
     uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, RECEIVER_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
     uart_driver_install(UART_NUM_2, 1024, 0, 0, NULL, 0);
 }

 void read_location() {
   uint8_t *data = (uint8_t *) malloc(1024);
   uart_set_line_inverse(UART_NUM_2, UART_INVERSE_RXD);
   int len = uart_read_bytes(UART_NUM_2, data, 1024, 20 / portTICK_RATE_MS);
   if (len > 0) {
     for (int i = 0; i < len; i++) {
       if ((int) data[i] == 10 && (int) data[i+1] < 4) {
         printf("%d\n", (int) data[i+1]);
         LOCATION = (int) data[i+1];
         break;
       }
     }
   }
   free(data);
 }


 /*H BRIDGE & MOTOR FUNCTIONS*/
static void mcpwm_example_gpio_initialize()
{
	//only using one of the MCPWM units
    printf("initializing mcpwm gpio...\n");
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
static void brushed_motor_forward(float duty_cycle)
{

    gpio_set_level(GPIO_MOTOR1_0, 1);
    gpio_set_level(GPIO_MOTOR1_1, 0);
    gpio_set_level(GPIO_MOTOR2_0, 0);
    gpio_set_level(GPIO_MOTOR2_1, 1);

   // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
   // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty_cycle);

}

/**
 * @brief motor moves in backward direction, with duty cycle = duty %
 */
static void brushed_motor_backward(float duty_cycle)
{
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty_cycle);

    gpio_set_level(GPIO_MOTOR1_0, 0);
    gpio_set_level(GPIO_MOTOR1_1, 1);
    gpio_set_level(GPIO_MOTOR2_0, 1);
    gpio_set_level(GPIO_MOTOR2_1, 0);
}

/**
 * @brief motor stop
 */
static void brushed_motor_stop()
{
   // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
   // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

  	mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0.0);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 0.0);

}

 static void brushed_motor_turn_left(float duty_cycle)
 {
  // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
  // mcpwm_set_duty_type(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

   mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle/2);
   mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty_cycle);

   gpio_set_level(GPIO_MOTOR2_0, 1);
   gpio_set_level(GPIO_MOTOR2_1, 0);
 }


  static void brushed_motor_turn_right(float duty_cycle)
  {
   // mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
   // mcpwm_set_duty_type(MCPWM_UNIT_1, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, duty_cycle/2);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);

    gpio_set_level(GPIO_MOTOR2_0, 1);
    gpio_set_level(GPIO_MOTOR2_1, 0);
  }
/**
 * @brief Configure MCPWM module for brushed dc motor
 */
static void mcpwm_example_brushed_motor_control(void *arg)
{
    //1. mcpwm gpio initialization
    mcpwm_example_gpio_initialize();

    gpio_pad_select_gpio(GPIO_MOTOR1_0);
    gpio_pad_select_gpio(GPIO_MOTOR1_1);
    gpio_pad_select_gpio(GPIO_MOTOR2_0);
    gpio_pad_select_gpio(GPIO_MOTOR2_1);
    gpio_pad_select_gpio(LED_GPIO);

    gpio_set_direction(GPIO_MOTOR1_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_MOTOR1_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_MOTOR2_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_MOTOR2_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(GPIO_MOTOR1_0, 0);
    gpio_set_level(GPIO_MOTOR1_1, 1);
    gpio_set_level(GPIO_MOTOR2_0, 1);
    gpio_set_level(GPIO_MOTOR2_1, 0);
    gpio_set_level(LED_GPIO, 0);

    //2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;

	//only need to initialize one unit
    pwm_config.frequency = 100;    //frequency = 500Hz,
    pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
   	pwm_config.cmpr_b = 0;	//duty cycle of PWMxB = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM with above settings

    //initialize LIDAR
    //init_lidar();

    //initialize IR
    init_ir();
    init_ir1();
    //initialize reciever
    init_uart();

    while (1) {
      read_location();
      IR_DIST = getDistance_ir();
      IR_DIST1 = getDistance_ir1();
      printf("Distance (ir1): %dcm\n", IR_DIST1);
      printf("Distance (ir): %dcm\n", IR_DIST);
  		mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //sets duty type of PWM0A
  		mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0); //sets duty type of PWM0B
  		//brushed_motor_forward(90.0); //drive forward with duty cycle of 90

      if (IR_DIST1 >= 50) {
        brushed_motor_backward(90.0);
      }
      while (IR_DIST1 < 50){
        brushed_motor_stop(); //drive backward with duty cycle of 50%
        brushed_motor_turn_right(90.0);
        vTaskDelay(1500/portTICK_RATE_MS);
        LOCATION = -1;
        IR_DIST1 = getDistance_ir1();
      }

      if(-1 < LOCATION  && LOCATION < 4 ) {
        gpio_set_level(LED_GPIO, 1);
      } else {
        gpio_set_level(LED_GPIO, 0);
      }

      if(IR_DIST < 30) {
        uint32_t temp_dist = IR_DIST;
        //MOTOR MOVES LEFT SLIGHTLY
        brushed_motor_turn_left(90.0);
        vTaskDelay(1000/portTICK_RATE_MS);
        brushed_motor_turn_right(90.0);
        vTaskDelay(500/portTICK_RATE_MS);
        brushed_motor_backward(90.0);
      }
    }
}

void app_main()
{
    printf("Testing brushed motor...\n");
    xTaskCreate(mcpwm_example_brushed_motor_control, "mcpwm_example_brushed_motor_control", 4096, NULL, 5, NULL);
}
