#!/usr/bin/env bash

LOG_DIRS=(
    "log"
)

DAYS_TO_KEEP=7

for LOG_DIR in "${LOG_DIRS[@]}"; do
    if [ ! -d "$LOG_DIR" ]; then
        echo "Log directory $LOG_DIR does not exist."
        continue
    fi

    find "$LOG_DIR" -type f \( -name "*.log" -o -name "*.error" \) -mtime +"$DAYS_TO_KEEP" -exec rm -f {} \;

    echo "Logs older than $DAYS_TO_KEEP days in $LOG_DIR have been deleted."
done