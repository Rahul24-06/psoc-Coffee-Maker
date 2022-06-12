/*
    Copyright 2021 Picovoice Inc.

    You may not use this file except in compliance with the license. A copy of the license is located in the "LICENSE"
    file accompanying this source.

    Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
    an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
    specific language governing permissions and limitations under the License.
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cy_rgb_led.h"
#include "cybsp.h"

#include "pv_audio_rec.h"
#include "pv_keywords.h"
#include "pv_picovoice.h"
#include "pv_psoc6.h"

//OLED
#include "mtb_ssd1306.h"
#include "GUI.h"


#define MEMORY_BUFFER_SIZE (70 * 1024)

static const char* ACCESS_KEY = "FnnUvjNIBUT4ydUlMFU2+//bj3LIOSknrBaD0HzjCewWoVdnPebMCw==";

static int8_t memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));

static const float PORCUPINE_SENSITIVITY = 0.75f;
static const float RHINO_SENSITIVITY = 0.5f;

static void wake_word_callback(void) {
    printf("[wake word]\r\n");
    cy_rgb_led_on(CY_RGB_LED_COLOR_GREEN, CY_RGB_LED_MAX_BRIGHTNESS);
}

static void inference_callback(pv_inference_t *inference) {
	static char* numberOfShots[100], size[100], beverage[100];
    bool read_IR_val;
    int s=0;

    cy_rgb_led_on(CY_RGB_LED_COLOR_BLUE, CY_RGB_LED_MAX_BRIGHTNESS);

    printf("{\r\n");
    printf("\tis_understood : '%s',\r\n", (inference->is_understood ? "true" : "false"));
    if (inference->is_understood) {
        printf("\tintent : '%s',\r\n", inference->intent);
        if (inference->num_slots > 0) {
            printf("\tslots : {\r\n");
            for (int32_t i = 0; i < inference->num_slots; i++) {
                printf("\t\t'%s' : '%s',\r\n", inference->slots[i], inference->values[i]);

                if (strcmp(inference->slots[i], "size") == 0) {
                	strcpy(size, inference->values[i]);
                } else if (strcmp(inference->slots[i], "numberOfShots") == 0) {
                	strcpy(numberOfShots, inference->values[i]);
                } else if (strcmp(inference->slots[i], "beverage") == 0) {
                	strcpy(beverage, inference->values[i]);
                }
            }
            printf("\t}\r\n");
        }

        // Operation
        GUI_Clear();
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt("COFFEE MAKER AI", 64, 1);
        GUI_DispStringAt("PREPARING", 1, 20);
        GUI_DispStringAt(beverage, 60, 20);
        printf("\nPreparing %s\r\n",beverage);

        // Warmer on
        cyhal_gpio_write(P0_2, true);

        // Detect Cup
        read_IR_val = cyhal_gpio_read(P0_3); //Reading IR Sensor
        printf("IR value: %d\r\n",read_IR_val);

        while(!read_IR_val){
        	read_IR_val = cyhal_gpio_read(P0_3); //Reading IR Sensor
            Cy_SysLib_Delay(10); //10ms
            printf("Place Cup\r\n");
            GUI_SetTextAlign(GUI_TA_HCENTER);
            GUI_DispStringAt("COFFEE MAKER AI", 64, 1);
            GUI_DispStringAt("PREPARING", 1, 20);
            GUI_DispStringAt(beverage, 60, 20);
            GUI_DispStringAt("PLACE THE CUP . . .", 1, 32);
        }

        printf("Cup Detected\r\n");
        GUI_Clear();
        GUI_SetTextAlign(GUI_TA_HCENTER);
        GUI_DispStringAt("COFFEE MAKER AI", 64, 1);
        GUI_DispStringAt("PREPARING", 1, 20);
        GUI_DispStringAt(beverage, 60, 20);
        GUI_DispStringAt("CUP DETECTED", 1, 38);
        GUI_DispStringAt("3", 85, 38);
        Cy_SysLib_Delay(1000); //1000ms
        GUI_DispStringAt("2", 85, 38);
        Cy_SysLib_Delay(1000); //1000ms
        GUI_DispStringAt("1", 85, 38);
        Cy_SysLib_Delay(1000); //1000ms
        GUI_DispStringAt(" ", 85, 38);

        // Adding Coffee and Sugar
        AddCoffeeSugar();

        // Filling Milk & Water
        printf("Filling Milk & Water\r\n");
        GUI_Clear();
		GUI_SetTextAlign(GUI_TA_HCENTER);
		GUI_DispStringAt("COFFEE MAKER AI", 64, 1);
		GUI_DispStringAt("PREPARING", 1, 20);
		GUI_DispStringAt(beverage, 60, 20);
        GUI_DispStringAt("Filling Milk & Water", 1, 38);

        if (strcmp(numberOfShots, "single shot") == 0) s=1;
        else if (strcmp(numberOfShots, "\0") == 0) s=1;
        else if (strcmp(numberOfShots, "double shot") == 0) s=2;
        else if (strcmp(numberOfShots, "triple shot") == 0) s=3;

		printf("Shot: %d\r\n", s);

        if (strcmp(beverage, "espresso") == 0){ //30ml
        	cyhal_gpio_write(P13_6, true); //Water pump on
        	for(int loop=0; loop<s; loop++) { Cy_SysLib_Delay(1000);  }
            cyhal_gpio_write(P13_6, false); //Water pump off
        }

        else if (strcmp(beverage, "americano") == 0){
        	//espresso
			cyhal_gpio_write(P13_6, true); //Water pump on
			for(int loop=0; loop<s; loop++) { Cy_SysLib_Delay(1000);  }
			cyhal_gpio_write(P13_6, false); //Water pump off

			Cy_SysLib_Delay(250);

			//water
			cyhal_gpio_write(P13_6, true); //Water pump on
			Cy_SysLib_Delay(1500);
			cyhal_gpio_write(P13_6, false); //Water pump off
		}

    	else if (strcmp(beverage, "cappuccino") == 0){
        	//espresso
			cyhal_gpio_write(P13_6, true); //Water pump on
			for(int loop=0; loop<s; loop++) { Cy_SysLib_Delay(1000);  }
			cyhal_gpio_write(P13_6, false); //Water pump off

			Cy_SysLib_Delay(250);

			//water
			cyhal_gpio_write(P13_6, true); //Water pump on
			Cy_SysLib_Delay(500);
			cyhal_gpio_write(P13_6, false); //Water pump off
		}

    	else if (strcmp(beverage, "coffee") == 0){
        	//espresso
			cyhal_gpio_write(P13_6, true); //Water pump on
			for(int loop=0; loop<s; loop++) { Cy_SysLib_Delay(1000);  }
			cyhal_gpio_write(P13_6, false); //Water pump off

			Cy_SysLib_Delay(250);

			//milk
			 cyhal_gpio_write(P1_3, true); //Milk pump on
			 Cy_SysLib_Delay(1000);
			 cyhal_gpio_write(P1_3, false); //Milk pump off
		}

    	else if (strcmp(beverage, "latte") == 0){
			//espresso
			cyhal_gpio_write(P13_6, true); //Water pump on
			for(int loop=0; loop<s; loop++) { Cy_SysLib_Delay(1000);  }
			cyhal_gpio_write(P13_6, false); //Water pump off

			Cy_SysLib_Delay(250);

			//milk
			 cyhal_gpio_write(P1_3, true); //Milk pump on
			 Cy_SysLib_Delay(1500);
			 cyhal_gpio_write(P1_3, false); //Milk pump off
		}

    	else if (strcmp(beverage, "mocha") == 0){
			//espresso
			cyhal_gpio_write(P13_6, true); //Water pump on
			for(int loop=0; loop<s; loop++) { Cy_SysLib_Delay(1000);  }
			cyhal_gpio_write(P13_6, false); //Water pump off

			Cy_SysLib_Delay(250);

			//cocoa
			 cyhal_gpio_write(P1_4, true); //cocoa pump on
			 Cy_SysLib_Delay(1000);
			 cyhal_gpio_write(P1_4, false); //cocoa pump off

			 Cy_SysLib_Delay(250);

			//milk
			 cyhal_gpio_write(P1_3, true); //Milk pump on
			 Cy_SysLib_Delay(500);
			 cyhal_gpio_write(P1_3, false); //Milk pump off
		}

        // Coffee is ready
		printf("Coffee is ready!\r\n");
		GUI_Clear();
		GUI_SetTextAlign(GUI_TA_HCENTER);
		GUI_DispStringAt("COFFEE MAKER AI", 64, 1);
		GUI_DispStringAt("PREPARING", 1, 20);
		GUI_DispStringAt(beverage, 60, 20);
		GUI_DispStringAt("Coffee is ready! ", 1, 38);

        GUI_DispStringAt("3", 105, 38);
		Cy_SysLib_Delay(1000); //1000ms
		GUI_DispStringAt("2", 105, 38);
		Cy_SysLib_Delay(1000); //1000ms
		GUI_DispStringAt("1", 105, 38);
		Cy_SysLib_Delay(1000); //1000ms
		GUI_DispStringAt(" ", 105, 38);

        cyhal_gpio_write(P0_2, false); //warmer off
        cyhal_gpio_write(P1_3, false); //Milk pump off
        cyhal_gpio_write(P13_6, false); //Water pump off

        // Detect Cup
		read_IR_val = cyhal_gpio_read(P0_3); //Reading IR Sensor
		printf("IR value: %d\r\n",read_IR_val);

		while(!read_IR_val){
			read_IR_val = cyhal_gpio_read(P0_3); //Reading IR Sensor
			Cy_SysLib_Delay(10); //10ms
		}
		GUI_Clear();
		GUI_SetTextAlign(GUI_TA_HCENTER);
		GUI_DispStringAt("COFFEE MAKER AI", 64, 1);
		GUI_DispStringAt("PREPARING", 1, 20);
		GUI_DispStringAt(beverage, 60, 20);
	    GUI_DispStringAt("THANK YOU!", 1, 38);
	    GUI_DispStringAt("              ", 1, 50);
		GUI_DispStringAt("VISIT AGAIN", 1, 50);
    }
    printf("}\r\n\n");


    //clearing buffer
	printf("Clearing Buffer . . .\r\n");
    strcpy(beverage, "\0");
    strcpy(numberOfShots,"\0");
    strcpy(size, "\0");

    cyhal_gpio_write(P0_2, false); //warmer off
    cyhal_gpio_write(P1_3, false); //Milk pump off
    cyhal_gpio_write(P13_6, false); //Water pump off
    cyhal_gpio_write(P1_4, false); //Cocoa pump off



    for (int32_t i = 0; i < 10; i++) {
        if (cy_rgb_led_get_brightness() == 0) {
            cy_rgb_led_set_brightness(CY_RGB_LED_MAX_BRIGHTNESS);
        } else {
            cy_rgb_led_set_brightness(0);
        }
        Cy_SysLib_Delay(30);
    }
    cy_rgb_led_off();

    pv_inference_delete(inference);
}

static void error_handler(void) {
    while(true);
}

static void printMainScreen(void)
{
	GUI_SetTextAlign(GUI_TA_HCENTER);
	GUI_DispStringAt("COFFEE MAKER AI", 64, 1);

    //GUI_SetTextAlign(GUI_TA_LEFT);

	GUI_SetTextAlign(GUI_TA_HCENTER);
	GUI_DispStringAt("ORDER YOUR DRINK", 64, 20);

	GUI_SetTextAlign(GUI_TA_HCENTER);
	GUI_DispStringAt("WITH YOUR VOICE", 64, 32);

	GUI_SetTextAlign(GUI_TA_HCENTER);
	GUI_DispStringAt("SAY 'PICOVOICE'", 64, 50);
	GUI_SetTextAlign(GUI_TA_LEFT);
}

int main(void) {
	cy_rslt_t result;

	cyhal_i2c_t i2c_obj;

	/* Initialize the device and board peripherals */
	result = cybsp_init();

	CY_ASSERT(result == CY_RSLT_SUCCESS);

	/* Initialize the I2C to use with the OLED display */
	result = cyhal_i2c_init(&i2c_obj, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);

	CY_ASSERT(result == CY_RSLT_SUCCESS);

	/* Initialize the OLED display */
	result = mtb_ssd1306_init_i2c(&i2c_obj);

	CY_ASSERT(result == CY_RSLT_SUCCESS);

	/* GPIO */
	result = cyhal_gpio_init(P0_2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false); // Relay

	result = cyhal_gpio_init(P1_3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false); // Milk pump

	result = cyhal_gpio_init(P13_6, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false); // Water pump€

	result = cyhal_gpio_init(P1_4, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, false); // Coca pump€

	result = cyhal_gpio_init(P0_3, CYHAL_GPIO_DIR_INPUT, CYHAL_GPIO_DRIVE_NONE, false); // IR Sensor

	CY_ASSERT(result == CY_RSLT_SUCCESS);

	cyhal_gpio_write(P0_2, false); //warmer off
	cyhal_gpio_write(P1_3, false); //Milk pump off
	cyhal_gpio_write(P13_6, false); //Water pump off
	cyhal_gpio_write(P1_4, false); //Cocoa pump off


	//cyhal_gpio_write(P0_2, true);

	GUI_Init();
    GUI_Clear();
	printMainScreen();


	pv_status_t status = pv_board_init();
    if (status != PV_STATUS_SUCCESS) {
        error_handler();
    }

    status = pv_message_init();
    if (status != PV_STATUS_SUCCESS) {
        error_handler();
    }

    const uint8_t *board_uuid = pv_get_uuid();
    printf("UUID: ");
    for (uint32_t i = 0; i < pv_get_uuid_size(); i++) {
        printf(" %.2x", board_uuid[i]);
    }
    printf("\r\n");

    status = pv_audio_rec_init();
    if (status != PV_STATUS_SUCCESS) {
        printf("Audio init failed with '%s'\r\n", pv_status_to_string(status));
        error_handler();
    }

    status = pv_audio_rec_start();
    if (status != PV_STATUS_SUCCESS) {
        printf("Recording audio failed with '%s'\r\n", pv_status_to_string(status));
        error_handler();
    }

    pv_picovoice_t *handle = NULL;

    status = pv_picovoice_init(
            ACCESS_KEY,
            MEMORY_BUFFER_SIZE,
            memory_buffer,
            sizeof(KEYWORD_ARRAY),
            KEYWORD_ARRAY,
            PORCUPINE_SENSITIVITY,
            wake_word_callback,
            sizeof(CONTEXT_ARRAY),
            CONTEXT_ARRAY,
            RHINO_SENSITIVITY,
            true,
            inference_callback,
            &handle);
    if (status != PV_STATUS_SUCCESS) {
        printf("Picovoice init failed with '%s'\r\n", pv_status_to_string(status));
        error_handler();
    }

    while (true) {
        const int16_t *buffer = pv_audio_rec_get_new_buffer();
        if (buffer) {
            const pv_status_t status = pv_picovoice_process(handle, buffer);
            if (status != PV_STATUS_SUCCESS) {
                printf("Picovoice process failed with '%s'\r\n", pv_status_to_string(status));
                error_handler();
            }
        }

    }
    pv_board_deinit();
    pv_audio_rec_deinit();
    pv_picovoice_delete(handle);
    GUI_Exit();
}
