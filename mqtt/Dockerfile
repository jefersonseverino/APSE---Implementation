FROM eclipse-mosquitto:2.0.21 as builder

WORKDIR /mosquitto

COPY mosquitto.conf .

ARG BROKER_USERNAME
ARG BROKER_PASSWORD

RUN mosquitto_passwd -b -c /mosquitto/config/mosquitto.passwd ${BROKER_USERNAME} ${BROKER_PASSWORD}

FROM eclipse-mosquitto:2.0.21

COPY --from=builder /mosquitto/config/mosquitto.passwd /mosquitto/config/mosquitto.passwd
COPY --from=builder /mosquitto/mosquitto.conf /mosquitto/config/mosquitto.conf

EXPOSE 1883

CMD ["/usr/sbin/mosquitto", "-c", "/mosquitto/config/mosquitto.conf"]