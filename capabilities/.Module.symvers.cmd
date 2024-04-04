cmd_/vagrant/capabilities/Module.symvers := sed 's/ko$$/o/' /vagrant/capabilities/modules.order | scripts/mod/modpost  -a   -o /vagrant/capabilities/Module.symvers -e -i Module.symvers   -T -
