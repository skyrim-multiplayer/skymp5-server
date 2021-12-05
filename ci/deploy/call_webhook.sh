#!/usr/bin/env bash

set -x
set -e

if [[ "$DEPLOY_STATUS_WEBHOOK" = "" ]]; then
  echo no webhook
  exit 1
fi

if [[ "$1" = "" ]]; then
  echo no message
  exit 1
fi

curl -v "$DEPLOY_STATUS_WEBHOOK" -H 'content-type: application/json' \
    --data "`echo "$1" | jq --raw-input '{content: .}'`"
