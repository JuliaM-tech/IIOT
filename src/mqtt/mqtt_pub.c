#include <stdio.h>
#include <mosquitto.h>

int main(){
	int rc;
	struct mosquitto *mosq; 
	int valor_1 = 10;
	char v_1 [10];
	
	sprintf(v_1, "%d", valor_1);
	
	mosquitto_lib_init();
	
	mosq = mosquitto_new("publisher-test", true, NULL);

	rc = mosquitto_connect(mosq, "localhost", 1883, 60);
	
	if (rc!= 0){
		printf("El client no s'ha pogut connectar! Codi d'error %d\n", rc);
		mosquitto_destroy(mosq);
		return-1;
	}
	
	printf ("We are now connected to the broker\n");
	mosquitto_publish(mosq,NULL, "temperatura",6, v_1, 0, false);
	mosquitto_publish(mosq,NULL, "humitat",6, v_1, 0, false);
	mosquitto_disconnect (mosq);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();
	return 0;
}

