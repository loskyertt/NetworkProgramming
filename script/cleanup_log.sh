#!/usr/bin/env bash

LOG_DIRS=(
    "log"
)

DAYS_TO_KEEP=3

for LOG_DIR in "${LOG_DIRS[@]}"; do
    if [ ! -d "$LOG_DIR" ]; then
        echo "Log directory $LOG_DIR does not exist."
        continue
    fi

    deleted=$(find "$LOG_DIR" \
        -type f \
        \( -name "*.log" -o -name "*.error" \) \
        -mtime +"$DAYS_TO_KEEP" \
        -print -delete)

    if [ -n "$deleted" ]; then
        echo "Deleted the following files:"
        printf '%s\n' "$deleted"
    else
        echo "No old log files found in $LOG_DIR."
    fi
done