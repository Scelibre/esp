./api.sh
cd ..
USER_ID=$UID docker-compose -f web/docker-compose.yml run --rm npm install
USER_ID=$UID docker-compose -f web/docker-compose.yml run --rm npm run build
