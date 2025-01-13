ivo.close()  # Cierra el archivo

# FUNCIONES, para conectar y recibir mensaje MQTT
# Al recibir CONNACK desde el servidor.
def on_connect(client, userdata, flags, rc):
    print("Conexión/código de resultado: "+str(rc))

    # Inicio o renovación de subscripción
    client.subscribe(topicolee)

    return()

# el tópico tiene una publicación
def on_message(client, userdata, msg):
    global cuentamensaje
    print(msg.topic+" "+str(msg.payload))
    unmensaje = msg.topic+" "+str(msg.payload)

    # Archivo en modo añadir 'append'
    archivo = open(nombrearchivo,'a')
    unalinea = unmensaje + '\n'
    archivo.write(unalinea)
    
    cuentamensaje = cuentamensaje + 1
    print('\n mensajes recibidos: ', cuentamensaje)

    return()
