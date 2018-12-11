#!/usr/bin/env bash
#
# Script to start syncd using supervisord
#

# Source the file that holds common code for systemd and supervisord
. /usr/bin/syncd_init_common.sh

config_syncd

export platform=$(sonic-cfggen -y /etc/sonic/sonic_version.yml -v asic_type)

exec ${CMD} ${CMD_ARGS}

