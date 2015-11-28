#README for build_vm

Environment
------
__VM:__ Windows 7 with IE 11 (same with previous hand made VMs)

__Script:__ Windows only (vagrant + powershell)

__External sources:__ [modernIE/w7-ie11](https://atlas.hashicorp.com/modernIE/boxes/w7-ie11),[Vagrant Provision Reboot Plugin](https://github.com/exratione/vagrant-provision-reboot) 

__prerequisite software:__ jre-7u79-windows-i586


Folder content
-----
This build-vm folder contains following things:<br>
:white_check_mark: [README.md file](https://github.com/SoftwareEngineeringToolDemos/ICSE-2011-ViewInfinity/blob/master/build-vm/README.md) <br>
:white_check_mark: [Vagrantfile](https://github.com/SoftwareEngineeringToolDemos/ICSE-2011-ViewInfinity/blob/master/build-vm/Vagrantfile) The script (& configuration) file of the VM<br>
:white_check_mark: [vagrant-provision-reboot-plugin.rb](https://github.com/SoftwareEngineeringToolDemos/ICSE-2011-ViewInfinity/blob/master/build-vm/vagrant-provision-reboot-plugin.rb) This is a reboot plugin for vagrant, which lets VM to reboot during executing provisions.<br>
:white_check_mark: [installScript folder](https://github.com/SoftwareEngineeringToolDemos/ICSE-2011-ViewInfinity/tree/master/build-vm/installScript) This folder contains scripts of settings, installations.<br>
:white_check_mark: [software folder](https://github.com/SoftwareEngineeringToolDemos/ICSE-2011-ViewInfinity/tree/master/build-vm/software) The perquisite JRE is in the folder.<br>



How to Use
-----
** This version of script only work for windows system.

1. Download Vagrant from [this website](https://www.vagrantup.com/)
2. Install Vagrant
3. Download VirtualBox from [this website](https://www.virtualbox.org/wiki/Downloads)
4. Install VirtualBox
5. Download all files from [build-vm folder](https://github.com/SoftwareEngineeringToolDemos/ICSE-2011-ViewInfinity/tree/master/build-vm), to where you want to install VM.
6. (In windows system)Open a command window
7. Change Directory to where you downloded the build-vm (VM directory).
8. Execute command "__vagrant up__"

Note:
------
- Login is not required.
- If the VM has not been executed in your computer before, it might take longer time to download and load th VM.
- The VM will reboot several time automatically to config environment and settings.

  host name change --> reboot --> Disable UAC --> reboot by plugin -->start installations --> copy files to startup --> reboot.
- Please be patient, wait until message "All set"
- The JRE is installed in c:\Program Files\Java\jre7
- The tool viewInfinity is in Desktop\View_Infinity_Binary

Reference and sources
-----
1. [Microsoft trial machine for vagrant](https://dev.windows.com/en-us/microsoft-edge/tools/vms/windows/)
2. [Setup modern.ie vagrant boxes](https://gist.github.com/andreptb/57e388df5e881937e62a)
3. [modernIE/w7-ie11](https://atlas.hashicorp.com/modernIE/boxes/w7-ie11)
4. [Vagrant environment with Windows 2008 R2 + SQL Server Express 2008](https://github.com/fgrehm/vagrant-mssql-express)
5. [java8-vagrant-box](https://github.com/shekhargulati/java8-vagrant-box/blob/master/cookbooks/java/README.md)
6. [Powershell can also be nice -Or Installing Java silently and waiting](http://ramblingsofaswtester.com/?p=552)
7. [How to install JDK from PowerShell silently](http://blag.koveras.org/2011/12/24/how-to-install-jdk-from-powershell-silently/)
8. [Vagrant Provision Reboot Plugin](https://github.com/exratione/vagrant-provision-reboot)
9. [ADD YOUR OWN CUSTOMIZATION TO VAGRANT BOXES](https://mkrmr.wordpress.com/2012/08/12/add-your-own-customization-to-vagrant-boxes/)
10. [How to install JDK from PowerShell silently](http://blag.koveras.org/2011/12/24/how-to-install-jdk-from-powershell-silently/)
11. [Calling Executable Files in PowerShell](http://www.andyparkhill.co.uk/2012/02/calling-executable-files-in-powershell.html)
12. [Powershell install Java silently](http://stackoverflow.com/questions/29799158/powershell-install-java-silently)
13. [VagrantDocs](http://docs-v1.vagrantup.com/v1/docs/getting-started/index.html)
14. [Windows : Disable UAC (User Account Control) with CMD Script & PowerShell](http://juventusitprofessional.blogspot.com/2014/05/windows-disable-uac-user-account.html)
15. [Powershell function to pin and unpin from Windows Taskbar](https://rcmtech.wordpress.com/2014/03/12/powershell-function-to-pin-and-unpin-from-windows-taskbar/)

Author
-----
The build-vm folder, README.md, script files in this repo were created by [Yi-Chun Chen](https://github.com/RimiChen) under the supervision of [Dr. Emerson Murphy-Hill](https://github.com/CaptainEmerson).
