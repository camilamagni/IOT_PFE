/* WiFi+MQTT Example
  */

#include "mbed.h"
#include "TCPSocket.h"
#include "wifi-ism43362/ISM43362Interface.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"
#include <cstdint>

#define logMessage printf
#define MQTTCLIENT_QOS2 1
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "HTS221Sensor.h"
#define MQTT_HOST               "20.226.44.12"
#define MQTT_PORT               1884
#define MQTT_TOPIC              "v1/devices/me/telemetry"
#include <string>

ISM43362Interface net;
DevI2C i2c_2(PB_11, PB_10);
HTS221Sensor hum_temp(&i2c_2);

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    logMessage("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    logMessage("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    //++arrivedcount;
}

int main(void){
   
    printf("\r\nBluetronics - WiFi+MQTT Example Demo\n");

    BSP_GYRO_Init();
    BSP_ACCELERO_Init();

    // Connect to Wifi
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = net.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }
    printf("Success\n\n");
    printf("MAC: %s\n", net.get_mac_address());
    printf("IP: %s\n", net.get_ip_address());
    printf("Netmask: %s\n", net.get_netmask());
    printf("Gateway: %s\n", net.get_gateway());
    printf("RSSI: %d\n\n", net.get_rssi());

    printf("\Wifi Example Done,MQTT Example Start\n");
  
    char assess_token[] = "GlQH3wvUfpEFM9EYabgO";
    NetworkInterface* network = &net;
    if (!network) {
        return -1;
    }

    MQTTNetwork mqttNetwork(network);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = MQTT_HOST;
    int port = MQTT_PORT;
    logMessage("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        logMessage("rc from TCP connect is %d\r\n", rc);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    //data.MQTTVersion = 3;
    // data.clientID.cstring = "Bluetronics-MQTT-Demo";
    data.username.cstring = "qumq7qbzva09cp9fozdn";
    data.password.cstring = "";
    if ((rc = client.connect(data)) != 0)
        logMessage("rc from MQTT connect is %d\r\n", rc);
 
    printf("successfully connect!\n\n");
    
    // Initialize sensors --------------------------------------------------
// while(1){
    
//     float VarGyro[3];BSP_GYRO_GetXYZ(VarGyro);
//     printf("Gyro: x = %7.0f, y = %7.0f, z = %7.0f", VarGyro[0], VarGyro[1], VarGyro[2]);


//     int16_t VarAccelero[3];BSP_ACCELERO_AccGetXYZ(VarAccelero);
//     printf("Acelero: x = %7.0i, y = %7.0i, z = %7.0i\r", VarAccelero[0], VarAccelero[1], VarAccelero[2]);

//     wait(0.2);

// }
uint8_t id;


hum_temp.init(NULL);
hum_temp.enable();
hum_temp.read_id(&id);
printf("HTS221  humidity & temperature sensor = 0x%X\r\n", id);



// Get data from sensors -----------------------------------------------

for (;;) {
    float temp, humid;

    hum_temp.get_temperature(&temp);
    hum_temp.get_humidity(&humid);
    printf("HTS221:  [temp] %.2f C, [hum]   %.2f%%\r\n", temp, humid);


    char msg[256];
    int n = snprintf(msg, sizeof(msg),
        "{\"temperature\":%f, \"humidity\":%f, \"active\": false}",
        temp, humid);

    void *payload = reinterpret_cast<void*>(msg);
    size_t payload_len = n;

    printf("publish to: %s %d %s\r\n", MQTT_HOST, MQTT_PORT, MQTT_TOPIC);

    if (client.publish(MQTT_TOPIC, payload, n) < 0) {
        printf("failed to publish MQTT message");
    }

    wait_ms(5000);

    NVIC_SystemReset();
}
   // net.disconnect();

}
