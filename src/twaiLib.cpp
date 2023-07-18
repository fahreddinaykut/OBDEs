#include "twaiLib.h"

twaiLib::twaiLib()
{
}
void twaiLib::init(variables *VARS, bleLib *BLE)
{
    libs = {VARS, BLE};
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_5, GPIO_NUM_4, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
    {
        Serial.print("Driver installed\n");
    }
    else
    {
        Serial.print("Failed to install driver\n");
        return;
    }
    // Start TWAI driver
    if (twai_start() == ESP_OK)
    {
        Serial.print("Driver started\n");
    }
    else
    {
        Serial.print("Failed to start driver\n");
        return;
    }

    xTaskCreate(&twai_receive_task, "hello_task", 4096, (void *)&libs, 5, NULL);
}
static void twai_receive_task(void *arg)
{
    GenericData_t *data = (GenericData_t *)arg;

    while (1)
    {
        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK)
        {
            String receivedMessage = "";
            for (int i=0;i<message.data_length_code;i++){
                receivedMessage = receivedMessage + "0x" + String(message.data[i],HEX) + " ";
            }
            DynamicJsonDocument doc(512);
            doc["type"] = "terminal";
            doc["value"] = "received data:"+receivedMessage;
            data->ble->sendJsonOverBle(doc);
            // Serial.println("Message received");
            //  Process received message
            if (message.extd)
            {

                //  Serial.print("Message is in Extended Format\n");
            }
            else
            {
                //  Serial.print("Message is in Standard Format\n");
            }
            // Serial.printf("indentifier: 0x%03x :", message.identifier);

            if (!(message.rtr))
            {
                String receivedMessage = "";
                for (int i = 0; i < message.data_length_code; i++)
                {

                    Serial.printf("0x%02x ", message.data[i]);
                    receivedMessage = receivedMessage + "0x" + String(message.data[i], HEX) + " ";
                }
                Serial.print("\n");
                if (message.data[2] == 0x0C)
                {
                    DynamicJsonDocument doc(512);
                    doc["type"] = "rpm";
                    doc["value"]= ((256 * message.data[3]) + message.data[4]) / 4;
                    data->ble->sendJsonOverBle(doc);
                }
                else if (message.data[2] == PID_VEHICLE_SPEED)
                {
                    DynamicJsonDocument doc(512);
                    doc["type"] = "speed";
                    doc["value"]= message.data[3];
                    data->ble->sendJsonOverBle(doc);
                }
            }
            else
            {
            
            }
        }
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}
void twaiLib::requestRPM()
{
    twai_message_t message;
    message.identifier = CAN_REQST_ID;
    message.extd = 1;
    message.data_length_code = 8;
    message.data[0] = 0x2;
    message.data[1] = 0x1;
    message.data[2] = PID_ENGINE_RPM;
    message.data[3] = 0x00;
    message.data[4] = 0x00;
    message.data[5] = 0x00;
    message.data[6] = 0x00;
    message.data[7] = 0x00;

    // Queue message for transmission
    twai_transmit(&message, pdMS_TO_TICKS(1000));
    // message.data[2] = PID_VEHICLE_SPEED;
    // twai_transmit(&message, pdMS_TO_TICKS(1000));
}
void twaiLib::requestSPEED()
{
    twai_message_t message;
    message.identifier = CAN_REQST_ID;
    message.extd = 1;
    message.data_length_code = 8;
    message.data[0] = 0x2;
    message.data[1] = 0x1;
    message.data[2] = PID_VEHICLE_SPEED;
    message.data[3] = 0x00;
    message.data[4] = 0x00;
    message.data[5] = 0x00;
    message.data[6] = 0x00;
    message.data[7] = 0x00;

    // Queue message for transmission
    twai_transmit(&message, pdMS_TO_TICKS(1000));
    // message.data[2] = PID_VEHICLE_SPEED;
    // twai_transmit(&message, pdMS_TO_TICKS(1000));
}