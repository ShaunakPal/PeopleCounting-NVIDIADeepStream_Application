# People Counting Application 

This is a Deepstreamm Application that uses YoloV8 and YoloV7-face to detect people and faces in a video stream. The application uses the AMQP protocol to send the detected objects to a message broker. The application is based on the Deepstream sample application 4. The application is tested on Deepstream 6.3 and Ubuntu 20.04



## Prerequisites


- DeepStream SDK 6.3
- AMQP broker (Docker image: https://hub.docker.com/r/rabbitmq:3-management)


## Compiling the Application


```
    make all
```


## Running the Application

The application requires a configuration file in the following format:

```
    ./ds_sch_people_count -i file:/opt/nvidia/deepstream/deepstream-6.3/samples/streams/sample_720p.mp4 -i rtsp://admin:sompassword@10.20.3.3:554/Streaming/Channels/101 -p /opt/nvidia/deepstream/deepstream-6.3/lib/libnvds_amqp_proto.so -c ds_cfg_amqp.txt -t test
```
