#Requires -Version 5.1
[CmdletBinding()]
param([string]$Version='1.1.1',[string]$CertificateThumbprint='787D83F3BFFD136E8D2F8AD3261FD15D393FAC7A')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$output=Join-Path $root 'SetupBuild'
if(Test-Path $output){Remove-Item $output -Recurse -Force}
New-Item $output -ItemType Directory -Force|Out-Null
$app=Join-Path $output 'CmdHelper.exe'
Copy-Item (Join-Path $root 'x64\Release\CMDHelper.exe') $app
$signTool=Get-ChildItem "${env:ProgramFiles(x86)}\Windows Kits\10\bin" -Filter signtool.exe -Recurse|Where-Object FullName -Match '\\x64\\signtool.exe$'|Sort-Object FullName -Descending|Select-Object -First 1 -ExpandProperty FullName
& $signTool sign /sha1 $CertificateThumbprint /fd SHA256 /tr http://timestamp.globalsign.com/tsa/r6advanced1 /td SHA256 $app
if($LASTEXITCODE-ne 0){throw 'Application signing failed.'}
$manifest=Join-Path $output 'setup.manifest'
@'
<?xml version="1.0" encoding="UTF-8" standalone="yes"?><assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0"><trustInfo xmlns="urn:schemas-microsoft-com:asm.v3"><security><requestedPrivileges><requestedExecutionLevel level="requireAdministrator" uiAccess="false" /></requestedPrivileges></security></trustInfo></assembly>
'@|Set-Content $manifest -Encoding UTF8
$setup=Join-Path $output "CMD-Help-Setup-$Version-x64.exe"
& "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:winexe /platform:x64 /win32manifest:$manifest /reference:System.Windows.Forms.dll /reference:System.Drawing.dll /resource:"$app,CMDHelpApplication" /resource:"$(Join-Path $root 'Authority_Gate_CMD.ico'),CMDHelpIcon" /out:$setup (Join-Path $PSScriptRoot 'CMDHelpSetup.cs')
if($LASTEXITCODE-ne 0){throw 'Setup wizard compilation failed.'}
& $signTool sign /sha1 $CertificateThumbprint /fd SHA256 /tr http://timestamp.globalsign.com/tsa/r6advanced1 /td SHA256 $setup
if($LASTEXITCODE-ne 0-or(Get-AuthenticodeSignature $setup).Status-ne 'Valid'){throw 'Setup wizard signing or verification failed.'}
Write-Host "Built signed setup wizard: $setup" -ForegroundColor Green
