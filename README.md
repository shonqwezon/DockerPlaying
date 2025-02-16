_Do following commands from appropriate folder_ 
## Client-Server
- Set up `.env` file
- Change command in `docker-compose.yml` if you need
- `docker compose up --build` to start
- `docker compose down` to stop

## Producer-Consumer
- Install RabbitMq by running `docker run -it --rm --name rabbitmq -p 5672:5672 -p 15672:15672 rabbitmq:4.0-management`
- Set up `.env` file 
- `./build` to build images
- `./run -prod <producer num> -con <consumer num>` to start program
- `./stop` to stop
