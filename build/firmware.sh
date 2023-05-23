cd ..
docker-compose -f projects/esp8266/docker-compose.yml build sdk
USER_ID=$UID docker-compose -f projects/esp8266/docker-compose.yml run --rm build
