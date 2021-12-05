#!/usr/bin/env bash

message() {
  msg="[DEPLOY $DEPLOY_BRANCH] $1"
  echo "$msg"
  ./ci/deploy/call_webhook.sh "$msg"
}

report_fail() {
  message "Something went wrong, please see GitHub logs for Ubuntu build for details"
  exit 1
}
trap report_fail ERR

# cheatsheet: https://stackoverflow.com/a/44606194
# ? without : accepts empty string
# Ensuring all required variables are set:
echo "${DEPLOY_TARGET_HOST:?}" > /dev/null
echo "${DEPLOY_TARGET_USER:?}" > /dev/null
echo "${DEPLOY_BRANCH:?}" > /dev/null
echo "${DEPLOY_SSH_PRIVATE_KEY:?}" > /dev/null

if [[ "$CI" != "" ]]; then
  sudo touch ssh_id
  sudo chown "`id -u`:`id -g`" ssh_id
else
  touch ssh_id
fi
chmod 600 ssh_id
echo "$DEPLOY_SSH_PRIVATE_KEY" > ssh_id

remote_server_connstr="$DEPLOY_TARGET_USER@$DEPLOY_TARGET_HOST"

message "Starting deploy of $DEPLOY_BRANCH to \`$remote_server_connstr\`"

run_remote() {
  # echo "====== begin remote ======"
  ssh -vv -i ssh_id "$remote_server_connstr" "$@"
  # code="$?"
  # echo "======  end  remote ======"
  # return "$code"
}

# set -x

remote_tmp_dir="/tmp/skymp_deploy_`cat /dev/urandom | tr -cd 'a-f0-9' | head -c 32`"
remote_branch_dir="skymp-server-$DEPLOY_BRANCH"

run_remote test -e "$remote_branch_dir" \
  || (echo "no branch on remote server" && exit 1)

cp skymp5-server/{package.json,yarn.lock} build/dist/server/
rsync --rsh="ssh -i $PWD/ssh_id" -vazPh \
    build/dist/server/ "$remote_server_connstr:$remote_branch_dir/server/"

message "Updated server files"

rsync --rsh="ssh -i $PWD/ssh_id" -vazPh \
    ci/deploy/remote/ "$remote_server_connstr:$remote_tmp_dir/"
run_remote "$remote_tmp_dir/pull_branch.sh" "$DEPLOY_BRANCH"

message "Finished successfully"
