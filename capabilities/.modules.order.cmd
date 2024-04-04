cmd_/vagrant/capabilities/modules.order := {   echo /vagrant/capabilities/revshell.ko; :; } | awk '!x[$$0]++' - > /vagrant/capabilities/modules.order
