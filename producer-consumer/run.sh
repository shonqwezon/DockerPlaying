#!/bin/bash

# Producer parameters
num_tasks_min=5
num_tasks_max=10
sleep_sec_min=2
sleep_sec_max=5

# Consumer parameters
handle_sleep_sec_min=5
handle_sleep_sec_max=8

# ---
prod_num=""
con_num=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    -prod)
      if [[ "$2" =~ ^[0-9]+$ ]]; then
        prod_num=$2
      else
        echo "Error: -prod parameter must be a number."
        exit 1
      fi
      shift 2
      ;;
    -con)
      if [[ "$2" =~ ^[0-9]+$ ]]; then
        con_num=$2
      else
        echo "Error: -con parameter must be a number."
        exit 1
      fi
      shift 2
      ;;
    *)
      echo "Usage: $0 -prod <num> -con <num>"
      exit 1
      ;;
  esac
done

if [[ -z "$prod_num" || -z "$con_num" ]]; then
  echo "Both -prod and -con parameters must be provided."
  exit 1
fi

echo "Producer count: $prod_num"
echo "Consumer count: $con_num"

generate_random() {
  echo $((RANDOM % ($2 - $1 + 1) + $1))
}

echo "Generating consumers..."
for ((i = 1; i <= con_num; i++)); do
  handle_sleep_sec=$(generate_random $handle_sleep_sec_min $handle_sleep_sec_max)

  container_name="consumer_${i}"
  echo ""
  echo "Running ${container_name} with: handle_sleep_sec=$handle_sleep_sec"
  docker run -d --name ${container_name} --env-file .env prod_con ./Consumer -s ${handle_sleep_sec}
done

echo "---"
echo "Generating producers..."
for ((i = 1; i <= prod_num; i++)); do
  num_tasks=$(generate_random $num_tasks_min $num_tasks_max)
  sleep_sec=$(generate_random $sleep_sec_min $sleep_sec_max)
  code=$((i * 100))
  container_name="producer_${i}"
  echo ""
  echo "Running ${container_name} with: num_tasks=$num_tasks, code=$code, sleep_sec=$sleep_sec"
  docker run -d --name ${container_name} --env-file .env prod_con ./Producer -n ${num_tasks} -c ${code} -s ${sleep_sec}
done