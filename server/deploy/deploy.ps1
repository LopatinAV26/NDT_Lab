# Выложить сервер синхронизации из текущего коммита (Windows):
#   .\server\deploy\deploy.ps1 [-HostName ndtlab]
# То же, что deploy.sh. Архив сначала пишется в файл: PowerShell 5 портит двоичные данные в конвейере
param([string]$HostName = 'ndtlab')

Push-Location (git -C $PSScriptRoot rev-parse --show-toplevel)
try
{
    $dirty = git status --porcelain -- server
    if ($dirty)
    {
        Write-Host 'В server/ есть незакоммиченные правки - закоммитьте их или отмените:'
        $dirty
        exit 1
    }

    $commit = git rev-parse --short HEAD
    $archive = Join-Path $env:TEMP 'ndtlab-server.tar'

    git archive -o $archive HEAD server
    if ($LASTEXITCODE -ne 0) { exit 1 }

    scp -q $archive "${HostName}:/tmp/ndtlab-server.tar"
    if ($LASTEXITCODE -ne 0) { exit 1 }

    ssh $HostName "rm -rf /tmp/ndtlab-deploy && mkdir /tmp/ndtlab-deploy && tar -xmf /tmp/ndtlab-server.tar -C /tmp/ndtlab-deploy && rm /tmp/ndtlab-server.tar && bash /tmp/ndtlab-deploy/server/deploy/remote-update.sh $commit"
    if ($LASTEXITCODE -ne 0) { exit 1 }
}
finally
{
    if ($archive) { Remove-Item $archive -ErrorAction SilentlyContinue }
    Pop-Location
}
