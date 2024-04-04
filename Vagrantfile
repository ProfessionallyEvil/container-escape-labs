# -*- mode: ruby -*-
# vi: set ft=ruby :

# Environment variables:
#
# SKIP_BCC_BUILD: Set to skip the building bcc from source

$install_deps = <<EOF
  apt update
  apt install make gcc -y
EOF

$kernel_downgrade = <<EOF
  echo "!!!!!!Downgrading Kernel!!!!!!"
  
  # Navigate to /tmp directory
  cd /tmp
  apt install wget
  wget https://raw.githubusercontent.com/pimlie/ubuntu-mainline-kernel.sh/master/ubuntu-mainline-kernel.sh
  chmod +x ubuntu-mainline-kernel.sh
  sudo mv ubuntu-mainline-kernel.sh /usr/local/bin/
 
  ubuntu-mainline-kernel.sh -r 5.9
  ubuntu-mainline-kernel.sh -i 5.9.0 --yes
  
  apt update
  update-grub
EOF

$modify_grub = <<EOF
# Set the specific kernel version you want to boot into
cp /etc/default/grub /etc/default/grub.backup

# Update the Grub configuration to set the default menu entry
sed -i "s/^GRUB_DEFAULT=.*/GRUB_DEFAULT='1>2'/" /etc/default/grub

# Update Grub to apply changes
update-grub

echo "The default boot kernel is set to $KERNEL_VERSION."
echo "Please reboot your system to apply the changes."

EOF

$docker_install = <<EOF
  wget -O get-docker.sh https://get.docker.com
  sudo sh ./get-docker.sh
EOF

$install_runc = <<EOF
  wget -O runc https://github.com/opencontainers/runc/releases/download/v1.0.2/runc.amd64
  ls -la
  sleep 3
  chmod +x runc
  cp runc /usr/local/sbin/runc
  cp runc /usr/bin/runc
EOF

$build_containers = <<EOF
  cd /vagrant/dirty_pipe
  docker build -t dirty_pipe .

  cd /vagrant/capabilities
  docker build -t caps .
  
  docker pull alpine
  docker pull ubuntu
EOF

$make_module = <<EOF
  cd /vagrant/capabilities
  make
EOF

$copy_files = <<EOF
  cp -r /vagrant/ labs
EOF

Vagrant.configure("2") do |config|
  boxes = {
    'ubuntu-22.04'     => {
      'image'          => 'bento/ubuntu-22.04',
      'scripts'        => [ $install_deps, $docker_install, $install_runc, $kernel_downgrade, $modify_grub, $build_containers, $make_module, $copy_files ],
      'fix_console'    => 0,
    }
  }

  boxes.each do | name, params |
    config.vm.define name do |box|
      box.vm.box = params['image']
      box.vm.provider "virtualbox" do |v|
        v.memory = 4096
        v.cpus = 2
        if params['fix_console'] == 1
          v.customize ["modifyvm", :id, "--uart1", "0x3F8", "4"]
          v.customize ["modifyvm", :id, "--uartmode1", "file", "./#{name}_ttyS0.log"]
        end
      end
      (params['scripts'] || []).each do |script|
        box.vm.provision :shell, inline: script
      end
      config.vm.post_up_message = <<-HEREDOC
#######
Ready to go!
#######
      HEREDOC
    end
  end
end
