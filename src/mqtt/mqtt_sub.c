#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "/home/pi/Desktop/IIOT/src/lib_email/email.h"
#include <mosquitto.h>

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
	printf("ID: %d\n", * (int *) obj);
	if(rc) {
		printf("Error with result code: %d\n", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, "temperatura", 0);
	mosquitto_subscribe(mosq, NULL, "humitat", 0);
	mosquitto_subscribe(mosq, NULL, "voc", 0);
	mosquitto_subscribe(mosq, NULL, "co2", 0);

}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("New message with topic %s: %s\n", msg->topic, (char *) msg->payload);
	//Es passa a int
	char *payload_str = (char *) msg->payload;
	int payload_value = atoi(payload_str);
	
		
		if (strcmp(msg->topic, "temperatura") == 0){
			if (payload_value == 0 ){
					email("172.20.0.21", "1523276@campus.euss.org", "1523276@campus.euss.org", "Alerta: valors per sota dels preestablerts", "---VALORS MINIMS de Temperatura---");
			}		
		
	
	if (payload_value == 1) {
					email("172.20.0.21", "1523276@campus.euss.org", "1523276@campus.euss.org", "Alerta: valors per sobre dels preestablerts", "---VALORS MAXIMS de Temperatura---");
		}
		}
		
	if (strcmp(msg->topic, "humitat") == 0){
			if (payload_value == 0 ){
					email("172.20.0.21", "1523276@campus.euss.org", "1523276@campus.euss.org", "Alerta: valors per sota dels preestablerts", "---VALORS MINIMS d'Humitat---");
			}		
		
	
	if (payload_value == 1) {
					email("172.20.0.21", "1523276@campus.euss.org", "1523276@campus.euss.org", "Alerta: valors per sobre dels preestablerts", "---VALORS MAXIMS d'Humitat---");
		}
		}
		
	if (strcmp(msg->topic, "voc") == 0){
	
	if (payload_value == 1) {
					email("172.20.0.21", "1523276@campus.euss.org", "1523276@campus.euss.org", "Alerta: valors per sobre dels preestablerts", "---VALORS MAXIMS de VOC---");
		}
	}
		
	if (strcmp(msg->topic, "co2") == 0){
			if (payload_value == 0 ){
					email("172.20.0.21", "1523276@campus.euss.org", "1523276@campus.euss.org", "Alerta: valors per sota dels preestablerts", "---VALORS MINIMS de CO2---");
			}		
	
	if (payload_value == 1) {
					email("172.20.0.21", "1523276@campus.euss.org", "1523276@campus.euss.org", "Alerta: valors per sobre dels preestablerts", "---VALORS MAXIMS de CO2---");
		}
	}

}

int main() {
	int rc, id=12;

	mosquitto_lib_init();

	struct mosquitto *mosq;

	mosq = mosquitto_new("subscribe-test", true, &id);
	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_message_callback_set(mosq, on_message);
	
	rc = mosquitto_connect(mosq, "localhost", 1883, 10);
	
	if(rc) {
		printf("Could not connect to Broker with return code %d\n", rc);
		return -1;
	}

	mosquitto_loop_start(mosq);
	printf("Press Enter to quit...\n");
	getchar();
	
			
	mosquitto_loop_stop(mosq, true);

	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	return 0;
}


