@echo off
set TARGET_PATH=%1
set SHORTCUT_PATH=%2
set DESCRIPTION=%3

powershell -Command "$WScriptShell = New-Object -ComObject WScript.Shell; $Shortcut = $WScriptShell.CreateShortcut('%SHORTCUT_PATH%'); $Shortcut.TargetPath = '%TARGET_PATH%'; $Shortcut.Description = '%DESCRIPTION%'; $Shortcut.Save()"
