$JavaFilePath = 'c:\vagrant\software'
$JavaInstallerFile = 'jre-7u79-windows-i586.exe'


Write-Host "Get desktop."
#Desktop Location
$desktop = [Environment]::GetFolderPath("Desktop")

# Copy Files to desktop
#Copy-Item -path c:\vagrant_Viewinfinity\* -Destination $desktop -Recurse
Copy-Item -path c:\vagrant_Viewinfinity\build-vm\VM_Essentials\* -Destination $desktop -Recurse
Copy-Item -path c:\vagrant_Viewinfinity\View_Infinity_Binary -Destination $desktop -Recurse
Copy-Item -path c:\vagrant_Viewinfinity\View_Infinity_SRC -Destination $desktop -Recurse
# copy bat file to startup folder
Copy-Item -path c:\vagrant_Viewinfinity\build-vm\installScript\test.bat -Destination "C:\Users\IEUser\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup" -Recurse

$LicenseFolder = "$desktop\License\"

Move-Item -path $desktop\eula.lnk -Destination $desktop\License\


# install java environment
set-alias EXECUTE "C:\vagrant\software\jre-7u79-windows-i586.exe"

EXECUTE /s

Write-Host "Finish installation"
Write-Host "Setting successful, JRE has been installed."
Write-Host "Start prepare tools"


# create shortcut for the tool
$WshShell = New-Object -ComObject WScript.Shell
$Shortcut = $WshShell.CreateShortcut("$desktop\ViewInfinity.lnk")
$Shortcut.TargetPath = "$desktop\View_Infinity_Binary\View_Infinity.exe"
$Shortcut.Save()


Write-Host "All set"
