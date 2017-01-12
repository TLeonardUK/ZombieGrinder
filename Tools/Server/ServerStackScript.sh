#!/bin/bash
#
# Installs, configures and launches a Zombie Grinder dedicated server
# infinitus@zombiegrinder.com
#
# UDF Variables required by stackscript include #39161
# <UDF name="ss_username" Label="Non-root Username" example="Your local account username.  You'll not be able to log in as root." default="zgserver" />
# <UDF name="ss_password" Label="Non-Root Password"  optional="false" />
# <UDF name="ss_sshkey" Label="Non-Root SSH Key"  optional="false" />
# <UDF name="ss_shell" Label="Users shell" oneof="/bin/zsh,/bin/bash" default="/bin/bash" />
# <UDF name="ss_hostname" Label="System's Hostname" />
# <UDF name="ss_fqdn" Label="System's Fully Qualified Domain Name" />
# <UDF name="ss_notify_email" Label="Email address to send notifications to." default="infinitus@zombiegrinder.com" />
# <UDF name="ss_game_username" Label="Username game runs under." default="Steam" />
#
# UDF Variables to configure ZG Server
# <UDF name="zg_server_name" Label="Name disabled in server browser." default="ZombieGrinder Server" />
# <UDF name="zg_secure" Label="Run with VAC on or off." OneOf="0,1" default="1" />
# <UDF name="zg_map" Label="Name of map server will default to." default="c1_s1" />
# <UDF name="zg_auth_port" Label="Steam authentication port." default="8767" />
# <UDF name="zg_master_port" Label="Steam master server port." default="27017" />
# <UDF name="zg_comms_port" Label="Game communication port." default="57898" />
# <UDF name="zg_username" Label="Admin panel username." default="zgserver" />
# <UDF name="zg_password" Label="Admin panel password." />
# <UDF name="zg_admin_port" Label="Admin panel port number." default="6060" />

USER_GROUPS=sudo
ZG_APPID=374980

source <ssinclude StackScriptID=1> # StackScript Bash Library

function lower {
    # helper function
    echo $1 | tr '[:upper:]' '[:lower:]'
}

function system_add_user {
    # system_add_user(username, password, groups, shell=/bin/bash)
    USERNAME=`lower $1`
    PASSWORD=$2
    SUDO_GROUP=$3
    SHELL=$4
    if [ -z "$4" ]; then
        SHELL="/bin/bash"
    fi
    useradd --create-home --shell "$SHELL" --user-group --groups "$SUDO_GROUP" "$USERNAME"
    echo "$USERNAME:$PASSWORD" | chpasswd
}

function system_add_system_user {
    # system_add_system_user(username, home, shell=/bin/bash)
    USERNAME=`lower $1`
    HOME_DIR=$2
    SHELL=$3
    if [ -z "$3" ]; then
        SHELL="/bin/bash"
    fi
    useradd --system --create-home --home-dir "$HOME_DIR" --shell "$SHELL" --user-group $USERNAME
}

function system_get_user_home {
    # system_get_user_home(username)
    cat /etc/passwd | grep "^$1:" | cut --delimiter=":" -f6
}

function system_user_add_ssh_key {
    # system_user_add_ssh_key(username, ssh_key)
    USERNAME=`lower $1`
    USER_HOME=`system_get_user_home "$USERNAME"`
    sudo -u "$USERNAME" mkdir "$USER_HOME/.ssh"
    sudo -u "$USERNAME" touch "$USER_HOME/.ssh/authorized_keys"
    sudo -u "$USERNAME" echo "$2" >> "$USER_HOME/.ssh/authorized_keys"
    chmod 0600 "$USER_HOME/.ssh/authorized_keys"
}

function system_lock_user {
    # system_lock_user(username)
    passwd -l "$1"
}

function system_update_hostname {
    # system_update_hostname(system hostname)
    if [ -z "$1" ]; then
        echo "system_update_hostname() requires the system hostname as its first argument"
        return 1;
    fi
    echo $1 > /etc/hostname
    hostname -F /etc/hostname
    echo -e "\n127.0.0.1 $1 $1.local\n" >> /etc/hosts
}

function system_sshd_edit_bool {
    # system_sshd_edit_bool (param_name, "Yes"|"No")
    VALUE=`lower $2`
    if [ "$VALUE" == "yes" ] || [ "$VALUE" == "no" ]; then
        sed -i "s/^#*\($1\).*/\1 $VALUE/" /etc/ssh/sshd_config
    fi
}

function system_sshd_permitrootlogin {
    system_sshd_edit_bool "PermitRootLogin" "$1"
}

function system_sshd_passwordauthentication {
    system_sshd_edit_bool "PasswordAuthentication" "$1"
}

function system_security_logcheck {
    aptitude -y install logcheck logcheck-database
    # configure email
    # start after setup
}

function system_security_fail2ban {
    aptitude -y install fail2ban
}

function system_security_ufw_install {
    aptitude -y install ufw
}

function system_security_ufw_configure_basic {
    # see https://help.ubuntu.com/community/UFW
    ufw logging off

    ufw default deny

    ufw allow ssh/tcp
    ufw limit ssh/tcp

    ufw allow http/tcp
    ufw allow https/tcp

    ufw enable
}

function restart_services {
    # restarts upstart services that have a file in /tmp/needs-restart/
    for service_name in $(ls /tmp/ | grep restart-* | cut -d- -f2-10); do
        service $service_name restart
        rm -f /tmp/restart-$service_name
    done
}

function restart_initd_services {
    # restarts upstart services that have a file in /tmp/needs-restart/
    for service_name in $(ls /tmp/ | grep restart_initd-* | cut -d- -f2-10); do
        /etc/init.d/$service_name restart
        rm -f /tmp/restart_initd-$service_name
    done
}

