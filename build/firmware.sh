cd ..
BUILD_UID=$(id -u) BUILD_GID=$(id -g) docker-compose -f projects/esp32/docker-compose.yml build sdk
docker-compose -f projects/esp32/docker-compose.yml run --rm build
docker-compose -f projects/esp32/docker-compose.yml run --rm flash