# Configure system
system_update
system_update_hostname "$SS_HOSTNAME"
echo $(system_primary_ip) $SS_HOSTNAME $SS_FQDN >> /etc/hosts

# Ensure we have 32bit libs.
dpkg --add-architecture i386
aptitude -y update

# Create main user
system_add_user "$SS_USERNAME" "$SS_PASSWORD" "$USER_GROUPS" "$SS_SHELL"
system_user_add_ssh_key "$SS_USERNAME" "$SS_SSHKEY"

# Configure sshd
system_sshd_permitrootlogin "no"
system_sshd_passwordauthentication "no"
touch /tmp/restart-ssh

# Lock the root account.
system_lock_user "root"

# Install postfix
postfix_install_loopback_only
aptitude -y install mailutils

# Install logcheck
system_security_logcheck

# Install fail2ban
system_security_fail2ban

# Setup firewall
system_security_ufw_install
system_security_ufw_configure_basic
ufw allow 27015 # steam
ufw allow 27020/udp # sourcetv
ufw allow $ZG_AUTH_PORT
ufw allow $ZG_MASTER_PORT
ufw allow $ZG_COMMS_PORT
ufw allow $ZG_ADMIN_PORT

# Install byobu
aptitude -y install byobu tmux

# Steam dependencies
aptitude -y install lib32gcc1

# ZG dependencies
aptitude -y install lib32stdc++6
aptitude -y install libgl1-mesa-glx:i386 
aptitude -y install libgl1-mesa-dev:i386
aptitude -y install libuuid1:i386

# Install steam
STEAM_HOME=/home/steam
STEAMCMD_DIR=$STEAM_HOME/steamcmd
ZG_DIR=$STEAM_HOME/zombiegrinder
mkdir -p $STEAMCMD_DIR
mkdir -p $ZG_DIR
cd $STEAM_HOME/steamcmd
wget http://media.steampowered.com/client/steamcmd_linux.tar.gz
tar -xvzf steamcmd_linux.tar.gz
rm steamcmd_linux.tar.gz
./steamcmd.sh +login anonymous +force_install_dir $ZG_DIR +app_update $ZG_APPID validate +exit

# Fix permissions on all the steam stuff
chown -R $SS_USERNAME:$SS_USERNAME $STEAM_HOME

# Fix .so cannot be found error -_-.
ln -s $STEAMCMD_DIR/linux32/steamclient.so $ZG_DIR/Binary/steamclient.so

# Set up the init script
cd /etc/init.d

cat > ./zgserver <<EOD
#!/bin/sh
### BEGIN INIT INFO
# Provides:          zgserver
# Required-Start:    \$remote_fs \$syslog \$network
# Required-Stop:     \$remote_fs \$syslog \$network
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: starts the ZombieGrinder dedicated server
# Description:       Script to simulate daemonizing ZombieGrinder. It doesn't truly run
#                    as a daemon, instead this script runs it inside a tmux
#                    session.
### END INIT INFO

# replace <newuser> with the user you created above
STEAM_USER="$SS_USERNAME"

# Do not change this path
PATH=/bin:/usr/bin:/sbin:/usr/sbin

SELF=\$(cd \$(dirname \$0); pwd -P)/\$(basename \$0)

# The path to the game you want to host. example = /home/newuser/dod
DIR=$ZG_DIR
DAEMON=\$DIR/Binary/Server

# Change all PARAMS to your needs.
PARAMS="-log 1 -nogui -server -server_name \"$ZG_SERVER_NAME\" -server_secure $ZG_SECURE -server_map_name \"$ZG_MAP\" -server_auth_port $ZG_AUTH_PORT -server_master_port $ZG_MASTER_PORT -server_port $ZG_COMMS_PORT -admin_username \"$ZG_USERNAME\" -admin_password \"$ZG_PASSWORD\" -admin_port $ZG_ADMIN_PORT"
NAME=Server
DESC="server"

case "\$1" in
    start)
        echo "Starting \$DESC:"
        if [ -e \$DIR ]; then
            cd \$DIR
            su \$STEAM_USER -l -c "tmux new -d -s \$NAME '\$DAEMON \$PARAMS'"
            echo " ... done."
        else
            echo "No such directory: $DIR!"
        fi
        ;;

    stop)
        if su \$STEAM_USER -l -c "tmux ls" |grep \$NAME; then
            echo -n "Stopping \$DESC:"
            su \$STEAM_USER -l -c "tmux kill-session -t \$NAME"
            echo " ... done."
        else
            echo "Couldn't find a running $DESC"
        fi
        ;;

    restart)
        \$SELF stop
        \$SELF start
        ;;

    status)
        # Check whether there's a "steam" process
        ps aux | grep -v grep | grep \$NAME > /dev/null
        CHECK=\$?
        [ \$CHECK -eq 0 ] && echo "Server is UP" || echo "Server is DOWN"
        ;;

    console)
        if su \$STEAM_USER -l -c "tmux ls" |grep \$NAME; then
            su \$STEAM_USER -l -c "tmux attach -t \$NAME"
        else
            echo "Couldn't find a running \$DESC"
        fi
        ;;
        
    *)
        echo "Usage: \$0 {start|stop|restart|status|console}"
        exit 1
        ;;
esac

exit 0
EOD

chmod +x ./zgserver
touch /tmp/restart-zgserver

restart_services
restart_initd_services

# Send info message
cat > ~/setup_message <<EOD
Hi,

Your server configuration has completed.

You will need to update /etc/init.d/zombiegrinder.sh with your server parameters.

EOD

mail -s "Zombie Grinder Server Ready" "$NOTIFY_EMAIL" < ~/setup_message